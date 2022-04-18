#include "rl/text.fileio.hpp"

#include "rl/data.endian.hpp"
#include "rl/unicode.hpp"

#include <codecvt>
#include <fstream>
#include <locale>





namespace rl
{

	/// <summary>
	/// A table of the Unicode values of values 0x80 through 0x9F of Codepage 1252
	/// </summary>
	const wchar_t cCP1252_Table[] =
	{
		L'\u20AC', L'\u0000', L'\u201A', L'\u0192', L'\u201E', L'\u2026', L'\u2020', L'\u2021',
		L'\u02C6', L'\u2030', L'\u0160', L'\u2039', L'\u0152', L'\u0000', L'\u017D', L'\u0000',
		L'\u0000', L'\u2018', L'\u2019', L'\u201C', L'\u201D', L'\u2022', L'\u2013', L'\u2014',
		L'\u02DC', L'\u2122', L'\u0161', L'\u203A', L'\u0153', L'\u0000', L'\u017E', L'\u0178'
	};


	bool GetTextFileInfo(const wchar_t* szFilePath, TextFileInfo& oDest, TextFileInfo_Get& oDestEx,
		uint8_t iFlags)
	{
		namespace flags = Flags::TextFileInfo;
		namespace flagsEx = Flags::TextFileInfo_Get;

		oDest = {};
		oDestEx = {};

		std::basic_ifstream<uint8_t> file(szFilePath, std::ios::binary);
		if (!file)
			return false; // couldn't open file

		if (file.eof()) // file empty --> assume ASCII with OS linebreaks
		{
			oDest.eEncoding = TextEncoding::ASCII;
			return true;
		}





		//------------------------------------------------------------------------------------------
		// CHECK BOM

		uint8_t iByte = 0;

		// read the first byte
		file.read(&iByte, 1);

		if (file.eof())
		{
			// only ASCII (part of UTF-8) and codepages support single-byte characters.

			if (iByte & 0x80)
				oDest.eEncoding = TextEncoding::Codepage;
			else
				oDest.eEncoding = TextEncoding::ASCII;

			return true;
		}

		/*
		  BOM POSSIBILITIES:

		  0xFE 0xFF           --> UTF-16 BE
		  0xEF 0xBB 0xBF      --> UTF-8 BOM
		  0xFF 0xFE 0x00 0x00 --> UTF-32 LE
		  0xFF 0xFE           --> UTF-16 LE
		  0x00 0x00 0xFE 0xFF --> UTF-32 BE
		*/

		switch (iByte)
		{
		case 0xFE: // UTF-16 BE?
			file.read(&iByte, 1);
			if (iByte == 0xFF)
			{
				oDest.eEncoding = TextEncoding::UTF16;
				oDest.iFlags = flags::HasBOM | flags::BigEndian;
			}
			else
				oDest.eEncoding = TextEncoding::Codepage;
			break;
		case 0xEF:
			file.read(&iByte, 1);
			if (iByte == 0xBB)
			{
				if (file.eof())
				{
					oDest.eEncoding = TextEncoding::Codepage;
					break;
				}
				file.read(&iByte, 1);
				if (iByte == 0xBF)
				{
					oDest.eEncoding = TextEncoding::UTF8;
					oDest.iFlags = flags::HasBOM;
				}
				else
					oDest.eEncoding = TextEncoding::Codepage;
			}
			else
				oDest.eEncoding = TextEncoding::Codepage;
			break;
		case 0xFF:
			file.read(&iByte, 1);
			if (file.eof() || iByte != 0xFE)
			{
				oDest.eEncoding = TextEncoding::Codepage;
				break;
			}

			// might be UTF-16 LE or UTF-32 LE

			if (file.eof())
			{
				oDest.eEncoding = TextEncoding::UTF16;
				oDest.iFlags = flags::HasBOM; // | LittleEndian
				return true; // whole file was already checked anyways
			}
			file.read(&iByte, 1);
			if (file.eof())
			{
				// The file was 3 bytes long.
				// Neither UTF-16 nore UTF-32 support single-byte characters
				// --> must be a codepage
				oDest.eEncoding = TextEncoding::Codepage;
				return true; // whole file was already checked anyways
			}
			if (iByte != 0x00)
			{
				oDest.eEncoding = TextEncoding::UTF16;
				oDest.iFlags = flags::HasBOM; // | LittleEndian
				break;
			}
			file.read(&iByte, 1);
			if (iByte != 0x00)
			{
				oDest.eEncoding = TextEncoding::UTF16;
				oDest.iFlags = flags::HasBOM; // | LittleEndian
				break;
			}
			oDest.eEncoding = TextEncoding::UTF32;
			oDest.iFlags = flags::HasBOM; // | LittleEndian
			break;
		case 0x00:
			// only valid BOM would be UTF-32 BE
			file.read(&iByte, 1);
			if (file.eof() || iByte != 0x00)
			{
				oDest.eEncoding = TextEncoding::Codepage;
				break;
			}
			file.read(&iByte, 1);
			if (file.eof() || iByte != 0xFE)
			{
				oDest.eEncoding = TextEncoding::Codepage;
				break;
			}
			file.read(&iByte, 1);
			if (iByte != 0xFF)
			{
				oDest.eEncoding = TextEncoding::Codepage;
				break;
			}
			oDest.eEncoding = TextEncoding::UTF32;
			oDest.iFlags = flags::HasBOM | flags::BigEndian;
			break;
		default:
			// ASCII as placeholder --> check later if ASCII, UTF-8 or Codepage
			oDest.eEncoding = TextEncoding::ASCII;
		}





		//------------------------------------------------------------------------------------------
		// CHECK FILESIZE RESTRICTIONS

		file.seekg(0, std::ios::end);
		const size_t iFilesize = file.tellg();

		switch (oDest.eEncoding)
		{
		case TextEncoding::UTF32:
			if (iFilesize % 4)
			{
				if (iFilesize % 2 || oDest.iFlags & flags::BigEndian)
					oDest.eEncoding = TextEncoding::Codepage;
				else
					oDest.eEncoding = TextEncoding::UTF16; // might be UTF-16 LE instead
			}
			break;
		case TextEncoding::UTF16:
			if (iFilesize % 2)
				oDest.eEncoding = TextEncoding::Codepage;
			break;
		}





		if (oDest.eEncoding != TextEncoding::ASCII &&
			(iFlags & Flags::GetTextFileInfo::CheckMinimum))
			return true;





		//------------------------------------------------------------------------------------------
		// CHECK FILE CONTENTS (FORMALLY)

		// skip BOM
		uint8_t lenBOM;
		switch (oDest.eEncoding)
		{
		case TextEncoding::UTF8:
			lenBOM = 3;
			break;
		case TextEncoding::UTF16:
			lenBOM = 2;
			break;
		case TextEncoding::UTF32:
			lenBOM = 4;
			break;

		default:
			lenBOM = 0;
		}
		file.clear();
		file.seekg(lenBOM);
		if (file.eof())
			return true; // nothing more to check


		// check if the endian must be swapped
		bool bSwapEndian = false;
		if constexpr (BigEndian) // system is big endian
			bSwapEndian = (oDest.iFlags & flags::BigEndian) == 0;
		else // system is little endian
			bSwapEndian = oDest.iFlags & flags::BigEndian;


		// check file contents
		// if a codepage was detected, the contents are valid anyways
		switch (oDest.eEncoding)
		{
		case TextEncoding::ASCII:
			while (!file.eof())
			{
				const auto pos = file.tellg();
				file.read(&iByte, 1);
				if (iByte & 0x80)
				{
					if (file.eof())
					{
						oDest.eEncoding = TextEncoding::Codepage;
						break; // while
					}
					oDest.eEncoding = TextEncoding::UTF8;
					file.clear();
					file.seekg(pos);
					break; // while
				}
			}
			if (oDest.eEncoding != TextEncoding::UTF8)
				break;
			[[fallthrough]]; // fallthrough: check if UTF-8
		case TextEncoding::UTF8:
		{
			uint8_t iRemainingBytes = 0;
			while (!file.eof())
			{
				file.read(&iByte, 1);
				if (iRemainingBytes == 0)
				{
					if (iByte & 0x80)
					{
						if ((iByte & 0xC0) == 0x80)
						{
							// no proper UTF-8
							oDest.eEncoding = TextEncoding::Codepage;
							break; // while
						}

						iRemainingBytes = 1;
						while (iRemainingBytes <= 3 && (iByte >> (6 - iRemainingBytes)) & 1)
							++iRemainingBytes;

						if (iRemainingBytes > 3)
						{
							// no proper UTF-8
							oDest.eEncoding = TextEncoding::Codepage;
							break; // while
						}
					}
				}
				else
				{
					// is marked as "sub-byte"
					if ((iByte & 0xC0) == 0x80)
						--iRemainingBytes;

					// no proper UTF-8
					else
					{
						oDest.eEncoding = TextEncoding::Codepage;
						break; // while
					}
				}
			}
			if (iRemainingBytes > 0)
			{
				// no proper UTF-8
				oDest.eEncoding = TextEncoding::Codepage;
				break; // while
			}
			break;
		}

		// UTF-32 LE is the only encoding checked for illegal characters at this point, since it
		// might be UTF-16 LE instead - both can start with FFFE, the first character in an UTF-16
		// text file might be NULL. This would make the first two words FFFE 0000, which is also the
		// BOM of UTF-16 LE
		case TextEncoding::UTF32:
			if (oDest.iFlags & flags::BigEndian)
				break; // only check little endian

			{
				bool bError = false;
				uint32_t iBuf = 0;

				while (!file.eof())
				{
					file.read(reinterpret_cast<uint8_t*>(&iBuf), 4);
					if constexpr (BigEndian) // only little endian is checked
						iBuf = Endian::Swap(iBuf);

					if (Unicode::IsNoncharacter(iBuf))
					{
						bError = true;
						break; // while
					}
				}

				if (!bError)
					break;
			}

			file.clear();
			file.seekg(lenBOM);
			[[fallthrough]]; // fallthrough: might be UTF-16 LE instead
		case TextEncoding::UTF16:
		{
			uint16_t iBuf = 0;
			while (!file.eof())
			{
				file.read(reinterpret_cast<uint8_t*>(&iBuf), 2);
				if (bSwapEndian)
					iBuf = Endian::Swap(iBuf);

				uint16_t iSurrogateHeader = iBuf & 0xFC00;
				if (iSurrogateHeader == 0b1101'1100'0000'0000)
				{
					// high surrogate missing --> Codepage/binary data
					oDest.eEncoding = TextEncoding::Codepage;
					break; // while
				}
				if (iSurrogateHeader == 0b1101'1000'0000'0000)
				{
					bool bLowSurrogate = false;
					if (!file.eof())
					{
						file.read(reinterpret_cast<uint8_t*>(&iBuf), 2);
						if (bSwapEndian)
							iBuf = Endian::Swap(iBuf);

						if ((iBuf & 0xFC00) == 0b1101'1100'0000'0000)
							bLowSurrogate = true;
					}

					if (!bLowSurrogate)
					{
						// low surrogate missing --> codepage/binary data
						oDest.eEncoding = TextEncoding::Codepage;
						break; // while
					}
				}
			}
			break;
		}
		}





		if (iFlags & Flags::GetTextFileInfo::CheckMinimum)
			return true;





		//------------------------------------------------------------------------------------------
		// CHECK FILE CONTENTS (VALUES)


		file.clear();
		file.seekg(lenBOM);
		// switch for all Unicode encodings (for ASCII and Codepage, see below)
		LineBreak lb{};
		bool bConsequentLinebreaks = true;
		bool bLinebreakRead = false;
		switch (oDest.eEncoding)
		{
		case TextEncoding::UTF8:
		{
			uint8_t iBuf = 0;
			uint32_t cRaw = 0;

			while (!file.eof())
			{
				file.read(&iBuf, 1);
				if (iBuf & 0x80) // multi-byte value
				{
					uint8_t iByteCount = 2;
					while (iByteCount < 4 && (iByte >> (8 - iByteCount)) & 1)
						++iByteCount;

					cRaw = (iBuf & (0xFF >> (iByteCount + 1))) << ((iByteCount - 1) * 6);
					for (uint8_t i = 1; i < iByteCount; ++i)
					{
						file.read(&iBuf, 1);
						cRaw |= (iBuf & 0x3F) << ((iByteCount - 1 - i) * 6);
					}

					if (Unicode::IsNoncharacter(cRaw))
					{
						oDest.eEncoding = TextEncoding::Codepage;
						break; // while
					}
				}
				else // ASCII value --> check linebreaks (common linebreaks are ASCII compatible)
				{
					cRaw = iBuf;

					switch (cRaw)
					{
					case '\n': // UNIX linebreak
						lb = LineBreak::UNIX;
						break;

					case '\r': // Macintosh/Windows linebreak
						if (file.peek() == '\n')
						{
							lb = LineBreak::Windows;
							file.seekg(+1, std::ios::cur);
						}
						else
							lb = LineBreak::Macintosh;
						break;

					default:
						continue; // there are no noncharacters in the ASCII range
					}

					if (bConsequentLinebreaks && bLinebreakRead && lb != oDest.eLineBreaks)
						bConsequentLinebreaks = false;

					oDest.eLineBreaks = lb;
					bLinebreakRead = true;
				}
			}

			break;
		}

		case TextEncoding::UTF16:
		{
			uint16_t iBuf = 0;
			while (!file.eof())
			{
				file.read(reinterpret_cast<uint8_t*>(&iBuf), 2);
				if (bSwapEndian)
					iBuf = Endian::Swap(iBuf);
				if ((iBuf & 0xFC00) == 0b1101'1000'0000'0000) // high surrogate
				{
					char32_t cRaw = char32_t(iBuf & 0x03FF) << 10;
					file.read(reinterpret_cast<uint8_t*>(&iBuf), 2);
					if (bSwapEndian)
						iBuf = Endian::Swap(iBuf);
					cRaw |= iBuf & 0x03FF;

					if (Unicode::IsNoncharacter(cRaw))
					{
						oDest.eEncoding = TextEncoding::Codepage;
						break; // while
					}
				}
				else // BMP value --> check linebreaks
				{
					switch (iBuf)
					{
					case '\n': // UNIX linebreak
						lb = LineBreak::UNIX;
						break; // switch

					case '\r': // Macintosh/Windows linebreak
						if (!file.eof())
						{
							const auto pos = file.tellg();
							file.read(reinterpret_cast<uint8_t*>(&iBuf), 2);
							if (bSwapEndian)
								iBuf = Endian::Swap(iBuf);

							if (iBuf == '\n')
								lb = LineBreak::Windows;
							else
							{
								file.clear();
								file.seekg(pos);
							}
						}
						else
							lb = LineBreak::Macintosh;

						break; // switch

					default:
						if (Unicode::IsNoncharacter(iBuf))
						{
							oDest.eEncoding = TextEncoding::Codepage;
							break; // switch
						}
						continue;
					}

					if (oDest.eEncoding == TextEncoding::Codepage)
						break; // while

					if (bConsequentLinebreaks && bLinebreakRead && lb != oDest.eLineBreaks)
						bConsequentLinebreaks = false;

					oDest.eLineBreaks = lb;
					bLinebreakRead = true;
				}
			}
			break;
		}

		case TextEncoding::UTF32:
		{
			char32_t cRaw = 0;
			while (!file.eof())
			{
				file.read(reinterpret_cast<uint8_t*>(&cRaw), 4);
				if (bSwapEndian)
					cRaw = Endian::Swap((uint32_t)cRaw);

				switch (cRaw)
				{
				case '\n': // UNIX linebreak
					lb = LineBreak::UNIX;
					break;

				case '\r': // Macintosh/Windows linebreak
					if (!file.eof())
					{
						char32_t cRaw2 = 0;
						const auto pos = file.tellg();
						file.read(reinterpret_cast<uint8_t*>(&cRaw2), 4);
						if (bSwapEndian)
							cRaw2 = Endian::Swap((uint32_t)cRaw2);
						if (cRaw2 == '\n')
							lb = LineBreak::Windows;
						else
						{
							if (Unicode::IsNoncharacter(cRaw2))
							{
								oDest.eEncoding = TextEncoding::Codepage;
								break; // switch
							}
							else
								lb = LineBreak::Macintosh;

							file.clear();
							file.seekg(pos);
						}
					}
					else
						lb = LineBreak::Macintosh;
					break;

				default:
					if (Unicode::IsNoncharacter(cRaw))
					{
						oDest.eEncoding = TextEncoding::Codepage;
						break; // switch
					}
					continue; // no linebreak
				}

				if (oDest.eEncoding == TextEncoding::Codepage)
					break; // while

				if (bConsequentLinebreaks && bLinebreakRead && lb != oDest.eLineBreaks)
					bConsequentLinebreaks = false;

				oDest.eLineBreaks = lb;
				bLinebreakRead = true;
			}
			break;
		}

		}

		// not part of the switch because another encoding might have contained noncharacters,
		// making it a codepage guess.
		if (oDest.eEncoding == TextEncoding::ASCII || oDest.eEncoding == TextEncoding::Codepage)
		{
			file.clear();
			file.seekg(0);

			// ASCII has no Unicode noncharacters --> only linebreaks are checked
			// Codepages are assumed to have no invalid values --> only linebreaks are checked

			uint8_t iBuf = 0;
			bLinebreakRead = false;
			bConsequentLinebreaks = true;
			while (!file.eof())
			{
				file.read(&iBuf, 1);

				switch (iBuf)
				{
				case '\n': // UNIX linebreak
					lb = LineBreak::UNIX;
					break;

				case '\r': // Macintosh/Windows linebreak
					if (file.peek() == '\n')
					{
						lb = LineBreak::Windows;
						file.seekg(+1, std::ios::cur);
					}
					else
						lb = LineBreak::Macintosh;
					break;

				default:
					continue;
				}

				if (bConsequentLinebreaks && bLinebreakRead && lb != oDest.eLineBreaks)
					bConsequentLinebreaks = false;

				oDest.eLineBreaks = lb;
				bLinebreakRead = true;
			}

			if (bConsequentLinebreaks)
				oDestEx.iFlags |= flagsEx::ConsequentLineBreaks;
		}

		return true;
	}





	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding)
	{
		TextFileReader reader(szFilePath, oEncoding);
		if (!reader)
			return false;

		reader.readLines(oLines);
		return true;
	}

	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines)
	{
		TextFileReader reader(szFilePath);
		if (!reader)
			return false;

		reader.readLines(oLines);
		return true;
	}

	bool WriteTextFile(const wchar_t* szFilePath, const std::vector<std::wstring>& oLines,
		const TextFileInfo& oEncoding, bool bTrailingLineBreak)
	{
		TextFileWriter writer(szFilePath, oEncoding);
		if (!writer)
			return false;

		writer.writeLines(oLines, bTrailingLineBreak);
		return true;
	}



	/***********************************************************************************************
	 class TextFileRead
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	TextFileReader::TextFileReader(const wchar_t* szFilePath) { open(szFilePath); }

	TextFileReader::TextFileReader(const wchar_t* szFilePath, const TextFileInfo& oEncoding)
	{
		open(szFilePath, oEncoding);
	}

	TextFileReader::~TextFileReader() { close(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void TextFileReader::open(const wchar_t* szFilePath)
	{
		close();

		TextFileInfo oEncoding{};
		TextFileInfo_Get oEncodingEx{};
		if (!GetTextFileInfo(szFilePath, oEncoding, oEncodingEx))
			return;

		open(szFilePath, oEncoding);
	}

	void TextFileReader::open(const wchar_t* szFilePath, const TextFileInfo& oEncoding)
	{
		close();
		m_oFile.open(szFilePath, std::ios::binary);
		if (!m_oFile)
			return; // couldn't open the file

		size_t lenBOM = 0;
		if (oEncoding.iFlags & Flags::TextFileInfo::HasBOM)
		{
			switch (oEncoding.eEncoding)
			{
			case TextEncoding::UTF8:
				lenBOM = 3;
				break;
			case TextEncoding::UTF16:
				lenBOM = 2;
				break;
			case TextEncoding::UTF32:
				lenBOM = 4;
				break;
			}
		}

		m_oFile.seekg(lenBOM, std::ios::beg);
		if (m_oFile.tellg() != lenBOM)
		{
			m_oFile.close();
			return; // file was shorter than the expected BOM length
		}

		m_oEncoding = oEncoding;
		m_bTrailingLinebreak = false;
	}

	void TextFileReader::read(char32_t& cDest)
	{
		if (eof())
		{
			cDest = 0;
			return;
		}

		bool bSwapEndian = m_oEncoding.iFlags & Flags::TextFileInfo::BigEndian;
		switch (m_oEncoding.eEncoding)
		{
		case TextEncoding::ASCII:
		{
			uint8_t cTMP = 0;
			m_oFile.read(&cTMP, 1);
			cDest = cTMP;

			break;
		}

		case TextEncoding::Codepage:
		{
			uint8_t cEnc = 0;
			m_oFile.read(&cEnc, 1);
			if (cEnc >= 0x80 && cEnc <= 0x9F)
			{
				cDest = cCP1252_Table[cEnc - 0x80];
				if (cDest == 0)
					cDest = '?';
			}
			else
				cDest = cEnc;

			break;
		}

		case TextEncoding::UTF8:
		{
			uint8_t iBuf[4]{};
			m_oFile.read(iBuf, 1);

			if (iBuf[0] & 0x80)
			{
				uint8_t iByteCount = 1;
				while (iByteCount <= 4 && ((iBuf[0] >> (7 - iByteCount)) & 1))
					++iByteCount;

				if (iByteCount > 4)
					throw "Invalid UTF-8: Initial bit sequence was larger than 4";

				m_oFile.read(iBuf + 1, (size_t)iByteCount - 1);
				cDest = (iBuf[0] & (0xFF >> (iByteCount + 1))) << ((iByteCount - 1) * 6);
				for (uint8_t i = 1; i < iByteCount; ++i)
				{
					cDest |= (iBuf[i] & 0x3F) << ((iByteCount - 1 - i) * 6);
				}

			}
			else
				cDest = iBuf[0];


			break;
		}

		case TextEncoding::UTF16:
		{
			uint8_t iBuf[2]{};
			uint16_t cEnc = 0;

			m_oFile.read(iBuf, 2);
			cEnc = *reinterpret_cast<const uint16_t*>(iBuf);
			if (bSwapEndian)
				cEnc = Endian::Swap(cEnc);

			if ((cEnc & 0xFC00) == 0b1101'1000'0000'0000)
			{
				cDest = (cEnc & 0x03FF) << 10;
				m_oFile.read(iBuf, 2);
				cEnc = *reinterpret_cast<const uint16_t*>(iBuf);
				if (bSwapEndian)
					cEnc = Endian::Swap(cEnc);

				cDest |= cEnc & 0x03FF;

			}
			else
				cDest = cEnc;

			break;
		}

		case TextEncoding::UTF32:
		{
			uint8_t iBuf[4];
			m_oFile.read(iBuf, 4);
			cDest = *reinterpret_cast<const uint32_t*>(iBuf);
			if (bSwapEndian)
				cDest = Endian::Swap(cDest);

			break;
		}

		default:
			throw "Reading unknown text encoding";
		}
	}

	void TextFileReader::read(std::wstring& sDest, size_t len)
	{
		sDest.clear();
		if (eof() || len == 0)
			return;

		sDest.reserve(len);

		while (!eof() && sDest.length() < len)
		{
			char32_t c = 0;
			read(c);

			if (c < 0x01'00'00)
				sDest += (wchar_t)c;
			else
			{
				c -= 0x01'00'00;
				sDest += 0b1101'1000'0000'0000 | (c >> 10);
				sDest += 0b1101'1100'0000'0000 | (c & 0x03FF);
			}
		}
	}

	void TextFileReader::readLine(std::wstring& sDest)
	{
		sDest.clear();

		if (eof())
			return;

		auto pos = m_oFile.tellg();

		char32_t c = 0;
		size_t len = 0;
		do
		{
			read(c);
			++len;
		} while (!eof() && c != '\r' && c != '\n');
		--len;

		m_oFile.clear(); // clear eofbit
		m_oFile.seekg(pos);
		read(sDest, len);

		if (m_oFile.peek() == EOF/*eof()*/)
			m_bTrailingLinebreak = false;
		else
		{
			read(c);
			if (c == '\n')
				m_oEncoding.eLineBreaks = LineBreak::UNIX;
			if (c == '\r')
			{
				if (!eof())
				{
					pos = m_oFile.tellg();
					read(c);
					if (c == '\n')
						m_oEncoding.eLineBreaks = LineBreak::Windows;
					else
					{
						m_oEncoding.eLineBreaks = LineBreak::Macintosh;
						m_oFile.clear(); // clear eofbit
						m_oFile.seekg(pos);
					}
				}
				else
					m_oEncoding.eLineBreaks = LineBreak::Macintosh;
			}

			m_bTrailingLinebreak = eof();
		}
	}

	void TextFileReader::readLines(std::vector<std::wstring>& oLines)
	{
		oLines.clear();

		while (!eof())
		{
			std::wstring s;
			readLine(s);
			oLines.push_back(std::move(s));
		}
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods





	/***********************************************************************************************
	 class TextFileWriter
	***********************************************************************************************/


	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	TextFileWriter::TextFileWriter(const wchar_t* szFilePath, const TextFileInfo& oEncoding)
	{
		open(szFilePath, oEncoding);
	}

	TextFileWriter::~TextFileWriter() { close(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void TextFileWriter::open(const wchar_t* szFilePath, const TextFileInfo& oEncoding)
	{
		close();

		m_oEncoding = oEncoding;
		m_oFile.open(szFilePath, std::ios::out | std::ios::binary);

		if (!m_oFile)
			return;


		// write the BOM (if applicable)
		if (m_oEncoding.iFlags & Flags::TextFileInfo::HasBOM)
		{
			bool bBigEndian = m_oEncoding.iFlags & Flags::TextFileInfo::BigEndian;

			uint8_t BOM[4]{};
			switch (m_oEncoding.eEncoding)
			{
			case TextEncoding::UTF8:
				BOM[0] = 0xEF;
				BOM[1] = 0xBB;
				BOM[2] = 0xBF;
				m_oFile.write(BOM, 3);
				break;
			case TextEncoding::UTF16:
				if (bBigEndian)
				{
					BOM[0] = 0xFE;
					BOM[1] = 0xFF;
				}
				else
				{
					BOM[0] = 0xFF;
					BOM[1] = 0xFE;
				}

				m_oFile.write(BOM, 2);
				break;
			case TextEncoding::UTF32:
				if (bBigEndian)
				{
					BOM[0] = 0x00;
					BOM[1] = 0x00;
					BOM[2] = 0xFE;
					BOM[3] = 0xFF;
				}
				else
				{
					BOM[0] = 0xFF;
					BOM[1] = 0xFE;
					BOM[2] = 0x00;
					BOM[3] = 0x00;
				}
				m_oFile.write(BOM, 4);
				break;
			}
		}
	}

	void TextFileWriter::write(char32_t c)
	{
		if (!isOpen())
			return;

		if (Unicode::IsNoncharacter(c))
			throw "Tried to write a noncharacter value to a file";


		bool bSwapEndian = m_oEncoding.iFlags & Flags::TextFileInfo::BigEndian;
		if constexpr (!BigEndian)
			bSwapEndian = !bSwapEndian;

		switch (m_oEncoding.eEncoding)
		{
		case TextEncoding::ASCII:
		{
			if (c > 0x7F)
				c = '?';

			uint8_t cEncoded = (uint8_t)c;
			m_oFile.write(&cEncoded, 1);
			break;
		}

		case TextEncoding::Codepage:
		{
			constexpr uint8_t iUnknown = (uint8_t)'?';
			if (c >= 0x80 && c <= 0x9F)
				m_oFile.write(&iUnknown, 1);

			else if (c > 0xFF)
			{
				for (uint8_t i = 0; i < 0x2F; ++i)
				{
					if (cCP1252_Table[i] == c)
					{
						m_oFile.write(&i, 1);
						return;
					}
				}
				m_oFile.write(&iUnknown, 1);
			}
			else
			{
				uint8_t cOutput = (uint8_t)c;
				m_oFile.write(&cOutput, 1);
			}

			break;
		}

		case TextEncoding::UTF8:
		{
			// ASCII value --> no encoding
			if (c < 0x80)
			{
				uint8_t cEncoded = (uint8_t)c;
				m_oFile.write(&cEncoded, 1);
				return;
			}


			// count bits necessary to represent the number.
			// at least 8 bits are being used - otherwise, it would be an ASCII value; see above.
			uint8_t iUsedBits = 8;
			while (c >> iUsedBits)
				++iUsedBits;

			// compute count of bytes needed to represent the character
			uint8_t iByteCount = (iUsedBits / 6) + 1;
			if (iUsedBits % 6 > (7 - iByteCount))
				++iByteCount;



			uint8_t* cEncoded = new uint8_t[iByteCount];

			cEncoded[0] = 0xFF << (8 - iByteCount); // write byte count bits
			cEncoded[0] |= c >> ((iByteCount - 1) * 6);
			for (uint8_t i = 1; i < iByteCount; ++i)
			{
				cEncoded[i] = 0x80 | ((c >> ((iByteCount - 1 - i) * 6)) & 0x3F);
			}
			m_oFile.write(cEncoded, iByteCount);

			delete[] cEncoded;


			break;
		}



		case TextEncoding::UTF16:
		{
			if (c < 0x1'00'00)
			{
				char16_t cEncoded = (char16_t)c;
				if (bSwapEndian)
					cEncoded = Endian::Swap((uint16_t)cEncoded);

				uint8_t cOutput[2] = { uint8_t(cEncoded >> 8), uint8_t(cEncoded) };
				m_oFile.write(cOutput, 2);
			}
			else
			{
				c -= 0x1'00'00;
				char16_t cEncoded[2] =
				{
					char16_t(0b1101'1000'0000'0000ui16 | (char16_t)(c >> 10)),
					char16_t(0b1101'1100'0000'0000ui16 | (char16_t)(c & 0x03FF))
				};
				if (bSwapEndian)
				{
					cEncoded[0] = Endian::Swap((uint16_t)cEncoded[0]);
					cEncoded[1] = Endian::Swap((uint16_t)cEncoded[1]);
				}

				uint8_t cOutput[4] =
				{
					uint8_t(cEncoded[0] >> 8),
					uint8_t(cEncoded[0]),
					uint8_t(cEncoded[1] >> 8),
					uint8_t(cEncoded[1])
				};
				m_oFile.write(cOutput, 4);
			}
			break;
		}



		case TextEncoding::UTF32:
		{
			if (bSwapEndian)
				c = Endian::Swap(c);

			uint8_t cOutput[4] =
			{
				uint8_t(c >> 24),
				uint8_t(c >> 16),
				uint8_t(c >> 8),
				uint8_t(c)
			};

			m_oFile.write(cOutput, 4);
		}

		}
	}

	void TextFileWriter::write(const wchar_t* szText, size_t len)
	{
		if (len == 0)
			len = wcslen(szText);

		for (size_t i = 0; i < len; ++i)
		{
			char32_t cRaw;
			if ((szText[i] & 0xFC00) == 0b1101'1000'0000'0000)
			{
				cRaw = szText[i] & 0x03FF;
				cRaw <<= 10;
				cRaw |= szText[i + 1] & 0x03FF;
				++i;
			}
			else
				cRaw = szText[i];

			write(cRaw);
		}
	}

	void TextFileWriter::writeLine(const wchar_t* szText, size_t len)
	{
		write(szText, len);
		switch (m_oEncoding.eLineBreaks)
		{
		case LineBreak::Windows:
			write('\r');
			write('\n');
			break;
		case LineBreak::UNIX:
			write('\n');
			break;
		case LineBreak::Macintosh:
			write('\r');
			break;
		}
	}

	void TextFileWriter::writeLines(const std::vector<std::wstring>& oLines, bool bTrailingLinebreak)
	{
		if (oLines.size() > 1)
		{
			for (size_t i = 0; i < oLines.size() - 1; ++i)
			{
				writeLine(oLines[i]);
			}
			if (bTrailingLinebreak)
				writeLine(oLines[oLines.size() - 1]);
			else
				write(oLines[oLines.size() - 1]);
		}
	}



}