#!/usr/bin/env python3
"""Envia um trecho de audio pro Gemini e pede uma critica textual real (sound design,
mix, energia, estrutura). E o mais perto que existe hoje de "ouvir com criterio" --
um modelo multimodal que processa o audio de verdade, nao so metadados/espectro.

Requer GEMINI_API_KEY no ambiente, com billing ativo e uma chave no formato padrao
do AI Studio (normalmente comeca com "AIza..."). Chaves com formato/escopo diferente
podem falhar com 401 ACCESS_TOKEN_TYPE_UNSUPPORTED mesmo sendo validas pra outros
endpoints (ex: listagem de modelos) -- se isso acontecer, gerar uma chave nova em
https://aistudio.google.com/apikey.

Usa a API REST diretamente (nao o SDK google-genai) -- foi o jeito que funcionou de
forma mais previsivel nos testes desta sessao.

Uso:
  GEMINI_API_KEY="..." python3 critique_gemini.py --input musica.wav [--duracao 60] \
      [--pergunta "texto customizado"]
"""
import argparse
import base64
import json
import os
import subprocess
import sys
import tempfile


DEFAULT_PERGUNTA = (
    "Voce e um engenheiro de mix/master e produtor experiente de musica eletronica "
    "(tech house / house). Escute este trecho de audio com atencao critica e descreva, "
    "em portugues, de forma direta e especifica (nao generica):\n"
    "1. Sound design: como estao o kick, bassline, percussao e qualquer elemento "
    "melodico/vocal -- textura, presenca, se algo soa cru/fraco/generico.\n"
    "2. Mix: balanco de frequencia (algo mascarando outra coisa? falta de sub? agudo "
    "estridente?), uso de espaco estereo, groove/timing.\n"
    "3. Energia e estrutura: a faixa segura o interesse? falta ou sobra elemento?\n"
    "4. Se tivesse que dar UM conselho pra melhorar essa faixa agora, qual seria?\n"
    "Seja honesto e especifico -- nao elogie por educacao."
)


def extrair_trecho(input_path, duracao):
    """Corta os primeiros N segundos e reexporta em mp3 de baixa taxa pra reduzir
    tamanho do payload (a API tem limite de tamanho de request)."""
    tmp_out = tempfile.NamedTemporaryFile(suffix=".mp3", delete=False).name
    cmd = [
        "ffmpeg", "-y", "-i", input_path, "-t", str(duracao),
        "-ac", "1", "-b:a", "64k", tmp_out,
    ]
    subprocess.run(cmd, check=True, capture_output=True)
    return tmp_out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True)
    ap.add_argument("--duracao", type=int, default=60, help="segundos a enviar (default 60, pra manter payload pequeno)")
    ap.add_argument("--pergunta", default=DEFAULT_PERGUNTA)
    ap.add_argument("--model", default="gemini-3.1-flash-lite")
    args = ap.parse_args()

    api_key = os.environ.get("GEMINI_API_KEY")
    if not api_key:
        print("ERRO: GEMINI_API_KEY nao esta no ambiente.", file=sys.stderr)
        sys.exit(1)

    import requests

    trecho_path = extrair_trecho(args.input, args.duracao)
    try:
        with open(trecho_path, "rb") as f:
            audio_b64 = base64.b64encode(f.read()).decode("ascii")

        url = f"https://generativelanguage.googleapis.com/v1beta/models/{args.model}:generateContent?key={api_key}"
        body = {
            "contents": [{
                "parts": [
                    {"text": args.pergunta},
                    {"inline_data": {"mime_type": "audio/mp3", "data": audio_b64}},
                ]
            }]
        }
        r = requests.post(url, json=body, timeout=120)
        if r.status_code != 200:
            print(f"ERRO {r.status_code}: {r.text}", file=sys.stderr)
            if r.status_code == 401:
                print(
                    "\nDica: 401 nesse endpoint costuma significar que a chave nao e do "
                    "formato padrao do AI Studio. Gerar uma nova em "
                    "https://aistudio.google.com/apikey e tentar de novo.",
                    file=sys.stderr,
                )
            sys.exit(1)

        data = r.json()
        texto = data["candidates"][0]["content"]["parts"][0]["text"]
        print(texto)
    finally:
        os.remove(trecho_path)


if __name__ == "__main__":
    main()
