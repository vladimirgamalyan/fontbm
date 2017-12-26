# fontbm
[![Build Status](https://travis-ci.org/vladimirgamalyan/fontbm.svg)](https://travis-ci.org/vladimirgamalyan/fontbm)
[![Build status](https://ci.appveyor.com/api/projects/status/boq0olngopfabaac?svg=true)](https://ci.appveyor.com/project/vladimirgamalyan/fontbm)
[![Coverage Status](https://coveralls.io/repos/github/vladimirgamalyan/fontbm/badge.svg?branch=master)](https://coveralls.io/github/vladimirgamalyan/fontbm?branch=master)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/14665/badge.svg)](https://scan.coverity.com/projects/14665)

Command line bitmap font generator, compatible with [BMFont](http://www.angelcode.com/products/bmfont/).  
Download compiled version (windows, linux) from [Releases](https://github.com/vladimirgamalyan/fontbm/releases)

![sample](/assets/sample0.png?raw=true)
![sample](/assets/sample1.png?raw=true)

## Usage
```
fontbm --font-file Vera.ttf --output vera
```
will produce vera.fnt and vera.png


Available options (required in **bold**):


option  | default | comment
------|-----|---------------
**--font-file** |  | path to ttf file, required
**--output** | | output files name without extension, required
--font-size | 32 | font size
--color | 255,255,255 | foreground RGB color, for example: 32,255,255
--background-color | | background RGB color, for example: 0,0,128, transparent by default
--chars | 32-127* | required characters, for example 32-64,92,120-126, *default value is 32-127 if 'chars-file' option is not defined
--chars-file | | optional path to UTF-8 text file with additional required characters (will be combined with 'chars' option)
--data-format | txt | output data file format, "txt", "xml", "bin" or "json"
--include-kerning-pairs | | include kerning pairs to output file
--padding-up | 0 | padding up
--padding-right | 0 | padding right
--padding-down | 0 | padding down
--padding-left | 0 | padding left
--spacing-vert | 0 | spacing vertical
--spacing-horiz | 0 | spacing horizontal
--texture-width | 256 | texture width
--texture-height | 256 | texture height

## Build
Required:
  >= GCC-4.9 or >= clang-3.8 or VS2015.
  >= cmake 3.0
  FreeType

## License

[MIT License](http://opensource.org/licenses/MIT)

The project also bundles third party software under its own licenses:
* [lvandeve/lodepng](https://github.com/lvandeve/lodepng) - PNG encoder and decoder in C and C++ - [zlib](https://github.com/lvandeve/lodepng/issues/25)
* [juj/RectangleBinPack](https://github.com/juj/RectangleBinPack) - 2d rectangular bin packing - Public Domain
* [leethomason/tinyxml2](https://github.com/leethomason/tinyxml2) - a simple, small, efficient, C++ XML parse - [zlib](https://github.com/leethomason/tinyxml2#license)
* [catchorg/Catch2](https://github.com/catchorg/Catch2) - A modern, C++-native, header-only, test framework for unit-tests - [BSL-1.0](https://github.com/catchorg/Catch2/blob/master/LICENSE.txt)
* [jarro2783/cxxopts](https://github.com/jarro2783/cxxopts) - Lightweight C++ command line option parser - [MIT](https://github.com/jarro2783/cxxopts/blob/master/LICENSE)
* [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++ - [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
