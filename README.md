# fontbm
[![Build Status](https://travis-ci.org/vladimirgamalian/fontbm.svg)](https://travis-ci.org/vladimirgamalian/fontbm)
[![Stories in Ready](https://badge.waffle.io/vladimirgamalian/fontbm.png?label=ready&title=Ready)](https://waffle.io/vladimirgamalian/fontbm)

Command line bitmap font generator, compatible with [BMFont](http://www.angelcode.com/products/bmfont/).

![sample](/assets/sample0.png?raw=true)
![sample](/assets/sample1.png?raw=true)

## Usage
```
fontbm --config <config file>
```

simple config file:
```javascript
{
    "fontFile": "Vera.ttf",
    "output": "vera",
}
```
will produce vera.fnt and vera.png.

Another options:


option  | default | comment
------|-----|---------------
fontFile |  | path to ttf font file, can be absolute or relative to config file directory. Required.
chars | \[\[32, 127\]\] | array of characters, each item can be character code or array of min..max character codes (include max).
color | \[255, 255, 255\] | array of R, G, B value, defines character color.
backgroundColor | null | array of R, G, B value, defines background color. If not present, result texture has transparent background.
fontSize | 32 | font size
paddingUp | 0 | padding up
paddingRight | 0 | padding right
paddingDown | 0 | padding down
paddingLeft | 0 | padding left
spacingVert | 0 | spacing vert
spacingHoriz | 0 | spacing horiz
textureWidth | 256 | texture width
textureHeight | 256 | texture height
output | null | output files name without extension, if not present using config file name. Can be absolute or relative to config file directory.
dataFormat | "xml" | "xml" or "txt"
includeKerningPairs | false | include kerning pairs

## Thanks to

* [libSDL2pp](https://github.com/AMDmi3/libSDL2pp)
* [RectangleBinPack](https://github.com/juj/RectangleBinPack)
* [www.libpng.org](http://www.libpng.org/)
* [SDL_SavePNG](https://github.com/driedfruit/SDL_SavePNG)
* [nlohmann/json](https://github.com/nlohmann/json)

## License

[MIT License](http://opensource.org/licenses/MIT)
