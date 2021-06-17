/***************************************************************************************************
 FILE:	parser.hpp
 CPP:	parser.cpp
 DESCR:	Parser for FON files (= 16-bit executables)
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_FON_PARSER
#define ROBINLE_FON_PARSER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#include <Windows.h> // required by <mutex>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#pragma comment(lib, "version.lib")



//==================================================================================================
// DECLARATION
namespace rl
{



	//----------------------------------------------------------------------------------------------
	// CONSTANTS
	
	namespace FontWeight
	{
		constexpr uint16_t
			Thin = 100,
			ExtraLight = 200,
			Light = 300,
			Regular = 400,
			Medium = 500,
			SemiBold = 600,
			Bold = 700,
			ExtraBold = 800,
			Black = 900;
	}


	
	//----------------------------------------------------------------------------------------------
	// HELP FUNCTIONS

	/// <summary>
	/// Convert a FON character set identifier to a Windows codepage identifier
	/// </summary>
	/// <returns>Could a codepage be found?</returns>
	bool CharSetToCodePage(uint8_t charset, uint16_t& codepage);

	/// <summary>
	/// Convert a character of a FON character set into a unicode codepoint
	/// </summary>
	/// <returns>Did the conversion succeed?</returns>
	bool CharSetIDToCodePoint(uint8_t ch, uint8_t charset, uint32_t& codepoint);

	/// <summary>
	/// Convert a unicode codepoint into a character of a FON character set
	/// </summary>
	/// <returns>Did the conversion succeed?</returns>
	bool CodePointToCharSetID(uint32_t codepoint, uint8_t charset, uint8_t& ch);



	//----------------------------------------------------------------------------------------------
	// HELP STRUCTS

	/// <summary>
	/// Language and codepage of a VERSIONINFO resource's strings
	/// </summary>
	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;

		bool operator<(const LANGANDCODEPAGE& other) const;
	};

	/// <summary>
	/// String data of a <c>VERSIONINFO</c> resource<para/>
	/// Instead of being empty, strings that weren't present do consist of a single null character
	/// </summary>
	struct VERSIONINFO_STRINGS
	{
		std::wstring Comments;
		std::wstring CompanyName;
		std::wstring FileDescription;
		std::wstring FileVersion;
		std::wstring InternalName;
		std::wstring LegalCopyright;
		std::wstring LegalTrademarks; // uncommon
		std::wstring OriginalFilename;
		std::wstring ProductName;
		std::wstring ProductVersion;
		std::wstring PrivateBuild; // uncommon
		std::wstring SpecialBuild; // uncommon

		VERSIONINFO_STRINGS();
	};

	/// <summary>
	/// Contents of a <c>VERSIONINFO</c> resource
	/// </summary>
	struct VERSIONINFO
	{
		uint32_t Signature;
		uint32_t StructVersion;
		uint64_t FileVersion;
		uint64_t ProductVersion;
		uint32_t FileFlagsMask;
		uint32_t FileFlags;
		uint32_t FileOS;
		uint32_t FileType;
		uint32_t FileSubtype;
		uint32_t FileDate;

		std::map<LANGANDCODEPAGE, VERSIONINFO_STRINGS> LangStrings;
	};

#pragma pack(push, 1) // disable padding, as these structs are directly assigned binary data
	struct FONTHDR
	{
		WORD  dfVersion;
		DWORD dfSize;
		CHAR  dfCopyright[60];
		WORD  dfType;
		WORD  dfPoints;
		WORD  dfVertRes;
		WORD  dfHorizRes;
		WORD  dfAscent;
		WORD  dfInternalLeading;
		WORD  dfExternalLeading;
		BYTE  dfItalic;
		BYTE  dfUnderline;
		BYTE  dfStrikeOut;
		WORD  dfWeight;
		BYTE  dfCharSet;
		WORD  dfPixWidth;
		WORD  dfPixHeight;
		BYTE  dfPitchAndFamily;
		WORD  dfAvgWidth;
		WORD  dfMaxWidth;
		BYTE  dfFirstChar;
		BYTE  dfLastChar;
		BYTE  dfDefaultChar;
		BYTE  dfBreakChar;
		WORD  dfWidthBytes;
		DWORD dfDevice;
		DWORD dfFace;
		DWORD dfReserved; // = dfBitsPointer

		/*
		CHAR  szDeviceName; // must be represented by a std::string or a char*
		CHAR  szFaceName; // must be represented by a std::string or a char*
		*/
	};
#pragma pack(pop)

	/// <summary>
	/// A general font header, alongside the two strings inclded in the file
	/// </summary>
	struct FONTHDR_STRINGS
	{
		FONTHDR oHeader;
		std::string sDeviceName;
		std::string sFaceName;
	};

	/// <summary>
	/// A single <c>FONTDIR</c> resource entry
	/// </summary>
	struct FONTDIRENTRY
	{
		// struct DIRENTRY
		// {
		WORD  fontOrdinal; // file-unique font ID
		// }

		FONTHDR oHeader;

		std::string sDeviceName;
		std::string sFaceName;
	};





	//----------------------------------------------------------------------------------------------
	// MAIN CLASSES

#define RL_CP_UNKNOWN 0xFFFF

#define RL_FONPARSER_E_NOERROR				0 // no error occured
#define RL_FONPARSER_E_FILEERROR			1 // file doesn't exist or couldn't be read
#define RL_FONPARSER_E_NOFONTFILE			2 // file is no font
#define RL_FONPARSER_E_NORASTERFONTFILE		3 // file is a font, but no raster font
#define RL_FONPARSER_E_NOFONTRESOURCE		4 // no FONT resource found
#define RL_FONPARSER_E_NORASTERFONTRESOURCE	5 // no raster FONT resource found
#define RL_FONPARSER_E_UNKNOWNVERSION		6 // the FNT version was not 2

#define RL_FONPARSER_W_NOVERSIONINFO	0b001 // no VERSIONINFO resource found
#define RL_FONPARSER_W_WRONGLANG		0b010 // VERSIONINFO language table was incorrect
#define RL_FONPARSER_W_INVALIDDATA		0b100 // invalid data in a FONT resource

	// forward declarations
	class MicrosoftRasterFont;
	class MicrosoftFONParser;



	/// <summary>
	/// A single character from a Microsoft raster font<para/>
	/// Must be obtained from a <c>MicrosoftRasterFont</c> object
	/// </summary>
	class MicrosoftRasterChar
	{
		friend class MicrosoftRasterFont;

	public: // methods

		MicrosoftRasterChar() {}
		MicrosoftRasterChar(const MicrosoftRasterChar& other);
		~MicrosoftRasterChar();

		/// <summary>
		/// Delete this char's data
		/// </summary>
		void clear();

		/// <summary>
		/// Does this object hold any data?
		/// </summary>
		bool hasData() const;



		/// <summary>
		/// Get the pixel at a certain location<para/>
		/// Return value is only valid if the object holds any data
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool getPixel(uint16_t x, uint16_t y) const;

		/// <summary>
		/// Get the width of this character<para/>
		/// Return value is only valid if the object holds any data
		/// </summary>
		inline uint16_t getWidth() const { return m_iCharWidth; }

		/// <summary>
		/// Get the width of this character<para/>
		/// Return value is only valid if the object holds any data
		/// </summary>
		inline uint16_t getHeight() const { return m_iCharHeight; }


	public: // operators

		MicrosoftRasterChar& operator=(const MicrosoftRasterChar& other);


	private: // methods

		// --> objects with data must be obtained from MicrosoftRasterFont
		MicrosoftRasterChar(uint16_t iWidth, uint16_t iHeight, uint8_t* pData, size_t len);


	private: // variables

		uint8_t* m_pData = nullptr;
		size_t m_iDataSize = 0;

		uint16_t m_iCharWidth = 0;
		uint16_t m_iCharHeight = 0;

	};



	/// <summary>
	/// Microsoft raster font data
	/// </summary>
	class MicrosoftRasterFont final
	{
	public: // methods

		MicrosoftRasterFont() {}
		MicrosoftRasterFont(const uint8_t* pData, size_t size);
		MicrosoftRasterFont(const MicrosoftRasterFont& other);
		~MicrosoftRasterFont();


		/// <summary>
		/// Create from raw data
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool create(const uint8_t* pData, size_t size);

		/// <summary>
		/// Delete this font's data
		/// </summary>
		void clear();

		/// <summary>
		/// Does this object hold any data?
		/// </summary>
		bool hasData() const;

		/// <summary>
		/// Get a pointer to this font's raw data
		/// </summary>
		/// <param name="ptr">= pointer that is supposed to point to the raw data afterwards</param>
		/// <param name="size">= size (in bytes) of the data pointed to by <c>ptr</c></param>
		/// <returns>Did the method succeed?</returns>
		bool getData(const uint8_t*& ptr, size_t& size) const;



		/// <summary>
		/// Get this font's character set<para/>
		/// The codepage has nothing to do with "modern" Windows codepages.<para/>
		/// Return value is only valid if this object contains data
		/// </summary>
		inline uint8_t getCharSet() const { return m_oHeader.dfCharSet; }

		/// <summary>
		/// Get the font's height<para/>
		/// Return value is only valid if this object contains data
		/// </summary>
		inline uint16_t getHeight() const { return m_iHeight; }

		/// <summary>
		/// Get a character's data
		/// </summary>
		/// <param name="ch">= charset ID of the searched char; not unicode!</param>
		/// <param name="dest">= object that should receive a copy of the character's data</param>
		/// <returns>Did the method succeed?</returns>
		bool getChar(uint8_t ch, MicrosoftRasterChar& dest) const;

		/// <summary>
		/// Get the font's header data
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool getHeader(FONTHDR& dest) const;

		/// <summary>
		/// Get the font's device name
		/// </summary>
		/// <returns><c>nullptr</c> if no data was parsed</returns>
		const char* deviceName() const;

		/// <summary>
		/// Get the font's face name
		/// </summary>
		/// <returns><c>nullptr</c> if no data was parsed</returns>
		const char* faceName() const;

		/// <summary>
		/// Does this font contain a certain character?
		/// </summary>
		/// <param name="ch">= charset ID of the searched char; not unicode!</param>
		bool containsChar(uint8_t ch) const;

		/// <summary>
		/// If there was an error, this method returns the correlating error code<para/>
		/// The error can be evaluated by using the <c>RL_FONPARSER_E_</c> defines</c><para/>
		/// Return value is <c>RL_FONPARSER_E_NOERROR</c> (0) if no error occured
		/// </summary>
		inline uint8_t getError() const { return m_iError; }


	public: // operators

		MicrosoftRasterFont& operator=(const MicrosoftRasterFont& other);


	private: // variables

		bool m_bData = false;
		uint8_t* m_pData = nullptr;
		size_t m_iDataSize = 0;
		FONTHDR m_oHeader = {};
		std::map<uint8_t, MicrosoftRasterChar> m_oChars;
		uint16_t m_iHeight = 0;
		uint8_t m_iError = RL_FONPARSER_E_NOERROR;

	};



	/// <summary>
		/// A parser for the Microsoft FON format (raster fonts)
		/// </summary>
	class MicrosoftFONParser final
	{
	public: // methods

		MicrosoftFONParser() {}
		~MicrosoftFONParser();


		/// <summary>
		/// Parse a FON file
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool parse(const wchar_t* szPath);

		/// <summary>
		/// Clear all parsed data
		/// </summary>
		void clear();

		/// <summary>
		/// Does this object hold any data?
		/// </summary>
		bool containsData() const;



		/// <summary>
		/// Get the contents of the <c>VERSIONINFO</c> resource
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool getVersionInfo(VERSIONINFO& dest) const;

		/// <summary>
		/// Get all entries of the <c>FONTDIR</c> resource (headers of all included fonts
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool getFontDir(std::vector<FONTDIRENTRY>& dest) const;

		/// <summary>
		/// Get a certain font
		/// </summary>
		/// <returns>Did the method succeed?</returns>
		bool getFont(WORD fontOrdinal, MicrosoftRasterFont& dest) const;

		/// <summary>
		/// Get the error that occured on the last call to <c>parse()<para/>
		/// The error can be evaluated by using the <c>RL_FONPARSER_E_</c> defines</c><para/>
		/// Return value is <c>RL_FONPARSER_E_NOERROR</c> (0) if no error occured
		/// </summary>
		uint8_t getParseError();

		/// <summary>
		/// Get the warning flags for the current file<para/>
		/// The warnings flags can be evaluated by using the <c>RL_FONPARSER_W_</c> defines
		/// </summary>
		uint8_t getWarnings();


	private: // variables

		mutable std::mutex m_mux; // for thread safety
		bool m_bParsed = false;
		uint8_t m_iError = RL_FONPARSER_E_NOERROR;
		uint8_t m_iWarnings = 0;

		VERSIONINFO m_oVI = {};
		std::map<WORD, MicrosoftRasterFont> m_oFonts;

	};

}





// #undef foward declared definitions

#endif // ROBINLE_FON_PARSER