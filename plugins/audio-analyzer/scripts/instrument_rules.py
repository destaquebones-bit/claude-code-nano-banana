import os
import re

AUDIO_EXTS = (".wav", ".aif", ".aiff", ".mp3", ".flac", ".m4a", ".ogg")

EXACT_RULES = [
    ("Bumbo (Kick)", ["kick", "kicks"]),
    ("Caixa e Clap", ["snare", "snares", "clap", "claps", "rimshot", "rim"]),
    ("Pratos e Hi-Hat", ["hat", "hats", "hihat", "hihats", "cymbal", "cymbals", "cym", "crash", "ride",
                          "tom", "toms", "splash"]),
    ("Percussao", ["conga", "congas", "bongo", "bongos", "djembe", "percussion", "perc", "shaker",
                    "tambourine", "clave", "cowbell"]),
    ("Baixo", ["bass", "bassline", "sub"]),
    ("Vocal", ["vocal", "vocals", "vox", "voice", "spoken", "elevenlabs"]),
    ("Cordas", ["string", "strings", "violin", "cello"]),
    ("Metais e Sopro", ["brass", "horn", "sax", "trumpet"]),
    ("Piano e Teclas", ["piano", "keys", "rhodes", "organ"]),
    ("Guitarra", ["guitar", "gtr"]),
    ("Sintetizador", ["synth", "syn", "pad", "lead", "sinewave", "diva", "chord", "stab", "arp", "arpeggio"]),
    ("FX e Efeitos", ["fx", "riser", "sweep", "impact", "whoosh", "ambience", "ambient", "drone",
                       "transition", "noise", "atmos", "atmosphere", "texture", "glitch", "siren",
                       "upfilter", "filter", "downlift", "downlifter", "downlifters", "reverse", "rev",
                       "echo", "reverb"]),
    ("Loops e Fills de Bateria", ["loop", "loops", "groove", "beat", "fill", "fills", "drum", "drums",
                                    "top", "tops"]),
]

SUBSTR_RULES = [
    ("Pratos e Hi-Hat", ["hat", "splash"]),
    ("FX e Efeitos", ["fx", "atmos", "reverb", "echo"]),
]

TONAL_CATEGORIAS = ["Baixo", "Sintetizador", "Cordas", "Piano e Teclas", "Guitarra", "Vocal"]

_EXACT_MAP = {}
for _categoria, _tokens in EXACT_RULES:
    for _t in _tokens:
        _EXACT_MAP.setdefault(_t, _categoria)


def categorize_by_name(filename):
    """Categoriza um sample por instrumento usando SOMENTE o nome do arquivo.

    Usar so o nome (nao o caminho/pastas do pack) evita que nomes de pack/genero
    (ex: pastas "... Bass House", "Drum_Loops") contaminem a categoria de
    arquivos que nao tem nada a ver com aquele instrumento.
    """
    name_no_ext = os.path.splitext(filename)[0]
    tokens = re.split(r"[^a-zA-Z0-9]+", name_no_ext.lower())
    for tok in tokens:
        if tok in _EXACT_MAP:
            return _EXACT_MAP[tok]
    lower = name_no_ext.lower()
    for categoria, subs in SUBSTR_RULES:
        for s in subs:
            if s in lower:
                return categoria
    return "Outros"
