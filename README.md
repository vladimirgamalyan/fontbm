# fontbm
[![Build Status](https://travis-ci.org/vladimirgamalian/fontbm.svg)](https://travis-ci.org/vladimirgamalian/fontbm)

Command line bitmap font generator, compatible with [BMFont](http://www.angelcode.com/products/bmfont/).

![sample](/assets/sample0.png?raw=true)
![sample](/assets/sample1.png?raw=true)

## Usage
```
fontbm --font-file Vera.ttf --output vera --font-size 24
```
will produce vera.fnt and vera.png


Another options:


option  | default | comment
------|-----|---------------
--font-file |  | path to ttf file, required
--chars | 32-128 | required character, for example 32-64,92,120-126
--chars-file | | path to UTF-8 text file with required characters, override 'chars' option if exists
--color | 255,255,255 | foreground RGB color, for example: 32,255,255
--background-color | | background color RGB color, for example: 0,0,128, transparent, if not exists
--font-size | 32 | font size
--padding-up | 0 | padding up
--padding-right | 0 | padding right
--padding-down | 0 | padding down
--padding-left | 0 | padding left
--spacing-vert | 0 | spacing vertical
--spacing-horiz | 0 | spacing horizontal
--texture-width | 256 | texture width
--texture-height | 256 | texture height
--output | | output files name without extension, required
--data-format | xml | output data file format, "xml" or "txt"
--include-kerning-pairs | false | include kerning pairs to output file

## Thanks

* [libSDL2pp](https://github.com/AMDmi3/libSDL2pp)
* [RectangleBinPack](https://github.com/juj/RectangleBinPack)
* [www.libpng.org](http://www.libpng.org/)
* [SDL_SavePNG](https://github.com/driedfruit/SDL_SavePNG)
* [nlohmann/json](https://github.com/nlohmann/json)

## License

[MIT License](http://opensource.org/licenses/MIT)

The project also bundles third party software under its own licenses:
* extlibs/SDL2pp (C++11 SDL2 wrapper library) - zlib license
* [RectangleBinPack](https://github.com/juj/RectangleBinPack) - Public Domain
* [SDL_SavePNG](https://github.com/driedfruit/SDL_SavePNG) - zlib/libpng license
* [www.libpng.org](http://www.libpng.org/) - [libpng license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)
