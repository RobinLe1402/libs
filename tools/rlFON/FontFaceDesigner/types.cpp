#include "types.hpp"

rl::OpenGLTexture HeaderButtonIconToTexture(const HeaderButtonIcon& icon)
{
	rl::OpenGLTexture result;

	rl::OpenGLTexture_Config cfg =
	{
		rl::OpenGLTextureFilter::NearestNeighbor,
		rl::OpenGLTextureFilter::NearestNeighbor,
		rl::OpenGLTextureWrap::Clamp,
		rl::OpenGLTextureWrap::Clamp
	};

	result.create(16, 16, cfg);
	for (uint8_t iX = 0; iX < 16; ++iX)
	{
		for (uint8_t iY = 0; iY < 16; ++iY)
		{
			uint8_t iVal = (uint8_t)(icon.data[iY] >> ((15 - iX) * 4));
			iVal &= 0x0F;
			iVal %= 5;

			iVal &= 0x0F;


			rl::Pixel px;

			if (iVal == 0 || iVal > 4)
				px = rl::BlankPixel;
			else
				px = icon.colors[iVal - 1];

			result.setPixel(iX, iY, px);
		}
	}

	return result;
}