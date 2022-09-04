#include "rl/lib/rlGUI/IFont.hpp"

#include "rl/unicode.hpp"

#include <memory>

namespace lib = rl::GUI;



lib::IFont::IFont(Color cl, char32_t chFallback) : m_chFallback(chFallback)
{
	m_cl = cl;
}

void lib::IFont::setColor(Color cl) noexcept
{
	if (m_cl == cl)
		return; // no change

	m_cl = cl;
	onSetColor(cl);
}

void lib::IFont::drawText(const wchar_t* szText, PxPos iX, PxPos iY) const noexcept
{
	checkFallback();

	size_t len;
	auto sp_szDecoded = rl::UTF16::DecodeString(szText, &len);

	if (len == 0)
		return; // empty string

	const auto szDecoded = sp_szDecoded.get();

	char32_t ch32 = 0;
	for (size_t i = 0; i < len; ++i)
	{
		if (i > 0)
		{
			iX += getCharWidth(ch32);
			iX += getDistanceBetweenChars(ch32, szDecoded[i]);
		}
		ch32 = szDecoded[i];
		if (!containsChar(ch32))
			ch32 = getFallbackChar();

		drawChar(ch32, iX, iY);
	}
}

void lib::IFont::drawText(const char* szText, PxPos iX, PxPos iY) const noexcept
{
	checkFallback();
	const size_t len = strlen(szText);
	auto up_szWide = std::make_unique<wchar_t[]>(len + 1);
	wchar_t* const szWide = up_szWide.get();
	for (size_t i = 0; i < len; ++i)
	{
		wchar_t ch = szText[i];
		if (ch & 0x80)
			ch = m_chFallback;

		szWide[i] = ch;
	}
	szWide[len] = 0;

	drawText(szWide, iX, iY);
}

void lib::IFont::checkFallback() const
{
	if (!containsChar(m_chFallback))
		throw std::exception("rl::GUI::IFont: Set fallback character is not defined");
}
