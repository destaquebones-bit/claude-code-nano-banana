#!/usr/bin/env python3
"""Análise técnica e perceptiva de faixas e stems.

Rode sempre que chegar áudio novo:

    python3 tools/analisa_audio.py faixa.wav
    python3 tools/analisa_audio.py minha.wav --ref referencia.wav
    python3 tools/analisa_audio.py kick.wav --contra bass.wav

Duas camadas, e a distinção importa:

  FÍSICA        o que o sinal mede — LUFS, true peak, espectro, fase.
                Reprodutível, exato, e não diz nada sobre como soa.

  PERCEPTIVA    o que um ouvido faria com esse sinal — bandas críticas,
                mascaramento, sonoridade em sones. É modelo, não medida:
                aproxima a percepção em vez de descrevê-la. É a coisa mais
                próxima de escutar que existe sem escutar.

Nada aqui julga se a música é boa.
"""

import argparse
import os
import sys

import numpy as np
import soundfile as sf
from scipy import signal

# ---------------------------------------------------------------- utilidades

def load(path, mono=True):
    x, sr = sf.read(path, always_2d=True, dtype="float64")
    nz = np.where(np.abs(x).max(axis=1) > 10 ** (-60 / 20))[0]
    if len(nz):
        x = x[nz[0]:nz[-1] + 1]          # tira silêncio de borda dos dois lados
    return (x.mean(axis=1) if mono else x), sr


def db(v, floor=1e-12):
    return 20.0 * np.log10(max(abs(v), floor))


BANDS = [("sub 20-60", 20, 60), ("bass 60-120", 60, 120), ("120-250", 120, 250),
         ("250-500", 250, 500), ("500-2k", 500, 2000), ("2k-6k", 2000, 6000),
         ("6k-16k", 6000, 16000)]


# ------------------------------------------------------------------- física

def nivel(path):
    import pyloudnorm as pyln
    x, sr = load(path, mono=False)
    meter = pyln.Meter(sr)
    lufs = float(meter.integrated_loudness(x))
    tp = max(np.max(np.abs(signal.resample_poly(c, 4, 1))) for c in x.T)
    m = x.mean(axis=1)
    w = int(sr * 3)
    cr = [np.max(np.abs(m[i:i + w])) / np.sqrt(np.mean(m[i:i + w] ** 2))
          for i in range(0, len(m) - w, sr) if np.sqrt(np.mean(m[i:i + w] ** 2)) > 1e-6]
    corr = None
    if x.shape[1] > 1:
        sos = signal.butter(4, [20, 120], btype="band", fs=sr, output="sos")
        l, r = signal.sosfilt(sos, x[:, 0]), signal.sosfilt(sos, x[:, 1])
        corr = float(np.sum(l * r) / np.sqrt(np.sum(l ** 2) * np.sum(r ** 2)))
    return dict(lufs=lufs, tp=db(tp), sp=db(np.max(np.abs(x))),
                psr=db(np.median(cr)) if cr else float("nan"), corr=corr, sr=sr,
                dur=len(m) / sr)


def curva_tonal(path):
    """Terços de oitava contra ruído rosa, normalizado por 200 Hz–2 kHz.

    Contra rosa e não contra o espectro cru: rosa é energia igual por oitava,
    a referência neutra que todo medidor de balanço tonal usa. Normalizar pela
    própria média da faixa faz a curva mostrar FORMA e não volume, que é o que
    permite comparar duas faixas de loudness diferente.
    """
    x, sr = load(path)
    f, p = signal.welch(x, sr, nperseg=32768, noverlap=16384, scaling="density")
    rows, fc = [], 25.0
    while fc < 18000:
        lo, hi = fc / 2 ** (1 / 6), fc * 2 ** (1 / 6)
        k = (f >= lo) & (f < hi)
        if k.sum() >= 2:
            rows.append((fc, 10 * np.log10(max(np.trapezoid(p[k], f[k]), 1e-20) / np.log(hi / lo))))
        fc *= 2 ** (1 / 3)
    a = np.array(rows)
    a[:, 1] -= a[(a[:, 0] >= 200) & (a[:, 0] <= 2000), 1].mean()
    sel = (f >= 20) & (f <= 16000)
    tot = np.trapezoid(p[sel], f[sel])
    shares = {n: float(np.trapezoid(p[(f >= lo) & (f < hi)], f[(f >= lo) & (f < hi)]) / tot)
              for n, lo, hi in BANDS}
    return a, shares


def bass_notas(path, bpm=None):
    """Fundamental e estrutura harmônica do baixo, nota a nota.

    Analisa uma janela que começa 130 ms depois de cada pico rítmico, para o
    transiente do kick não contaminar a estimativa de pitch.
    """
    x, sr = load(path)
    low = signal.sosfilt(signal.butter(4, 200, btype="low", fs=sr, output="sos"), x)
    env = signal.sosfilt(signal.butter(2, 30, btype="low", fs=sr, output="sos"),
                         np.abs(signal.hilbert(low)))
    if bpm is None:
        hop, nfft = 512, 2048
        _, _, Z = signal.stft(x, sr, nperseg=nfft, noverlap=nfft - hop)
        flux = np.maximum(0, np.diff(np.log1p(1000 * np.abs(Z)), axis=1)).sum(axis=0)
        fse = sr / hop
        ac = np.correlate(flux - flux.mean(), flux - flux.mean(), mode="full")[len(flux) - 1:]
        bpm = max(np.arange(110, 140, 0.05),
                  key=lambda b: sum(np.interp(60 * fse / b * m, np.arange(len(ac)), ac)
                                    for m in (1, 2, 4)))
    per = sr * 60 / bpm
    pk, _ = signal.find_peaks(env, distance=int(per * 0.7), height=np.percentile(env, 80))

    def yin(s, fmin=30, fmax=200):
        N = len(s); tmin, tmax = int(sr / fmax), int(sr / fmin)
        d = np.empty(tmax + 1)
        for t in range(tmax + 1):
            df = s[:N - tmax] - s[t:t + N - tmax]
            d[t] = np.dot(df, df)
        cum = np.ones(tmax + 1); run = 0.0
        for t in range(1, tmax + 1):
            run += d[t]; cum[t] = d[t] * t / max(run, 1e-12)
        best = -1
        for t in range(tmin, tmax):
            if cum[t] < 0.3 and cum[t] <= cum[t - 1] and cum[t] <= cum[t + 1]:
                best = t; break
        if best < 0:
            best = int(np.argmin(cum[tmin:tmax])) + tmin
            if cum[best] > 0.6:
                return None, 0.0
        a, b, c = cum[best - 1], cum[best], cum[best + 1]
        return sr / (best + 0.5 * (a - c) / max(a - 2 * b + c, 1e-12)), 1 - float(cum[best])

    def tone(sig, hz, w=2.5):
        lo, hi = hz / 2 ** (w / 12), hz * 2 ** (w / 12)
        if hi >= sr / 2 * 0.95:
            return -99.0
        sos = signal.butter(4, [max(15, lo), hi], btype="band", fs=sr, output="sos")
        return db(np.sqrt(np.mean(signal.sosfilt(sos, sig) ** 2)))

    W = int(0.20 * sr)
    f0s, H = [], {h: [] for h in (1, 2, 3, 4, 5, 6, 8)}
    for p_ in pk:
        s = p_ + int(0.13 * sr)
        if s + W > len(x):
            break
        seg = x[s:s + W]
        if np.sqrt(np.mean(seg ** 2)) < 1e-4:
            continue
        f0, c = yin(seg)
        if f0 is None or c < 0.5:
            continue
        f0s.append(f0)
        for h in H:
            H[h].append(tone(seg, f0 * h))
    if not f0s:
        return None
    f0s = np.array(f0s); base = np.mean(H[1])
    g, fold, cnt = 200, np.zeros(200), 0
    for i in range(int(len(env) / per)):
        s = env[int(i * per):int((i + 1) * per)]
        if len(s) >= g:
            fold += s[np.linspace(0, len(s) - 1, g).astype(int)]; cnt += 1
    fold /= max(cnt, 1)
    return dict(bpm=bpm, n=len(f0s), f0=float(np.median(f0s)),
                lo=float(np.percentile(f0s, 10)), hi=float(np.percentile(f0s, 90)),
                harm={h: float(np.mean(H[h]) - base) for h in H},
                duck=float(db(fold.max() / max(fold.min(), 1e-9))),
                vale=float(np.argmin(fold) / g))


# -------------------------------------------------------------- perceptiva

# O ouvido não ouve dB por Hz. Ele soma energia dentro de BANDAS CRÍTICAS
# (escala Bark), espalha cada banda sobre as vizinhas -- que é o mecanismo do
# mascaramento -- e comprime o resultado por uma potência de ~0,23. Um espectro
# em dB ignora as três coisas, e é por isso que ele responde "quanto mede" e
# nunca "como soa".
BARKS = np.arange(0.5, 24.5, 1.0)
BARK_HZ = np.array([600 * np.sinh(z / 6) for z in BARKS])
CAL_SPL = 96.0     # dB SPL assumido para 0 dBFS. Desloca tudo junto; comparações relativas não mudam.


def _spread(dz):
    return 15.81 + 7.5 * (dz + 0.474) - 17.5 * np.sqrt(1 + (dz + 0.474) ** 2)


def _tq(f):
    f = np.maximum(f, 20) / 1000.0
    return 3.64 * f ** -0.8 - 6.5 * np.exp(-0.6 * (f - 3.3) ** 2) + 1e-3 * f ** 4


def excitacao(path):
    x, sr = load(path)
    f, p = signal.welch(x, sr, nperseg=16384, noverlap=8192, scaling="spectrum")
    spl = 10 * np.log10(np.maximum(p, 1e-20)) + CAL_SPL
    z = 13 * np.arctan(0.00076 * f) + 3.5 * np.arctan((f / 7500.0) ** 2)
    E = np.array([10 * np.log10(np.sum(10 ** (spl[(z >= zc - .5) & (z < zc + .5)] / 10)))
                  if ((z >= zc - .5) & (z < zc + .5)).sum() else -100.0 for zc in BARKS])
    return np.array([10 * np.log10(np.sum(10 ** ((E + _spread(zi - BARKS)) / 10))) for zi in BARKS])


def sonoridade(E):
    E0 = 10 ** (_tq(BARK_HZ) / 10)
    return np.maximum(0.08 * ((0.5 + 0.5 * 10 ** (E / 10) / E0) ** 0.23 - 1), 0)


def mascaramento(mascarador, alvo):
    """Quantas bandas críticas do alvo ficam sob o limiar de mascaramento.

    LIMITAÇÃO IMPORTANTE: isto roda sobre o espectro médio da faixa inteira.
    Mascaramento também é fenômeno temporal -- um kick mascara o baixo no
    instante em que bate e solta 100 ms depois, e uma média longa apaga isso.
    Leia como "existe mascaramento estrutural", nunca como "o kick nunca
    esconde o baixo".
    """
    Em, Ea = excitacao(mascarador), excitacao(alvo)
    out = []
    for i, hz in enumerate(BARK_HZ):
        if hz < 25 or hz > 1200:
            continue
        thr = Em[i] - 6.0
        rel = Ea[i] >= Em[i] - 25
        out.append(dict(hz=float(hz), masc=float(Em[i]), alvo=float(Ea[i]),
                        limiar=float(thr), relevante=rel, mascarado=bool(rel and Ea[i] < thr)))
    return out


# ------------------------------------------------------------------ relatório

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--ref", help="faixa de referência para comparar")
    ap.add_argument("--contra", help="outro stem, para análise de mascaramento")
    ap.add_argument("--bass", action="store_true", help="forçar análise de nota do baixo")
    a = ap.parse_args()
    if not os.path.exists(a.path):
        print(f"não encontrei {a.path}"); return 1

    print("=" * 74)
    print(f"  {os.path.basename(a.path)}")
    print("=" * 74)

    n = nivel(a.path)
    print(f"\n--- FÍSICA ---")
    print(f"  duração {int(n['dur']//60)}:{int(n['dur']%60):02d}   {n['sr']} Hz")
    print(f"  LUFS integrado   {n['lufs']:8.2f}")
    print(f"  true peak        {n['tp']:8.2f} dBTP   {'OK' if n['tp']<=-1 else '<<< ACIMA DE -1,0'}")
    print(f"  pico por amostra {n['sp']:8.2f} dBFS")
    print(f"  PSR              {n['psr']:8.2f} dB")
    if n['corr'] is not None:
        print(f"  correlação grave {n['corr']:8.3f}   {'mono' if n['corr']>0.95 else '<<< não é mono'}")

    curva, sh = curva_tonal(a.path)
    print(f"\n  fração de energia por banda")
    for name, _, _ in BANDS:
        print(f"    {name:14s} {sh[name]*100:6.1f}%")

    if a.ref and os.path.exists(a.ref):
        cr, shr = curva_tonal(a.ref)
        A = {c: v for c, v in curva}; B = {c: v for c, v in cr}
        print(f"\n  curva tonal vs {os.path.basename(a.ref)}  (dB vs rosa)")
        print(f"    {'Hz':>7}{'esta':>8}{'ref':>8}{'dif':>8}")
        for c in sorted(A):
            if c in B and 25 <= c <= 16200:
                d = A[c] - B[c]
                print(f"    {c:7.0f}{A[c]:+8.1f}{B[c]:+8.1f}{d:+8.1f}{'  <<<' if abs(d)>=5 else ''}")

    bs = bass_notas(a.path)
    if bs and (a.bass or bs["f0"] < 200):
        print(f"\n--- BAIXO ---")
        print(f"  {bs['bpm']:.1f} BPM   fundamental mediana {bs['f0']:.1f} Hz"
              f"   faixa {bs['lo']:.0f}-{bs['hi']:.0f} Hz   ({bs['n']} notas)")
        print(f"  harmônicos relativos à fundamental:")
        for h in sorted(bs["harm"]):
            print(f"    {h}f0 = {bs['f0']*h:6.0f} Hz : {bs['harm'][h]:+6.1f} dB")
        print(f"  ducking real {bs['duck']:.1f} dB, vale em {bs['vale']*100:.0f}% do beat")

    E = excitacao(a.path); N = sonoridade(E)
    print(f"\n--- PERCEPTIVA (modelo, não medida) ---")
    print(f"  sonoridade total {np.sum(N):.2f} sones")
    print(f"  pesa mais em     {BARK_HZ[np.argmax(N)]:.0f} Hz")
    print(f"  as cinco bandas críticas mais fortes ao ouvido:")
    for i in np.argsort(-N)[:5]:
        print(f"    {BARK_HZ[i]:7.0f} Hz  {N[i]:6.3f} sones")

    if a.contra and os.path.exists(a.contra):
        print(f"\n  mascaramento: {os.path.basename(a.path)} sobre {os.path.basename(a.contra)}")
        rows = mascaramento(a.path, a.contra)
        m = sum(r["mascarado"] for r in rows); t = sum(r["relevante"] for r in rows)
        for r in rows:
            if not r["relevante"]:
                continue
            print(f"    {r['hz']:7.0f} Hz  mascarador {r['masc']:6.1f}  alvo {r['alvo']:6.1f}"
                  f"  limiar {r['limiar']:6.1f}  {'<<< MASCARADO' if r['mascarado'] else 'audível'}")
        print(f"    -> {m} de {t} bandas críticas relevantes ficam mascaradas")
        print(f"    (média de longo prazo: não capta o mascaramento instantâneo do transiente)")

    print(f"\n  A camada perceptiva é um modelo de Zwicker simplificado, calibrado a")
    print(f"  {CAL_SPL:.0f} dB SPL para 0 dBFS. Aproxima a percepção; não é ISO 532-1 certificado,")
    print(f"  e nada aqui diz se a música é boa.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
