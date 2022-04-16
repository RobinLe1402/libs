/***************************************************************************************************
 FILE:	Texture.hpp
 LIB:	OpenGLWin.lib
 DESCR:	Data structures for drawing OpenGL textures
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TEXTURE
#define ROBINLE_TEXTURE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
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
	namespace OpenGLWin
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
			constexpr Pixel(const Pixel& other) : iRaw(other.iRaw) {}
			constexpr Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 0xFF) :
				r(r), g(g), b(b), alpha(alpha) {}

			Pixel& operator=(const Pixel& other) { iRaw = other.iRaw; return *this; }



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
			void destroy();

			void upload();
			void undoUpload();

			TextureDrawingCoordinates coordsUnscaled(GLint iX, GLint iY,
				const TexturePixelSize& oViewportSize) const;
			TextureDrawingCoordinates coordsUnscaled(const TexturePixelRect& rect,
				GLint iX, GLint iY, const TexturePixelSize& oViewportSize) const;

			TextureDrawingCoordinates coordsScaled(const TexturePixelRect& oDestinationRect,
				const TexturePixelSize& oViewportSize);
			TextureDrawingCoordinates coordsScaled(const TexturePixelRect& oSourceRect,
				const TexturePixelRect& oDestinationRect,
				const TexturePixelSize& oViewportSize);

			void draw(const TextureDrawingCoordinates& coords);


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





#endif // ROBINLE_TEXTURE