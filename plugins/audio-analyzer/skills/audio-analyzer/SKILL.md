---
name: audio-analyzer
description: Organize audio samples into instrument subfolders, detect musical key/BPM/loudness/mood via Essentia (incl. pretrained mood/danceability classifiers), generate spectrogram images to visually inspect a track, separate full songs into instrument stems via Demucs (Moises-like), compare a draft track against reference tracks, and (once a working Gemini key is available) get a real audio-critique from a multimodal model. Use when the user wants to sort a sample library by instrument, tag samples with their key/BPM, split a mixed song into stems, get production feedback on their own track, or otherwise get as close as possible to "critical listening" without native audio perception.
---

# Audio Analyzer

CLI scripts under `scripts/`, all depending on one isolated venv (Essentia-TensorFlow only
works with `numpy<2`, which must not be forced onto the user's system Python; Demucs/PyTorch/
google-genai/matplotlib all live in the same venv for convenience).

**Context on why this plugin looks the way it does**: it started as sample organization +
key/BPM tagging, then grew stem separation, then genre-benchmark reference docs, then (this
round) an honest attempt at real "critical listening" capability. The user explicitly asked
"go as far as you possibly can" on the listening front — the `mood`/`deep`/`compare`/
`critique_gemini` tools below are the result of that push, each with real, stated limitations.
Don't undersell them as "just numbers" nor oversell them as true perceptual judgment — be
specific about what each one actually measures.

## Setup (once per machine)

```bash
bash "<plugin_dir>/scripts/setup.sh"
```

Creates `<plugin_dir>/.venv` with `numpy<2`, `essentia-tensorflow` (not plain `essentia` —
the TensorFlow-enabled build is required for the `mood` command's pretrained classifiers),
`librosa`, `mutagen`, `torch`, `torchaudio`, `demucs`, `matplotlib`, `google-genai`. Idempotent
— safe to re-run. This is a heavy install (PyTorch alone is several hundred MB, plus the
Demucs model downloads ~80-300MB on first use) — check `df -h ~` before running setup if disk
space looks tight. The `models/` directory (small pretrained `.pb` files, ~5MB total, used by
`mood`) is committed to the repo, not downloaded by setup.sh.

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

## analyze.py deep — texture/energy descriptors

```bash
python3 scripts/analyze.py deep --path faixa.wav
```

Adds to the base `report` output: `spectral_centroid_hz`/`rolloff_85_hz` (brightness —
higher = more high-frequency presence), `onset_rate_per_sec` (transient density, i.e. how
busy the track is rhythmically), `danceability` (Essentia's rhythmic-regularity measure, 0
to ~3), `dynamic_complexity` (frame-to-frame loudness variation — different metric from
`loudness_range_lu`, more sensitive to short-term change), `stereo_correlation` (-1 to 1;
near 1 = nearly mono, near 0 or negative = very wide, possible mono-compatibility risk).
Useful as comparison numbers (this track vs a reference track), not as absolute
pass/fail thresholds — there's no universal "correct" spectral centroid.

## analyze.py mood — pretrained mood/danceability classifiers (Essentia-TensorFlow)

```bash
python3 scripts/analyze.py mood --path faixa.wav
```

Runs the small MusiCNN-based classifiers in `models/` (happy, aggressive, relaxed, sad,
party, electronic, acoustic, danceable — each a probability 0-1). This is the closest thing
to qualitative language this plugin can produce **without an external API** — genuinely
useful as one more signal, but **flag its real limitations to the user every time you use
it, don't present it as settled fact**:
- These are independent binary classifiers, not mutually exclusive — a track can (and often
  does) score high on both `relaxed` and `aggressive` simultaneously. That's not a bug to
  fix, it's how the models work; don't try to force a single coherent mood label out of them.
- Some heads are trained on very small datasets (mood_happy: ~300 tracks total). Expect noise,
  especially on genres underrepresented in that training data.
- They were trained on a general MTG in-house collection, not tech house/house specifically —
  don't be surprised if `sad`/`aggressive` come back extreme (near 0 or 1) on a track that a
  human would call neither. Confidence in the 0.3-0.7 range is more informative than a clean
  0.0/1.0 split.
- Models/heads live in `models/*.pb` (~5MB total, already downloaded and committed to the
  repo — see `analisar_mood()` in `analyze.py` for the exact graph/tensor names if adding a
  new classification head from https://essentia.upf.edu/models.html; stick to `-msd-musicnn-1`
  variants for consistency and to keep the embedding extraction shared across heads).

## spectrogram.py — see the audio (no listening tool exists, so look instead)

```bash
python3 scripts/spectrogram.py --input faixa.wav --output grafico.png
```

There is no audio-listening tool in this environment. This generates a waveform + log-frequency
spectrogram PNG — **read the resulting PNG with the Read tool** (Claude reads images) to
visually inspect arrangement structure (section boundaries, build-ups, silence gaps),
frequency balance (is there a hot/static low end? missing highs?), and transient density,
without relying only on the numeric descriptors above. This caught real issues before (e.g.
a "raw" draft that turned out to be a single 4-minute loop with a static, non-sidechained low
end, plus a separately-bounced isolated vocal at the tail — all visible at a glance, none of
it obvious from `report`/`deep` numbers alone). Always describe only what's visually present;
don't claim to have judged the sound quality from a picture.

## compare.py — draft vs reference tracks

```bash
python3 scripts/compare.py --target draft.wav --referencias ref1.wav ref2.wav ref3.wav
python3 scripts/compare.py --target draft.wav --referencias-dir ~/Referencias/
```

Runs `report` + `deep` + `mood` on the target and every reference file, then prints the
target's values against the reference set's averages for every numeric field, plus a
side-by-side for the mood scores. Use this whenever the user has both a draft and a set of
tracks they consider a quality/style benchmark (their own past hits, purchased references,
tracks by artists they're targeting for support) — it turns the raw numbers into "your track
runs brighter/darker/busier than your references" instead of isolated values with no context.

## critique_gemini.py — real audio-informed critique (working as of 2026-08-15)

```bash
GEMINI_API_KEY="..." python3 scripts/critique_gemini.py --input faixa.wav [--duracao 60]
```

Sends a trimmed audio clip (default first 60s, downsampled to mono 64kbps mp3 via `ffmpeg` to
keep the request small) to a Gemini model with a production-critique prompt, and prints back
its actual text response. This is the closest thing to genuine perceptual critique available —
a real multimodal model processing the waveform, not a metadata/spectral proxy.

**Status (2026-08-15): confirmed working**, model defaults to `gemini-3.1-flash-lite`. The
original blocker (a key returning `401 ACCESS_TOKEN_TYPE_UNSUPPORTED`) was resolved when the
user generated a new key — that same new key also works for `nano-banana` image generation.
Model names on this API drift fast: `gemini-2.5-flash` and `gemini-2.5-flash-lite` both 404
("no longer available to new users") even though they were valid weeks earlier — if the default
model 404s again, pass `--model <name>` with something from `curl .../v1beta/models?key=$KEY`
(prefer names without a version-locked date/number, e.g. `-latest` or `-lite` variants, they
get remapped forward instead of hard-deprecated). A `503 UNAVAILABLE` is transient server load,
not a code problem — just retry.

**Treat its output as opinion, not measurement** — it already contradicted the `tech-house-audio`
skill's actual measured data once (recommended "aggressive sidechain" on a track whose ducking
was independently measured at 1.4dB, well within the genre's normal 1.5-5.1dB range). When its
critique conflicts with a number the `tech-house-audio` skill already measured, say so explicitly
to the user rather than silently picking one — the measured number is the more trustworthy one,
but the disagreement itself is often the useful part to surface.

Uses direct REST calls (`requests`), not the SDK — that was the more predictable path in this session's
testing, keep it that way rather than reintroducing the SDK.

## reference/ — genre benchmark snapshots

`reference/tech-house-2026.md` and `reference/house-2026.md` are dated snapshots of Beatport's
Top 100 charts (BPM/key/label distribution) for those two genres, plus general production
conventions (kick/bass/percussion/mix/master targets). The user follows both genres closely and
wants production feedback benchmarked against current market data — when they ask for feedback
on a track, read the file matching that genre first and use it as the comparison baseline
(their track's BPM/key/loudness from `analyze.py report` against the doc's numbers) — don't
re-derive genre conventions from scratch each time. These are snapshots, not eternal truth: if
one is more than ~2-3 months old, consider re-scraping the current chart before relying on it
for anything that matters (method: read-only page browsing of `beatport.com/genre/<slug>/<id>/top-100`
via `get_page_text`, no audio downloads — Beatport's ToS doesn't allow scraping preview audio).
Same process extends to other genres if the user asks — follow the same method and file naming
(`reference/<genre-slug>-<year>.md`).

## Before running at scale

- Large `--mode copy` runs can fill the disk fast (sample libraries are often multiple GB). Check `df -h ~` first and prefer `--mode symlink` when space is tight.
- `analyze.py report --path <folder>` on thousands of files takes a while (Essentia itself is fast, ~0.01-0.1s/file after the venv is warm, but I/O adds up) — for big batches, consider running in the background and reporting a summary rather than the full per-file JSON.
