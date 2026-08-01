#!/usr/bin/env python3
"""Separa uma musica em stems por instrumento (vocal, bateria, baixo, resto), via Demucs.

Uso:
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 4
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 2  # so vocal/instrumental

Por padrao exporta em MP3 (nao WAV) para economizar espaco em disco --
stems em WAV sem compressao podem passar de 60-70MB cada, por faixa.
Use --wav se precisar de qualidade sem perdas.
"""
import argparse
import os
import subprocess
import sys


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True, help="Arquivo de audio ou pasta com varios arquivos")
    ap.add_argument("--output", required=True, help="Pasta de destino dos stems")
    ap.add_argument("--stems", choices=["2", "4", "6"], default="4",
                     help="2 = vocal/instrumental | 4 = vocal/bateria/baixo/resto (default) | 6 = 4 + guitarra/piano")
    ap.add_argument("--wav", action="store_true", help="Exporta em WAV sem perdas (bem mais espaco em disco)")
    args = ap.parse_args()

    output = os.path.expanduser(args.output)
    os.makedirs(output, exist_ok=True)

    model = "htdemucs_6s" if args.stems == "6" else "htdemucs"

    cmd = [sys.executable, "-m", "demucs.separate", "-n", model, "-o", output]
    if args.stems == "2":
        cmd += ["--two-stems=vocals"]
    if not args.wav:
        cmd += ["--mp3"]
    cmd.append(os.path.expanduser(args.input))

    print("Rodando:", " ".join(cmd))
    subprocess.run(cmd, check=True)
    print(f"\nStems salvos em: {output}/{model}/")


if __name__ == "__main__":
    main()
