/***************************************************************************************************
 FILE:	graphics.fonts.bitmap.creator.hpp
 CPP:	graphics.fonts.bitmap.creator.cpp
		graphics.fonts.bitmap.reader.cpp
		unicode.cpp
 DESCR:	Classes for creating/editing rlFNT files (v2.0)
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR
#define ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <headername>
// forward declarations


#include <rl/graphics.fonts.bitmap.reader.hpp>

#include <map>
#include <string>



//==================================================================================================
// DECLARATION
namespace rl
{

	// forward declaration
	class BitmapFontFaceCreator;

	/// <summary>
	/// A single character of a bitmap font face
	/// </summary>
	class BitmapFontChar
	{
		friend class BitmapFontFaceCreator;
	public: // constructors, destructors

		BitmapFontChar() = default;
		BitmapFontChar(const BitmapFontChar& other);
		BitmapFontChar(BitmapFontChar&& rval) noexcept;
		BitmapFontChar(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel,
			FontFaceBinaryFormat Format);
		~BitmapFontChar();


	public: // operators

		BitmapFontChar& operator=(const BitmapFontChar& other);
		BitmapFontChar& operator=(BitmapFontChar&& other) noexcept;


	public: // methods

		bool create(uint16_t Width, uint16_t Height, uint8_t BitsPerPixel,
			FontFaceBinaryFormat Format);
		void destroy();

		inline bool created() { return m_pData; }

		inline uint16_t getWidth() const { return m_iWidth; }
		inline uint16_t getHeight() const { return m_iHeight; }
		inline uint8_t getBitsPerPixel() const { return m_iBPP; }
		inline const uint8_t* getData() const { return m_pData; }
		inline size_t getDataSize() const { return m_iDataSize; }

		void setPixel(uint16_t X, uint16_t Y, uint32_t Value);
		uint32_t getPixel(uint16_t X, uint16_t Y);


	private: // methods

		/// <summary>
		/// Check if this character has any data<para/>
		/// Throws an <c>std::exception</c> if it doesn't
		/// </summary>
		void checkData();

		/// <summary>
		/// Check if a position is valid for this character<para/>
		/// Throws an <c>std::exception</c> if it isn't
		/// </summary>
		void checkPos(uint16_t iX, uint16_t iY);


	private: // variables

		uint8_t* m_pData = nullptr;
		size_t m_iDataSize = 0;
		uint16_t m_iWidth = 0;
		uint16_t m_iHeight = 0;
		uint16_t m_iHeightRounded = 0;
		uint8_t m_iBPP = 0;
		FontFaceBinaryFormat m_eFormat = FontFaceBinaryFormat::BitPlanes;

	};





	/// <summary>
	/// A simplified version of the <c>rl::FontFaceHeader</c> struct, containing only the data
	/// necessary for creating a font face via the <c>rl::BitmapFontFaceCreator</c> class
	/// </summary>
	struct FontFaceCreatorConfig
	{
		char32_t iFallbackChar;
		uint8_t iFaceVersion[4];
		uint16_t iWeight;
		uint8_t iPaddingFlags; // made up of the RL_FNT_PADFLAG_[...] defines
		bool bItalic;
		bool bGenericUse;
		bool bHighRes;
		bool bSymbols;
		bool bEmoji; // if set, bSymbols must also be set
		FontFaceClassification eClassification;
	};

	constexpr size_t len = sizeof(FontFaceCreatorConfig);


	class BitmapFontFaceCreator
	{
	public: // types

		using iterator = std::map<char32_t, BitmapFontChar>::iterator;
		using const_iterator = std::map<char32_t, BitmapFontChar>::const_iterator;


	public: // operators

		BitmapFontFaceCreator& operator=(BitmapFontFaceCreator&& rval) noexcept;
		BitmapFontFaceCreator& operator=(const BitmapFontFaceCreator& other);
		BitmapFontFaceCreator& operator=(const FontFaceClass& other);

		/// <summary>
		/// Throws an <c>std::exception</c> if there is no data for the given character
		/// </summary>
		const BitmapFontChar& operator[](char32_t ch) const;

		inline iterator begin() { return m_oChars.begin(); }
		inline const_iterator begin() const { return m_oChars.begin(); }

		inline iterator end() { return m_oChars.end(); }
		inline const_iterator end() const { return m_oChars.end(); }


	public: // methods

		BitmapFontFaceCreator() = default;
		BitmapFontFaceCreator(const BitmapFontFaceCreator& other);
		BitmapFontFaceCreator(BitmapFontFaceCreator&& rval) noexcept;
		BitmapFontFaceCreator(const FontFaceClass& other);



		/// <summary>
		/// Initialize the font face
		/// </summary>
		/// <param name="BitsPerPixel">
		/// [ignored if <c>BinaryFormat</c> is <c>RGB</c> or <c>RGBA</c>]
		/// </param>
		bool create(const FontFaceCreatorConfig& Config, FontFaceBinaryFormat BinaryFormat,
			uint8_t BitsPerPixel, uint16_t CharHeight, uint16_t GlobalCharWidth,
			const char* szFontFamName, const char* szFontFaceName,
			const char* szCopyright = nullptr);

		/// <summary>
		/// Clear all data, uninitialize the font face
		/// </summary>
		void clear();


		/// <summary>
		/// Get a reference to the configuration to read/update it
		/// </summary>
		inline FontFaceCreatorConfig& getConfig() { return m_oConfig; }

		/// <summary>
		/// Get the font's binary format
		/// </summary>
		inline FontFaceBinaryFormat getBinaryFormat() const { return m_eBinFormat; }

		/// <summary>
		/// Get the font's bits per pixel
		/// </summary>
		inline uint8_t getBitsPerPixel() const { return m_iBitsPerPixel; }

		/// <summary>
		/// Get the font's character height
		/// </summary>
		inline uint16_t getCharHeight()  const { return m_iCharHeight; }


		/// <summary>
		/// Has the font face been initialized?
		/// </summary>
		inline bool isCreated() { return m_bCreated; }


		/// <summary>
		/// Add/replace a single character<para />
		/// Throws an <c>std::exception</c> if <c>ch</c> is a noncharacter
		/// </summary>
		/// <param name="ch">= raw unicode value of the character to add/replace</param>
		void setChar(char32_t ch, const BitmapFontChar& data);

		/// <summary>
		/// Add a single character<para />
		/// Throws an <c>std::exception</c> if <c>ch</c> is a noncharacter
		/// </summary>
		/// <param name="ch">= raw unicode value of the character to add/replace</param>
		void setChar(char32_t ch, BitmapFontChar&& data);

		/// <summary>
		/// Create a new character via the font face settings, only manually specifying the minimum
		/// information necessary<para />
		/// WARNING: If the character already has data, it is discarded when calling this function
		/// <para />
		/// Throws an <c>std::exception</c> if <c>ch</c> is a noncharacter
		/// </summary>
		/// <param name="width">[ignored if font is fixed-width]</param>
		BitmapFontChar& createChar(char32_t ch, uint16_t width);


		/// <summary>
		/// Get a character's data<para />
		/// Throws an <c>std::exception</c> if the character isn't defined
		/// (check via method <c>containsChar()</c> beforehand)
		/// </summary>
		/// <param name="ch">= raw unicode value of the character</param>
		const BitmapFontChar& getChar(char32_t ch) const;


		/// <summary>
		/// Does the font face contain a certain character?
		/// </summary>
		/// <param name="ch">= raw unicode value of the character to add/replace</param>
		inline bool constainsChar(char32_t ch) const
		{
			return m_oChars.find(ch) == m_oChars.end();
		}


		inline const std::string& getFontFamName() const { return m_sFontFamName; }
		void setFontFamName(const char* szFontFamName);

		inline const std::string& getFontFaceName() const { return m_sFontFaceName; }
		void setFontFaceName(const char* szFontFaceName);

		inline const std::string& getCopyright() const { return m_sCopyright; }
		void setCopyright(const char* szCopyright);


		/// <summary>
		/// Load the font face from a file (via <c>FontFaceClass</c>)
		/// </summary>
		/// <returns>Was the font face loaded successfully?</returns>
		bool loadFromFile(const wchar_t* szFileName);

		/// <summary>
		/// Save the font face to a file
		/// </summary>
		/// <returns>Could the file be created?</returns>
		bool saveToFile(const wchar_t* szFileName) const;


	private: // static methods

		/// <summary>
		/// Make an <c>std::string</c> instance ASCII-only by replacing non-ASCII characters with
		/// <c>'?'</c>
		/// </summary>
		/// <param name="s"></param>
		void makeASCII(std::string& s);

		/// <summary>
		/// Check if a value is a noncharacter in Unicode<para />
		/// If it is, an <c>std::exception</c> is thrown
		/// </summary>
		/// <param name="ch"></param>
		void checkNoncharacter(char32_t ch);


	private: // variables

		bool m_bCreated = false;
		std::map<char32_t, BitmapFontChar> m_oChars;
		FontFaceCreatorConfig m_oConfig = {};

		FontFaceBinaryFormat m_eBinFormat = FontFaceBinaryFormat::BitPlanes;
		uint8_t m_iBitsPerPixel = 0;
		uint16_t m_iCharHeight = 0;
		uint16_t m_iGlobalCharWidth = 0;

		std::string m_sFontFamName;
		std::string m_sFontFaceName;
		std::string m_sCopyright;

	};

}





// #undef foward declared definitions

#endif // ROBINLE_GRAPHICS_FONTS_BITMAP_CREATOR