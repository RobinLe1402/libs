/***************************************************************************************************
 FILE:	RasterFontReader.hpp
 LIB:	RasterFontReader.lib
 DESCR:	Read data from old-school 1bpp (1 bit per pixel) Microsoft raster fonts in various formats
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_RASTERFONTREADER
#define ROBINLE_LIB_RASTERFONTREADER

#ifdef RASTERFONTREADER_DLL // DLL
#ifdef LIBRARY_EXPORTS
#define RASTERFONT_API __declspec(dllexport)
#else
#define RASTERFONT_API __declspec(dllimport)
#endif

#else // LIB
#define RASTERFONT_API
#endif // RASTERFONTREADER_DLL





//==================================================================================================
// INCLUDES

#include <map>
#include <string>
#include <vector>





//==================================================================================================
// FORWARD DECLARATION

// <stdint.h>
using uint16_t = unsigned short;





//==================================================================================================
// DECLARATION
namespace rl
{
	namespace RasterFontReader
	{
		// forward declaration
		class RASTERFONT_API RasterFontFace;

		/// <summary>A single 1bpp raster character.</summary>
		class RASTERFONT_API RasterChar final
		{
			friend class RasterFont;
		public: // methods

			RasterChar(unsigned int iWidth, unsigned int iHeight);
			RasterChar(const RasterChar& other);
			RasterChar(RasterChar&& rval) noexcept;
			~RasterChar();

			/// <summary>
			/// Get the value of a single pixel of this character.
			/// </summary>
			/// <param name="iX">The pixel offset from the left.</param>
			/// <param name="iY">The pixel offset from the top.</param>
			/// <returns><c>true</c> if the pixel is opaque.</returns>
			bool getPixel(unsigned int iX, unsigned int iY) const;
			/// <summary>
			/// Set the value of a single pixel of this character.
			/// </summary>
			/// <param name="iX">The pixel offset from the left.</param>
			/// <param name="iY">The pixel offset from the top.</param>
			/// <param name="bSet">Set this pixel opaque?</param>
			void setPixel(unsigned int iX, unsigned int iY, bool bSet);

			/// <summary>Get the width of this character, in pixels.</summary>
			auto width() const { return m_iWidth; }
			/// <summary>Get the height of this character, in pixels.</summary>
			auto height() const { return m_iHeight; }


		private: // variables

			const unsigned int m_iWidth;
			const unsigned int m_iHeight;
			const unsigned int m_iBytesPerRow; // count of bytes per row
			const size_t m_iSize;
			uint8_t* m_pData; // only not "uint8_t* const" because of the move constructor

		};





		/*

		METADATA AVAILABILITY BY FORMAT

		VARIABLE    CPI FNT

		sFaceName   [ ] [X]
		sCopyright  [ ] [X]
		sDeviceName [X] [X]
		eType       [ ] [X]
		iPoints     [ ] [X]
		iAscent     [ ] [X]
		bItalic     [ ] [X]
		bUnderline  [ ] [X]
		bStrikeOut  [ ] [X]
		iWeight     [ ] [X]
		iCodepage   [X] [X]
		iHeight     [X] [X]
		iMonoWidth  [X] [X]
		iMinWidth   [X] [X]
		iMaxWidth   [X] [X]
		cFallback   [ ] [X]

		*/

		/// <summary>Generic font types.</summary>
		enum class FontType
		{
			/// <summary>Don't care or don't know.</summary>
			DontCare,
			/// <summary>Font is proportional and uses serifs.</summary>
			Roman,
			/// <summary><see cref="Roman"/></summary>
			Serif = Roman,
			/// <summary>Font is proportinal and doesn't use serifs.</summary>
			Swiss,
			/// <summary><see cref="Swiss"/></summary>
			SansSerif = Swiss,
			/// <summary>Font is monowidth and might or might not use serifs.</summary>
			Modern,
			/// <summary><see cref="Modern"/></summary>
			Mono = Modern,
			/// <summary>Font is designed to look like handwriting.</summary>
			Script,
			/// <summary>Font is a novelty font.</summary>
			Decorative
		};

		/// <summary>Metadata for a raster font.</summary>
		struct RasterFontFaceMeta
		{
			/// <summary>Name of the fontface.</summary>
			std::string sFaceName;
			/// <summary>Copyright string for the fontface.</summary>
			std::string sCopyright;
			/// <summary>Device name associated with the fontface.</summary>
			std::string sDeviceName;
			/// <summary>General type of the fontface (DontCare if unknown).</summary>
			FontType eType;
			/// <summary>The fontface's point size (0 if unknown).</summary>
			unsigned int iPoints;
			/// <summary>Count of pixels from top of character to baseline (0 if unknown).</summary>
			unsigned int iAscent;
			/// <summary>Is the fontface italic? (<c>false</c> if unknown).</summary>
			bool bItalic;
			/// <summary>Is the fontface underlined? (<c>false</c> if unknown).</summary>
			bool bUnderline;
			/// <summary>Is the fontface crossed out? (<c>false</c> if unknown).</summary>
			bool bStrikeOut;
			/// <summary>The weight of the fontface (0 if unknown).</summary>
			unsigned int iWeight;
			/// <summary>The original codepage of the fontface.</summary>
			uint16_t iCodepage;
			/// <summary>The height (in pixels) of the fontface's characters.</summary>
			unsigned int iHeight;
			/// <summary>The global character width (0 if not monospaced).</summary>
			unsigned int iMonoWidth;
			/// <summary>The minimum width (in pixels) of a character in this fontface.</summary>
			unsigned int iMinWidth;
			/// <summary>The maximum width (in pixels) of a character in this fontface.</summary>
			unsigned int iMaxWidth;
			/// <summary>
			/// The fallback character for when an undefined character is used (0 if undefined).
			/// </summary>
			char32_t cFallback;
		};



		/// <summary>The result of trying to load the fontface from Microsoft FNT data.</summary>
		enum class LoadResult_FNT
		{
			/// <summary>The data was loaded successfully.</summary>
			Success,
			/// <summary>Couldn't open the file for reading.</summary>
			FileNotOpened,
			/// <summary>The data stream ended unexpectedly.</summary>
			UnexpectedEOF,
			/// <summary>An internal error occured while parsing the data.</summary>
			InternalError,

			/// <summary>The font was a vector font instead of a raster font.</summary>
			VectorFont,
			/// <summary>
			/// Unsupported FNT version.<para/>
			/// This API only supports FNT 2.0 and FNT 3.0.
			/// </summary>
			UnsupportedVersion,
			/// <summary>
			/// The character set of the font couldn't be translated to a Windows codepage.
			/// </summary>
			UnknownCharSet,
			/// <summary>Graphics were not 1bpp</summary>
			MultiColor
		};

		/// <summary>One face of a raster font.</summary>
		class RASTERFONT_API RasterFontFace final
		{
		public:
			RasterFontFace();
			RasterFontFace(const RasterFontFace& other);
			RasterFontFace(RasterFontFace&& rval) noexcept;
			~RasterFontFace() = default;

			/// <summary>Load an old Windows raster font file.</summary>
			/// <returns>Could the fontface be loaded?</returns>
			LoadResult_FNT loadFromFile_FNT(const wchar_t* szFilepath,
				uint16_t iFallbackCodepage = 0);
			/// <summary>Load an old Windows raster font from memory.</summary>
			/// <returns>Could the fontface be loaded?</returns>
			LoadResult_FNT loadFromData_FNT(const void* pData, size_t iSize,
				uint16_t iFallbackCodepage = 0);
			/// <summary>Clear all data associated with this fontface.</summary>
			void clear();

			/// <summary>Get this fontface's metadata.</summary>
			auto& meta() { return m_oMeta; }
			/// <summary>Get this fontface's metadata.</summary>
			const auto& meta() const { return m_oMeta; }

			/// <summary>Get this fontface's character map.</summary>
			auto& chars() { return m_oChars; }
			/// <summary>Get this fontface's character map.</summary>
			const auto& chars() const { return m_oChars; }


		private: // variables

			RasterFontFaceMeta m_oMeta;
			std::map<char32_t, RasterChar> m_oChars;

		};



		/// <summary>
		/// The result of trying to load fontfaces from Microsoft MS-DOS font data.
		/// </summary>
		enum class LoadResult_CPI
		{
			/// <summary>The data was loaded successfully.</summary>
			Success,
			/// <summary>Couldn't open the file for reading.</summary>
			FileNotOpened,
			/// <summary>The data stream ended unexpectedly.</summary>
			UnexpectedEOF,
			/// <summary>An internal error occured while parsing the data.</summary>
			InternalError,

			/// <summary>The data contained a value that wasn't expected.</summary>
			UnexpectedValue
		};

		/// <summary>The result of trying to load fontfaces from Microsoft FNT data.</summary>
		enum class LoadResult_FON
		{
			/// <summary>The data was loaded successfully.</summary>
			Success,
			/// <summary>Couldn't open the file for reading.</summary>
			FileNotOpened,
			/// <summary>The data stream ended unexpectedly.</summary>
			UnexpectedEOF,
			/// <summary>An internal error occured while parsing the data.</summary>
			InternalError
		};

		/// <summary>A collection of raster fontfaces.</summary>
		class RASTERFONT_API RasterFontFaceCollection final
		{
		public: // methods

			RasterFontFaceCollection() = default;
			RasterFontFaceCollection(const RasterFontFaceCollection& other);
			RasterFontFaceCollection(RasterFontFaceCollection&& rval) noexcept;
			~RasterFontFaceCollection() = default;

			/// <summary>Load all fontfaces from a MS-DOS CodePageInfo file.</summary>
			/// <returns>Could the fontfaces be loaded?</returns>
			LoadResult_CPI loadFromFile_CPI(const wchar_t* szFilepath);
			/// <summary>Load all fontfaces from an old Windows raster font file.</summary>
			/// <returns>Could the fontfaces be loaded?</returns>
			LoadResult_FON loadFromFile_FON(const wchar_t* szFilepath,
				uint16_t iFallbackCodepage = 0);


			/// <summary>Load all fontfaces from MS-DOS CodePageInfo data.</summary>
			/// <returns>Could the fontfaces be loaded?</returns>
			LoadResult_CPI loadFromData_CPI(const void* pData, size_t iSize);
			/// <summary>Load all fontfaces from old Windows raster font data.</summary>
			/// <returns>Could the fontfaces be loaded?</returns>
			LoadResult_FON loadFromData_FON(const void* pData, size_t iSize,
				uint16_t iFallbackCodepage = 0);

			/// <summary>Unload all font data.</summary>
			void clear() { m_oFonts.clear(); }

			/// <summary>Get a list of all fontfaces loaded.</summary>
			const auto& fontfaces() const { return m_oFonts; }


		private: // variables

			std::vector<RasterFontFace> m_oFonts;
		};

	}
}





#endif // ROBINLE_LIB_RASTERFONTREADER