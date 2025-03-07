# fontbm

[BMFont](http://www.angelcode.com/products/bmfont/) compatible, cross-platform (Linux/macOS/Windows) command line bitmap font generator (FreeType2 based renderer) including the Adobe OpenType-SVG format.

![sample](/.github/img/sample0.png?raw=true)
![sample](/.github/img/sample1.png?raw=true)
![sample](/.github/img/sample2.png?raw=true)

## Status

Linux | macOS | Windows
------|-------|--------
[![Actions Status](https://github.com/vladimirgamalyan/fontbm/actions/workflows/linux_build.yml/badge.svg)](https://github.com/vladimirgamalyan/fontbm/actions/workflows/linux_build.yml) | [![Actions Status](https://github.com/vladimirgamalyan/fontbm/actions/workflows/mac_build.yml/badge.svg)](https://github.com/vladimirgamalyan/fontbm/actions/workflows/mac_build.yml) | [![Build status](https://ci.appveyor.com/api/projects/status/boq0olngopfabaac?svg=true)](https://ci.appveyor.com/project/vladimirgamalyan/fontbm)

## Usage

Download compiled version (fontbm.zip for Windows, fontbm for Linux) from [Releases](https://github.com/vladimirgamalyan/fontbm/releases). Run:

```
fontbm --font-file FreeSans.ttf --output myfont
```
It will produce myfont.fnt ([file format](https://www.angelcode.com/products/bmfont/doc/file_format.html)) and myfont_0.png ([how to render text](https://www.angelcode.com/products/bmfont/doc/render_text.html)).

Available options (**bold** options are required):


option  | default | comment
--------|---------|--------
**--font-file** |  | path to ttf file, required
**--output** | | output files name without extension, required
--font-size | 32 | font size (it matches to BMFont size, when "Match char height" option in Font Settings dialog is ticked)
--chars | 32-126 | required characters, for example 32-64,92,120-126 (without spaces), default value is 32-126 if 'chars-file' option is not defined
--texture-size | 32x32,64x32,64x64,128x64, 128x128,256x128,256x256, 512x256,512x512,1024x512, 1024x1024,2048x1024,2048x2048 | comma separated list of allowed texture sizes (without spaces), the first suitable size will be used
--texture-crop-width | | crop unused parts of output textures (width)
--texture-crop-height | | crop unused parts of output textures (height)
--color | 255,255,255 | foreground RGB color, for example: 32,255,255 (without spaces)
--background-color | | background RGB color, for example: 0,0,128 (without spaces), transparent by default
--chars-file | | optional path to UTF-8 text file with additional required characters (will be combined with 'chars' option), can be set multiple times
--data-format | txt | output data file format: txt, xml, bin, [json](https://github.com/Jam3/load-bmfont/blob/master/json-spec.md), [cbor](http://cbor.io/)
--kerning-pairs | disabled | generate kerning pairs: disabled, basic, regular (tuned by hinter), extended (bigger output size, but more precise)
--padding-up | 0 | padding up
--padding-right | 0 | padding right
--padding-down | 0 | padding down
--padding-left | 0 | padding left
--spacing-vert | 0 | spacing vertical
--spacing-horiz | 0 | spacing horizontal
--monochrome | | disable anti-aliasing
--extra-info | | write extra information to data file
--max-texture-count | | maximum generated texture count (unlimited if not set)
--texture-name-suffix | index_aligned | texture name suffix: "index_aligned", "index" or "none"

## Building Linux

Dependencies:

* GCC-4.9
* CMake 3.0
* [FreeType](https://www.freetype.org/)
* [LunaSVG](https://github.com/sammycage/lunasvg)

Build:

```
cmake .  
make
```

## Building Windows (using [vcpkg](https://github.com/Microsoft/vcpkg))

Download and install [vcpkg](https://github.com/Microsoft/vcpkg) and [CMake 3.10.2](https://cmake.org/) (or above). Run:

```
vcpkg install freetype
cmake -G "Visual Studio 14 2015" -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg dir>/scripts/buildsystems/vcpkg.cmake
```
Open .sln in Visual Studio 2015 and rebuild all.

## Building Windows

Download and install [CMake 3.0](https://cmake.org/) (or above) and [FreeType](https://www.freetype.org/). Run: 

```
cmake -G "Visual Studio 14 2015"
```

Open .sln file in Visual Studio 2015, configure paths to FreeType and rebuild all.

## Building macOS

(thanx to [andycarle](https://github.com/andycarle) https://github.com/Moddable-OpenSource/moddable/issues/325#issuecomment-769615337)

```
brew install freetype
brew install cmake
git clone https://github.com/vladimirgamalyan/fontbm.git
cd fontbm
cmake .
make
```

## Contributors

* [AMDmi3](https://github.com/AMDmi3)
* [cherniid](https://github.com/cherniid)
* [Radfordhound](https://github.com/Radfordhound)
* [phoddie](https://github.com/phoddie)
* [benbaker76](https://github.com/benbaker76)

## Special thanks

* [phoddie](https://github.com/phoddie)

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
* [sammycage/lunasvg](https://github.com/sammycage/lunasvg) - SVG rendering library in C++ - [MIT](https://github.com/sammycage/lunasvg/blob/master/LICENSE)