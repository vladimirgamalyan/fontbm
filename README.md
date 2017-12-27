# fontbm
![sample](/assets/sample0.png?raw=true)
![sample](/assets/sample1.png?raw=true)

[![Build Status](https://travis-ci.org/vladimirgamalyan/fontbm.svg)](https://travis-ci.org/vladimirgamalyan/fontbm)
[![Build status](https://ci.appveyor.com/api/projects/status/boq0olngopfabaac?svg=true)](https://ci.appveyor.com/project/vladimirgamalyan/fontbm)

Command line bitmap font generator, compatible with [BMFont](http://www.angelcode.com/products/bmfont/).  
Download compiled version (fontbm.zip for windows, fontbm for linux) from [Releases](https://github.com/vladimirgamalyan/fontbm/releases).



## Usage
```
fontbm --font-file Vera.ttf --output vera
```
Will produce vera.fnt ([file format](http://www.angelcode.com/products/bmfont/doc/file_format.html)) and vera.png ([how to render text](http://www.angelcode.com/products/bmfont/doc/render_text.html)).

Available options (**bold** options are required):


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

## Building

Dependencies:

* GCC-4.9 (or VS2015 Update 3, for windows)
* CMake 3.0 (CMake 3.10, for use vcpkg on windows)
* [FreeType](https://www.freetype.org/)

Quickstart (ubuntu):
```
cmake .  
make
```

Quickstart (windows):

Generate project files:
```
cmake -G "Visual Studio 14 2015"
```
Download and install FreeType library, open .sln file in Visual Studio 2015, configure paths to FreeType and rebuild all.

Or, if [vcpkg](https://github.com/Microsoft/vcpkg) is used:
```
vcpkg install freetype
cmake -G "Visual Studio 14 2015" -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg dir>/scripts/buildsystems/vcpkg.cmake
```
Then open .sln in Visual Studio 2015 and rebuild all.

## License

[MIT License](http://opensource.org/licenses/MIT)

The project also bundles third party software under its own licenses:
* [lvandeve/lodepng](https://github.com/lvandeve/lodepng) - PNG encoder and decoder in C and C++ - [zlib](https://github.com/lvandeve/lodepng/issues/25)
* [juj/RectangleBinPack](https://github.com/juj/RectangleBinPack) - 2d rectangular bin packing - Public Domain
* [leethomason/tinyxml2](https://github.com/leethomason/tinyxml2) - a simple, small, efficient, C++ XML parse - [zlib](https://github.com/leethomason/tinyxml2#license)
* [UTF8-CPP](http://utfcpp.sourceforge.net/) - UTF-8 with C++ in a Portable Way - [BSL-1.0](http://www.boost.org/users/license.html)
* [catchorg/Catch2](https://github.com/catchorg/Catch2) - A modern, C++-native, header-only, test framework for unit-tests - [BSL-1.0](https://github.com/catchorg/Catch2/blob/master/LICENSE.txt)
* [jarro2783/cxxopts](https://github.com/jarro2783/cxxopts) - Lightweight C++ command line option parser - [MIT](https://github.com/jarro2783/cxxopts/blob/master/LICENSE)
* [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++ - [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
