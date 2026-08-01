---
name: audio-analyzer
description: Organize audio samples into instrument subfolders, detect musical key/BPM/loudness via Essentia, and separate full songs into instrument stems (vocals/drums/bass/other) via Demucs — a local, Moises-like stem splitter. Use when the user wants to sort a sample library by instrument, tag samples with their musical key, detect BPM, or split a mixed song into isolated vocal/drum/bass/instrumental tracks.
---

# Audio Analyzer

Three independent tools, all CLI scripts under `scripts/`. They depend on an isolated venv (Essentia only works with `numpy<2`, which must not be forced onto the user's system Python; Demucs/PyTorch live in the same venv for convenience).

## Setup (once per machine)

```bash
bash "<plugin_dir>/scripts/setup.sh"
```

Creates `<plugin_dir>/.venv` with `numpy<2`, `essentia`, `librosa`, `mutagen`, `torch`, `torchaudio`, `demucs`. Idempotent — safe to re-run. This is a heavy install (PyTorch alone is several hundred MB, plus the Demucs model downloads ~80MB on first use) — check `df -h ~` before running setup if disk space looks tight.

Every script call below must activate that venv first:

```bash
source "<plugin_dir>/.venv/bin/activate"
python3 "<plugin_dir>/scripts/categorize.py" ...
```

## categorize.py — sort samples into instrument folders

```bash
python3 scripts/categorize.py --source ~/Splice/sounds/packs --source ~/Music/Ableton \
    --dest ~/Desktop/Samples --mode copy
```

- `--source` repeatable, any folder tree.
- `--mode copy` duplicates data (check free disk space first: `df -h ~`). `--mode symlink` creates symlinks instead — near-zero disk cost, use it when source data is large or disk space is tight.
- Categorization uses **only the filename**, never the parent folder path. This was a deliberate fix: pack/genre folder names (e.g. a pack literally named "... Bass House", or a subfolder "Drum_Loops") pollute path-based categorization — a ride cymbal sitting inside a "Bass House"-named pack would wrongly get filed under Baixo if folder names were considered. Keep it this way.
- Categories are defined in `instrument_rules.py` (`EXACT_RULES` / `SUBSTR_RULES`) — extend the keyword lists there if a new category or term comes up, don't hardcode elsewhere.
- Files that match nothing land in "Outros" — expect roughly 1-5% with decently-named sample packs. Anything higher signals a source with unusual naming (worth sampling `Outros` and extending the keyword lists rather than accepting it).

## analyze.py — musical key / BPM / loudness via Essentia

```bash
# read-only, prints JSON, does not touch files
python3 scripts/analyze.py report --path arquivo.wav
python3 scripts/analyze.py report --path pasta/

# renames files in place, appending " - <Key>" at the end of the filename
python3 scripts/analyze.py rename --path ~/Desktop/Samples \
    [--categorias "Baixo,Sintetizador,Cordas,Piano e Teclas,Guitarra,Vocal"]
```

`rename` logic, in order:
1. If the filename already has a key token (e.g. `..._Gm.wav`, `..._F#min.wav`), extract it, normalize (`min`→`m`, `maj`→dropped), strip it from the middle, and append it at the end: `nome - Gm.wav`. This path is reliable — trust it.
2. If no key token is found, run Essentia's `KeyExtractor` on the audio and append the estimate marked `(est)`: `nome - G# (est).wav`. This is a best-effort estimate — tell the user explicitly it can be wrong, especially on short one-shots or loops with drums mixed into the tonal content. Check `key_strength` from `report` if you need a confidence signal (below ~0.5 is weak).

**Only run key detection on tonal categories** — Baixo, Sintetizador, Cordas, Piano e Teclas, Guitarra, Vocal (the `TONAL_CATEGORIAS` list in `instrument_rules.py`). Kick/Snare/Hat/Percussion/FX are overwhelmingly atonal; running key detection on them produces meaningless labels. `rename` defaults to this list already — only override `--categorias` if the user explicitly wants something else, and warn them if they ask to include percussive categories.

BPM detection (`RhythmExtractor2013`) works on any rhythmic loop but `bpm_confidence` is frequently 0.0 on short one-shots/loops in practice — don't present a BPM as trustworthy without checking that confidence value first.

## separate.py — split a full song into instrument stems (Demucs)

```bash
python3 scripts/separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 4
python3 scripts/separate.py --input musica.mp3 --output ~/Desktop/Stems --stems 2   # vocals vs instrumental only
python3 scripts/separate.py --input pasta_de_musicas/ --output ~/Desktop/Stems --stems 6  # + guitar/piano
```

This is the "like Moises" capability — locally-run source separation using Meta's Demucs. Runs on CPU (works fine on Apple Silicon; no CUDA needed).

- `--stems 2`: vocals / instrumental. `--stems 4` (default): vocals / drums / bass / other. `--stems 6`: adds guitar / piano on top of the 4.
- **Runs in max-quality mode by default** (user-requested standard, not a suggestion — keep this default): `--shifts 5 --overlap 0.75`, and model `htdemucs_ft` (the fine-tuned bag-of-4-models, official best-quality option for 4/2 stems) instead of plain `htdemucs`. This costs roughly **5x** the processing time of the fast path. Pass `--fast` only when the user explicitly wants a quick/draft pass instead of quality — don't default to it, and don't silently drop to it to save time.
- For `--stems 6` there is no fine-tuned variant (`htdemucs_6s` only) — quality gain there comes only from the shifts/overlap boost, not a better base model.
- **Hard ceiling, be upfront about it**: 6 stems (vocals/drums/bass/guitar/piano/other) is the max granularity Demucs — or any comparable public tool, including what Moises uses — separates today. Synths, strings, brass, electric keys, FX/atmosphere have no dedicated stem and always land in "other". If the user asks for "nothing left in other" or more than 6 instrument tracks, say plainly that it's not achievable with current tooling rather than attempting it silently.
- Output defaults to **MP3**, not WAV — a 4-stem WAV export can be 250-300MB for a single ~4 minute song (each stem is roughly the size of the original, uncompressed). Pass `--wav` only when the user explicitly needs lossless stems (e.g. for remixing/mastering) and confirm they have the disk space first.
- Processing speed at max quality: expect roughly 5x the fast-mode time (fast mode was ~40-60% of real-time per track on an M2 CPU, e.g. a 6-7 min track took ~4 min fast / expect ~15-20 min at max quality). Always run in the background and report progress rather than blocking, especially at max quality.
- First run downloads the model checkpoint (~80-100MB depending on model) to `~/.cache/torch/hub/checkpoints/` — one-time cost per model, cached afterward. `htdemucs_ft` downloads 4 checkpoints (bag of models), more than the ~80MB single-model download.
- **Always check `df -h ~` before separating multiple tracks or a whole folder** — this is the operation most likely to fill the disk in this plugin. Prefer `--stems 2` (fewer, smaller output files) over `--stems 4`/`6` when the user only needs vocal isolation, not a full multitrack breakdown.

## Before running at scale

- Large `--mode copy` runs can fill the disk fast (sample libraries are often multiple GB). Check `df -h ~` first and prefer `--mode symlink` when space is tight.
- `analyze.py report --path <folder>` on thousands of files takes a while (Essentia itself is fast, ~0.01-0.1s/file after the venv is warm, but I/O adds up) — for big batches, consider running in the background and reporting a summary rather than the full per-file JSON.
