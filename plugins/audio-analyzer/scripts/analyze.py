#!/usr/bin/env python3
"""Analisa samples de audio: tonalidade, BPM e loudness (via Essentia).

Modos:
  report   - imprime a analise de um arquivo ou pasta (nao altera nada)
  rename   - renomeia arquivos de uma pasta adicionando " - <Key>" no final
             (extrai do nome quando possivel; senao usa deteccao de audio,
             marcando com "(est)")

Uso:
  python3 analyze.py report --path arquivo.wav
  python3 analyze.py report --path pasta/
  python3 analyze.py rename --path pasta/ [--categorias "Baixo,Sintetizador,..."]
"""
import argparse
import json
import os
import re
import sys
import warnings

warnings.filterwarnings("ignore")
os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from instrument_rules import AUDIO_EXTS, TONAL_CATEGORIAS

MODELS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "models")

MOOD_HEADS = {
    "mood_happy": ("happy", "non_happy"),
    "mood_aggressive": ("aggressive", "non_aggressive"),
    "mood_relaxed": ("relaxed", "non_relaxed"),
    "mood_sad": ("sad", "non_sad"),
    "mood_party": ("party", "non_party"),
    "mood_electronic": ("electronic", "non_electronic"),
    "mood_acoustic": ("acoustic", "non_acoustic"),
    "danceability": ("danceable", "non_danceable"),
}

KEY_TOKEN_RE = re.compile(r'^([A-Ga-g])(#|b)?(min|maj|m)?$')


def normaliza_key(letra, acidente, modo):
    letra = letra.upper()
    acidente = acidente or ""
    sufixo = "m" if modo and modo.lower() in ("min", "m") else ""
    return f"{letra}{acidente}{sufixo}"


def extrai_key_do_nome(name_no_ext):
    tokens = re.split(r'([^A-Za-z0-9#]+)', name_no_ext)
    for i, tok in enumerate(tokens):
        if not tok or not re.match(r'^[A-Za-z0-9#]+$', tok) or len(tok) > 5:
            continue
        m = KEY_TOKEN_RE.match(tok)
        if m:
            letra, acidente, modo = m.groups()
            key_norm = normaliza_key(letra, acidente, modo)
            restante = "".join(tokens[:i] + tokens[i + 1:])
            restante = re.sub(r'[-_ ]+', '_', restante).strip('_- ')
            return key_norm, restante
    return None, None


def analisar_audio(filepath):
    """Roda Essentia no arquivo. Retorna dict com key, scale, key_strength, bpm, bpm_conf,
    lufs_integrated, loudness_range, true_peak_dbtp.

    Loudness e medida via LoudnessEBUR128 (padrao EBU R128 / ITU-R BS.1770 -- o mesmo usado
    por DAWs, plataformas de streaming e engenheiros de mastering) e TruePeakDetector (dBTP,
    com oversampling, pra pegar clipping inter-sample que um peak meter comum nao mostra).
    NAO usar essentia.standard.Loudness() puro -- e uma medida de energia bruta (escala de
    milhares), nao comparavel com LUFS e inutil pra comparar contra qualquer referencia real.
    """
    import math
    import numpy as np
    import essentia.standard as es

    mono = es.MonoLoader(filename=filepath, sampleRate=44100)()
    if len(mono) < 1024:
        return None

    key, scale, strength = es.KeyExtractor()(mono)
    result = {
        "key": key,
        "scale": scale,
        "key_strength": round(float(strength), 3),
    }
    try:
        bpm, _, beats_conf, _, _ = es.RhythmExtractor2013()(mono)
        result["bpm"] = round(float(bpm), 1)
        result["bpm_confidence"] = round(float(beats_conf), 3)
    except Exception:
        result["bpm"] = None
        result["bpm_confidence"] = None

    try:
        stereo, sr, channels, _, _, _ = es.AudioLoader(filename=filepath)()
        if channels == 1:
            stereo = np.column_stack([stereo[:, 0], stereo[:, 0]])
        _, _, integrated, lra = es.LoudnessEBUR128(sampleRate=sr)(stereo)
        result["lufs_integrated"] = round(float(integrated), 2)
        result["loudness_range_lu"] = round(float(lra), 2)

        left = stereo[:, 0].astype(np.float32)
        right = stereo[:, 1].astype(np.float32)
        _, out_l = es.TruePeakDetector()(left)
        _, out_r = es.TruePeakDetector()(right)
        peak = max(np.max(np.abs(out_l)), np.max(np.abs(out_r)))
        result["true_peak_dbtp"] = round(20 * math.log10(peak), 2) if peak > 0 else None
    except Exception as e:
        result["lufs_integrated"] = None
        result["loudness_range_lu"] = None
        result["true_peak_dbtp"] = None
        result["loudness_error"] = str(e)

    return result


def key_para_notacao_curta(key, scale):
    letra = key.replace("b", "b").strip()
    if scale == "minor":
        return f"{letra}m"
    return letra


def analisar_audio_profundo(filepath):
    """Descritores adicionais (nao criticos de tonalidade/loudness, mas uteis pra
    caracterizar textura/energia e comparar contra faixas de referencia):

    - spectral_centroid_hz: 'brilho' medio (centro de massa do espectro). Mais alto =
      mais presenca de agudo/brilho; mais baixo = som mais grave/abafado.
    - rolloff_85_hz: frequencia abaixo da qual estao 85% da energia. Complementa o
      centroid pra descrever balanco espectral.
    - onset_rate: onsets (transientes) por segundo -- proxy de "quao ocupada" a faixa
      e ritmicamente. Mais alto = mais elementos percussivos/transientes por segundo.
    - danceability: medida de regularidade ritmica do Essentia (0 a ~3, mais alto =
      mais "dancavel"/regular).
    - dynamic_complexity: variacao de loudness ao longo do tempo (nao confundir com
      loudness_range_lu do EBU R128 -- essa e outra metrica, mais sensivel a mudanca
      frame a frame).
    - stereo_correlation: correlacao entre canais L/R (-1 a 1). Perto de 1 = quase mono
      (pouca informacao estereo); perto de 0 ou negativo = muito largo, risco de
      problema de compatibilidade mono (cancelamento de fase).
    """
    import numpy as np
    import essentia.standard as es

    mono = es.MonoLoader(filename=filepath, sampleRate=44100)()
    if len(mono) < 4096:
        return None

    result = {}

    w = es.Windowing(type="hann")
    spectrum = es.Spectrum()
    centroid_algo = es.Centroid(range=22050)
    rolloff_algo = es.RollOff()

    centroids = []
    rolloffs = []
    for frame in es.FrameGenerator(mono, frameSize=2048, hopSize=1024, startFromZero=True):
        spec = spectrum(w(frame))
        if np.sum(spec) <= 0:
            continue
        centroids.append(centroid_algo(spec))
        rolloffs.append(rolloff_algo(spec))

    if centroids:
        result["spectral_centroid_hz"] = round(float(np.mean(centroids)), 1)
        result["rolloff_85_hz"] = round(float(np.mean(rolloffs)), 1)
    else:
        result["spectral_centroid_hz"] = None
        result["rolloff_85_hz"] = None

    try:
        _, onset_rate = es.OnsetRate()(mono)
        result["onset_rate_per_sec"] = round(float(onset_rate), 2)
    except Exception:
        result["onset_rate_per_sec"] = None

    try:
        danceability, _ = es.Danceability()(mono)
        result["danceability"] = round(float(danceability), 3)
    except Exception:
        result["danceability"] = None

    try:
        dyn_complexity, _ = es.DynamicComplexity()(mono)
        result["dynamic_complexity"] = round(float(dyn_complexity), 3)
    except Exception:
        result["dynamic_complexity"] = None

    try:
        stereo, sr, channels, _, _, _ = es.AudioLoader(filename=filepath)()
        if channels >= 2:
            left = stereo[:, 0].astype(np.float64)
            right = stereo[:, 1].astype(np.float64)
            n = min(len(left), len(right))
            corr = float(np.corrcoef(left[:n], right[:n])[0, 1])
            result["stereo_correlation"] = round(corr, 3)
        else:
            result["stereo_correlation"] = None
    except Exception:
        result["stereo_correlation"] = None

    return result


def analisar_mood(filepath):
    """Classificadores de humor/dancabilidade via modelos MusiCNN pre-treinados
    (essentia-tensorflow, dataset MTG). CONFIABILIDADE LIMITADA -- ver ressalva
    no SKILL.md: sao classificadores binarios independentes (nao mutuamente
    exclusivos), alguns treinados em poucas centenas de faixas, e podem
    contradizer uns aos outros (ex: 'relaxed' e 'aggressive' altos ao mesmo
    tempo) ou nao generalizar bem pra tech house/house moderno. Usar como MAIS
    UM sinal, nunca como veredito isolado.
    """
    import numpy as np
    from essentia.standard import MonoLoader, TensorflowPredictMusiCNN, TensorflowPredict2D

    base_model_path = os.path.join(MODELS_DIR, "msd-musicnn-1.pb")
    if not os.path.isfile(base_model_path):
        return {"erro": f"modelo nao encontrado em {base_model_path} -- ver scripts/download_models.sh"}

    audio = MonoLoader(filename=filepath, sampleRate=16000, resampleQuality=4)()
    embedding_model = TensorflowPredictMusiCNN(graphFilename=base_model_path, output="model/dense/BiasAdd")
    embeddings = embedding_model(audio)

    result = {}
    for head, (pos_label, neg_label) in MOOD_HEADS.items():
        head_path = os.path.join(MODELS_DIR, f"{head}-msd-musicnn-1.pb")
        if not os.path.isfile(head_path):
            continue
        model = TensorflowPredict2D(graphFilename=head_path, input="model/Placeholder", output="model/Softmax")
        predictions = model(embeddings)
        avg = np.mean(predictions, axis=0)
        result[pos_label] = round(float(avg[0]), 3)
    return result


def cmd_mood(args):
    path = os.path.expanduser(args.path)
    if os.path.isfile(path):
        arquivos = [path]
    else:
        arquivos = [
            os.path.join(root, f)
            for root, _, files in os.walk(path)
            for f in files
            if f.lower().endswith(AUDIO_EXTS)
        ]

    resultados = []
    for fp in arquivos:
        r = analisar_mood(fp) or {}
        r["arquivo"] = fp
        resultados.append(r)

    print(json.dumps(resultados, indent=2, ensure_ascii=False))


def cmd_deep(args):
    path = os.path.expanduser(args.path)
    if os.path.isfile(path):
        arquivos = [path]
    else:
        arquivos = [
            os.path.join(root, f)
            for root, _, files in os.walk(path)
            for f in files
            if f.lower().endswith(AUDIO_EXTS)
        ]

    resultados = []
    for fp in arquivos:
        base = analisar_audio(fp) or {}
        deep = analisar_audio_profundo(fp) or {}
        r = {**base, **deep, "arquivo": fp}
        resultados.append(r)

    print(json.dumps(resultados, indent=2, ensure_ascii=False))


def cmd_report(args):
    path = os.path.expanduser(args.path)
    if os.path.isfile(path):
        arquivos = [path]
    else:
        arquivos = [
            os.path.join(root, f)
            for root, _, files in os.walk(path)
            for f in files
            if f.lower().endswith(AUDIO_EXTS)
        ]

    resultados = []
    for fp in arquivos:
        r = analisar_audio(fp)
        if r:
            r["arquivo"] = fp
            resultados.append(r)

    print(json.dumps(resultados, indent=2, ensure_ascii=False))


def cmd_rename(args):
    path = os.path.expanduser(args.path)
    categorias = args.categorias.split(",") if args.categorias else TONAL_CATEGORIAS

    stats = {"extraido_do_nome": 0, "estimado_audio": 0, "falhou": 0}

    def processa_pasta(dir_path):
        for f in list(os.listdir(dir_path)):
            fp = os.path.join(dir_path, f)
            if not os.path.isfile(fp) or not f.lower().endswith(AUDIO_EXTS):
                continue
            name_no_ext, ext = os.path.splitext(f)
            key, resto = extrai_key_do_nome(name_no_ext)
            if key:
                novo_base = f"{resto} - {key}"
                stats["extraido_do_nome"] += 1
            else:
                r = analisar_audio(fp)
                if not r:
                    stats["falhou"] += 1
                    continue
                key_curta = key_para_notacao_curta(r["key"], r["scale"])
                novo_base = f"{name_no_ext} - {key_curta} (est)"
                stats["estimado_audio"] += 1

            novo_fp = os.path.join(dir_path, f"{novo_base}{ext}")
            if novo_fp != fp:
                if os.path.exists(novo_fp):
                    novo_fp = os.path.join(dir_path, f"{novo_base}_dup{ext}")
                os.rename(fp, novo_fp)

    # subpastas por categoria (uso tipico: pasta organizada por instrumento)
    for cat in categorias:
        cat_dir = os.path.join(path, cat)
        if os.path.isdir(cat_dir):
            processa_pasta(cat_dir)
        elif os.path.isdir(path) and os.path.basename(path) == cat:
            processa_pasta(path)

    print(json.dumps(stats, indent=2, ensure_ascii=False))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    p_report = sub.add_parser("report", help="Analisa e imprime resultado em JSON, sem alterar arquivos")
    p_report.add_argument("--path", required=True)
    p_report.set_defaults(func=cmd_report)

    p_deep = sub.add_parser("deep", help="Como 'report', mais descritores de textura/energia (brilho, densidade ritmica, dancabilidade, estereo)")
    p_deep.add_argument("--path", required=True)
    p_deep.set_defaults(func=cmd_deep)

    p_mood = sub.add_parser("mood", help="Classificadores de humor/dancabilidade (MusiCNN pre-treinado) -- confiabilidade limitada, ver SKILL.md")
    p_mood.add_argument("--path", required=True)
    p_mood.set_defaults(func=cmd_mood)

    p_rename = sub.add_parser("rename", help="Renomeia arquivos adicionando a tonalidade no final")
    p_rename.add_argument("--path", required=True)
    p_rename.add_argument("--categorias", default=None, help="Lista separada por virgula (default: categorias tonais)")
    p_rename.set_defaults(func=cmd_rename)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
