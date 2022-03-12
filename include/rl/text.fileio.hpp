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
		/// Windows Codepage 1252, often wrongly referred to as "ANSI"
		/// </summary>
		CP1252,
		/// <summary>
		/// UTF-8
		/// </summary>
		UTF8,
		/// <summary>
		/// UTF-8 with a Byte Order Mark
		/// </summary>
		UTF8BOM,
		/// <summary>
		/// UTF-16 (Little Endian)
		/// </summary>
		UTF16LE,
		/// <summary>
		/// UTF-16 (Big Endian)
		/// </summary>
		UTF16BE,
		/// <summary>
		/// UTF-32 (Little Endian)
		/// </summary>
		UTF32LE,
		/// <summary>
		/// UTF-32 (Big Endian)
		/// </summary>
		UTF32BE
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
		Macintosh
	};





	/// <summary>
	/// Guess the encoding of a text file.<para />
	/// <para />
	/// While it could always be Codepage 1252
	/// (due to all values of a byte being valid in this encoding), 
	/// it is only chosen if the file doesn't comply with any other encoding.<para />
	/// If the first few bytes represent a Byte Order Mark, it's assumed that this is no
	/// coincidence.
	/// </summary>
	bool GuessTextEncoding(const wchar_t* szFilepath, TextEncoding& eGuessedEncoding,
		bool bPreferUTF8overCP1252, bool bCheckWholeFile = false);

	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines,
		TextEncoding eEncoding);
	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines);

	bool WriteTextFile(const wchar_t* szFilePath, const std::vector<std::wstring>& oLines,
		bool bTrailingLineBreak = false, TextEncoding eEncoding = TextEncoding::UTF8BOM,
		LineBreak eLineBreak = LineBreak::Windows);





	class TextFileReader
	{
	public: // operators

		inline bool operator!() { return !m_oFile; }


	public: // methods

		TextFileReader() = default;
		TextFileReader(const wchar_t* szFilePath);
		TextFileReader(const wchar_t* szFilePath, TextEncoding eEncoding);
		virtual ~TextFileReader();

		/// <summary>
		/// Open a text file, guess the encoding
		/// </summary>
		void open(const wchar_t* szFilePath);
		/// <summary>
		/// Open a text file using an explicit encoding
		/// </summary>
		void open(const wchar_t* szFilePath, TextEncoding eEncoding);
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
		inline auto encoding() { return m_eEncoding; }
		inline auto linebreak() { return m_eLineBreak; }
		inline bool trailingLinebreak() { return m_bTrailingLinebreak; }


	private: // variables

		TextEncoding m_eEncoding = TextEncoding::UTF8;
		LineBreak m_eLineBreak = LineBreak::Windows;
		bool m_bTrailingLinebreak = false;
		std::basic_ifstream<uint8_t> m_oFile;
	};



	class TextFileWriter
	{
	public: // operators

		inline bool operator!() { return !m_oFile; }


	public: // methods

		TextFileWriter() = default;
		TextFileWriter(const wchar_t* szFilePath, TextEncoding eEncoding,
			LineBreak eLineBreak = LineBreak::Windows);
		virtual ~TextFileWriter();

		void open(const wchar_t* szFilePath, TextEncoding eEncoding,
			LineBreak eLineBreak = LineBreak::Windows);
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
		inline auto encoding() { return m_eEncoding; }
		inline auto linebreak() { return m_eLineBreak; }


	private: // variables

		TextEncoding m_eEncoding = TextEncoding::UTF8;
		LineBreak m_eLineBreak = LineBreak::Windows;
		std::basic_ofstream<uint8_t> m_oFile;
	};

}





// #undef foward declared definitions

#endif // ROBINLE_TEXT_FILEIO