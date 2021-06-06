/***************************************************************************************************
 FILE:	graphics.fonts.bitmap.hpp
 CPP:	graphics.fonts.bitmap.cpp
 DESCR:	Class containing a bitmap font in one size
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_FONTS_BITMAP
#define ROBINLE_GRAPHICS_FONTS_BITMAP





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;


#include <map>
#include <vector>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Pixel representation of a single character in a bitmap font
	/// </summary>
	class BitmapChar
	{
	public: // methods

		BitmapChar() {}
		BitmapChar(const BitmapChar& other) { assign(other); }
		~BitmapChar() { destroy(); }


		// write methods

		void create(uint8_t width, uint8_t height, uint8_t BitsPerPixel);
		void create(uint8_t width, uint8_t height, uint8_t BitsPerPixel, uint8_t* data);
		void destroy();

		void setPixel(uint8_t x, uint8_t y, uint8_t val);

		void assign(const BitmapChar& other);
		BitmapChar& operator=(const BitmapChar& other);


		// readonly methods

		inline bool isValid() const { return m_pData != nullptr; }
		inline uint8_t getBitsPerPixel() const { return m_iBitsPerPixel; }
		inline uint8_t getWidth() const { return m_iWidth; }
		inline uint8_t getHeight() const { return m_iHeight; }
		uint8_t getPixel(uint8_t x, uint8_t y) const;


	private: // methods

		void createInternal(uint8_t iWidth, uint8_t iHeight, uint8_t iBitsPerPixel);
		void checkValid() const; // throws std::exception if m_pData is nullptr
		void checkPos(uint8_t iX, uint8_t iY) const; // throws std::exception if position is invalid
		bool validPos(uint8_t iX, uint8_t iY) const;


	private: // variables

		uint8_t* m_pData = nullptr;
		uint8_t m_iBitsPerPixel = 0; // must be between 1 and 8
		uint8_t m_iWidth = 0, m_iHeight = 0;
		uint8_t m_iColOffset = 0; // bytes per column per bit layer
		uint16_t m_iLayerOffset = 0; // bytes per bit layer

	};





	// font weight definitions
#define RL_FW_UNDEF			0 // undefined weight
#define RL_FW_THIN			1
#define RL_FW_EXTRALIGHT	2
#define RL_FW_LIGHT			3
#define RL_FW_REGULAR		4
#define RL_FW_MEDIUM		5
#define RL_FW_SEMIBOLD		6
#define RL_FW_BOLD			7
#define RL_FW_EXTRABOLD		8
#define RL_FW_BLACK			9

	// font flag definitions
#define RL_FF_MONOSPACE		0b00000001
#define RL_FF_ITALIC		0b00000010
#define RL_FF_OUTLINE		0b00000100

	class BitmapFontFace
	{
	public: // types

		struct Meta
		{
			uint8_t weight; // 0 through 9, see RL_FW_[]
			uint8_t BitsPerPixel; // must be between 1 and 8
			uint8_t height;
			uint8_t globalwidth; // flag RL_FF_MONOSPACE determines if this is used
			uint8_t flags; // see RL_FF_[]
		};


	public: // methods

		BitmapFontFace() {}
		~BitmapFontFace() { destroy(); }



		void create(Meta meta);
		void destroy();


		/// <summary>
		/// Add a character to this font<para/>
		/// <c>val</c> will be copied into this font<para/>
		/// Function will fail if a bitmap character already exists for <c>codepoint</c>
		/// </summary>
		bool addChar(uint32_t codepoint, BitmapChar& val);
		void removeChar(uint32_t codepoint);


		uint32_t decodeChar(char ch) const;
		uint32_t decodeChar(const wchar_t* ch) const;

		bool contains(uint32_t codepoint) const;


		/// <summary>
		/// Get character data
		/// </summary>
		/// <returns>
		/// If there is data for <c>codepoint</c> --> pointer to data<para/>
		/// Else --> nullptr
		/// </returns>
		BitmapChar* getChar(uint32_t codepoint);

		/// <summary>
		/// Get character data
		/// </summary>
		/// <returns>
		/// If there is data for <c>codepoint</c> --> pointer to data<para/>
		/// Else --> nullptr
		/// </returns>
		const BitmapChar* getChar(uint32_t codepoint) const;


		/// <summary>
		/// Get the generic fallback char fitted to this bitmap font ("[]")<para/>
		/// Throws <c>std::exception</c> when this font is invalid
		/// </summary>
		inline BitmapChar* getFallbackChar() const { return m_oFallbackChar; }

		inline bool isValid() const { return m_bCreated; }



		// iterators

		inline std::map<uint32_t, BitmapChar*>::iterator begin() { return m_oChars.begin(); }
		inline std::map<uint32_t, BitmapChar*>::const_iterator begin() const
		{
			return m_oChars.begin();
		}
		inline std::map<uint32_t, BitmapChar*>::iterator end() { return m_oChars.end(); }
		inline std::map<uint32_t, BitmapChar*>::const_iterator end() const
		{
			return m_oChars.end();
		}


	private: // variables

		std::map<uint32_t, BitmapChar*> m_oChars;
		bool m_bCreated = false;
		Meta m_oMeta = {};
		uint8_t m_iWhitespaceWidth = 0;
		BitmapChar* m_oFallbackChar = nullptr;

	};





	// font family flag definitions
#define RL_FFF_SERIF		0b00000001
#define RL_FFF_MONOSPACE	0b00000010
#define RL_FFF_HANDWRITING	0b00000100

	/// <summary>
	/// Class representation of a .rlFON file, with possibly multiple <c>BitmapFontFace</c> variants
	/// </summary>
	class BitmapFontFaceFile
	{
	public: // types

		struct Meta
		{
			char* szFamilyName;
			uint8_t flags; // see RL_FFF_[]
		};

		struct FileInfo
		{
			Meta meta;
			std::vector<BitmapFontFace::Meta> fonts;
		};


	public: // methods

		/// <summary>
		/// Validate a rlFON file
		/// </summary>
		static bool validateFile(const wchar_t* path);

		/// <summary>
		/// Read metadata and font list from a file<para/>
		/// <c>dest</c> will be cleared before being rewritten
		/// </summary>
		static bool getFileInfo(const wchar_t* path, FileInfo* dest);

		void create(Meta meta);
		void destroy();

		bool loadFromFile(const wchar_t* path);
		bool saveToFile(const wchar_t* path);


	private: // variables

		std::vector<BitmapFontFace*> m_oFonts;
		bool m_bCreated = false;
		Meta m_oMeta = {};

	};

}





// #undef foward declared definitions

#endif // ROBINLE_GRAPHICS_FONTS_BITMAP