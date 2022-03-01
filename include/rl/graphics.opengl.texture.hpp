/***************************************************************************************************
 FILE:	graphics.opengl.texture.hpp
 CPP:	graphics.opengl.texture.cpp
 DESCR:	Classes for OpenGL texture management
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_OPENGL_TEXTURE
#define ROBINLE_GRAPHICS_OPENGL_TEXTURE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <gl/gl.h>
typedef float GLfloat;
typedef int GLsizei;
typedef int GLint;
typedef unsigned int GLuint;

//--------------------------------------------------------------------------------------------------
// <gdiplus.h>
namespace Gdiplus
{
	class Bitmap;
}

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;


#include <Windows.h>
#include <vector>

#include "graphics.opengl.types.hpp"



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Holds the data of a single RGBA pixel
	/// </summary>
	struct Pixel
	{
		union
		{
			struct
			{
				uint8_t r; // red
				uint8_t g; // green
				uint8_t b; // blue
				uint8_t alpha; // alpha
			};

			// single integer representation of this pixel's color
			// shall not be treated as RGBA, as this value depends on the system endian
			uint32_t val;
		};

		Pixel() : r(0), g(0), b(0), alpha(0) {}
		constexpr Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 0xFF)
			: r(r), g(g), b(b), alpha(alpha) {}

		Pixel(const Pixel& other) = default;

		inline bool operator==(const Pixel& other) const { return this->val == other.val; }
		inline bool operator!=(const Pixel& other) const { return this->val != other.val; }





		/// <summary>
		/// Create <c>rl::Pixel</c> by RGB value<para/>
		/// Alpha will be set to <c>0xFF</c> (opaque)
		/// </summary>
		static constexpr inline Pixel ByRGB(uint32_t RGB);

		/// <summary>
		/// Create <c>rl::Pixel</c> by ARGB value
		/// </summary>
		static constexpr inline Pixel ByARGB(uint32_t ARGB);

		/// <summary>
		/// Create <c>rl::Pixel</c> by RGBA value
		/// </summary>
		static constexpr inline Pixel ByRGBA(uint32_t RGBA);





		/// <summary>
		/// Set this pixel by RGB<para/>
		/// Alpha will be set to <c>0xFF</c> (opaque)
		/// </summary>
		void setRGB(uint32_t RGB);

		/// <summary>
		/// Set this pixel by ARGB
		/// </summary>
		void setARGB(uint32_t ARGB);

		/// <summary>
		/// Set this pixel by RGBA
		/// </summary>
		void setRGBA(uint32_t RGBA);



		/// <summary>
		/// Get this pixel's ARGB value
		/// </summary>
		uint32_t getARGB() const;

		/// <summary>
		/// Get this pixel's RGBA value
		/// </summary>
		uint32_t getRGBA() const;

	};
	


	//==============================================================================================
	// Inline functions

	constexpr Pixel Pixel::ByRGB(uint32_t RGB)
	{
		return Pixel(
			(uint8_t)(RGB >> 16), // r
			(uint8_t)(RGB >> 8), // g
			(uint8_t)RGB, // b
			0xFF // alpha
		);
	}

	constexpr Pixel Pixel::ByARGB(uint32_t ARGB)
	{
		return Pixel(
			(uint8_t)(ARGB >> 16), // r
			(uint8_t)(ARGB >> 8), // g
			(uint8_t)ARGB, // b
			(uint8_t)(ARGB >> 24) // alpha
		);
	}

	constexpr Pixel Pixel::ByRGBA(uint32_t RGBA)
	{
		return Pixel(
			(uint8_t)(RGBA >> 24), // r
			(uint8_t)(RGBA >> 16), // g
			(uint8_t)(RGBA >> 8), // b
			(uint8_t)RGBA // alpha
		);
	}

	// Inline functions
	//==============================================================================================



	constexpr Pixel BlankPixel = Pixel::ByRGBA(0);
	namespace Color
	{
#define DECLARE_PIXEL(name, hex) constexpr Pixel name = Pixel::ByRGB(0x##hex)

		DECLARE_PIXEL(White, FFFFFF);
		DECLARE_PIXEL(Black, 000000);

		namespace WinCon
		{
			enum IDs
			{
				Black,
				DarkBlue,
				DarkGreen,
				DarkCyan,
				DarkRed,
				DarkMagenta,
				DarkYellow,
				DarkWhite,
				BrightBlack,
				BrightBlue,
				BrightGreen,
				BrightCyan,
				BrightRed,
				BrightMagenta,
				BrightYellow,
				White
			};

			/// <summary>
			/// The classic Windows console color scheme
			/// </summary>
			constexpr Pixel Legacy[16] =
			{
				Pixel::ByRGB(0x000000), // black
				Pixel::ByRGB(0x000080), // dark blue
				Pixel::ByRGB(0x008000), // dark green
				Pixel::ByRGB(0x008080), // dark cyan
				Pixel::ByRGB(0x800000), // dark red
				Pixel::ByRGB(0x800080), // dark magenta
				Pixel::ByRGB(0x808000), // dark yellow
				Pixel::ByRGB(0xC0C0C0), // dark white
				Pixel::ByRGB(0x808080), // bright black
				Pixel::ByRGB(0x0000FF), // bright blue
				Pixel::ByRGB(0x00FF00), // bright green
				Pixel::ByRGB(0x00FFFF), // bright cyan
				Pixel::ByRGB(0xFF0000), // bright red
				Pixel::ByRGB(0xFF00FF), // bright magenta
				Pixel::ByRGB(0xFFFF00), // bright yellow
				Pixel::ByRGB(0xFFFFFF), // white
			};

			/// <summary>
			/// The new default Windows console color scheme
			/// </summary>
			constexpr Pixel Campbell[16] =
			{
				Pixel::ByRGB(0x0C0C0C), // black
				Pixel::ByRGB(0x0037DA), // dark blue
				Pixel::ByRGB(0x13A10E), // dark green
				Pixel::ByRGB(0x3A96DD), // dark cyan
				Pixel::ByRGB(0xC50F1F), // dark red
				Pixel::ByRGB(0x881798), // dark magenta
				Pixel::ByRGB(0xC19C00), // dark yellow
				Pixel::ByRGB(0xCCCCCC), // dark white
				Pixel::ByRGB(0x767676), // bright black
				Pixel::ByRGB(0x3B78FF), // bright blue
				Pixel::ByRGB(0x16C60C), // bright green
				Pixel::ByRGB(0x61D6D6), // bright cyan
				Pixel::ByRGB(0xE74856), // bright red
				Pixel::ByRGB(0xB4009E), // bright magenta
				Pixel::ByRGB(0xF9F1A5), // bright yellow
				Pixel::ByRGB(0xF2F2F2), // white
			};
		}

#undef DECLARE_PIXEL
	}


	/// <summary>
	/// Determine how two pixels should be added up
	/// </summary>
	enum class PixelAddMode
	{
		Copy, // overwrite bottom pixel, including alpha
		OpaquePart, // overwrite bottom pixel if top pixel is at least partly visible
		OpaqueFull, // overwrite bottom pixel if top pixel is opaque (alpha = 255)
		Transparent // calculate new value, respecting both pixel's alpha values
	};

	/// <summary>
	/// Add up two pixels
	/// </summary>
	/// <param name="bottom">= the original pixel</param>
	/// <param name="top">= the pixel to be added</param>
	/// <param name="mode">= how should the output pixel be calculated?</param>
	Pixel AddPixels(Pixel bottom, Pixel top, PixelAddMode mode);





	/// <summary>
	/// OpenGL texture filter modes
	/// </summary>
	enum class OpenGLTextureFilter
	{
		Linear,
		NearestNeighbor
	};

	/// <summary>
	/// OpenGL texture wrapping modes
	/// </summary>
	enum class OpenGLTextureWrap
	{
		Repeat, // loop the texture
		Clamp // nothing is drawn outside the texture area

		// not included in gl.h (introduced in OpenGL 1.2, gl.h is OpenGL 1.1)
		/*
		MirroredRepeat, // loop the texture, but mirror every other iteration
		ClampToEdge, // border pixels are repeated
		ClampToBorder // nothing is drawn outside the texture area
		*/
	};


	/// <summary>
	/// OpenGL texture behavior configuration
	/// </summary>
	struct OpenGLTexture_Config
	{
		// texture filter to apply when the texture is displayed smaller than it really is
		OpenGLTextureFilter filterMag = OpenGLTextureFilter::Linear;
		// texture filter to apply when the texture is displayed bigger than it really is
		OpenGLTextureFilter filterMin = OpenGLTextureFilter::Linear;

		// wrapping mode applied horizontally
		OpenGLTextureWrap wrapX = OpenGLTextureWrap::Repeat;
		// wrapping mode applied vertically
		OpenGLTextureWrap wrapY = OpenGLTextureWrap::Repeat;
	};



	/// <summary>
	/// Holds the data of an 2D OpenGL (RGBA) texture
	/// </summary>
	class OpenGLTexture
	{
	public: // methods

		OpenGLTexture() {}
		OpenGLTexture(const OpenGLTexture& other);
		OpenGLTexture(OpenGLTexture&& rval) noexcept;
		~OpenGLTexture();



		/// <summary>
		/// Create a texture<para/>
		/// Current data gets destroyed, if existing
		/// </summary>
		/// <param name="config">= texture behavior configuration</param>
		/// <param name="InitVal">= value to initialize the texture with</param>
		/// <returns>Was the texture successfully created?</returns>
		bool create(GLsizei width, GLsizei height, OpenGLTexture_Config config,
			Pixel InitVal = BlankPixel);

		/// <summary>
		/// Create a texture from an GDI+ Bitmap object
		/// </summary>
		void createFromGDIPlusBitmap(Gdiplus::Bitmap* bmp, OpenGLTexture_Config config);

		/// <summary>
		/// Create a texture from a bitmap resource
		/// </summary>
		/// <returns>Was the texture successfully created?</returns>
		bool createFromBitmapResource(int ResourceID, OpenGLTexture_Config config);

		/// <summary>
		/// Create a texture from binary image data in RAM<para/>
		/// Supported image formats: All GDI+ decodable formats
		/// </summary>
		/// <returns>Was the texture successfully created?</returns>
		bool createFromImageData(const void* data, uint32_t size, OpenGLTexture_Config config);



		/// <summary>
		/// Destroy all data associated with this texture
		/// (both in RAM and on graphics card, if existing)
		/// </summary>
		void destroy();



		/// <summary>
		/// Draw the full texture<para/>
		/// <c>upload()</c> must have been called before drawing<para/>
		/// Values are OpenGL viewport coordinates
		/// (center is (0|0), top left is (-1|1), bottom left is (1|-1))<para/>
		/// Calling thread must have a current OpenGL rendering context
		/// </summary>
		void drawToScreen(OpenGLRect rect) const;

		/// <summary>
		/// Draw the full texture with a defined opacity<para/>
		/// <c>upload()</c> must have been called before drawing<para/>
		/// Values are OpenGL viewport coordinates
		/// (center is (0|0), top left is (-1|1), bottom left is (1|-1))<para/>
		/// Calling thread must have a current OpenGL rendering context
		/// </summary>
		void drawToScreen(OpenGLRect rect, GLfloat opacity)  const;



		/// <summary>
		/// Draw a texture onto this texture
		/// </summary>
		/// <param name="alpha">
		/// Should the alpha values be considered?<para/>
		/// <para/>
		/// <c>true</c>: The new pixels will be calculated based on the alpha values<para/>
		/// <c>false</c>: The new pixels will be copied 1:1 from the source texture, including alpha
		/// </param>
		void draw(const OpenGLTexture& texture, GLsizei x, GLsizei y, bool alpha);



		/// <summary>
		/// Clear the texture to a certain color
		/// </summary>
		void clear(Pixel val = BlankPixel);



		/// <summary>
		/// Set a single pixel value<para/>
		/// Throws an <c>std:exception</c> if the coordinates are invalid or if there is no
		/// texture<para/>
		/// Changes won't be displayed until <c>upload()</c> is called
		/// </summary>
		void setPixel(GLsizei x, GLsizei y, Pixel val);

		/// <summary>
		/// Get a single pixel value<para/>
		/// Throws an <c>std:exception</c> if the coordinates are invalid or if there is no texture
		/// </summary>
		Pixel getPixel(GLsizei x, GLsizei y) const;


		/// <summary>
		/// Draw a pixel on top of a pixel (respects alpha values)<para/>
		/// Throws an <c>std:exception</c> if the coordinates are invalid or if there is no
		/// texture<para/>
		/// Changes won't be displayed until <c>upload()</c> is called
		/// </summary>
		void drawPixel(GLsizei x, GLsizei y, Pixel val);



		/// <summary>
		/// Get the texture's width<para/>
		/// Returns 0 if there is no texture
		/// </summary>
		inline GLsizei getWidth() const { return m_iWidth; }

		/// <summary>
		/// Get the texture's height<para/>
		/// Returns 0 if there is no texture
		/// </summary>
		inline GLsizei getHeight() const { return m_iHeight; }


		/// <summary>
		/// Does this texture have data?
		/// </summary>
		inline bool isValid() const { return m_pData != nullptr; }



		/// <summary>
		/// Upload the texture to the graphics card in it's current state<para/>
		/// Calling thread must have a current OpenGL rendering context
		/// </summary>
		void upload(bool transparency = true);

		/// <summary>
		/// Remove the texture from the graphics card
		/// </summary>
		void removeUpload();

		/// <summary>
		/// Is this texture currenty present on the graphics card?
		/// </summary>
		inline bool uploaded() const { return m_bUploaded; }



		/// <summary>
		/// Read this texture's behavior
		/// </summary>
		/// <returns></returns>
		OpenGLTexture_Config getConfig() const;

		/// <summary>
		/// Copy the texture to a GDI+ Bitmap object
		/// </summary>
		/// <param name="bmp">= a pointer to an uninitialized Bitmap pointer</param>
		/// <returns>Was the texture successfully copied?</returns>
		bool copyToBitmap(Gdiplus::Bitmap** bmp) const;


	public: // operators

		OpenGLTexture& operator=(const OpenGLTexture& other);
		OpenGLTexture& operator=(OpenGLTexture&& rval) noexcept;


	private: // methods

		void create(GLsizei iWidth, GLsizei iHeight, GLint iFilterMag, GLint iFilterMin,
			GLint iWrapX, GLint iWrapY);

		static GLint getFilterInt(OpenGLTextureFilter filter);
		static GLint getWrapInt(OpenGLTextureWrap wrap);

		static OpenGLTextureFilter getFilterEnum(GLint filter);
		static OpenGLTextureWrap getWrapEnum(GLint wrap);


	private: // variables

		Pixel* m_pData = nullptr; // must be nullptr if texture doesn't exist
		GLsizei m_iWidth = 0, m_iHeight = 0; // the textures's dimensions
		GLuint m_iID = 0; // OpenGL ID of this texture
		bool m_bUploaded = false; // has the texture been uploaded to the graphics card?
		bool m_bUploadTransparency = false; // does the uploaded texture have transparency?

		GLint m_iMinFilter = 0;
		GLint m_iMagFilter = 0;
		GLint m_iWrapX = 0;
		GLint m_iWrapY = 0;
	};





	// some common OpenGLTexture configuration constants
	namespace OpenGL
	{
		const OpenGLTexture_Config TextureConfig_Pixelart =
		{
			OpenGLTextureFilter::NearestNeighbor,
			OpenGLTextureFilter::NearestNeighbor,
			OpenGLTextureWrap::Clamp,
			OpenGLTextureWrap::Clamp
		};
	}

}





#endif // ROBINLE_GRAPHICS_OPENGL_TEXTURE