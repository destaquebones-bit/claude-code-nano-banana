#!/bin/bash
# Cria (ou reaproveita) um venv isolado com as dependencias de analise de audio.
# Isolado de proposito: essentia so funciona com numpy<2, e isso nao deve
# afetar o Python "do sistema" do usuario.
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV="$DIR/.venv"

if [ ! -d "$VENV" ]; then
  python3 -m venv "$VENV"
fi

source "$VENV/bin/activate"
pip install -q --upgrade pip
pip install -q -r "$DIR/requirements.txt"
echo "Ambiente pronto em $VENV"
