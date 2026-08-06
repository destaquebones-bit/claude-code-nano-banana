#!/usr/bin/env python3
"""Compara uma faixa (draft) contra um conjunto de faixas de referencia, usando os
mesmos descritores do analyze.py (tonalidade/BPM/loudness + brilho/dancabilidade/mood).

Uso:
  python3 compare.py --target draft.wav --referencias ref1.wav ref2.wav ref3.wav
  python3 compare.py --target draft.wav --referencias-dir ~/Referencias/
"""
import argparse
import json
import os
import statistics
import sys
import warnings

warnings.filterwarnings("ignore")
os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from instrument_rules import AUDIO_EXTS
from analyze import analisar_audio, analisar_audio_profundo, analisar_mood

CAMPOS_NUMERICOS = [
    "bpm", "lufs_integrated", "loudness_range_lu", "true_peak_dbtp",
    "spectral_centroid_hz", "rolloff_85_hz", "onset_rate_per_sec",
    "danceability", "dynamic_complexity", "stereo_correlation",
]


def analisar_completo(filepath):
    r = {}
    r.update(analisar_audio(filepath) or {})
    r.update(analisar_audio_profundo(filepath) or {})
    try:
        r.update(analisar_mood(filepath) or {})
    except Exception:
        pass
    return r


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", required=True)
    ap.add_argument("--referencias", nargs="*", default=[])
    ap.add_argument("--referencias-dir", default=None)
    args = ap.parse_args()

    refs = list(args.referencias)
    if args.referencias_dir:
        d = os.path.expanduser(args.referencias_dir)
        refs += [
            os.path.join(root, f)
            for root, _, files in os.walk(d)
            for f in files
            if f.lower().endswith(AUDIO_EXTS)
        ]

    if not refs:
        print("Nenhuma faixa de referencia fornecida.", file=sys.stderr)
        sys.exit(1)

    print(f"Analisando faixa alvo: {os.path.basename(args.target)}...", file=sys.stderr)
    target_data = analisar_completo(os.path.expanduser(args.target))

    ref_data = []
    for rf in refs:
        print(f"Analisando referencia: {os.path.basename(rf)}...", file=sys.stderr)
        ref_data.append(analisar_completo(os.path.expanduser(rf)))

    print("\n=== TOM/BPM ===")
    print(f"Alvo: {target_data.get('key')} {target_data.get('scale')} @ {target_data.get('bpm')} BPM")
    tons_refs = [f"{d.get('key')} {d.get('scale')}" for d in ref_data]
    bpms_refs = [d.get("bpm") for d in ref_data if d.get("bpm")]
    print(f"Referencias: {', '.join(tons_refs)}")
    if bpms_refs:
        print(f"BPM medio das referencias: {statistics.mean(bpms_refs):.1f}")

    print("\n=== COMPARACAO NUMERICA (alvo vs media das referencias) ===")
    for campo in CAMPOS_NUMERICOS:
        alvo_val = target_data.get(campo)
        ref_vals = [d.get(campo) for d in ref_data if d.get(campo) is not None]
        if alvo_val is None or not ref_vals:
            continue
        media_ref = statistics.mean(ref_vals)
        delta = alvo_val - media_ref
        sinal = "+" if delta >= 0 else ""
        print(f"  {campo}: alvo={alvo_val:.2f}  refs_media={media_ref:.2f}  delta={sinal}{delta:.2f}")

    print("\n=== MOOD/DANCABILIDADE (alvo vs media das referencias) ===")
    mood_campos = ["happy", "aggressive", "relaxed", "sad", "party", "electronic", "acoustic", "danceable"]
    for campo in mood_campos:
        alvo_val = target_data.get(campo)
        ref_vals = [d.get(campo) for d in ref_data if d.get(campo) is not None]
        if alvo_val is None or not ref_vals:
            continue
        media_ref = statistics.mean(ref_vals)
        print(f"  {campo}: alvo={alvo_val:.2f}  refs_media={media_ref:.2f}")

    print("\n(Lembrete: mood/dancabilidade sao classificadores binarios com confiabilidade "
          "limitada -- usar como sinal a mais, nao como veredito.)")


if __name__ == "__main__":
    main()
