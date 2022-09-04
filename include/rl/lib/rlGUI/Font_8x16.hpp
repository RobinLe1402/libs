#pragma once
#ifndef ROBINLE_LIB_GUI_FONT_8X16
#define ROBINLE_LIB_GUI_FONT_8X16





#include "IFont.hpp"

#include "Texture.hpp"



namespace rl
{
	namespace GUI
	{

		constexpr PxSize BitmapCharWidth = 8;
		constexpr PxSize BitmapCharHeight = 16;
		constexpr unsigned char BitmapASCIIFontFirstChar = 0x20; // space
		constexpr unsigned char BitmapASCIIFontCharCount = 0x7F - BitmapASCIIFontFirstChar + 1;

		using BitmapCharRow = unsigned char;
		using BitmapChar = BitmapCharRow[BitmapCharHeight];
		using BitmapASCIIFont = BitmapChar[BitmapASCIIFontCharCount];

		class ASCIIFont8x16 final : public IFont
		{
		public: // methods

			ASCIIFont8x16(const BitmapASCIIFont& data, Color cl);
			~ASCIIFont8x16() = default;

			PxSize getCharHeight() const override { return BitmapCharHeight; }
			bool containsChar(char32_t ch) const override
			{
				return (ch >= BitmapASCIIFontFirstChar &&
					ch < BitmapASCIIFontFirstChar + BitmapASCIIFontCharCount);
			}
			PxSize getCharWidth(char32_t ch) const override { return BitmapCharWidth; }


		protected: // methods

			void drawChar(char32_t ch, PxPos iX, PxPos iY) const override;
			void onSetColor(Color cl) noexcept override;


		private: // variables

#define TEXINIT { 8, 16, TextureScalingMethod::NearestNeighbor }
#define TEXINIT_X8 TEXINIT, TEXINIT, TEXINIT, TEXINIT, TEXINIT, TEXINIT, TEXINIT, TEXINIT
#define TEXINIT_X32 TEXINIT_X8, TEXINIT_X8, TEXINIT_X8, TEXINIT_X8
			Texture m_oTextures[BitmapASCIIFontCharCount] =
			{
				TEXINIT_X32, TEXINIT_X32, TEXINIT_X32
			};
#undef TEXINIT_X32
#undef TEXINIT_X8
#undef TEXINIT
			BitmapASCIIFont m_oData{};
		};

	}
}





#endif // ROBINLE_LIB_GUI_FONT_8X16