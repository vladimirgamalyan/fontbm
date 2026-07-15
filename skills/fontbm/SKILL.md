---
name: fontbm
description: This skill should be used when the user asks to generate bitmap fonts, use fontbm, create a font atlas, export a .fnt file, configure font texture, asks about fontbm options or parameters, or wants to run the fontbm command-line utility.
version: 1.1.0
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

The metadata file always has the `.fnt` extension, even with `--data-format json` or `xml`.
There is no option to change it, so do not expect `myfont.json`.

Run `fontbm --help` for the full reference; it documents every option, the defaults and the produced files.

## Required Options

| Option | Description |
|--------|-------------|
| `--font-file <path>` | Path to the TTF/OTF font file |
| `--output <name>` | Output filename without extension |

## Optional Parameters

| Option | Default | Description |
|--------|---------|-------------|
| `--font-size <n>` | 32 | Character height in pixels |
| `--chars <ranges>` | 32-126 | Characters as **decimal Unicode code points**: single values and/or first-last ranges, comma separated, e.g. `32-126` or `32-64,92,120-126`. Not literal characters: use `1040-1103`, not `А-я`. The default applies only when `--chars-file` is not given |
| `--chars-file <path>` | — | UTF-8 text file, every character in it is added to `--chars`; may be given several times |
| `--color <r,g,b>` | 255,255,255 | Foreground RGB color, decimal 0-255 each |
| `--background-color <r,g,b>` | transparent | Background RGB color, decimal 0-255 each |
| `--monochrome` | off | Disable anti-aliasing |
| `--data-format <fmt>` | txt | Format of the `.fnt` file: `txt`, `xml`, `bin`, `json` |
| `--kerning-pairs <mode>` | disabled | Kerning: `disabled`, `basic`, `regular` (tuned by hinter), `extended` (more precise, bigger output) |
| `--texture-size <sizes>` | see below | Allowed page sizes as `<width>x<height>`, comma separated, e.g. `256x256,512x512`. Tried left to right, the first one all glyphs fit into is used |
| `--texture-crop-width` | off | Shrink every page to the rightmost used pixel |
| `--texture-crop-height` | off | Shrink every page to the lowest used pixel |
| `--max-texture-count <n>` | unlimited | Fail if more pages than this are needed |
| `--texture-name-suffix <s>` | index_aligned | How the page number is added to the `.png` name: `index_aligned` (`myfont_00.png`, zero padded to the highest page number), `index` (`myfont_0.png`), `none` (`myfont.png`, single page only) |
| `--padding-up <n>` | 0 | Pixels added above each glyph, inside its rectangle |
| `--padding-right <n>` | 0 | Pixels added right of each glyph, inside its rectangle |
| `--padding-down <n>` | 0 | Pixels added below each glyph, inside its rectangle |
| `--padding-left <n>` | 0 | Pixels added left of each glyph, inside its rectangle |
| `--spacing-vert <n>` | 0 | Pixels left between glyph rectangles, vertically |
| `--spacing-horiz <n>` | 0 | Pixels left between glyph rectangles, horizontally |
| `--align-horiz <n>` | 1 | Round glyph rectangle width up to a multiple of this, must be greater than 0 |
| `--align-vert <n>` | 1 | Round glyph rectangle height up to a multiple of this, must be greater than 0 |
| `--verbose` | off | Print the FreeType version being used |

Default `--texture-size` value:

```
32x32,64x32,64x64,128x64,128x128,256x128,256x256,512x256,512x512,1024x512,1024x1024,2048x1024,2048x2048
```

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

Add Cyrillic to the Latin range (code points, not literal characters):
```
fontbm --font-file FreeSans.ttf --output ru_font --chars 32-126,1040-1103 --kerning-pairs regular
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

The file name is `<output>.fnt` for all four.

## Exit Status

Exits with 0 on success, 1 on error. Errors and warnings (for example a glyph missing from the
font) go to stderr; `--help` and `--verbose` go to stdout.

## Running fontbm

The binary should be in the project directory or on PATH:
```
./fontbm --font-file FreeSans.ttf --output myfont
```

On Windows:
```
fontbm.exe --font-file FreeSans.ttf --output myfont
```
