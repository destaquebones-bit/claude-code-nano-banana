# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

This is a **Claude Code plugin marketplace**, not an application. It's the source repo for
`claude-code-nano-banana`, published as a marketplace via `.claude-plugin/marketplace.json`.
Users add it in Claude Code with `/plugin marketplace add jawhnycooke/claude-code-nano-banana`
(or a local path) and then install individual plugins from it. There is no build step, no
package manager, and no test suite for the repo as a whole — the deliverable is markdown
(commands/skills) plus, for one plugin, a set of standalone Python CLI scripts.

## Repository structure

```
.claude-plugin/marketplace.json   # marketplace manifest — lists every plugin and where it lives
plugins/
  nano-banana/                    # slash command + skill: NL image description -> structured JSON prompt
    .claude-plugin/plugin.json
    commands/nano-banana.md       # /nano-banana slash command
    skills/json-prompt-translator/SKILL.md
  audio-analyzer/                 # Python CLI toolkit + skill for sample/track audio analysis
    .claude-plugin/plugin.json
    skills/audio-analyzer/SKILL.md
    scripts/*.py, setup.sh
    models/*.pb                   # committed pretrained Essentia mood/danceability classifiers
    reference/*.md                # dated genre-benchmark snapshots (Beatport charts)
    requirements.txt
```

Each plugin is self-contained under `plugins/<name>/` with its own `.claude-plugin/plugin.json`
(name/description/version/author). Adding a new plugin means: create that directory shape, then
add an entry to `.claude-plugin/marketplace.json`'s `plugins` array with `name`, `description`,
`author`, and a `source` pointing at the plugin's directory (relative to repo root).

## Key convention: SKILL.md and command files must stay in sync

`plugins/nano-banana/commands/nano-banana.md` (the `/nano-banana` slash command) and
`plugins/nano-banana/skills/json-prompt-translator/SKILL.md` (the skill that triggers on the
same intent even without the explicit command) define **the same JSON schema and the same
rules** for translating a natural-language image request into a structured prompt. They are
intentionally duplicated content, not a shared include (Claude Code plugins have no include
mechanism) — when editing the schema or rules in one, mirror the change in the other.

## Plugin: nano-banana

Converts a natural-language image description into a structured JSON prompt (subject, action,
setting, style, composition, lighting, color_palette, mood, details, quality_modifiers,
negative_prompt) for Nano Banana Pro image generation. It only produces the JSON spec — it does
not call an image-generation model itself. Rules baked into both the command and the skill:
omit inapplicable fields, never invent unstated details, only ask a clarifying question if the
request is too vague to identify a subject, and flag which fields were inferred vs. explicit in
a one-line caveat after the JSON.

## Plugin: audio-analyzer

A set of independent Python CLI scripts under `plugins/audio-analyzer/scripts/`, invoked by the
`audio-analyzer` skill, for sample-library organization and track analysis/critique. All of them
share one isolated virtualenv.

### Environment setup

```bash
bash plugins/audio-analyzer/scripts/setup.sh          # creates plugins/audio-analyzer/.venv, idempotent
source plugins/audio-analyzer/.venv/bin/activate       # required before running any script below
```

The venv is isolated on purpose: `essentia-tensorflow` requires `numpy<2`, which must never be
forced onto the user's system Python. `essentia-tensorflow` (not plain `essentia`) is required
specifically because the `mood` command needs its TensorFlow-enabled pretrained classifiers.
`models/*.pb` (~5MB, used by `analyze.py mood`) is committed to the repo, not downloaded by
setup — everything else in `requirements.txt` (torch, torchaudio, demucs, matplotlib,
google-genai, librosa, mutagen) is a heavy install pulled fresh per machine.

### Scripts (run with `python3 scripts/<name>.py ...` after activating the venv)

- **`categorize.py`** — sorts audio samples into instrument-named folders (`--source` repeatable,
  `--mode copy|symlink`). Categorization uses **only the filename**, never the parent folder
  path — pack/genre folder names would otherwise pollute results. Category keyword lists live in
  `instrument_rules.py` (`EXACT_RULES` / `SUBSTR_RULES` / `TONAL_CATEGORIAS`) — extend those, don't
  hardcode category logic elsewhere.
- **`analyze.py report`** — read-only, prints key/BPM/loudness JSON via Essentia.
- **`analyze.py rename`** — renames files in place, appending a normalized `- <Key>` suffix.
  Prefers an existing key token already in the filename over Essentia's `KeyExtractor` estimate
  (marked `(est)`). Only meaningful on tonal categories (`TONAL_CATEGORIAS`); defaults to those.
- **`analyze.py deep`** — adds texture/energy descriptors (spectral centroid, onset rate,
  danceability, dynamic complexity, stereo correlation) — comparative numbers, not pass/fail
  thresholds.
- **`analyze.py mood`** — runs the committed MusiCNN classifiers in `models/` (happy, aggressive,
  relaxed, sad, party, electronic, acoustic, danceable). These are independent, non-exclusive
  binary classifiers trained on a general collection, not genre-specific — always caveat their
  limitations to the user rather than presenting scores as settled fact.
- **`separate.py`** — splits a full song into instrument stems via Demucs (2/4/6-stem). Defaults
  to max-quality mode (`--shifts 5 --overlap 0.75`, `htdemucs_ft`) — a deliberate, ~5x-slower
  default; only drop to `--fast` when the user explicitly wants a quick pass. 6 stems is Demucs's
  hard ceiling (no dedicated synth/strings/brass/FX stem — always lands in "other"). Output
  defaults to MP3, not WAV, to avoid multi-hundred-MB exports.
- **`spectrogram.py`** — generates a waveform + spectrogram PNG. There is no audio-listening tool
  in this environment, so this is the way to visually inspect a track — read the resulting PNG
  with the Read tool.
- **`compare.py`** — runs `report` + `deep` + `mood` on a target track against a set of reference
  tracks and prints the target against the reference averages.
- **`critique_gemini.py`** — sends a trimmed, downsampled audio clip to a Gemini model for a real
  audio-informed production critique (needs `GEMINI_API_KEY`). Uses direct REST calls, not the
  `google-genai` SDK, by design.

### Operational conventions specific to this plugin

- Check `df -h ~` before any large `--mode copy` run or before running `separate.py` on multiple
  tracks/a folder — these are the operations most likely to fill disk. Prefer `--mode symlink` or
  `--stems 2` when space is tight.
- `reference/<genre-slug>-<year>.md` files are dated Beatport Top-100 snapshots used as the
  benchmark baseline when giving production feedback — read the matching genre file first rather
  than re-deriving genre conventions; re-scrape (read-only page browsing, no audio downloads) if a
  snapshot is more than ~2-3 months old.
- Every script here has known, stated limitations (documented in
  `plugins/audio-analyzer/skills/audio-analyzer/SKILL.md`) — e.g. `bpm_confidence` is often 0.0 on
  short one-shots, `critique_gemini.py` currently blocked by a key-format issue as of 2026-08-01.
  Surface these caveats to the user rather than presenting derived numbers as certainties.
