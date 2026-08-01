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

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from instrument_rules import AUDIO_EXTS, TONAL_CATEGORIAS

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
    """Roda Essentia no arquivo. Retorna dict com key, scale, key_strength, bpm, bpm_conf, loudness."""
    import essentia.standard as es

    audio = es.MonoLoader(filename=filepath, sampleRate=44100)()
    if len(audio) < 1024:
        return None

    key, scale, strength = es.KeyExtractor()(audio)
    result = {
        "key": key,
        "scale": scale,
        "key_strength": round(float(strength), 3),
    }
    try:
        bpm, _, beats_conf, _, _ = es.RhythmExtractor2013()(audio)
        result["bpm"] = round(float(bpm), 1)
        result["bpm_confidence"] = round(float(beats_conf), 3)
    except Exception:
        result["bpm"] = None
        result["bpm_confidence"] = None
    try:
        result["loudness"] = round(float(es.Loudness()(audio)), 2)
    except Exception:
        result["loudness"] = None
    return result


def key_para_notacao_curta(key, scale):
    letra = key.replace("b", "b").strip()
    if scale == "minor":
        return f"{letra}m"
    return letra


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

    p_rename = sub.add_parser("rename", help="Renomeia arquivos adicionando a tonalidade no final")
    p_rename.add_argument("--path", required=True)
    p_rename.add_argument("--categorias", default=None, help="Lista separada por virgula (default: categorias tonais)")
    p_rename.set_defaults(func=cmd_rename)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
