#include "rl/graphics.fonts.bitmap.legacy.hpp"

#include "rl/tools.textencoding.hpp"

#include <algorithm> // std::min
#include <exception>
#include <stdint.h>





namespace rl
{

	/***********************************************************************************************
	 class BitmapChar
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void BitmapChar::create(uint8_t width, uint8_t height, uint8_t BitsPerPixel)
	{
		createInternal(width, height, BitsPerPixel);
		memset(m_pData, 0, (size_t)m_iBitsPerPixel * m_iLayerOffset); // initialize to all zero
	}

	void BitmapChar::create(uint8_t width, uint8_t height, uint8_t BitsPerPixel, uint8_t* data)
	{
		createInternal(width, height, BitsPerPixel);
		memcpy(m_pData, data, m_iLayerOffset);
	}

	void BitmapChar::destroy()
	{
		if (m_pData == nullptr)
			return;

		delete[] m_pData;
		m_pData = nullptr;
		m_iBitsPerPixel = 0;
		m_iWidth = 0;
		m_iHeight = 0;
	}

	void BitmapChar::setPixel(uint8_t x, uint8_t y, uint8_t val)
	{
		checkValid();
		checkPos(x, y);

		for (uint16_t i = 0; i < m_iBitsPerPixel; i++)
		{
			uint8_t& iByte = m_pData[i * m_iLayerOffset + x * m_iColOffset + y / 8];
			iByte &= ~(1 << (7 - (y % 8))); // mask out old bit
			iByte |= ((val >> (8 - (y % 8))) & 1) << i; // set new bit
		}
	}

	void BitmapChar::assign(const BitmapChar& other)
	{
		/*
		uint8_t* m_pData = nullptr;
		uint8_t m_iBitsPerPixel = 0; // must be between 1 and 8
		uint8_t m_iWidth = 0, m_iHeight = 0;
		uint8_t m_iColOffset = 0; // bytes per column per bit layer
		uint16_t m_iLayerOffset = 0; // bytes per bit layer
		*/

		destroy();

		if (!other.isValid())
			throw std::exception("rl::BitmapChar: Tried to assign invalid BitmapChar");

		create(other.m_iWidth, other.m_iHeight, other.m_iBitsPerPixel);
		memcpy(this->m_pData, other.m_pData, (size_t)m_iLayerOffset * m_iBitsPerPixel);
	}

	BitmapChar& BitmapChar::operator=(const BitmapChar& other)
	{
		assign(other);
		return *this;
	}

	uint8_t BitmapChar::getPixel(uint8_t x, uint8_t y) const
	{
		checkValid();
		checkPos(x, y);

		uint8_t result = 0;
		for (uint16_t i = 0; i < m_iBitsPerPixel; i++)
			result |= (m_pData[i * m_iLayerOffset + x * m_iColOffset + y / 8] & (1 << (7 - y % 8)));

		return result;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void BitmapChar::createInternal(uint8_t iWidth, uint8_t iHeight, uint8_t iBitsPerPixel)
	{
		destroy();

		// parameter check
		if (iWidth == 0 || iHeight == 0)
			throw std::exception("rl::BitmapChar: Size was 0");
		if (iBitsPerPixel == 0 || iBitsPerPixel > 8)
			throw std::exception("rl::BitmapChar: Bits per pixel invalid");

		m_iWidth = iWidth;
		m_iHeight = iHeight;
		m_iBitsPerPixel = iBitsPerPixel;

		m_iColOffset = std::min<uint8_t>(1, (uint8_t)ceil(m_iHeight / 8.0f));
		m_iLayerOffset = m_iWidth * m_iColOffset;

		m_pData = new uint8_t[(size_t)m_iBitsPerPixel * m_iLayerOffset];
	}

	void BitmapChar::checkValid() const
	{
		if (m_pData == nullptr)
			throw std::exception("rl::BitmapChar: Had no data");
	}

	void BitmapChar::checkPos(uint8_t iX, uint8_t iY) const
	{
		if (!validPos(iX, iY))
			throw std::exception("rl::BitmapChar: Invalid position");
	}

	bool BitmapChar::validPos(uint8_t iX, uint8_t iY) const
	{
		return iX < m_iWidth&& iY < m_iHeight;
	}










	/***********************************************************************************************
	 class BitmapFontFace
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
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void BitmapFontFace::create(Meta meta)
	{
		destroy();

		if (meta.BitsPerPixel == 0 || meta.BitsPerPixel > 8)
			throw std::exception("rl::BitmapFontFace: Invalid bits per pixel");
		if (meta.flags & RL_FF_MONOSPACE && meta.globalwidth == 0)
			throw std::exception("rl::BitmapFontFace: Global width for a monospace font was 0");

		m_oMeta = meta;
		m_bCreated = true;

		// calculate whitespace width
		if (m_oMeta.flags & RL_FF_MONOSPACE)
			m_iWhitespaceWidth = m_oMeta.globalwidth;
		else
			m_iWhitespaceWidth = std::min<uint8_t>(1, m_oMeta.height / 2);

		m_oFallbackChar = new BitmapChar();
		m_oFallbackChar->create(m_iWhitespaceWidth, m_oMeta.height, m_oMeta.BitsPerPixel);
		
		for (uint16_t iX = 0; iX < m_iWhitespaceWidth; iX++)
		{
			m_oFallbackChar->setPixel(iX, 0, 1);
			m_oFallbackChar->setPixel(iX, m_oMeta.height - 1, 1);
		}
		if (m_oMeta.height > 2)
		{
			for (uint16_t iY = 1; iY < m_oMeta.height - 1; iY++)
			{
				m_oFallbackChar->setPixel(0, iY, 1);
				m_oFallbackChar->setPixel(m_iWhitespaceWidth - 1, iY, 1);
			}
		}
	}

	void BitmapFontFace::destroy()
	{
		if (!m_bCreated)
			return;


		m_oMeta = {};
		for (auto& o : m_oChars)
			delete o.second;
		m_oChars.clear();
		delete m_oFallbackChar;
		m_oFallbackChar = nullptr;

		m_bCreated = false;
	}

	bool BitmapFontFace::addChar(uint32_t codepoint, BitmapChar& val)
	{
		if (!m_bCreated || contains(codepoint) || !val.isValid())
			return false;

		m_oChars.emplace(codepoint, new BitmapChar(val));
		return true;
	}

	void BitmapFontFace::removeChar(uint32_t codepoint)
	{
		if (!contains(codepoint))
			return;

		delete m_oChars[codepoint];
		m_oChars.erase(codepoint);
	}

	uint32_t BitmapFontFace::decodeChar(char ch) const { return DecodeChar(ch); }

	uint32_t BitmapFontFace::decodeChar(const wchar_t* ch) const { return DecodeWideChar(ch); }

	bool BitmapFontFace::contains(uint32_t codepoint) const
	{
		return m_oChars.find(codepoint) != m_oChars.end();
	}

	BitmapChar* BitmapFontFace::getChar(uint32_t codepoint)
	{
		if (!contains(codepoint))
			return nullptr;

		return m_oChars[codepoint];
	}

	const BitmapChar* BitmapFontFace::getChar(uint32_t codepoint) const
	{
		// return non-const function's result
		return const_cast<BitmapFontFace*>(this)->getChar(codepoint);
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class BitmapFontFaceFile
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