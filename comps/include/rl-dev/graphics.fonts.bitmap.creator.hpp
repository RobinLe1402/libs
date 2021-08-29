/***************************************************************************************************
 FILE:	graphics.fonts.bitmap.creator.hpp
 CPP:	graphics.fonts.bitmap.creator.cpp
		graphics.fonts.bitmap.reader.cpp
 DESCR:	Classes for creating/editing rlFNT files (v2.0)
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR
#define ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <headername>
// forward declarations


#include "../rl/graphics.fonts.bitmap.reader.hpp"



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// A single character of a bitmap font face
	/// </summary>
	class BitmapFontChar
	{
	public: // constructors, destructors

		BitmapFontChar() = default;
		BitmapFontChar(const BitmapFontChar & other);
		BitmapFontChar(BitmapFontChar && rval) noexcept;
		BitmapFontChar(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel);
		~BitmapFontChar();


	public: // operators

		BitmapFontChar& operator=(const BitmapFontChar& other);
		BitmapFontChar& operator=(BitmapFontChar&& other) noexcept;


	public: // methods

		void create(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel);
		void destroy();

		inline bool created() { return m_pData; }

		inline uint16_t getWidth() const { return m_iWidth; }
		inline uint16_t getHeight() const { return m_iHeight; }
		inline uint8_t getBitsPerPixel() const { return m_iBPP; }


	private: // variables

		uint8_t* m_pData = nullptr;
		uint16_t m_iWidth = 0;
		uint16_t m_iHeight = 0;
		uint16_t m_iHeightRounded = 0;
		uint8_t m_iBPP = 0;

	};
	
	class BitmapFontFaceCreator
	{

	};
	
}





// #undef foward declared definitions

#endif // ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR