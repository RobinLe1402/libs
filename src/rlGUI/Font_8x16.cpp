#include "rl/lib/rlGUI/Font_8x16.hpp"

namespace lib = rl::GUI;



lib::ASCIIFont8x16::ASCIIFont8x16(const BitmapASCIIFont& data, Color cl) :
	IFont(cl)
{
	memcpy_s(m_oData, sizeof(m_oData), data, sizeof(data));
	onSetColor(cl);
}

void lib::ASCIIFont8x16::drawChar(char32_t ch, PxPos iX, PxPos iY) const
{
	m_oTextures[ch - BitmapASCIIFontFirstChar].draw(iX, iY);
}

void lib::ASCIIFont8x16::onSetColor(Color cl) noexcept
{
	for (PxSize iChar = 0; iChar < BitmapASCIIFontCharCount; ++iChar)
	{
		for (PxSize iRow = 0; iRow < BitmapCharHeight; ++iRow)
		{
			BitmapCharRow iRowData = m_oData[iChar][iRow];
			for (PxSize iCol = 0; iCol < BitmapCharWidth; ++iCol)
			{
				if (iRowData & 0x80)
					m_oTextures[iChar].setPixel(iCol, iRow, cl);
				/*else
					m_oTextures[iChar].setPixel(iCol, iRow, Colors::Blank);*/
				// not necessary as only the color but not the data can change

				iRowData <<= 1;
			}
		}
	}
}
