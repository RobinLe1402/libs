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
	/// The most common text encodings.
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
	/// The most common types of linebreak.
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
	constexpr TextFileInfo TextFileInfo_ASCII   (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF8    (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF8BOM (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF16BE (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF16LE (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF32BE (LineBreak eLineBreaks = LineBreak::OS) noexcept;
	constexpr TextFileInfo TextFileInfo_UTF32LE (LineBreak eLineBreaks = LineBreak::OS) noexcept;


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





	/// <summary>
	/// Read an entire text file, use a certain encoding.
	/// </summary>
	/// <param name="szFilePath">The text file to read.</param>
	/// <param name="oLines">The variable the lines should be written to.</param>
	/// <param name="oEncoding">
	/// The text encoding to use. Member <c>eLineBreaks</c> is ignored.
	/// </param>
	/// <returns>Could the text be read?</returns>
	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding);

	/// <summary>
	/// Read an entire text file, automatically determine the encoding.
	/// </summary>
	/// <param name="szFilePath">The text file to read.</param>
	/// <param name="oLines">The variable the lines should be written to.</param>
	/// <returns>Could the text be read?</returns>
	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines);

	/// <summary>
	/// Write a text file from strings, use a certain encoding.
	/// </summary>
	/// <param name="szFilePath">The text file to create.</param>
	/// <param name="oLines">The text to write.</param>
	/// <param name="oEncoding">The encoding to use.</param>
	/// <param name="bTrailingLineBreak">Should the text file end on an empty line?</param>
	/// <returns>Could the text file be created?</returns>
	bool WriteTextFile(const wchar_t* szFilePath, const std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding = TextFileInfo_UTF8BOM(), bool bTrailingLineBreak = false);





	/// <summary>
	/// A reader for text files.
	/// </summary>
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
		/// Open a text file, guess the encoding.
		/// </summary>
		void open(const wchar_t* szFilePath);
		/// <summary>
		/// Open a text file using an explicit encoding.
		/// </summary>
		void open(const wchar_t* szFilePath, const TextFileInfo& oEncoding);
		/// <summary>
		/// Close an opened text file.
		/// </summary>
		inline void close() { m_oFile.close(); }

		/// <summary>
		/// Read a single character.
		/// </summary>
		void read(char32_t& cDest);
		/// <summary>
		/// Read a defined number of characters into a <c>std::wstring</c>.
		/// </summary>
		void read(std::wstring& sDest, size_t len);
		/// <summary>
		/// Read until the EOF/the next linebreak.
		/// </summary>
		void readLine(std::wstring& sDest);
		/// <summary>
		/// Read all lines until the EOF.
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



	/// <summary>
	/// A writer for text files.
	/// </summary>
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


		/// <summary>
		/// Write a single character.
		/// </summary>
		/// <param name="c">The character to be written, as UTF-32 (= raw unicode value).</param>
		void write(char32_t c);

		/// <summary>
		/// Write a string.
		/// </summary>
		/// <param name="szText">The string to be written.</param>
		/// <param name="len">
		/// The length, in characters, of the string pointed to by the <c>szText</c> parameter.
		/// <para/>
		/// Can be zero, in which case the string is assumed to be null-terminated.
		/// </param>
		void write(const wchar_t* szText, size_t len = 0);

		/// <summary>
		/// Write a string.
		/// </summary>
		inline void write(const std::wstring& sText) { write(sText.c_str(), sText.length()); }

		/// <summary>
		/// Write a string, followed by a linebreak.
		/// </summary>
		/// <param name="szText">The string to be written.</param>
		/// <param name="len">
		/// The length, in characters, of the string pointed to by the <c>szText</c> parameter.
		/// <para/>
		/// Can be zero, in which case the string is assumed to be null-terminated.
		/// </param>
		void writeLine(const wchar_t* szText, size_t len = 0);

		/// <summary>
		/// Write a string, followed by a linebreak.
		/// </summary>
		inline void writeLine(const std::wstring& sText)
		{
			writeLine(sText.c_str(), sText.length());
		}

		/// <summary>
		/// Write multiple strings as lines.
		/// </summary>
		/// <param name="oLines">
		/// The text to be written.<para/> Each string is treated as a line.
		/// </param>
		/// <param name="bTrailingLinebreak">
		/// Should a linebreak be written after the last line?
		/// </param>
		void writeLines(const std::vector<std::wstring>& oLines, bool bTrailingLinebreak = true);


		/// <summary>
		/// Is a file currently opened for writing?
		/// </summary>
		inline bool isOpen() { return m_oFile.is_open(); }

		/// <summary>
		/// The encoding used for writing to the file.
		/// </summary>
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