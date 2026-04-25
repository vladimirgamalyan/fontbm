---
name: fontbm
description: This skill should be used when the user asks to generate bitmap fonts, use fontbm, create a font atlas, export a .fnt file, configure font texture, asks about fontbm options or parameters, or wants to run the fontbm command-line utility.
version: 1.0.0
allowed-tools: [Bash]
---

# fontbm — Bitmap Font Generator

`fontbm` is a command-line utility that converts TTF/OTF fonts into bitmap font atlases compatible with the AngelCode BMFont format.

## Basic Usage

```
fontbm --font-file FreeSans.ttf --output myfont
```

Produces two files:
- `myfont.fnt` — metadata (character positions, kerning, etc.)
- `myfont_0.png` — texture atlas image

## Required Options

| Option | Description |
|--------|-------------|
| `--font-file <path>` | Path to the TTF/OTF font file |
| `--output <name>` | Output filename without extension |

## Optional Parameters

| Option | Default | Description |
|--------|---------|-------------|
| `--font-size <n>` | 32 | Font size in pixels |
| `--chars <range>` | 32-126 | Character range(s), e.g. `32-126` or `32-64,92,120-126` |
| `--chars-file <path>` | — | UTF-8 text file with additional characters to include |
| `--color <r,g,b>` | 255,255,255 | Foreground RGB color |
| `--background-color <r,g,b>` | transparent | Background RGB color |
| `--monochrome` | off | Disable anti-aliasing (1-bit rendering) |
| `--data-format <fmt>` | txt | Output format: `txt`, `xml`, `bin`, `json` |
| `--kerning-pairs <mode>` | disabled | Kerning: `disabled`, `basic`, `regular`, `extended` |
| `--texture-size <w,h>` | auto | Allowed texture dimensions (comma-separated list of sizes) |
| `--texture-crop-width` | off | Crop unused horizontal space from the texture |
| `--texture-crop-height` | off | Crop unused vertical space from the texture |
| `--max-texture-count <n>` | unlimited | Maximum number of texture pages |
| `--texture-name-suffix <s>` | index_aligned | Suffix style: `index_aligned`, `index`, `none` |
| `--padding-up <n>` | 0 | Top padding per glyph (pixels) |
| `--padding-right <n>` | 0 | Right padding per glyph (pixels) |
| `--padding-down <n>` | 0 | Bottom padding per glyph (pixels) |
| `--padding-left <n>` | 0 | Left padding per glyph (pixels) |
| `--spacing-vert <n>` | 0 | Vertical spacing between glyphs |
| `--spacing-horiz <n>` | 0 | Horizontal spacing between glyphs |

## Examples

Generate a 48px font with Latin characters:
```
fontbm --font-file Arial.ttf --output arial48 --font-size 48
```

Generate with a custom character range and JSON output:
```
fontbm --font-file FreeSans.ttf --output ui_font --font-size 24 --chars 32-126 --data-format json
```

Generate a monochrome font with padding for SDF-like workflows:
```
fontbm --font-file FreeSans.ttf --output mono_font --monochrome --padding-up 4 --padding-right 4 --padding-down 4 --padding-left 4
```

Generate a colored font on a dark background:
```
fontbm --font-file FreeSans.ttf --output colored --color 255,200,0 --background-color 0,0,0
```

Include characters from a file (useful for Unicode/CJK):
```
fontbm --font-file NotoSans.ttf --output cjk_font --chars-file my_chars.txt
```

## Output Format Notes

- **txt** (default): AngelCode BMFont text format — widely supported
- **xml**: AngelCode BMFont XML format
- **bin**: AngelCode BMFont binary format
- **json**: JSON format

## Running fontbm

The binary should be in the project directory or on PATH:
```
./fontbm --font-file FreeSans.ttf --output myfont
```

On Windows:
```
fontbm.exe --font-file FreeSans.ttf --output myfont
```
