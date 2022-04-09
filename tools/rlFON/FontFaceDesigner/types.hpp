#pragma once
#ifndef _ROBINLE_FONTFACEDESIGNER_TYPES_
#define _ROBINLE_FONTFACEDESIGNER_TYPES_





#include "rl/graphics.opengl.texture.hpp"



using rgb_t = unsigned int;
using uint16_t = unsigned short;
using uint64_t = unsigned long long;

using HeaderButtonIconData = uint64_t[16];
using HeaderButtonIconColors = rl::Pixel[4];

struct HeaderButtonIcon
{
	HeaderButtonIconData data;
	HeaderButtonIconColors colors;
};





rl::OpenGLTexture HeaderButtonIconToTexture(const HeaderButtonIcon& icon);





#endif // _ROBINLE_FONTFACEDESIGNER_TYPES_