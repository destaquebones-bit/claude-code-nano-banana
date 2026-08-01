#!/usr/bin/env python3
"""Organiza samples de audio em subpastas por instrumento.

Uso:
  python3 categorize.py --source ~/Splice/sounds/packs [--source outra/pasta ...] \
      --dest ~/Desktop/Samples --mode copy
"""
import argparse
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from instrument_rules import AUDIO_EXTS, categorize_by_name


def coletar_arquivos(source_dirs):
    for src in source_dirs:
        src = os.path.expanduser(src)
        for root, _, files in os.walk(src):
            for f in files:
                if f.lower().endswith(AUDIO_EXTS):
                    yield os.path.join(root, f)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--source", action="append", required=True, help="Pasta de origem (repetivel)")
    ap.add_argument("--dest", required=True, help="Pasta de destino (sera criada)")
    ap.add_argument("--mode", choices=["copy", "symlink"], default="copy")
    args = ap.parse_args()

    dest_root = os.path.expanduser(args.dest)
    counts = {}
    total_bytes = 0

    for filepath in coletar_arquivos(args.source):
        categoria = categorize_by_name(os.path.basename(filepath))
        dest_dir = os.path.join(dest_root, categoria)
        os.makedirs(dest_dir, exist_ok=True)

        base_name = os.path.basename(filepath)
        dest_path = os.path.join(dest_dir, base_name)
        if os.path.exists(dest_path) or os.path.islink(dest_path):
            name, ext = os.path.splitext(base_name)
            pack = os.path.basename(os.path.dirname(filepath))
            dest_path = os.path.join(dest_dir, f"{name} [{pack}]{ext}")
            counter = 1
            while os.path.exists(dest_path) or os.path.islink(dest_path):
                dest_path = os.path.join(dest_dir, f"{name} [{pack}]_{counter}{ext}")
                counter += 1

        if args.mode == "copy":
            shutil.copy2(filepath, dest_path)
            total_bytes += os.path.getsize(dest_path)
        else:
            os.symlink(filepath, dest_path)

        counts[categoria] = counts.get(categoria, 0) + 1

    print("Resumo por categoria:")
    for categoria, n in sorted(counts.items(), key=lambda x: -x[1]):
        print(f"  {categoria}: {n}")
    print(f"\nTotal: {sum(counts.values())} arquivos")
    if args.mode == "copy":
        print(f"Tamanho copiado: {total_bytes / (1024**3):.2f} GB")
    print(f"Destino: {dest_root}")


if __name__ == "__main__":
    main()
