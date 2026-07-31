---
name: json-prompt-translator
description: Convert a natural-language image request into a structured JSON prompt optimized for Nano Banana Pro image generation. Use when the user wants a "JSON prompt", asks to structure/format an image description for Nano Banana Pro, or explicitly invokes /nano-banana.
---

# JSON Prompt Translator for Nano Banana Pro

Convert the user's natural-language image description into a single structured JSON object. Do not generate an actual image with this skill — this skill only produces the JSON prompt spec. If the user wants the image generated, hand off to the image-generation skill after producing the JSON.

## Schema

```json
{
  "subject": "primary subject(s) of the image",
  "action": "what the subject is doing, if anything",
  "setting": "location/environment/background",
  "style": "art style, medium, or rendering approach (e.g. photorealistic, watercolor, 3D render, anime)",
  "composition": {
    "shot_type": "e.g. close-up, wide shot, portrait, macro",
    "camera_angle": "e.g. eye-level, low-angle, bird's-eye",
    "aspect_ratio": "e.g. 1:1, 16:9, 9:16, 4:5"
  },
  "lighting": "lighting condition and mood (e.g. golden hour, studio softbox, dramatic rim light)",
  "color_palette": ["dominant colors or palette description"],
  "mood": "overall emotional tone",
  "details": ["specific notable details, textures, props, or accessories"],
  "quality_modifiers": ["e.g. highly detailed, sharp focus, 8k, professional"],
  "negative_prompt": "elements to avoid, if any"
}
```

## Rules

- Omit fields that don't apply. Never invent details the user didn't imply beyond reasonable, clearly-flagged inference.
- Ask a clarifying question only if the request is too vague to identify a subject or setting at all.
- For unstated but useful fields (style, lighting, composition), make a reasonable inference that fits the request's intent, and add a one-line caveat after the JSON noting which fields were inferred vs. explicit.
- Output valid JSON only inside the code block — no comments inside the JSON itself.
- Keep string values concise phrases, not full sentences.
