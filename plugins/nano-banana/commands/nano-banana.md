---
description: Translate a natural-language image description into a structured JSON prompt for Nano Banana Pro
argument-hint: <description of the image you want>
---

Convert the user's request into a single structured JSON prompt optimized for Nano Banana Pro image generation.

User's request: $ARGUMENTS

Follow this schema, omitting fields that don't apply and never inventing details the user didn't imply:

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

Rules:
- Ask a clarifying question only if the request is too vague to produce a meaningful subject/setting (e.g. a single ambiguous word).
- Otherwise, make reasonable inferences for unstated fields (style, lighting, composition) that best match the request's intent, and note in a one-line caveat below the JSON which fields were inferred vs. explicitly requested.
- Output valid JSON only in the code block — no comments inside the JSON itself.
- Keep string values concise phrases, not full sentences.
