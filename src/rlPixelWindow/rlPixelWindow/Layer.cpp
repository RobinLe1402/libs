#include "PrivateIncludes/Layer.hpp"

internal::Layer::Layer(PixelWindowSize iWidth, PixelWindowSize iHeight, PixelWindowPixel pxInit)
	:
	m_iWidth(iWidth), m_iHeight(iHeight)
{
	if (iWidth == 0 || iHeight == 0)
		throw std::exception("Invalid layer size");

	const size_t iPixelCount = (size_t)m_iWidth * m_iHeight;
	m_upPixels = std::make_unique<PixelWindowPixel[]>(iPixelCount);

	for (size_t i = 0; i < iPixelCount; ++i)
	{
		m_upPixels.get()[i] = pxInit;
	}
}

const PixelWindowPixel &internal::Layer::getPixel(PixelWindowSize iX, PixelWindowSize iY) const
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		throw std::exception("Invalid coordinates used with Layer::getPixel");

	return m_upPixels.get()[(m_iHeight - 1 - iY) * m_iWidth + iX];
}

void internal::Layer::setPixel(PixelWindowSize iX, PixelWindowSize iY,
	const PixelWindowPixel &px)
{
	if (iX >= m_iWidth || iY >= m_iHeight)
		throw std::exception("Invalid coordinates used with Layer::setPixel");

	m_upPixels.get()[(m_iHeight - 1 - iY) * m_iWidth + iX] = px;
}
