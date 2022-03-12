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

	uint8_t GetBOMLength(TextEncoding enc)
	{
		switch (enc)
		{
		case TextEncoding::UTF8:
			return 0;
		case TextEncoding::UTF16BE: // fallthrough: UTF-16 always has the same BOM length
		case TextEncoding::UTF16LE:
			return 2;
		case TextEncoding::UTF8BOM:
			return 3;
		case TextEncoding::UTF32BE: // fallthrough: UTF-32 always has the same BOM length
		case TextEncoding::UTF32LE:
			return 4;
			break;

		default:
			throw "Invalid rl::TextEncoding value";
		}
	}



	bool GuessTextEncoding(const wchar_t* szFilepath,
		TextEncoding& eGuessedEncoding, bool bPreferUTF8overCP1252, bool bCheckWholeFile)
	{
		std::basic_ifstream<uint8_t> oFile(szFilepath);
		if (!oFile || oFile.eof())
			return false; // file couldn't be opened/was empty

		uint8_t iByte = 0;

		// read the first byte
		oFile.read(&iByte, 1);

		if (oFile.eof())
		{
			// only Codepage 1252 and UTF-8 support single-byte characters

			if (iByte & 0x80 && !bPreferUTF8overCP1252)
				eGuessedEncoding = TextEncoding::CP1252;
			else
				eGuessedEncoding = TextEncoding::UTF8;
		}

		/*
		  POSSIBILITIES:

		  0xFE 0xFF           --> UTF-16 BE
		  0xEF 0xBB 0xBF      --> UTF-8 BOM
		  0xFF 0xFE 0x00 0x00 --> UTF-32 LE
		  0xFF 0xFE           --> UTF-16 LE
		  0x00 0x00 0xFE 0xFF --> UTF-32 BE
		*/

		switch (iByte)
		{
		case 0xFE: // UTF-16 BE?
			oFile.read(&iByte, 1);
			if (iByte == 0xFF)
				eGuessedEncoding = TextEncoding::UTF16BE;
			else
				eGuessedEncoding = TextEncoding::CP1252;
			break;
		case 0xEF:
			oFile.read(&iByte, 1);
			if (iByte == 0xBB)
			{
				if (oFile.eof())
				{
					eGuessedEncoding = TextEncoding::CP1252;
					break;
				}
				oFile.read(&iByte, 1);
				if (iByte == 0xBF)
					eGuessedEncoding = TextEncoding::UTF8BOM;
				else
					eGuessedEncoding = TextEncoding::CP1252;
			}
			else
				eGuessedEncoding = TextEncoding::CP1252;
			break;
		case 0xFF:
			oFile.read(&iByte, 1);
			if (oFile.eof() || iByte != 0xFE)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				break;
			}

			// might be UTF-16 LE or UTF-32 LE
			if (oFile.eof())
			{
				eGuessedEncoding = TextEncoding::UTF16LE;
				return true; // whole file was already checked anyways
			}
			oFile.read(&iByte, 1);
			if (oFile.eof())
			{
				// The file was 3 bytes long.
				// Neither UTF-16 nore UTF-32 support single-byte characters
				// --> must be CP1252
				eGuessedEncoding = TextEncoding::CP1252;
				return true; // whole file was already checked anyways
			}
			if (iByte != 0x00)
			{
				eGuessedEncoding = TextEncoding::UTF16LE;
				break;
			}
			oFile.read(&iByte, 1);
			if (iByte != 0x00)
			{
				eGuessedEncoding = TextEncoding::UTF16LE;
				break;
			}
			eGuessedEncoding = TextEncoding::UTF32LE;
			break;
		case 0x00:
			// only valid BOM would be UTF-32 BE
			oFile.read(&iByte, 1);
			if (oFile.eof() || iByte != 0x00)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				break;
			}
			oFile.read(&iByte, 1);
			if (oFile.eof() || iByte != 0xFE)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				break;
			}
			oFile.read(&iByte, 1);
			if (iByte != 0xFF)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				break;
			}
			eGuessedEncoding = TextEncoding::UTF32BE;
			break;
		default:
			// assume UTF-8, force check of complete file
			eGuessedEncoding = TextEncoding::UTF8;
			bCheckWholeFile = true;
		}

		if (!bCheckWholeFile || eGuessedEncoding == TextEncoding::CP1252)
			return true;


		const auto iBOMLength = GetBOMLength(eGuessedEncoding);


		oFile.seekg(std::ios::end);
		const size_t iFilesize = oFile.tellg();

		// check filesize restrictions
		switch (eGuessedEncoding)
		{
		case TextEncoding::UTF16BE: // fallthrough: UTF-16 always has the same filesize restrictions
		case TextEncoding::UTF16LE:
			if (iFilesize % 2)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				return true;
			}
			break;
		case TextEncoding::UTF32BE: // fallthrough: UTF-32 always has the same filesize restrictions
		case TextEncoding::UTF32LE:
			if (iFilesize % 4)
			{
				eGuessedEncoding = TextEncoding::CP1252;
				return true;
			}
			break;
		}


		oFile.seekg(iBOMLength); // go to after the BOM
		if (oFile.eof())
			return true; // nothing more to check


		// check if the endian must be swapped
		bool bSwapEndian = false;
		if constexpr (rl::BigEndian) // system is big endian
			bSwapEndian =
			eGuessedEncoding == TextEncoding::UTF16LE ||
			eGuessedEncoding == TextEncoding::UTF32LE;
		else // system is little endian
			bSwapEndian =
			eGuessedEncoding == TextEncoding::UTF16BE ||
			eGuessedEncoding == TextEncoding::UTF32BE;





		// check the file contents
		char32_t cRaw = 0;
		switch (eGuessedEncoding)
		{
		case TextEncoding::UTF8: // fallthrough: UTF-8 BOM is identical to UTF-8 after the BOM
		case TextEncoding::UTF8BOM:
		{
			uint8_t iBuf[4]{};

			while (!oFile.eof())
			{
				oFile.read(iBuf, 1);
				if (iBuf[0] & 0x80) // multibyte
				{
					uint8_t iByteCount = 1;
					while (iByteCount < 8 && ((iBuf[0] >> (7 - iByteCount)) & 1))
					{
						++iByteCount;
					}
					if (iByteCount == 1 || iByteCount > 4 ||
						iFilesize - (uint64_t)oFile.tellg() <
						(uint64_t)std::streamoff(iByteCount - 1))
					{
						// in UTF-8, the first codeunit cannot be 0b10XX'XXXX
						// UTF-8 only supports up to 4 codeunits per codepoint
						// the file must be big enough to hold the expected remaining values
						eGuessedEncoding = TextEncoding::CP1252;
						return true;
					}

					// read the bytes needed
					oFile.read(iBuf + 1, (std::streamsize)iByteCount - 1);

					// write contents of first codeunit
					cRaw = uint32_t(iBuf[0] & (0xFF >> (iByteCount + 1))) <<
						((iByteCount - 1) * 6);
					// write contents of remaining codeunits
					for (uint8_t i = 1; i < iByteCount; ++i)
					{
						if ((iBuf[i] & 0xC0) != 0x80)
						{
							// subsequent byte wasn't 0b10XX'XXXX
							eGuessedEncoding = TextEncoding::CP1252;
							return true;
						}

						cRaw |= uint32_t(iBuf[i] & 0x3F) << ((iByteCount - 1 - i) * 6);
					}
				}
				else // single byte
					cRaw = iBuf[0];


				if (Unicode::IsNoncharacter(cRaw))
				{
					// encoded value is an invalid Unicode value
					eGuessedEncoding = TextEncoding::CP1252;
					return true;
				}
			}
		}
		break;



		case TextEncoding::UTF16BE: // fallthrough: UTF-16
		case TextEncoding::UTF16LE:
		{
			uint16_t iVal = 0;
			while (!oFile.eof())
			{
				oFile.read(reinterpret_cast<uint8_t*>(&iVal), 2);
				if (bSwapEndian)
					iVal = Endian::Swap(iVal);

				if ((iVal & 0xFC00) == 0b1101'1000) // high surrogate
				{
					if (oFile.eof())
					{
						// lacking 2nd code unit
						eGuessedEncoding = TextEncoding::CP1252;
						return true;
					}

					uint16_t iVal2 = 0;
					oFile.read(reinterpret_cast<uint8_t*>(&iVal2), 2);
					if (bSwapEndian)
						iVal2 = Endian::Swap(iVal2);

					if ((iVal & 0xFC00) != 0b1101'1100)
					{
						// invalid low surrogate
						eGuessedEncoding = TextEncoding::CP1252;
						return true;
					}

					cRaw = (iVal2 & 0x03FF) | (((uint32_t)iVal & 0x03FF) << 10);
					cRaw += 0x01'00'00;
				}
				else
					cRaw = iVal;

				if (Unicode::IsNoncharacter(cRaw))
				{
					// encoded value is an invalid Unicode value
					eGuessedEncoding = TextEncoding::CP1252;
					return true;
				}
			}
		}
		break;



		case TextEncoding::UTF32BE: // fallthrough: UTF-32
		case TextEncoding::UTF32LE:
		{
			while (!oFile.eof())
			{
				oFile.read(reinterpret_cast<uint8_t*>(&cRaw), 4);
				if (bSwapEndian)
					cRaw = Endian::Swap(cRaw);

				if (Unicode::IsNoncharacter(cRaw))
				{
					// encoded value is an invalid Unicode value
					eGuessedEncoding = TextEncoding::CP1252;
					return true;
				}
			}
		}
		break;
		}

		return true;
	}



	bool ReadAllLines(const wchar_t* szFilePath, std::vector<std::wstring>& oLines,
		TextEncoding eEncoding)
	{
		TextFileReader reader(szFilePath, eEncoding);
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
		bool bTrailingLineBreak, TextEncoding eEncoding, LineBreak eLineBreak)
	{
		TextFileWriter writer(szFilePath, eEncoding, eLineBreak);
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

	TextFileReader::TextFileReader(const wchar_t* szFilePath, TextEncoding eEncoding)
	{
		open(szFilePath, eEncoding);
	}

	TextFileReader::~TextFileReader() { close(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void TextFileReader::open(const wchar_t* szFilePath)
	{
		close();

		TextEncoding eEncoding{};
		if (!GuessTextEncoding(szFilePath, eEncoding, false, true))
			return;

		open(szFilePath, eEncoding);
	}

	void TextFileReader::open(const wchar_t* szFilePath, TextEncoding eEncoding)
	{
		close();
		m_oFile.open(szFilePath, std::ios::binary);
		if (!m_oFile)
			return; // couldn't open the file

		size_t lenBOM = 0;
		switch (eEncoding)
		{
		case TextEncoding::UTF8BOM:
			lenBOM = 3;
			break;
		case TextEncoding::UTF16BE:
		case TextEncoding::UTF16LE:
			lenBOM = 2;
			break;
		case TextEncoding::UTF32BE:
		case TextEncoding::UTF32LE:
			lenBOM = 4;
			break;
		}

		m_oFile.seekg(lenBOM, std::ios::cur);
		if (m_oFile.tellg() != lenBOM)
		{
			m_oFile.close();
			return; // file was shorter than the expected BOM length
		}

		m_eEncoding = eEncoding;
		m_bTrailingLinebreak = false;
		m_eLineBreak = {};
	}

	void TextFileReader::read(char32_t& cDest)
	{
		if (eof())
		{
			cDest = 0;
			return;
		}

		bool bSwapEndian = false;
		switch (m_eEncoding)
		{
		case TextEncoding::CP1252:
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
		case TextEncoding::UTF8BOM:
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

		case TextEncoding::UTF16BE:
			bSwapEndian = true;
			[[fallthrough]]; // fallthrough: UTF-16 BE and LE are treated very similarly
		case TextEncoding::UTF16LE:
		{
			uint8_t iBuf[2]{};
			uint16_t cEnc = 0;

			m_oFile.read(iBuf, 2);
			cEnc = *reinterpret_cast<const uint16_t*>(iBuf);
			if (bSwapEndian)
				cEnc = Endian::Swap(cEnc);

			if ((cEnc & 0xFC) == 0b1101'1000'0000'0000)
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

		case TextEncoding::UTF32BE:
			bSwapEndian = true;
			[[fallthrough]]; // fallthrough: UTF-32 BE and LE are treated very similarly
		case TextEncoding::UTF32LE:
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
				m_eLineBreak = LineBreak::UNIX;
			if (c == '\r')
			{
				if (!eof())
				{
					pos = m_oFile.tellg();
					read(c);
					if (c == '\n')
						m_eLineBreak = LineBreak::Windows;
					else
					{
						m_eLineBreak = LineBreak::Macintosh;
						m_oFile.clear(); // clear eofbit
						m_oFile.seekg(pos);
					}
				}
				else
					m_eLineBreak = LineBreak::Macintosh;
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

	TextFileWriter::TextFileWriter(const wchar_t* szFilePath, TextEncoding eEncoding,
		LineBreak eLineBreak)
	{
		open(szFilePath, eEncoding, eLineBreak);
	}

	TextFileWriter::~TextFileWriter() { close(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void TextFileWriter::open(const wchar_t* szFilePath, TextEncoding eEncoding,
		LineBreak eLineBreak)
	{
		close();

		m_eEncoding = eEncoding;
		m_eLineBreak = eLineBreak;
		m_oFile.open(szFilePath, std::ios::out | std::ios::binary);

		if (!m_oFile)
			return;


		// write the BOM (if applicable)
		uint8_t BOM[4]{};
		switch (m_eEncoding)
		{
		case TextEncoding::UTF8BOM:
			BOM[0] = 0xEF;
			BOM[1] = 0xBB;
			BOM[2] = 0xBF;
			m_oFile.write(BOM, 3);
			break;
		case TextEncoding::UTF16BE:
			BOM[0] = 0xFE;
			BOM[1] = 0xFF;
			m_oFile.write(BOM, 2);
			break;
		case TextEncoding::UTF16LE:
			BOM[0] = 0xFF;
			BOM[1] = 0xFE;
			m_oFile.write(BOM, 2);
			break;
		case TextEncoding::UTF32BE:
			BOM[0] = 0x00;
			BOM[1] = 0x00;
			BOM[2] = 0xFE;
			BOM[3] = 0xFF;
			m_oFile.write(BOM, 4);
			break;
		case TextEncoding::UTF32LE:
			BOM[0] = 0xFF;
			BOM[1] = 0xFE;
			BOM[2] = 0x00;
			BOM[3] = 0x00;
			m_oFile.write(BOM, 4);
			break;
		}
	}

	void TextFileWriter::write(char32_t c)
	{
		if (!isOpen())
			return;

		if (Unicode::IsNoncharacter(c))
			throw "Tried to write a noncharacter value to a file";


		bool bSwapEndian = false;
		switch (m_eEncoding)
		{
		case TextEncoding::CP1252:
		{
			const uint8_t iUnknown = (uint8_t)'?';
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

		case TextEncoding::UTF8: // fallthrough: doesn't matter if with or without BOM
		case TextEncoding::UTF8BOM:
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



		case TextEncoding::UTF16LE:
			bSwapEndian = true;
			[[fallthrough]]; // fallthrough: UTF-16 BE and LE are treated very similarly
		case TextEncoding::UTF16BE:
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



		case TextEncoding::UTF32LE:
			c = Endian::Swap((uint32_t)c);
			[[fallthrough]]; // fallthrough: UTF-16 BE and LE are treated very similarly
		case TextEncoding::UTF32BE:
		{
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
		switch (m_eLineBreak)
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