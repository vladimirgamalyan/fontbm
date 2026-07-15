---
name: fontbm
description: This skill should be used when the user asks to generate bitmap fonts, use fontbm, create a font atlas, export a .fnt file, configure font texture, asks about fontbm options or parameters, or wants to run the fontbm command-line utility.
version: 2.0.0
allowed-tools: [Bash]
---

# fontbm — Bitmap Font Generator

`fontbm` is a command-line utility that converts TTF/OTF fonts into bitmap font atlases compatible with the AngelCode BMFont format.

```
fontbm --font-file FreeSans.ttf --output myfont
```

Produces `myfont.fnt` (metrics) and `myfont_0.png` (texture atlas). The metrics file always has the
`.fnt` extension, even with `--data-format json` or `xml`, so do not expect `myfont.json`.

## Full reference

**Run `fontbm --help`.** It documents every option with its default, the produced files, the
character range syntax and ready to run examples. Read it instead of guessing option names or values.

## Running fontbm

The binary should be in the project directory or on PATH:

```
./fontbm --font-file FreeSans.ttf --output myfont
```

On Windows use `fontbm.exe`. Exits with 0 on success, 1 on error; errors and warnings go to stderr.
