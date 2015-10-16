#include <stdexcept>
#include <fstream>
#include "Font.h"
#include "tinyxml2/tinyxml2.h"

void Font::debugFillValues()
{
    info.face = "Bitstream Vera Sans";
    info.size = 48;
    info.bold = 0;
    info.italic = 0;
    info.charset = 0;
    info.unicode = 1;
    info.stretchH = 100;
    info.smooth = 1;
    info.aa = 1;
    info.padding.up = 0;
    info.padding.right = 0;
    info.padding.down = 0;
    info.padding.left = 0;
    info.spacing.horizontal = 0;
    info.spacing.vertical = 0;
    info.outline = 0;

    common.lineHeight = 48;
    common.base = 38;
    common.scaleW = 256;
    common.scaleH = 256;
    common.pages = 1;
    common.packed = 0;
    common.alphaChnl = 0;
    common.redChnl = 4;
    common.greenChnl = 4;
    common.blueChnl = 4;

    pages.emplace_back(Page{0, "sample_0.png"});

    chars.emplace_back(Char{32, 3, 40, 3, 1, -1, 47, 13, 0, 15});
    chars.emplace_back(Char{33, 246, 62, 4, 30, 6, 8, 16, 0, 15});
    chars.emplace_back(Char{34, 177, 181, 10, 11, 4, 8, 19, 0, 15});
    chars.emplace_back(Char{35, 219, 127, 28, 29, 3, 9, 34, 0, 15});
    chars.emplace_back(Char{36, 54, 0, 19, 37, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{37, 20, 38, 34, 31, 2, 7, 39, 0, 15});
    chars.emplace_back(Char{38, 82, 37, 28, 31, 2, 7, 32, 0, 15});
    chars.emplace_back(Char{39, 253, 31, 3, 11, 4, 8, 11, 0, 15});
    chars.emplace_back(Char{40, 73, 0, 9, 37, 4, 7, 16, 0, 15});
    chars.emplace_back(Char{41, 100, 0, 9, 37, 3, 7, 16, 0, 15});
    chars.emplace_back(Char{42, 107, 182, 18, 18, 1, 7, 21, 0, 15});
    chars.emplace_back(Char{43, 15, 164, 25, 25, 4, 13, 34, 0, 15});
    chars.emplace_back(Char{44, 247, 123, 6, 10, 3, 33, 13, 0, 15});
    chars.emplace_back(Char{45, 215, 93, 11, 3, 2, 25, 15, 0, 15});
    chars.emplace_back(Char{46, 247, 133, 4, 5, 5, 33, 13, 0, 15});
    chars.emplace_back(Char{47, 173, 0, 14, 34, 0, 8, 14, 0, 15});
    chars.emplace_back(Char{48, 203, 32, 21, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{49, 184, 127, 18, 30, 5, 8, 26, 0, 15});
    chars.emplace_back(Char{50, 122, 67, 20, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{51, 102, 68, 20, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{52, 46, 130, 22, 30, 2, 8, 26, 0, 15});
    chars.emplace_back(Char{53, 165, 128, 19, 30, 3, 8, 26, 0, 15});
    chars.emplace_back(Char{54, 182, 34, 21, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{55, 146, 128, 19, 30, 3, 8, 26, 0, 15});
    chars.emplace_back(Char{56, 224, 32, 21, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{57, 0, 73, 21, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{58, 251, 88, 4, 21, 5, 17, 14, 0, 15});
    chars.emplace_back(Char{59, 250, 62, 6, 26, 3, 17, 14, 0, 15});
    chars.emplace_back(Char{60, 0, 192, 26, 22, 4, 14, 34, 0, 15});
    chars.emplace_back(Char{61, 125, 182, 26, 11, 4, 20, 34, 0, 15});
    chars.emplace_back(Char{62, 230, 156, 26, 22, 4, 14, 34, 0, 15});
    chars.emplace_back(Char{63, 161, 67, 16, 31, 3, 7, 22, 0, 15});
    chars.emplace_back(Char{64, 109, 0, 36, 36, 3, 9, 41, 0, 15});
    chars.emplace_back(Char{65, 0, 104, 27, 30, 0, 8, 28, 0, 15});
    chars.emplace_back(Char{66, 68, 130, 21, 30, 4, 8, 28, 0, 15});
    chars.emplace_back(Char{67, 136, 36, 24, 31, 2, 7, 29, 0, 15});
    chars.emplace_back(Char{68, 132, 98, 25, 30, 4, 8, 32, 0, 15});
    chars.emplace_back(Char{69, 108, 129, 19, 30, 4, 8, 26, 0, 15});
    chars.emplace_back(Char{70, 202, 127, 17, 30, 4, 8, 24, 0, 15});
    chars.emplace_back(Char{71, 110, 36, 26, 31, 2, 7, 32, 0, 15});
    chars.emplace_back(Char{72, 0, 134, 23, 30, 4, 8, 31, 0, 15});
    chars.emplace_back(Char{73, 242, 63, 4, 30, 4, 8, 12, 0, 15});
    chars.emplace_back(Char{74, 44, 0, 10, 38, -2, 8, 12, 0, 15});
    chars.emplace_back(Char{75, 181, 97, 24, 30, 4, 8, 27, 0, 15});
    chars.emplace_back(Char{76, 127, 129, 19, 30, 4, 8, 23, 0, 15});
    chars.emplace_back(Char{77, 215, 63, 27, 30, 4, 8, 35, 0, 15});
    chars.emplace_back(Char{78, 205, 97, 23, 30, 4, 8, 31, 0, 15});
    chars.emplace_back(Char{79, 54, 37, 28, 31, 2, 7, 32, 0, 15});
    chars.emplace_back(Char{80, 89, 129, 19, 30, 4, 8, 25, 0, 15});
    chars.emplace_back(Char{81, 145, 0, 28, 36, 2, 7, 32, 0, 15});
    chars.emplace_back(Char{82, 228, 93, 23, 30, 4, 8, 28, 0, 15});
    chars.emplace_back(Char{83, 21, 69, 21, 31, 3, 7, 26, 0, 15});
    chars.emplace_back(Char{84, 106, 99, 26, 30, -1, 8, 25, 0, 15});
    chars.emplace_back(Char{85, 23, 134, 23, 30, 4, 8, 30, 0, 15});
    chars.emplace_back(Char{86, 27, 100, 27, 30, 0, 8, 28, 0, 15});
    chars.emplace_back(Char{87, 177, 67, 38, 30, 1, 8, 41, 0, 15});
    chars.emplace_back(Char{88, 80, 99, 26, 30, 1, 8, 28, 0, 15});
    chars.emplace_back(Char{89, 54, 100, 26, 30, -1, 8, 25, 0, 15});
    chars.emplace_back(Char{90, 157, 98, 24, 30, 2, 8, 28, 0, 15});
    chars.emplace_back(Char{91, 82, 0, 9, 37, 4, 7, 16, 0, 15});
    chars.emplace_back(Char{92, 187, 0, 14, 34, 0, 8, 14, 0, 15});
    chars.emplace_back(Char{93, 91, 0, 9, 37, 4, 7, 16, 0, 15});
    chars.emplace_back(Char{94, 151, 182, 26, 11, 4, 8, 34, 0, 15});
    chars.emplace_back(Char{95, 46, 160, 21, 3, 0, 45, 21, 0, 15});
    chars.emplace_back(Char{96, 187, 180, 10, 8, 3, 5, 21, 0, 15});
    chars.emplace_back(Char{97, 133, 159, 19, 23, 2, 15, 25, 0, 15});
    chars.emplace_back(Char{98, 82, 68, 20, 31, 4, 7, 26, 0, 15});
    chars.emplace_back(Char{99, 152, 158, 18, 23, 2, 15, 23, 0, 15});
    chars.emplace_back(Char{100, 62, 68, 20, 31, 2, 7, 26, 0, 15});
    chars.emplace_back(Char{101, 72, 160, 21, 23, 2, 15, 25, 0, 15});
    chars.emplace_back(Char{102, 241, 0, 15, 31, 0, 7, 14, 0, 15});
    chars.emplace_back(Char{103, 0, 41, 20, 32, 2, 15, 26, 0, 15});
    chars.emplace_back(Char{104, 142, 67, 19, 31, 4, 7, 26, 0, 15});
    chars.emplace_back(Char{105, 245, 31, 4, 31, 4, 7, 11, 0, 15});
    chars.emplace_back(Char{106, 3, 0, 9, 40, -1, 7, 11, 0, 15});
    chars.emplace_back(Char{107, 42, 69, 20, 31, 4, 7, 24, 0, 15});
    chars.emplace_back(Char{108, 249, 31, 4, 31, 4, 7, 11, 0, 15});
    chars.emplace_back(Char{109, 40, 164, 32, 23, 4, 15, 40, 0, 15});
    chars.emplace_back(Char{110, 114, 159, 19, 23, 4, 15, 26, 0, 15});
    chars.emplace_back(Char{111, 93, 159, 21, 23, 2, 15, 25, 0, 15});
    chars.emplace_back(Char{112, 221, 0, 20, 32, 4, 15, 26, 0, 15});
    chars.emplace_back(Char{113, 201, 0, 20, 32, 2, 15, 26, 0, 15});
    chars.emplace_back(Char{114, 187, 157, 13, 23, 4, 15, 17, 0, 15});
    chars.emplace_back(Char{115, 170, 158, 17, 23, 2, 15, 21, 0, 15});
    chars.emplace_back(Char{116, 0, 164, 15, 28, 1, 10, 16, 0, 15});
    chars.emplace_back(Char{117, 70, 187, 19, 22, 4, 16, 26, 0, 15});
    chars.emplace_back(Char{118, 26, 189, 22, 22, 1, 16, 24, 0, 15});
    chars.emplace_back(Char{119, 200, 157, 30, 22, 2, 16, 34, 0, 15});
    chars.emplace_back(Char{120, 48, 187, 22, 22, 1, 16, 24, 0, 15});
    chars.emplace_back(Char{121, 160, 36, 22, 31, 1, 16, 24, 0, 15});
    chars.emplace_back(Char{122, 89, 183, 18, 22, 2, 16, 22, 0, 15});
    chars.emplace_back(Char{123, 12, 0, 16, 38, 5, 7, 26, 0, 15});
    chars.emplace_back(Char{124, 0, 0, 3, 41, 5, 7, 14, 0, 15});
    chars.emplace_back(Char{125, 28, 0, 16, 38, 5, 7, 26, 0, 15});
    chars.emplace_back(Char{126, 197, 180, 26, 7, 4, 21, 34, 0, 15});

    kernings.emplace_back(Kerning{45, 65, -1});
    kernings.emplace_back(Kerning{45, 66, -1});
    kernings.emplace_back(Kerning{67, 89, -1});
    kernings.emplace_back(Kerning{74, 45, -1});
    kernings.emplace_back(Kerning{83, 65, 1});
    kernings.emplace_back(Kerning{45, 71, 2});
    kernings.emplace_back(Kerning{71, 89, -2});
    kernings.emplace_back(Kerning{45, 74, 2});
    kernings.emplace_back(Kerning{71, 84, -1});
    kernings.emplace_back(Kerning{121, 58, -3});
    kernings.emplace_back(Kerning{45, 79, 1});
    kernings.emplace_back(Kerning{121, 46, -6});
    kernings.emplace_back(Kerning{45, 81, 2});
    kernings.emplace_back(Kerning{121, 45, -1});
    kernings.emplace_back(Kerning{45, 84, -4});
    kernings.emplace_back(Kerning{45, 86, -2});
    kernings.emplace_back(Kerning{45, 87, -2});
    kernings.emplace_back(Kerning{45, 88, -2});
    kernings.emplace_back(Kerning{45, 89, -5});
    kernings.emplace_back(Kerning{120, 99, -1});
    kernings.emplace_back(Kerning{75, 79, -2});
    kernings.emplace_back(Kerning{120, 111, -1});
    kernings.emplace_back(Kerning{45, 111, 1});
    kernings.emplace_back(Kerning{120, 101, -1});
    kernings.emplace_back(Kerning{45, 118, -1});
    kernings.emplace_back(Kerning{75, 84, -3});
    kernings.emplace_back(Kerning{45, 121, -1});
    kernings.emplace_back(Kerning{119, 58, -2});
    kernings.emplace_back(Kerning{119, 46, -4});
    kernings.emplace_back(Kerning{118, 58, -2});
    kernings.emplace_back(Kerning{118, 46, -3});
    kernings.emplace_back(Kerning{118, 45, -1});
    kernings.emplace_back(Kerning{114, 99, -1});
    kernings.emplace_back(Kerning{75, 85, -1});
    kernings.emplace_back(Kerning{114, 103, -1});
    kernings.emplace_back(Kerning{114, 120, -1});
    kernings.emplace_back(Kerning{114, 114, -1});
    kernings.emplace_back(Kerning{114, 113, -1});
    kernings.emplace_back(Kerning{114, 111, -1});
    kernings.emplace_back(Kerning{114, 110, -1});
    kernings.emplace_back(Kerning{114, 109, -1});
    kernings.emplace_back(Kerning{114, 104, -1});
    kernings.emplace_back(Kerning{75, 89, -1});
    kernings.emplace_back(Kerning{114, 101, -1});
    kernings.emplace_back(Kerning{114, 100, -1});
    kernings.emplace_back(Kerning{75, 87, -1});
    kernings.emplace_back(Kerning{114, 58, -1});
    kernings.emplace_back(Kerning{114, 46, -4});
    kernings.emplace_back(Kerning{114, 45, -3});
    kernings.emplace_back(Kerning{111, 120, -1});
    kernings.emplace_back(Kerning{111, 46, -1});
    kernings.emplace_back(Kerning{111, 45, 1});
    kernings.emplace_back(Kerning{107, 121, -1});
    kernings.emplace_back(Kerning{75, 45, -4});
    kernings.emplace_back(Kerning{107, 117, -1});
    kernings.emplace_back(Kerning{107, 111, -1});
    kernings.emplace_back(Kerning{65, 45, -1});
    kernings.emplace_back(Kerning{65, 46, -1});
    kernings.emplace_back(Kerning{65, 58, -1});
    kernings.emplace_back(Kerning{65, 65, 1});
    kernings.emplace_back(Kerning{65, 67, -1});
    kernings.emplace_back(Kerning{65, 71, -1});
    kernings.emplace_back(Kerning{65, 79, -1});
    kernings.emplace_back(Kerning{65, 81, -1});
    kernings.emplace_back(Kerning{107, 101, -1});
    kernings.emplace_back(Kerning{65, 84, -3});
    kernings.emplace_back(Kerning{107, 97, -1});
    kernings.emplace_back(Kerning{65, 86, -3});
    kernings.emplace_back(Kerning{65, 87, -2});
    kernings.emplace_back(Kerning{102, 121, -1});
    kernings.emplace_back(Kerning{65, 89, -3});
    kernings.emplace_back(Kerning{65, 99, -1});
    kernings.emplace_back(Kerning{65, 100, -1});
    kernings.emplace_back(Kerning{65, 101, -1});
    kernings.emplace_back(Kerning{65, 102, -1});
    kernings.emplace_back(Kerning{65, 111, -1});
    kernings.emplace_back(Kerning{65, 113, -1});
    kernings.emplace_back(Kerning{65, 116, -1});
    kernings.emplace_back(Kerning{102, 119, -1});
    kernings.emplace_back(Kerning{65, 118, -2});
    kernings.emplace_back(Kerning{65, 119, -2});
    kernings.emplace_back(Kerning{65, 121, -3});
    kernings.emplace_back(Kerning{102, 116, -1});
    kernings.emplace_back(Kerning{102, 58, -1});
    kernings.emplace_back(Kerning{102, 46, -3});
    kernings.emplace_back(Kerning{102, 45, -2});
    kernings.emplace_back(Kerning{101, 120, -1});
    kernings.emplace_back(Kerning{90, 45, -1});
    kernings.emplace_back(Kerning{89, 67, -2});
    kernings.emplace_back(Kerning{75, 65, -1});
    kernings.emplace_back(Kerning{89, 117, -5});
    kernings.emplace_back(Kerning{89, 111, -5});
    kernings.emplace_back(Kerning{89, 105, -1});
    kernings.emplace_back(Kerning{89, 101, -5});
    kernings.emplace_back(Kerning{89, 97, -6});
    kernings.emplace_back(Kerning{89, 79, -2});
    kernings.emplace_back(Kerning{75, 67, -2});
    kernings.emplace_back(Kerning{89, 65, -3});
    kernings.emplace_back(Kerning{89, 58, -5});
    kernings.emplace_back(Kerning{89, 46, -8});
    kernings.emplace_back(Kerning{89, 45, -5});
    kernings.emplace_back(Kerning{88, 67, -3});
    kernings.emplace_back(Kerning{76, 45, -1});
    kernings.emplace_back(Kerning{88, 101, -2});
    kernings.emplace_back(Kerning{88, 84, -1});
    kernings.emplace_back(Kerning{88, 79, -3});
    kernings.emplace_back(Kerning{70, 83, -1});
    kernings.emplace_back(Kerning{88, 45, -2});
    kernings.emplace_back(Kerning{87, 121, -1});
    kernings.emplace_back(Kerning{87, 117, -1});
    kernings.emplace_back(Kerning{87, 114, -2});
    kernings.emplace_back(Kerning{87, 111, -2});
    kernings.emplace_back(Kerning{87, 105, -1});
    kernings.emplace_back(Kerning{87, 101, -2});
    kernings.emplace_back(Kerning{87, 97, -3});
    kernings.emplace_back(Kerning{87, 65, -2});
    kernings.emplace_back(Kerning{87, 58, -2});
    kernings.emplace_back(Kerning{87, 46, -5});
    kernings.emplace_back(Kerning{87, 45, -2});
    kernings.emplace_back(Kerning{86, 121, -1});
    kernings.emplace_back(Kerning{86, 117, -3});
    kernings.emplace_back(Kerning{75, 111, -2});
    kernings.emplace_back(Kerning{86, 111, -3});
    kernings.emplace_back(Kerning{75, 97, -1});
    kernings.emplace_back(Kerning{75, 117, -2});
    kernings.emplace_back(Kerning{75, 101, -2});
    kernings.emplace_back(Kerning{75, 121, -3});
    kernings.emplace_back(Kerning{86, 105, -1});
    kernings.emplace_back(Kerning{66, 67, -1});
    kernings.emplace_back(Kerning{66, 71, -1});
    kernings.emplace_back(Kerning{66, 79, -1});
    kernings.emplace_back(Kerning{66, 83, -1});
    kernings.emplace_back(Kerning{66, 86, -1});
    kernings.emplace_back(Kerning{66, 87, -1});
    kernings.emplace_back(Kerning{66, 89, -2});
    kernings.emplace_back(Kerning{86, 101, -3});
    kernings.emplace_back(Kerning{86, 97, -3});
    kernings.emplace_back(Kerning{86, 79, -1});
    kernings.emplace_back(Kerning{86, 65, -3});
    kernings.emplace_back(Kerning{86, 58, -3});
    kernings.emplace_back(Kerning{86, 46, -5});
    kernings.emplace_back(Kerning{86, 45, -2});
    kernings.emplace_back(Kerning{85, 90, -1});
    kernings.emplace_back(Kerning{84, 99, -7});
    kernings.emplace_back(Kerning{84, 67, -2});
    kernings.emplace_back(Kerning{76, 87, -4});
    kernings.emplace_back(Kerning{76, 101, -1});
    kernings.emplace_back(Kerning{84, 115, -7});
    kernings.emplace_back(Kerning{84, 121, -6});
    kernings.emplace_back(Kerning{76, 85, -2});
    kernings.emplace_back(Kerning{76, 86, -5});
    kernings.emplace_back(Kerning{76, 79, -1});
    kernings.emplace_back(Kerning{76, 84, -6});
    kernings.emplace_back(Kerning{84, 119, -7});
    kernings.emplace_back(Kerning{84, 117, -6});
    kernings.emplace_back(Kerning{76, 117, -1});
    kernings.emplace_back(Kerning{74, 65, -1});
    kernings.emplace_back(Kerning{84, 114, -6});
    kernings.emplace_back(Kerning{84, 111, -7});
    kernings.emplace_back(Kerning{84, 105, -1});
    kernings.emplace_back(Kerning{84, 101, -7});
    kernings.emplace_back(Kerning{76, 89, -5});
    kernings.emplace_back(Kerning{84, 97, -7});
    kernings.emplace_back(Kerning{84, 84, -1});
    kernings.emplace_back(Kerning{76, 111, -1});
    kernings.emplace_back(Kerning{84, 65, -3});
    kernings.emplace_back(Kerning{84, 58, -5});
    kernings.emplace_back(Kerning{84, 46, -5});
    kernings.emplace_back(Kerning{84, 45, -4});
    kernings.emplace_back(Kerning{72, 46, -1});
    kernings.emplace_back(Kerning{82, 67, -2});
    kernings.emplace_back(Kerning{76, 121, -4});
    kernings.emplace_back(Kerning{68, 65, -1});
    kernings.emplace_back(Kerning{68, 86, -1});
    kernings.emplace_back(Kerning{82, 121, -2});
    kernings.emplace_back(Kerning{68, 89, -2});
    kernings.emplace_back(Kerning{82, 117, -2});
    kernings.emplace_back(Kerning{82, 111, -2});
    kernings.emplace_back(Kerning{82, 101, -2});
    kernings.emplace_back(Kerning{82, 97, -1});
    kernings.emplace_back(Kerning{82, 89, -3});
    kernings.emplace_back(Kerning{82, 87, -2});
    kernings.emplace_back(Kerning{82, 86, -2});
    kernings.emplace_back(Kerning{82, 84, -3});
    kernings.emplace_back(Kerning{79, 45, 1});
    kernings.emplace_back(Kerning{82, 65, -2});
    kernings.emplace_back(Kerning{82, 58, -1});
    kernings.emplace_back(Kerning{82, 46, -1});
    kernings.emplace_back(Kerning{82, 45, -2});
    kernings.emplace_back(Kerning{70, 46, -7});
    kernings.emplace_back(Kerning{70, 58, -3});
    kernings.emplace_back(Kerning{70, 65, -4});
    kernings.emplace_back(Kerning{76, 65, 1});
    kernings.emplace_back(Kerning{70, 84, -1});
    kernings.emplace_back(Kerning{70, 97, -4});
    kernings.emplace_back(Kerning{70, 101, -2});
    kernings.emplace_back(Kerning{70, 105, -3});
    kernings.emplace_back(Kerning{70, 111, -1});
    kernings.emplace_back(Kerning{70, 114, -3});
    kernings.emplace_back(Kerning{70, 117, -2});
    kernings.emplace_back(Kerning{70, 121, -4});
    kernings.emplace_back(Kerning{81, 45, 1});
    kernings.emplace_back(Kerning{80, 115, -1});
    kernings.emplace_back(Kerning{80, 117, -1});
    kernings.emplace_back(Kerning{79, 46, -2});
    kernings.emplace_back(Kerning{80, 114, -1});
    kernings.emplace_back(Kerning{80, 111, -1});
    kernings.emplace_back(Kerning{80, 110, -1});
    kernings.emplace_back(Kerning{80, 105, -1});
    kernings.emplace_back(Kerning{80, 101, -1});
    kernings.emplace_back(Kerning{80, 97, -2});
    kernings.emplace_back(Kerning{80, 89, -1});
    kernings.emplace_back(Kerning{80, 65, -3});
    kernings.emplace_back(Kerning{80, 46, -6});
    kernings.emplace_back(Kerning{80, 45, -1});
    kernings.emplace_back(Kerning{79, 89, -2});
    kernings.emplace_back(Kerning{79, 88, -3});
    kernings.emplace_back(Kerning{79, 86, -1});
    kernings.emplace_back(Kerning{79, 65, -1});
    kernings.emplace_back(Kerning{79, 58, -1});
}

void Font::writeToXmlFile(const std::string &fileName) const
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration("xml version=\"1.0\"");
    doc.InsertFirstChild(declaration);

    tinyxml2::XMLElement* root = doc.NewElement("font");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* infoElement = doc.NewElement("info");
    infoElement->SetAttribute("face", info.face.c_str());
    infoElement->SetAttribute("size", info.size);
    infoElement->SetAttribute("bold", info.bold);
    infoElement->SetAttribute("italic", info.italic);
    infoElement->SetAttribute("charset", info.charset);
    infoElement->SetAttribute("unicode", info.unicode);
    infoElement->SetAttribute("stretchH", info.stretchH);
    infoElement->SetAttribute("smooth", info.smooth);
    infoElement->SetAttribute("aa", info.aa);
    infoElement->SetAttribute("padding", info.padding.toString().c_str());
    infoElement->SetAttribute("spacing", info.spacing.toString().c_str());
    infoElement->SetAttribute("outline", info.outline);
    root->InsertEndChild(infoElement);

    tinyxml2::XMLElement* commonElement = doc.NewElement("common");
    commonElement->SetAttribute("lineHeight", common.lineHeight);
    commonElement->SetAttribute("base", common.base);
    commonElement->SetAttribute("scaleW", common.scaleW);
    commonElement->SetAttribute("scaleH", common.scaleH);
    commonElement->SetAttribute("pages", common.pages);
    commonElement->SetAttribute("packed", common.packed);
    commonElement->SetAttribute("alphaChnl", common.alphaChnl);
    commonElement->SetAttribute("redChnl", common.redChnl);
    commonElement->SetAttribute("greenChnl", common.greenChnl);
    commonElement->SetAttribute("blueChnl", common.blueChnl);
    root->InsertEndChild(commonElement);

    tinyxml2::XMLElement* pagesElement = doc.NewElement("pages");
    root->InsertEndChild(pagesElement);

    tinyxml2::XMLElement* charsElement = doc.NewElement("chars");
    charsElement->SetAttribute("count", static_cast<int>(chars.size()));
    root->InsertEndChild(charsElement);

    tinyxml2::XMLElement* kerningsElement = doc.NewElement("kernings");
    kerningsElement->SetAttribute("count", static_cast<int>(kernings.size()));
    root->InsertEndChild(kerningsElement);

    for(auto p: pages )
    {
        tinyxml2::XMLElement* pageElement = doc.NewElement("page");
        pageElement->SetAttribute("id", p.id);
        pageElement->SetAttribute("file", p.file.c_str());
        pagesElement->InsertEndChild(pageElement);
    }

    for(auto c: chars )
    {
        tinyxml2::XMLElement* charElement = doc.NewElement("char");
        charElement->SetAttribute("id", c.id);
        charElement->SetAttribute("x", c.x);
        charElement->SetAttribute("y", c.y);
        charElement->SetAttribute("width", c.width);
        charElement->SetAttribute("height", c.height);
        charElement->SetAttribute("xoffset", c.xoffset);
        charElement->SetAttribute("yoffset", c.yoffset);
        charElement->SetAttribute("xadvance", c.xadvance);
        charElement->SetAttribute("page", c.page);
        charElement->SetAttribute("chnl", c.chnl);
        charsElement->InsertEndChild(charElement);
    }

    for (auto k: kernings)
    {
        tinyxml2::XMLElement* kerningElement = doc.NewElement("kerning");
        kerningElement->SetAttribute("first", k.first);
        kerningElement->SetAttribute("second", k.second);
        kerningElement->SetAttribute("amount", k.amount);
        kerningsElement->InsertEndChild(kerningElement);
    }

    tinyxml2::XMLError err = doc.SaveFile(fileName.c_str(), false);
    if (err)
        throw std::runtime_error("xml write to file error");
}

void Font::writeToTextFile(const std::string &fileName) const
{
    std::ofstream f(fileName);
    f << "info " << info.toString() << std::endl;
    f << "common " << common.toString() << std::endl;
    for(auto p: pages)
        f << "page " << p.toString() << std::endl;
    f << "chars count=" << chars.size() << std::endl;
    for(auto c: chars)
        f << "char " << c.toString() << std::endl;
    f << "kernings count=" << kernings.size() << std::endl;
    for(auto k: kernings)
        f << "kerning " << k.toString() << std::endl;
}

void Font::writeToBinFile(const std::string &fileName) const
{
    std::fstream f(fileName, std::ios::binary);

#pragma pack(push)
#pragma pack(1)
    struct InfoBlock
    {
        int            blockSize;
        unsigned short fontSize;
        char           reserved    :4;
        char           bold        :1;
        char           italic      :1;
        char           unicode     :1;
        char           smooth      :1;
        unsigned char  charSet;
        unsigned short stretchH;
        char           aa;
        unsigned char  paddingUp;
        unsigned char  paddingRight;
        unsigned char  paddingDown;
        unsigned char  paddingLeft;
        unsigned char  spacingHoriz;
        unsigned char  spacingVert;
        unsigned char  outline;
        char           fontName[1];
    };

    struct CommonBlock
    {
        int blockSize;
        unsigned short lineHeight;
        unsigned short base;
        unsigned short scaleW;
        unsigned short scaleH;
        unsigned short pages;
        unsigned char  packed:1;
        unsigned char  reserved:7;
        unsigned char  alphaChnl;
        unsigned char  redChnl;
        unsigned char  greenChnl;
        unsigned char  blueChnl;
    };
#pragma pack(pop)

    InfoBlock infoBlock;
    infoBlock.blockSize = sizeof(InfoBlock) - sizeof(InfoBlock::blockSize) + info.face.length() + sizeof('\0');
    infoBlock.fontSize = info.size;
    infoBlock.bold = info.bold;
    infoBlock.italic = info.italic;
    infoBlock.unicode = info.unicode;
    infoBlock.smooth = info.smooth;
    infoBlock.charSet = info.charset;
    infoBlock.stretchH = info.stretchH;
    infoBlock.aa = info.aa;
    infoBlock.paddingUp = info.padding.up;
    infoBlock.paddingRight = info.padding.right;
    infoBlock.paddingDown = info.padding.down;
    infoBlock.paddingLeft = info.padding.left;
    infoBlock.spacingHoriz = info.spacing.horizontal;
    infoBlock.spacingVert = info.spacing.vertical;
    infoBlock.outline = info.outline;

    f << '\1';
    f.write((const char*)&infoBlock, sizeof(infoBlock);
    f.write(info.face.c_str(), info.face.length() + 1);


    CommonBlock commonBlock;
    commonBlock.blockSize = sizeof(CommonBlock);
    commonBlock.lineHeight = common.lineHeight;
    commonBlock.base = common.base;
    commonBlock.scaleW = common.scaleW;
    commonBlock.scaleH = common.scaleH;
    commonBlock.pages = common.pages;
    commonBlock.packed = common.packed;
    commonBlock.alphaChnl = common.alphaChnl;
    commonBlock.redChnl = common.redChnl;
    commonBlock.greenChnl = common.greenChnl;
    commonBlock.blueChnl = common.blueChnl;

    

}
