/***************************************************************************************************
 FILE:	lib/rlOpenGL/Texture.hpp
 LIB:	rlOpenGL.lib
 DESCR:	Data structures for drawing OpenGL textures
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_OPENGL__TEXTURE
#define ROBINLE_LIB_OPENGL__TEXTURE





//==================================================================================================
// INCLUDES

#include "Pixel.hpp"



//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <cstdint>
using uint8_t = unsigned char;
using uint32_t = unsigned;

//--------------------------------------------------------------------------------------------------
// <gl/GL.h>
using GLint = int;
using GLuint = unsigned;
using GLfloat = float;



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGL
	{



		struct TexturePoint
		{
			GLfloat fX;
			GLfloat fY;
		};

		struct TextureQuad
		{
			TexturePoint topLeft;
			TexturePoint bottomLeft;
			TexturePoint bottomRight;
			TexturePoint topRight;

			static constexpr TextureQuad Rect(TexturePoint topLeft, TexturePoint bottomRight)
			{
				return TextureQuad(
					topLeft,
					{ topLeft.fX, bottomRight.fY },
					bottomRight,
					{ bottomRight.fX, topLeft.fY }
				);
			}
		};

		constexpr auto FullTexture = TextureQuad::Rect({ 0.0f, 1.0f }, { 1.0f, 0.0f });
		constexpr auto FullScreen = TextureQuad::Rect({ -1.0f, 1.0f }, { 1.0f, -1.0f });

		struct TextureDrawingCoordinates
		{
			TextureQuad screen;
			TextureQuad texture;
		};

		struct TexturePixelRect
		{
			GLint iLeft;
			GLint iTop;
			GLint iRight;
			GLint iBottom;
		};

		struct TexturePixelSize
		{
			unsigned iWidth;
			unsigned iHeight;
		};

		constexpr GLfloat PixelToViewport_X(GLint iPixelX, GLuint iViewportWidth)
		{
			const GLfloat fCenterViewport = iViewportWidth / 2.0f;
			return (iPixelX - fCenterViewport) / fCenterViewport;
		}
		constexpr GLfloat PixelToViewport_Y(GLint iPixelY, GLuint iViewportHeight)
		{
			const GLfloat fCenterViewport = iViewportHeight / 2.0f;
			return (-iPixelY + fCenterViewport) / fCenterViewport;
		}
		constexpr GLfloat PixelToTexture_X(GLint iPixelX, GLuint iTextureWidth)
		{
			return (GLfloat)iPixelX / iTextureWidth;
		}
		constexpr GLfloat PixelToTexture_Y(GLint iPixelY, GLuint iTextureHeight)
		{
			return ((GLfloat)iTextureHeight - iPixelY) / iTextureHeight;
		}



		/// <summary>
		/// The method used for scaling a texture
		/// </summary>
		enum class TextureScalingMethod
		{
			Linear,
			NearestNeighbor
		};

		/// <summary>
		/// The method used when drawing outside the main texture area
		/// </summary>
		enum class TextureWrapMethod
		{
			Repeat,
			Clamp
		};

		/// <summary>
		/// Texture data for OpenGL
		/// </summary>
		class Texture
		{
		public: // operators

			operator bool() const { return m_pData != nullptr; }
			bool operator!() const { return m_pData == nullptr; }
			Texture& operator=(const Texture& other);
			Texture& operator=(Texture&& rval) noexcept;


		public: // methods

			Texture();
			Texture(const Texture& other);
			Texture(Texture&& rval) noexcept;
			virtual ~Texture();

			bool create(GLuint iWidth, GLuint iHeight);
			bool create(GLuint iWidth, GLuint iHeight, const Pixel& pxColor);
			void destroy();

			void upload();
			void undoUpload();

			TextureDrawingCoordinates coordsUnscaled(GLint iX, GLint iY,
				const TexturePixelSize& oViewportSize) const;
			TextureDrawingCoordinates coordsUnscaled(const TexturePixelRect& rect,
				GLint iX, GLint iY, const TexturePixelSize& oViewportSize) const;

			TextureDrawingCoordinates coordsScaled(const TexturePixelRect& oDestinationRect,
				const TexturePixelSize& oViewportSize) const;
			TextureDrawingCoordinates coordsScaled(const TexturePixelRect& oSourceRect,
				const TexturePixelRect& oDestinationRect,
				const TexturePixelSize& oViewportSize) const;

			void draw(const TextureDrawingCoordinates& coords) const;


			// setters

			bool setPixel(GLuint iX, GLuint iY, const Pixel& px);
			void setTransparency(bool bTransparency);
			void setOpacity(GLfloat fOpacity);
			void setUpscalingMethod(TextureScalingMethod eMethod);
			void setDownscalingMethod(TextureScalingMethod eMethod);
			void setScalingMethod(TextureScalingMethod eMethod);
			void setWrapMethodX(TextureWrapMethod eMethod);
			void setWrapMethodY(TextureWrapMethod eMethod);
			void setWrapMethod(TextureWrapMethod eMethod);


			// getters

			Pixel getPixel(GLuint iX, GLuint iY) const;
			auto width() const { return m_iWidth; }
			auto height() const { return m_iHeight; }
			bool uploaded() const { return m_iID != 0; }
			auto id() const { return m_iID; }
			auto transparency() const { return m_bTransparency; }
			auto opacity() const { return m_fOpacity; }
			auto downscalingMethod() const { return m_eDownscalingMethod; }
			auto upscalingMethod() const { return m_eUpscalingMethod; }
			auto wrapMethodX() const { return m_eWrapMethodX; }
			auto wrapMethodY() const { return m_eWrapMethodY; }


		private: // methods

			bool createUninitialized(GLuint iWidth, GLuint iHeight);


		private: // variables

			// main data
			Pixel* m_pData; // stored upside-down, since OpenGL assignes Y=0 to the bottom
			unsigned m_iWidth, m_iHeight;

			// OpenGL-related data
			GLuint m_iID;
			bool m_bTransparency;
			GLfloat m_fOpacity;
			TextureScalingMethod m_eUpscalingMethod;
			TextureScalingMethod m_eDownscalingMethod;
			TextureWrapMethod m_eWrapMethodX;
			TextureWrapMethod m_eWrapMethodY;

		};





	}
}





#endif // ROBINLE_LIB_OPENGL__TEXTURE