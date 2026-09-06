#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Corte A - Reels 28s "Como fiz meus bass traps" - Spectral Sun
Roda na SUA maquina. Precisa de: python3 e ffmpeg.

USO:
  1. Preencha BPM e a secao CLIPES com os caminhos dos seus arquivos.
  2. python3 montar.py
  3. Sai reels_corteA.mp4 em 1080x1920.

Clipe nao preenchido vira placeholder cinza com o nome da cena,
entao da pra rodar e ver a estrutura antes de ter tudo em maos.
"""
import os, subprocess, shutil, sys, math

# ============ CONFIG ============
BPM      = 126.0                 # <<< O BPM DA "STOMP". Troque aqui.
MUSICA   = "stomp.wav"           # <<< caminho da faixa
MUSICA_IN= 0.0                   # segundo da faixa onde comecar (o drop, de preferencia)
OUT      = "reels_corteA.mp4"
CARDS    = "."                   # pasta com c01_gancho.png ... c10_sala.png

# Cada cena: (arquivo, segundo_inicial_do_clipe, cartela_ou_None)
# Deixe o arquivo como "" para virar placeholder.
CLIPES = {
 "gancho"    : ("", 0.0, "c01_gancho"),   # trap deslizando pro canto
 "so_um_lado": ("", 0.0, "c02_soum"),     # grafico / sala
 "faixa"     : ("", 0.0, "c03_faixa"),    # print do grafico, zoom no buraco
 "porta"     : ("", 0.0, "c04_porta"),    # planta com a seta na porta
 "quadrado"  : ("", 0.0, "c05_quadrado"), # marcando o quadrado na chapa
 "diagonal"  : ("", 0.0, None),           # o corte da diagonal - TEMPO REAL
 "parafuso"  : ("", 0.0, "c06_angulo"),   # furadeira / parafusando
 "esqueleto" : ("", 0.0, None),           # esqueleto de pe, vazio
 "la"        : ("", 0.0, "c07_la"),       # a la de PET entrando
 "tecido"    : ("", 0.0, None),           # fechando com tecido
 "antes"     : ("", 0.0, "c08_antes"),    # grafico antes
 "depois"    : ("", 0.0, "c09_depois"),   # grafico depois
 "sala"      : ("", 0.0, "c10_sala"),     # plano aberto da sala
}

# Estrutura em COMPASSOS (1 compasso = 4 tempos). (cena, compassos, velocidade)
# velocidade 1.0 = tempo real; 8.0 = 8x acelerado.
ROTEIRO = [
 ("gancho",    2, 1.0),
 ("faixa",     2, 1.0),
 ("porta",     1, 1.0),
 ("so_um_lado",1, 1.0),
 ("quadrado",  1, 6.0),
 ("diagonal",  1, 1.0),   # tempo real: e o momento satisfatorio
 ("parafuso",  1, 10.0),
 ("esqueleto", 1, 1.0),
 ("la",        1, 8.0),
 ("tecido",    1, 8.0),
 ("antes",     1, 1.0),
 ("depois",    1, 1.0),
 ("sala",      2, 1.0),
]
# ============ FIM DA CONFIG ============

W,H,FPS = 1080,1920,30
BEAT = 60.0/BPM
BAR  = 4*BEAT
TMP  = "_tmp_reels"

def sh(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print("\n[ffmpeg falhou]\n"," ".join(cmd[:12]),"...\n",r.stderr[-1500:]); sys.exit(1)

def placeholder(nome, dur, path):
    txt = nome.replace("_"," ").upper()
    sh(["ffmpeg","-hide_banner","-loglevel","error","-y",
        "-f","lavfi","-i",f"color=c=0x4a4a46:s={W}x{H}:r={FPS}:d={dur:.3f}",
        "-vf",f"drawtext=text='{txt}':fontcolor=0x8a8579:fontsize=64:x=(w-tw)/2:y=(h-th)/2,"
              f"drawbox=x=40:y=40:w={W-80}:h={H-80}:color=0x8a8579@0.5:t=3",
        "-c:v","libx264","-pix_fmt","yuv420p","-an",path])

def normaliza(src, ss, dur_saida, vel, path):
    """Corta, acelera e enquadra em 1080x1920 sem distorcer."""
    dur_fonte = dur_saida*vel
    vf = (f"scale={W}:{H}:force_original_aspect_ratio=increase,"
          f"crop={W}:{H},setsar=1,fps={FPS},setpts={1.0/vel:.6f}*PTS")
    sh(["ffmpeg","-hide_banner","-loglevel","error","-y",
        "-ss",f"{ss:.3f}","-t",f"{dur_fonte:.3f}","-i",src,
        "-vf",vf,"-c:v","libx264","-crf","18","-pix_fmt","yuv420p","-an",
        "-t",f"{dur_saida:.3f}",path])

def main():
    if not shutil.which("ffmpeg"):
        print("ffmpeg nao encontrado. Instale antes:\n"
              "  macOS:  brew install ffmpeg\n"
              "  Ubuntu: sudo apt install ffmpeg\n"
              "  Windows: https://ffmpeg.org/download.html"); sys.exit(1)
    os.makedirs(TMP, exist_ok=True)
    segs, t, marcas = [], 0.0, []
    print(f"BPM {BPM:g}  |  tempo {BEAT:.3f}s  |  compasso {BAR:.3f}s\n")
    for i,(cena,compassos,vel) in enumerate(ROTEIRO):
        dur = compassos*BAR
        src, ss, card = CLIPES[cena]
        out = f"{TMP}/s{i:02d}.mp4"
        if src and os.path.exists(src):
            normaliza(src, ss, dur, vel, out)
            tag = f"{os.path.basename(src)} @{ss:g}s x{vel:g}"
        else:
            placeholder(cena, dur, out)
            tag = "PLACEHOLDER" if not src else f"NAO ENCONTRADO: {src}"
        print(f"  {t:6.2f}s  {cena:<11} {compassos} comp  {dur:5.2f}s  {tag}")
        if card: marcas.append((card, t, t+dur))
        segs.append(out); t += dur
    total = t

    with open(f"{TMP}/lista.txt","w") as f:
        for s in segs: f.write(f"file '{os.path.basename(s)}'\n")
    sh(["ffmpeg","-hide_banner","-loglevel","error","-y","-f","concat","-safe","0",
        "-i",f"{TMP}/lista.txt","-c","copy",f"{TMP}/video.mp4"])

    # cartelas por cima, com fade de 6 quadros
    ins, fc, last = [], [], "0:v"
    for n,(card,a,b) in enumerate(marcas):
        p = os.path.join(CARDS, card+".png")
        if not os.path.exists(p):
            print(f"  ! cartela ausente: {p}"); continue
        ins += ["-loop","1","-t",f"{total:.3f}","-framerate",str(FPS),"-i",p]
        fc.append(f"[{n+1}:v]format=rgba,fade=t=in:st={a:.2f}:d=0.2:alpha=1,"
                  f"fade=t=out:st={b-0.25:.2f}:d=0.2:alpha=1[c{n}];"
                  f"[{last}][c{n}]overlay=0:0:enable='between(t,{a:.2f},{b:.2f})'[v{n}]")
        last = f"v{n}"
    cmd = ["ffmpeg","-hide_banner","-loglevel","error","-y","-i",f"{TMP}/video.mp4"]+ins
    if fc: cmd += ["-filter_complex",";".join(fc),"-map",f"[{last}]"]
    cmd += ["-c:v","libx264","-crf","18","-pix_fmt","yuv420p","-an",f"{TMP}/comcard.mp4"]
    sh(cmd)

    # musica
    if os.path.exists(MUSICA):
        sh(["ffmpeg","-hide_banner","-loglevel","error","-y",
            "-i",f"{TMP}/comcard.mp4","-ss",f"{MUSICA_IN:.3f}","-i",MUSICA,
            "-filter_complex",f"[1:a]atrim=0:{total:.3f},afade=t=out:st={total-1.2:.2f}:d=1.2[a]",
            "-map","0:v","-map","[a]","-c:v","copy","-c:a","aac","-b:a","192k",
            "-shortest",OUT])
    else:
        print(f"\n  ! {MUSICA} nao encontrado - saindo mudo")
        shutil.copy(f"{TMP}/comcard.mp4", OUT)
    print(f"\nPRONTO: {OUT}   {total:.2f}s   {W}x{H}")
    print(f"Descarte a pasta {TMP}/ quando estiver satisfeito.")

if __name__=="__main__": main()
