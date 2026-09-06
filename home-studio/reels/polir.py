#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
POLIR - corrige o amadorismo de um Reels ja montado e reencaixa na batida.
Spectral Sun / bass traps.  Precisa de python3 + ffmpeg.

O que ele conserta, na ordem em que importa:
  1. CLIPE HORIZONTAL COM BLUR FILL -> recorta a faixa nitida e preenche a tela
  2. SUBEXPOSICAO                   -> exposicao/contraste/gama por clipe
  3. RUIDO DE ISO ALTO              -> hqdn3d leve
  4. CAMERA TREMIDA                 -> deshake opcional por clipe
  5. PLANO GERAL SEM LEITURA        -> punch-in digital (zoom) no que interessa
  6. TRALHA NO ENQUADRAMENTO        -> mascara (borrao) em regiao configuravel
  7. RITMO SOLTO                    -> tudo cortado na grade de compassos do BPM
  8. TIPOGRAFIA FRACA               -> cartelas grandes por cima
"""
import os, subprocess, shutil, sys

# ================= CONFIG =================
SRC   = "bass_traps_reel_v5.mp4"   # <<< o seu video
BPM   = 126.0                      # <<< BPM da faixa
MUSICA= ""                         # <<< faixa nova; vazio = mantem o audio do SRC
OUT   = "reels_v6.mp4"
CARDS = "."
W,H,FPS = 1080,1920,30

# MASCARAS: regioes a borrar no video inteiro. (x, y, largura, altura)
# Use para sumir com objeto indesejado. Coordenadas em pixels de 1080x1920.
MASCARAS = [
    # ("cinzeiro", 620, 980, 300, 260),   # <<< descomente e ajuste
]

# EDL: (t_in, compassos, velocidade, modo, exposicao, cartela)
#   modo "fill"          -> enquadra preenchendo a tela
#   modo "band:Y0:ALT"   -> recorta a faixa nitida (clipe horizontal) e preenche
#   modo "zoom:F:CX:CY"  -> punch-in de fator F centrado em CX,CY (0..1)
#   exposicao: (brilho, contraste, gama) ou None
EDL = [
 (36.4, 2, 1.0, "zoom:1.25:0.50:0.45", None,              "c01_gancho"),
 (43.2, 2, 1.0, "fill",                (0.04,1.10,1.05),  "c03_faixa"),
 ( 2.1, 1, 1.0, "zoom:1.34:0.50:0.36", (0.06,1.12,1.08),  "c04_porta"),
 (10.3, 1, 1.5, "zoom:1.35:0.62:0.72", (0.10,1.20,1.12),  "c05_quadrado"),
 ( 5.2, 1, 1.3, "zoom:1.22:0.48:0.42", (0.05,1.12,1.06),  None),
 (18.3, 1, 2.0, "zoom:1.30:0.52:0.62", (0.08,1.15,1.10),  "c06_angulo"),
 (14.2, 1, 1.5, "fill",                (0.07,1.12,1.06),  None),
 (26.2, 1, 1.5, "zoom:1.20:0.50:0.50", (0.05,1.10,1.05),  "c07_la"),
 (31.2, 1, 1.5, "fill",                (0.05,1.10,1.05),  None),
 (22.6, 1, 1.0, "band:660:560",        (0.07,1.14,1.08),  None),
 (37.0, 1, 1.0, "fill",                (0.03,1.08,1.03),  "c08_antes"),
 (44.0, 1, 1.0, "fill",                (0.03,1.08,1.03),  "c09_depois"),
 (49.5, 2, 1.0, "zoom:1.38:0.50:0.34", (0.03,1.08,1.03),  "c10_sala"),
]
DESHAKE   = True    # estabiliza tudo (leve)
DENOISE   = True    # tira grao dos planos escuros
QUATRO_K  = False   # True = exporta 2160x3840 (NAO cria detalhe; so upscale)
# ================= FIM =================

BEAT=60.0/BPM; BAR=4*BEAT; TMP="_tmp_polir"

def sh(c):
    r=subprocess.run(c,capture_output=True,text=True)
    if r.returncode!=0:
        print("\n[ffmpeg falhou]\n"," ".join(c[:14]),"...\n",r.stderr[-1600:]); sys.exit(1)

def cadeia(modo, expo):
    """monta o filtro de video de um segmento"""
    f=[]
    if modo.startswith("band:"):
        _,y0,alt=modo.split(":")
        f.append(f"crop={W}:{alt}:0:{y0}")               # so a faixa nitida
        f.append(f"scale={W}:{H}:force_original_aspect_ratio=increase")
        f.append(f"crop={W}:{H}")
    elif modo.startswith("zoom:"):
        _,fa,cx,cy=modo.split(":"); fa=float(fa); cx=float(cx); cy=float(cy)
        cw,ch=1.0/fa,1.0/fa
        f.append(f"crop=iw*{cw:.4f}:ih*{ch:.4f}:iw*{max(0,min(1-cw,cx-cw/2)):.4f}:ih*{max(0,min(1-ch,cy-ch/2)):.4f}")
        f.append(f"scale={W}:{H}:force_original_aspect_ratio=increase")
        f.append(f"crop={W}:{H}")
    else:
        f.append(f"scale={W}:{H}:force_original_aspect_ratio=increase")
        f.append(f"crop={W}:{H}")
    if expo:
        b,c,g=expo; f.append(f"eq=brightness={b}:contrast={c}:gamma={g}")
    if DENOISE: f.append("hqdn3d=3:2:5:4")
    if DESHAKE: f.append("deshake=rx=16:ry=16")
    f.append("unsharp=5:5:0.5")          # devolve micro-contraste depois do denoise
    f.append(f"setsar=1,fps={FPS}")
    return f

def mascara_filtro():
    """borra as regioes indesejadas (aplicado no video ja montado)"""
    if not MASCARAS: return None
    parts=[]
    for i,(nome,x,y,w,h) in enumerate(MASCARAS):
        parts.append(f"[{'0:v' if i==0 else f'm{i-1}'}]split[a{i}][b{i}];"
                     f"[b{i}]crop={w}:{h}:{x}:{y},boxblur=28:3[bl{i}];"
                     f"[a{i}][bl{i}]overlay={x}:{y}[m{i}]")
    return ";".join(parts), f"m{len(MASCARAS)-1}"

def main():
    if not shutil.which("ffmpeg"): print("instale o ffmpeg"); sys.exit(1)
    if not os.path.exists(SRC):    print(f"nao achei {SRC}"); sys.exit(1)
    os.makedirs(TMP,exist_ok=True)
    segs=[]; t=0.0; marcas=[]
    print(f"BPM {BPM:g} | tempo {BEAT:.3f}s | compasso {BAR:.3f}s\n")
    for i,(tin,comp,vel,modo,expo,card) in enumerate(EDL):
        dur=comp*BAR; fonte=dur*vel; out=f"{TMP}/s{i:02d}.mp4"
        vf=cadeia(modo,expo)
        if vel!=1.0: vf.append(f"setpts={1.0/vel:.6f}*PTS")
        sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-ss",f"{tin:.3f}",
            "-t",f"{fonte:.3f}","-i",SRC,"-vf",",".join(vf),
            "-c:v","libx264","-crf","17","-pix_fmt","yuv420p","-an",
            "-t",f"{dur:.3f}",out])
        print(f"  {t:6.2f}s  fonte {tin:5.1f}s  {comp}c {dur:5.2f}s  x{vel:<4g} {modo}")
        if card: marcas.append((card,t,t+dur))
        segs.append(out); t+=dur
    total=t

    with open(f"{TMP}/l.txt","w") as f:
        for s in segs: f.write(f"file '{os.path.basename(s)}'\n")
    sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-f","concat","-safe","0",
        "-i",f"{TMP}/l.txt","-c","copy",f"{TMP}/v.mp4"])

    base=f"{TMP}/v.mp4"
    mf=mascara_filtro()
    if mf:
        fc,lbl=mf
        sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-i",base,
            "-filter_complex",fc,"-map",f"[{lbl}]","-c:v","libx264","-crf","17",
            "-pix_fmt","yuv420p","-an",f"{TMP}/vm.mp4"]); base=f"{TMP}/vm.mp4"

    ins=[]; fc=[]; last="0:v"
    for n,(card,a,b) in enumerate(marcas):
        p=os.path.join(CARDS,card+".png")
        if not os.path.exists(p): print(f"  ! falta {p}"); continue
        ins+=["-loop","1","-t",f"{total:.3f}","-framerate",str(FPS),"-i",p]
        fc.append(f"[{n+1}:v]format=rgba,fade=t=in:st={a:.2f}:d=0.18:alpha=1,"
                  f"fade=t=out:st={b-0.22:.2f}:d=0.18:alpha=1[c{n}];"
                  f"[{last}][c{n}]overlay=0:0:enable='between(t,{a:.2f},{b:.2f})'[v{n}]")
        last=f"v{n}"
    cmd=["ffmpeg","-hide_banner","-loglevel","error","-y","-i",base]+ins
    if fc: cmd+=["-filter_complex",";".join(fc),"-map",f"[{last}]"]
    cmd+=["-c:v","libx264","-crf","17","-pix_fmt","yuv420p","-an",f"{TMP}/vc.mp4"]
    sh(cmd)

    fonte_audio = MUSICA if (MUSICA and os.path.exists(MUSICA)) else SRC
    final=f"{TMP}/vc.mp4"
    if QUATRO_K:
        sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-i",final,
            "-vf","scale=2160:3840:flags=lanczos","-c:v","libx264","-crf","18",
            "-pix_fmt","yuv420p","-an",f"{TMP}/v4k.mp4"]); final=f"{TMP}/v4k.mp4"
    sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-i",final,"-i",fonte_audio,
        "-filter_complex",f"[1:a]atrim=0:{total:.3f},asetpts=N/SR/TB,"
                          f"afade=t=in:st=0:d=0.15,afade=t=out:st={total-1.0:.2f}:d=1.0[a]",
        "-map","0:v","-map","[a]","-c:v","copy","-c:a","aac","-b:a","192k","-shortest",OUT])
    print(f"\nPRONTO: {OUT}  {total:.2f}s  {'2160x3840' if QUATRO_K else f'{W}x{H}'}")

if __name__=="__main__": main()
