/***************************************************************************************************
 FILE:	lib/rlOpenGL/Pixel.hpp
 LIB:	rlOpenGL.lib (in theory; currently header-only)
 DESCR:	A data structure representing an RGBA pixel value
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_OPENGL__PIXEL
#define ROBINLE_LIB_OPENGL__PIXEL





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <cstdint>
using uint8_t = unsigned char;
using uint32_t = unsigned;



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGL
	{



		struct Pixel
		{
			union
			{
				struct
				{
					uint8_t r;
					uint8_t g;
					uint8_t b;
					uint8_t alpha;
				};
				uint32_t iRaw;
			};

			Pixel() : r(0), g(0), b(0), alpha(0) {}
			constexpr Pixel(const Pixel& other) = default;
			constexpr Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 0xFF) :
				r(r), g(g), b(b), alpha(alpha) {}

			Pixel& operator=(const Pixel& other) = default;



			static constexpr Pixel ByRGB(uint32_t iRGB)
			{
				return Pixel(
					uint8_t(iRGB >> 16),
					uint8_t(iRGB >> 8),
					uint8_t(iRGB)
				);
			}

			static constexpr Pixel ByRGBA(uint32_t iRGBA)
			{
				return Pixel(
					uint8_t(iRGBA >> 24),
					uint8_t(iRGBA >> 16),
					uint8_t(iRGBA >> 8),
					uint8_t(iRGBA)
				);
			}

			static constexpr Pixel ByARGB(uint32_t iARGB)
			{
				return Pixel(
					uint8_t(iARGB >> 16),
					uint8_t(iARGB >> 8),
					uint8_t(iARGB),
					uint8_t(iARGB >> 24)
				);
			}

			static constexpr Pixel ByBGR(uint32_t iBGR)
			{
				return Pixel(
					uint8_t(iBGR),
					uint8_t(iBGR >> 8),
					uint8_t(iBGR >> 16)
				);
			}

			static constexpr Pixel ByABGR(uint32_t iABGR)
			{
				return Pixel(
					uint8_t(iABGR),
					uint8_t(iABGR >> 8),
					uint8_t(iABGR >> 16),
					uint8_t(iABGR >> 24)
				);
			}

			static constexpr Pixel ByBGRA(uint32_t iBGRA)
			{
				return Pixel(
					uint8_t(iBGRA >> 8),
					uint8_t(iBGRA >> 16),
					uint8_t(iBGRA >> 24),
					uint8_t(iBGRA)
				);
			}

		};

		namespace Color
		{
			constexpr Pixel White = Pixel::ByRGB(0xFFFFFF);
			constexpr Pixel Black = Pixel::ByRGB(0x000000);
			constexpr Pixel Blank = Pixel::ByRGBA(0x000000'00);
		}



	}
}

#endif // ROBINLE_LIB_OPENGL__PIXEL
