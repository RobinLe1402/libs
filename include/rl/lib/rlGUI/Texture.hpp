#pragma once
#ifndef ROBINLE_LIB_GUI_TEXTURE
#define ROBINLE_LIB_GUI_TEXTURE





#include "BaseTypes.hpp"

#include <Windows.h>
#include <gl/GL.h>

#include <map>
#include <memory>



// TODO:
// Central management of GL rendering contexts including textures
namespace rl
{
	namespace GUI
	{

		enum class TextureScalingMethod
		{
			Linear,
			NearestNeighbor
		};
				


		class Texture
		{
		public: // methods

			Texture(PxSize iWidth, PxSize iHeight,
				TextureScalingMethod eScalingMethod = TextureScalingMethod::Linear,
				Color clInit = Colors::Blank);
			Texture(const Texture& other);
			~Texture();

			Color getPixel(PxPos x, PxPos y) const;
			void setPixel(PxPos x, PxPos y, Color cl);

			PxSize getWidth() const { return m_iWidth; }
			PxSize getHeight() const { return m_iHeight; }

			void draw(PxPos iX, PxPos iY);
			void drawStretched(PxPos iX, PxPos iY, PxSize iWidth, PxSize iHeight);


		private: // types

			struct Upload
			{
				bool bUpToDate;
				GLuint iTextureID;
			};


		private: // methods

			void upload();


		private: // variables

			const PxSize m_iWidth, m_iHeight;
			const size_t m_iPixelCount;
			const TextureScalingMethod m_eScalingMethod;
			std::unique_ptr<Color[]> m_upData;

			std::map<HGLRC, Upload> m_oUploads;
		};

	}
}





#endif // ROBINLE_LIB_GUI_TEXTURE