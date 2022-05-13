#include "include/FontWrapper.hpp"



FontWrapper::operator bool() const
{
	if (m_oChars.size() == 0)
		return false;

	return m_oChars.contains(m_cFallback);
}

FontWrapper::FontWrapper(rlConsole_Font& oFont) : m_cFallback(oFont.cFallback)
{
	for (size_t i = 0; i < oFont.iCharCount; ++i)
	{
		auto& oCh = oFont.pData[i];

		WrappedCharData wcd{};
		memcpy_s(wcd.iData, WrappedCharDataSize, oCh.iData, WrappedCharDataSize);

		m_oChars[oCh.c] = wcd; // if duplicate --> use last
	}
}

const WrappedCharData* FontWrapper::getChar(rlCon_char c) const
{
	if (!*this)
		return nullptr;

	auto it = m_oChars.find(c);
	if (it != m_oChars.end())
		return &it->second;

	return &m_oChars.at(m_cFallback);
}
