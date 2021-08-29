#include "graphics.fonts.bitmap.creator.hpp"

#include <memory>





namespace rl
{
	
	/***********************************************************************************************
	 class BitmapFontChar
	***********************************************************************************************/
	
	//==============================================================================================
	// STATIC VARIABLES
	
	// static variables
	
	
	
	
	
	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS
	
	BitmapFontChar::BitmapFontChar(const BitmapFontChar& other)
	{
		*this = other;
	}

	BitmapFontChar::BitmapFontChar(BitmapFontChar&& rval) noexcept
	{
		*this = std::move(rval);
	}

	BitmapFontChar::BitmapFontChar(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel)
	{
		if (Width == 0 || Height == 0 || BitsPerPixel == 0 || BitsPerPixel > 32)
			return;

		m_iWidth = Width;
		m_iHeight = Height;
		m_iHeightRounded = Height / 8;
		if (Height % 8)
			++m_iHeightRounded;

		m_pData = new uint8_t[(size_t)m_iHeightRounded * m_iWidth]; // todo: fix! not functional yet!!!!
	}

	BitmapFontChar::~BitmapFontChar()
	{
		destroy();
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// STATIC METHODS
	
	// static methods
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS
	
	// public methods
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS
	
	// protected methods
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS
	
	// private methods










	/***********************************************************************************************
	 class XXX
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods
	
}