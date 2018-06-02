# fontbm
![sample](/assets/sample0.png?raw=true)
![sample](/assets/sample1.png?raw=true)

[![Build Status](https://travis-ci.org/vladimirgamalyan/fontbm.svg)](https://travis-ci.org/vladimirgamalyan/fontbm)
[![Build status](https://ci.appveyor.com/api/projects/status/boq0olngopfabaac?svg=true)](https://ci.appveyor.com/project/vladimirgamalyan/fontbm)

Command line bitmap font generator, compatible with [BMFont](http://www.angelcode.com/products/bmfont/).  


## Usage

Download compiled version (fontbm.zip for **windows**, fontbm for **linux**) from [Releases](https://github.com/vladimirgamalyan/fontbm/releases). Run (fontbm.exe for windows, of course):

```
fontbm --font-file Vera.ttf --output vera
```
It will produce vera.fnt ([file format](http://www.angelcode.com/products/bmfont/doc/file_format.html)) and vera.png ([how to render text](http://www.angelcode.com/products/bmfont/doc/render_text.html)).

Available options (**bold** options are required):


option  | default | comment
------|-----|---------------
**--font-file** |  | path to ttf file, required
**--output** | | output files name without extension, required
--font-size | 32 | font size
--color | 255,255,255 | foreground RGB color, for example: 32,255,255 (without spaces)
--background-color | | background RGB color, for example: 0,0,128 (without spaces), transparent by default
--chars | 32-127* | required characters, for example 32-64,92,120-126 (without spaces), *default value is 32-127 if 'chars-file' option is not defined
--chars-file | | optional path to UTF-8 text file with additional required characters (will be combined with 'chars' option).
--data-format | txt | output data file format: txt, xml, bin, json
--include-kerning-pairs | | include kerning pairs to output file
--padding-up | 0 | padding up
--padding-right | 0 | padding right
--padding-down | 0 | padding down
--padding-left | 0 | padding left
--spacing-vert | 0 | spacing vertical
--spacing-horiz | 0 | spacing horizontal
--texture-width | 256 | texture width
--texture-height | 256 | texture height

## Building Linux

Dependencies:

* GCC-4.9
* CMake 3.0
* [FreeType](https://www.freetype.org/)

Build:

```
cmake .  
make
```

## Building Windows (using [vcpkg](https://github.com/Microsoft/vcpkg) )

Download and install [vcpkg](https://github.com/Microsoft/vcpkg) and [CMake 3.10.2](https://cmake.org/) (or above). Run:

```
vcpkg install freetype
cmake -G "Visual Studio 14 2015" -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg dir>/scripts/buildsystems/vcpkg.cmake
```
Open .sln in Visual Studio 2015 and rebuild all.

## Building Windows

Download and install CMake 3.0 (or above) and [FreeType](https://www.freetype.org/). Run: 

```
cmake -G "Visual Studio 14 2015"
```

Open .sln file in Visual Studio 2015, configure paths to FreeType and rebuild all.

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
