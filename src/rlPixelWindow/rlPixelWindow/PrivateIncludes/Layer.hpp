// todo: delete

#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__LAYER
#define ROBINLE_DLL_PIXEL_WINDOW__LAYER



#include "rl/dll/rlPixelWindow/Types.h"

#include <memory>



namespace internal
{



	/// <summary>
	/// A layer of a PixelWindow.<para/>
	/// Resembles a bitmap image.<para/>
	/// OpenGL compatible (RGBA, left to right, bottom to top)
	/// </summary>
	class Layer
	{
	public: // operators

		operator bool() { return (bool)m_upPixels; }

	public: // methods

		Layer() = default;
		Layer(PixelWindowSize iWidth, PixelWindowSize iHeight,
			PixelWindowPixel pxInit = PXWIN_COLOR_BLANK);
		Layer(const Layer &other) = default;
		Layer(Layer &&rval) = default;
		~Layer() = default;

		/// <summary>
		/// Get a pointer to the pixel data of this layer.
		/// </summary>
		const PixelWindowPixel *data() const { return m_upPixels.get(); }
		/// <summary>
		/// Get the size of the pixel data of this layer, in bytes.
		/// </summary>
		size_t datasize() const { return (size_t)m_iWidth * m_iHeight * sizeof(PixelWindowPixel); }

		const PixelWindowPixel &getPixel(PixelWindowSize iX, PixelWindowSize iY) const;
		void setPixel(PixelWindowSize iX, PixelWindowSize iY, const PixelWindowPixel &px);

		auto width() const { return m_iWidth; }
		auto height() const { return m_iHeight; }


	private:

		PixelWindowSize m_iWidth = 0, m_iHeight = 0;
		// Actual pixel data. Compatible with OpenGL (RGBA, left to right, bottom to top)
		std::unique_ptr<PixelWindowPixel[]> m_upPixels;

	};



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__LAYER