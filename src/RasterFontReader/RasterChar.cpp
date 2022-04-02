#include "rl/lib/RasterFontReader.hpp"

namespace lib = rl::RasterFontReader;

#include "include/Assert.h"





//==================================================================================================
// RasterChar

lib::RasterChar::RasterChar(unsigned int iWidth, unsigned int iHeight) :
	m_iWidth(iWidth), m_iHeight(iHeight),
	m_iBytesPerRow(m_iWidth / 8 + (m_iWidth % 8 ? 1 : 0)),
	m_iSize((size_t)m_iBytesPerRow * m_iHeight),
	m_pData(new uint8_t[m_iSize])
{
	FONT__ASSERT(iWidth > 0);
	FONT__ASSERT(iHeight > 0);
}

lib::RasterChar::RasterChar(const RasterChar& other) :
	m_iWidth(other.m_iWidth), m_iHeight(other.m_iHeight),
	m_iBytesPerRow(other.m_iBytesPerRow),
	m_iSize(other.m_iSize),
	m_pData(new uint8_t[m_iSize])
{
	memcpy_s(m_pData, m_iSize, other.m_pData, other.m_iSize);
}

lib::RasterChar::RasterChar(RasterChar&& rval) noexcept :
	m_iWidth(rval.m_iWidth), m_iHeight(rval.m_iHeight),
	m_iBytesPerRow(rval.m_iBytesPerRow),
	m_iSize(rval.m_iSize),
	m_pData(rval.m_pData)
{
	rval.m_pData = nullptr;
}

lib::RasterChar::~RasterChar() { delete[] m_pData; }

bool lib::RasterChar::getPixel(unsigned int iX, unsigned int iY) const
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		return false;

	const uint8_t iByte = *(m_pData + (iY * m_iBytesPerRow) + (iX / 8));
	return (iByte >> (7 - (iX & 7))) & 1;
}

void lib::RasterChar::setPixel(unsigned int iX, unsigned int iY, bool bSet)
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		return;

	uint8_t* const pByte = m_pData + (iY * m_iBytesPerRow) + (iX / 8);
	const uint8_t iShift = 7 - (iX & 7);
	*pByte &= ~(1 << iShift);
	*pByte |= (bSet ? 1 : 0) << iShift;
}
