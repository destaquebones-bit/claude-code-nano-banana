#!/usr/bin/env python3
"""Separa uma musica em stems por instrumento (vocal, bateria, baixo, resto), via Demucs.

Uso:
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 6
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 2  # so vocal/instrumental
  python3 separate.py --input musica.mp3 --output ~/Desktop/Stems --fast    # sem shifts/overlap extra, bem mais rapido

Roda em MAXIMA QUALIDADE por padrao (shifts + overlap altos). Isso multiplica
o tempo de processamento por ~5x em troca de separacao mais limpa -- espere
alguns minutos a mais por musica. Use --fast se precisar de velocidade em vez
de qualidade (ex: teste rapido).

Teto de granularidade: 6 stems (vocal/bateria/baixo/guitarra/piano/resto) e o
maximo que o Demucs (e qualquer ferramenta publica equivalente, ex. Moises)
separa hoje. Sintetizador, cordas, metais, teclado eletronico e FX nao tem
stem dedicado e sempre caem em "resto" (other) -- isso e um teto do modelo,
nao um parametro que da pra ajustar.

Por padrao exporta em MP3 (nao WAV) para economizar espaco em disco --
stems em WAV sem compressao podem passar de 60-70MB cada, por faixa.
Use --wav se precisar de qualidade sem perdas.
"""
import argparse
import os
import subprocess
import sys

SHIFTS_MAX_QUALIDADE = 5
OVERLAP_MAX_QUALIDADE = "0.75"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True, help="Arquivo de audio ou pasta com varios arquivos")
    ap.add_argument("--output", required=True, help="Pasta de destino dos stems")
    ap.add_argument("--stems", choices=["2", "4", "6"], default="4",
                     help="2 = vocal/instrumental | 4 = vocal/bateria/baixo/resto (default) | 6 = 4 + guitarra/piano")
    ap.add_argument("--wav", action="store_true", help="Exporta em WAV sem perdas (bem mais espaco em disco)")
    ap.add_argument("--fast", action="store_true",
                     help="Desliga shifts/overlap extra (processa bem mais rapido, qualidade padrao do modelo)")
    args = ap.parse_args()

    output = os.path.expanduser(args.output)
    os.makedirs(output, exist_ok=True)

    if args.stems == "6":
        model = "htdemucs_6s"
    else:
        # htdemucs_ft = bag de 4 modelos fine-tuned, melhor qualidade oficial pra 4 stems
        # (nao existe variante "ft" pro modelo de 6 stems)
        model = "htdemucs_ft" if not args.fast else "htdemucs"

    cmd = [sys.executable, "-m", "demucs.separate", "-n", model, "-o", output]
    if args.stems == "2":
        cmd += ["--two-stems=vocals"]
    if not args.wav:
        cmd += ["--mp3"]
    if not args.fast:
        cmd += ["--shifts", str(SHIFTS_MAX_QUALIDADE), "--overlap", OVERLAP_MAX_QUALIDADE]
    cmd.append(os.path.expanduser(args.input))

    print("Rodando:", " ".join(cmd))
    if not args.fast:
        print(f"(modo maxima qualidade: shifts={SHIFTS_MAX_QUALIDADE}, overlap={OVERLAP_MAX_QUALIDADE} "
              f"-- ~5x mais lento que o modo rapido; use --fast pra pular isso)")
    subprocess.run(cmd, check=True)
    print(f"\nStems salvos em: {output}/{model}/")


if __name__ == "__main__":
    main()
