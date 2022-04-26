#pragma once
#ifndef ROBINLE_FONT_DESIGNER__CONSTANTS
#define ROBINLE_FONT_DESIGNER__CONSTANTS





#include "font.hpp"
#include "rl/lib/rlOpenGL/Pixel.hpp"

using rl::OpenGL::Pixel;
namespace StdColor = rl::OpenGL::Color;



inline constexpr int iButtonHeight = 24;
inline constexpr int iButtonTextTop = (iButtonHeight - Font::iCharHeight) / 2;
inline constexpr int iButtonTextBottom = iButtonTextTop + Font::iCharHeight;
inline constexpr int iButtonMarginH = 8; // horizontal margin between buttons
inline constexpr float fButtonAnimDuration = 0.25f;


inline constexpr int iFooterClientHeight = 32;
inline constexpr int iFooterBorderHeight = 1;
inline constexpr int iFooterHeight = iFooterClientHeight + iFooterBorderHeight;
inline constexpr int iFooterPaddingH = 8;

inline constexpr int iFooterStatusY = (iFooterClientHeight - Font::iCharHeight +
	Font::iCharBaseline) / 2;
inline constexpr int iFooterStatusX = iFooterStatusY;

inline constexpr int iFooterButtonTop = (iFooterClientHeight - iButtonHeight) / 2;
inline constexpr int iFooterButtonBottom = iFooterButtonTop + iButtonHeight;
inline constexpr int iFooterButtonWidth = 100;

// from right
inline constexpr int iFooterButton_Next_R = iFooterPaddingH;
inline constexpr int iFooterButton_Next_L = iFooterPaddingH + iFooterButtonWidth;
inline constexpr int iFooterButton_Prev_R = iFooterButton_Next_L + iButtonMarginH;
inline constexpr int iFooterButton_Prev_L = iFooterButton_Prev_R + iFooterButtonWidth;





namespace Color
{
	inline constexpr Pixel Background = StdColor::White;


	inline constexpr Pixel FooterClient = Pixel::ByRGB(0xF0F0F0);
	inline constexpr Pixel FooterBorder = StdColor::Black;


	inline constexpr Pixel ButtonFace = Pixel::ByRGB(0xE1E1E1);
	inline constexpr Pixel ButtonBorder = Pixel::ByRGB(0xADADAD);

	inline constexpr Pixel ButtonFace_Hover = Pixel::ByRGB(0xE5F1FB);
	inline constexpr Pixel ButtonBorder_Hover = Pixel::ByRGB(0x0078D7);

	inline constexpr Pixel ButtonFace_Click = Pixel::ByRGB(0xCCE4F7);
	inline constexpr Pixel ButtonBorder_Click = Pixel::ByRGB(0x005499);
}





#endif // ROBINLE_FONT_DESIGNER__CONSTANTS