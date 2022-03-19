/***************************************************************************************************
 FILE:	text.fileio.hpp
 CPP:	text.fileio.cpp
		unicode.cpp
 DESCR:	Code for working with text files in one of the multiple common encodings
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TEXT_FILEIO
#define ROBINLE_TEXT_FILEIO





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;


#include <fstream>
#include <string>
#include <vector>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// The most common text encodings
	/// </summary>
	enum class TextEncoding
	{
		/// <summary>
		/// Codepage/binary data.<para/>
		/// <para/>
		/// When detected, the data wasn't valid with any other encoding. Might also
		/// indicate a binary file.<para/>
		/// When encoding or decoding with this value, Codepage 1252 is used.
		/// </summary>
		Codepage,
		/// <summary>
		/// ASCII
		/// </summary>
		ASCII,
		/// <summary>
		/// UTF-8
		/// </summary>
		UTF8,
		/// <summary>
		/// UTF-16
		/// </summary>
		UTF16,
		/// <summary>
		/// UTF-32
		/// </summary>
		UTF32
	};

	/// <summary>
	/// The most common types of linebreak
	/// </summary>
	enum class LineBreak
	{
		/// <summary>
		/// Windows linebreak (<c>\r</c><c>\n</c>)
		/// </summary>
		Windows,
		/// <summary>
		/// UNIX linebreak (<c>\n</c>)
		/// </summary>
		UNIX,
		/// <summary>
		/// Macintosh linebreak (<c>\r</c>)
		/// </summary>
		Macintosh,

		/// <summary>
		/// Linebreak of the current operating system
		/// </summary>
		OS =
#if defined(_WIN32) || defined(_WIN64)
		Windows
#else // all modern mainstream operating systems except Windows use the UNIX linebreak ("\n")
		UNIX
#endif
	};



	namespace Flags
	{
		namespace TextFileInfo
		{
			constexpr uint8_t HasBOM = 1;

			constexpr uint8_t BigEndian = 2;
		}
	}

	struct TextFileInfo
	{
		TextEncoding eEncoding;
		LineBreak eLineBreaks;
		uint8_t iFlags;
	};

	constexpr TextFileInfo TextFileInfo_Codepage(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_ASCII(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF8(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF8BOM(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF16BE(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF16LE(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF32BE(LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF32LE(LineBreak eLineBreaks = LineBreak::OS) noexcept;


	namespace Flags
	{
		namespace TextFileInfo_Get
		{
			constexpr uint8_t ConsequentLineBreaks = 1;
		}
	}

	struct TextFileInfo_Get
	{
		uint8_t iFlags;
	};

	namespace Flags
	{
		namespace GetTextFileInfo
		{
			/// <summary>
			/// Only check the minimum required for detecting the encoding<para />
			/// Doesn't check the linebreak style.<para />
			/// When a BOM is found, it's assumed to be correct.
			/// </summary>
			constexpr uint8_t CheckMinimum = 1;
		}
	}

	/// <summary>
	/// Get information about a text file
	/// </summary>
	/// <param name="iFlags">Flags from <c>Flags::GetTextFileInfo</c></param>
	/// <returns>Did the check succeed?</returns>
	bool GetTextFileInfo(const wchar_t* szFilePath, TextFileInfo& oDest, TextFileInfo_Get& oDestEx,
		uint8_t iFlags = 0);





	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding);
	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines);

	bool WriteTextFile(const wchar_t* szFilePath, const std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding = TextFileInfo_UTF8BOM(), bool bTrailingLineBreak = false);





	class TextFileReader
	{
	public: // operators

		inline bool operator!() { return !m_oFile; }


	public: // methods

		TextFileReader() = default;
		TextFileReader(const wchar_t* szFilePath);
		TextFileReader(const wchar_t* szFilePath, const TextFileInfo& oEncoding);
		virtual ~TextFileReader();

		/// <summary>
		/// Open a text file, guess the encoding
		/// </summary>
		void open(const wchar_t* szFilePath);
		/// <summary>
		/// Open a text file using an explicit encoding
		/// </summary>
		void open(const wchar_t* szFilePath, const TextFileInfo& oEncoding);
		/// <summary>
		/// Close an opened text file
		/// </summary>
		inline void close() { m_oFile.close(); }

		/// <summary>
		/// Read a single character
		/// </summary>
		void read(char32_t& cDest);
		/// <summary>
		/// Read a defined number of characters into a <c>std::wstring</c>
		/// </summary>
		void read(std::wstring& sDest, size_t len);
		/// <summary>
		/// Read until the EOF/the next linebreak
		/// </summary>
		void readLine(std::wstring& sDest);
		/// <summary>
		/// Read all lines until the EOF
		/// </summary>
		void readLines(std::vector<std::wstring>& oLines);

		/// <summary>
		/// Has the EOF been reached?
		/// </summary>
		inline bool eof() { return m_oFile.eof(); }
		/// <summary>
		/// Is a file opened?
		/// </summary>
		inline bool isOpen() { return m_oFile.is_open(); }
		/// <summary>
		/// Get the encoding used for reading the file.<para/>
		/// Was either explicitly set or automatically detected.
		/// </summary>
		inline auto encoding() { return m_oEncoding; }
		inline bool trailingLinebreak() { return m_bTrailingLinebreak; }


	private: // variables

		TextFileInfo m_oEncoding{};
		bool m_bTrailingLinebreak = false;
		std::basic_ifstream<uint8_t> m_oFile;
	};



	class TextFileWriter
	{
	public: // operators

		inline bool operator!() { return !m_oFile; }


	public: // methods

		TextFileWriter() = default;
		TextFileWriter(const wchar_t* szFilePath, const TextFileInfo& oEncoding);
		virtual ~TextFileWriter();

		void open(const wchar_t* szFilePath, const TextFileInfo& oEncoding);
		inline void close() { m_oFile.close(); }

		void write(char32_t c);
		void write(const wchar_t* szText, size_t len = 0);
		inline void write(const std::wstring& sText) { write(sText.c_str(), sText.length()); }
		void writeLine(const wchar_t* szText, size_t len = 0);
		inline void writeLine(const std::wstring& sText)
		{
			writeLine(sText.c_str(), sText.length());
		}
		void writeLines(const std::vector<std::wstring>& oLines, bool bTrailingLinebreak = true);

		inline bool isOpen() { return m_oFile.is_open(); }
		inline auto encoding() { return m_oEncoding; }


	private: // variables

		TextFileInfo m_oEncoding = TextFileInfo_UTF8BOM();
		std::basic_ofstream<uint8_t> m_oFile;
	};










	//==============================================================================================
	// DEFINITIONS

	constexpr TextFileInfo TextFileInfo_Codepage(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::Codepage;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = 0;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_ASCII(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::ASCII;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = 0;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF8(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF8;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = 0;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF8BOM(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF8;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = flags::HasBOM;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF16BE(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF16;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = flags::HasBOM | flags::BigEndian;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF16LE(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF16;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = flags::HasBOM; // | LittleEndian

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF32BE(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF32;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = flags::HasBOM | flags::BigEndian;

		return result;
	}

	constexpr TextFileInfo TextFileInfo_UTF32LE(LineBreak eLineBreaks) noexcept
	{
		namespace flags = Flags::TextFileInfo;

		TextFileInfo result{};
		result.eEncoding = TextEncoding::UTF32;
		result.eLineBreaks = eLineBreaks;
		result.iFlags = flags::HasBOM; // | LittleEndian

		return result;
	}

}





// #undef foward declared definitions

#endif // ROBINLE_TEXT_FILEIO