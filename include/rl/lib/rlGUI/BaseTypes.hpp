#pragma once
#ifndef ROBINLE_LIB_GUI_BASETYPES
#define ROBINLE_LIB_GUI_BASETYPES





#include <memory>



// forward declarations
using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using int32_t = int;



namespace rl
{
	namespace GUI
	{

		using GLPos = float;
		using PxSize = uint32_t;
		using PxPos = int32_t;

		struct GLCoord
		{
			PxPos X;
			PxPos Y;
		};

		/// <summary>
		/// OpenGL color.
		/// </summary>
		union Color
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};

			uint32_t iData;

			constexpr Color() noexcept : r(0), g(0), b(0), a(0) {}
			constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) noexcept :
				r(r), g(g), b(b), a(a) {}
			constexpr Color(const Color& other) noexcept = default;
			~Color() = default;

			static constexpr Color ByRGB(uint32_t iRGB)
			{
				return Color(
					/* r */ uint8_t(iRGB >> 16),
					/* g */ uint8_t(iRGB >> 8),
					/* b */ uint8_t(iRGB),
					/* a */ 0xFF
				);
			}
			static constexpr Color ByARGB(uint32_t iARGB)
			{
				return Color(
					/* r */ uint8_t(iARGB >> 16),
					/* g */ uint8_t(iARGB >> 8),
					/* b */ uint8_t(iARGB),
					/* a */ uint8_t(iARGB >> 24)
				);
			}

			bool operator==(const Color& other) const noexcept
			{
				return memcmp(this, &other, sizeof(Color)) == 0;
			}
			bool operator!=(const Color& other) const noexcept
			{
				return !(*this == other);
			}
		};

		namespace Colors
		{
			constexpr Color White = Color::ByRGB(0xFFFFFF);
			constexpr Color Black = Color::ByRGB(0x000000);
			constexpr Color Blank = Color::ByARGB(0x00'000000);
		}

	}
}





#endif // ROBINLE_LIB_GUI_BASETYPES