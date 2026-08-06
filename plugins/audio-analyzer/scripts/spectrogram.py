#!/usr/bin/env python3
"""Gera uma imagem (forma de onda + espectrograma) de um arquivo de audio.

Nao existe forma de "ouvir" audio nesta sessao -- mas Claude le imagens. Gerar essa
imagem e depois ler o PNG resultante (via a ferramenta Read) e o jeito mais direto de
inspecionar visualmente estrutura de arranjo, balanco de frequencia e presenca de
transientes sem depender so dos numeros do analyze.py.

Uso:
  python3 spectrogram.py --input musica.wav --output grafico.png
"""
import argparse
import warnings

warnings.filterwarnings("ignore")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--input", required=True)
    ap.add_argument("--output", required=True)
    args = ap.parse_args()

    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np
    import librosa
    import librosa.display

    y, sr = librosa.load(args.input, sr=44100, mono=True)
    duration = len(y) / sr

    fig, axes = plt.subplots(2, 1, figsize=(16, 9), gridspec_kw={"height_ratios": [1, 3]})

    times = np.linspace(0, duration, len(y))
    axes[0].plot(times, y, linewidth=0.3, color="steelblue")
    axes[0].set_xlim(0, duration)
    axes[0].set_title("Forma de onda")
    axes[0].set_ylabel("Amplitude")
    axes[0].set_xlabel("Tempo (s)")

    D = librosa.amplitude_to_db(np.abs(librosa.stft(y, n_fft=4096, hop_length=512)), ref=np.max)
    img = librosa.display.specshow(D, sr=sr, hop_length=512, x_axis="time", y_axis="log", ax=axes[1], cmap="magma")
    axes[1].set_title("Espectrograma (log freq)")
    fig.colorbar(img, ax=axes[1], format="%+2.0f dB")

    fig.tight_layout()
    fig.savefig(args.output, dpi=130)
    print(f"Salvo: {args.output} (duracao: {duration:.1f}s)")


if __name__ == "__main__":
    main()
