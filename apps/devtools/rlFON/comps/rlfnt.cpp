#include "rlfnt.hpp"

#include <exception>
#include <memory.h> // memcpy
#include <stdint.h>
#include <vector>
#include <Windows.h>





namespace rl
{

	namespace BitmapFontWeightStrings
	{
		const char* GetString(uint8_t weight) noexcept
		{
			if (weight < 10)
			{
				switch (weight)
				{
				case RL_FNT_WEIGHT_UNDEFINED:
					return szUndefined;
				default:
					return nullptr;
				}
			}

			weight = weight / 10 * 10;

			switch (weight)
			{
			case RL_FNT_WEIGHT_THIN:
				return szThin;
			case RL_FNT_WEIGHT_EXTRALIGHT:
				return szExtraLight;
			case RL_FNT_WEIGHT_LIGHT:
				return szLight;
			case RL_FNT_WEIGHT_REGULAR:
				return szRegular;
			case RL_FNT_WEIGHT_MEDIUM:
				return szMedium;
			case RL_FNT_WEIGHT_SEMIBOLD:
				return szSemiBold;
			case RL_FNT_WEIGHT_BOLD:
				return szBold;
			case RL_FNT_WEIGHT_EXTRABOLD:
				return szExtraBold;
			case RL_FNT_WEIGHT_BLACK:
				return szBlack;

			default:
				return nullptr;
			}
		}
	}

	const char sBitmapFontFaceMagicNumber[10] = { 'r','l','F','O','N','T','F','A','C','E' };



#pragma pack(push, 1) // disable padding, as this struct is directly accessed as binary data
	struct FontFaceHeader
	{
		char sMagicNo[10] = { 'r','l','F','O','N','T','F','A','C','E' };
		uint8_t iFiletypeVersion[2] = { 1, 0 };
		uint64_t iFamilyNameOffset = 0;
		uint64_t iFaceNameOffset = 0;
		uint64_t iCopyrightOffset = 0;
		uint8_t iFileVersion[4] = { 0, 0, 0, 0 };
		uint8_t iBitsPerPixel = 0;
		uint16_t iFixedCharWidth = 0;
		uint16_t iCharHeight = 0;
		uint16_t iPoints = 0;
		uint8_t iWeight = 0;
		uint32_t iFallback = 0;
		uint8_t iFlags = 0;
	};
#pragma pack(pop)



	/// <summary>
	/// Read a zero-terminated string from a hFile
	/// </summary>
	/// <returns>Did the method succeed?</returns>
	bool FileReadASCIISZ(HANDLE hFile, std::string& sDest)
	{
		sDest.clear();
		size_t len = 0;

		LARGE_INTEGER liFilePtr;
		SetFilePointerEx(hFile, { 0 }, &liFilePtr, FILE_CURRENT); // save file pointer

		char ch;
		DWORD dwRead = 0;
		try
		{
			// get string length
			do
			{
				if (!ReadFile(hFile, &ch, 1, &dwRead, NULL))
					throw 0;
				len++;
			} while (ch != 0);
			len--; // exclude terminating zero from length

			// prepare the destination string
			sDest.resize(len);

			// read the string
			SetFilePointerEx(hFile, liFilePtr, NULL, FILE_BEGIN);
			if (!ReadFile(hFile, (void*)sDest.c_str(), (DWORD)len, &dwRead, NULL))
				throw 0;

			SetFilePointerEx(hFile, liFilePtr, NULL, FILE_BEGIN); // reset file pointer

			return true;
		}
		catch (...)
		{
			SetFilePointerEx(hFile, liFilePtr, NULL, FILE_BEGIN); // reset file pointer
			sDest.clear();
			return false;

		}
	}










	/***********************************************************************************************
	 class BitmapFont::Face::Char
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	BitmapFont::Char::Char() {}

	BitmapFont::Char::Char(const Char& other) { *this = other; }

	BitmapFont::Char::Char(uint8_t BitsPerPixel, uint32_t width, uint32_t height)
	{
		if (!create(BitsPerPixel, width, height))
			throw std::exception("rl::BitmapFont::Char: Invalid constructor parameters");
	}

	BitmapFont::Char::~Char() { destroy(); }





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	BitmapFont::Char& BitmapFont::Char::operator=(const Char& other)
	{
		destroy();
		if (!other.hasData())
			return *this;

		const size_t iDataSize = DataSize(other.m_iBitsPerPixel, other.m_iWidth, other.m_iHeight);
		m_pData = new uint8_t[iDataSize];
		memcpy_s(m_pData, iDataSize, other.m_pData, iDataSize);
		m_iBitsPerPixel = other.m_iBitsPerPixel;
		m_iWidth = other.m_iWidth;
		m_iHeight = other.m_iHeight;

		return *this;
	}

	bool BitmapFont::Char::operator==(const Char& other) const
	{
		if (!hasData() || !other.hasData())
			return false; // uninitialized characters can't be compared

		return (m_iBitsPerPixel == other.m_iBitsPerPixel && m_iWidth == other.m_iWidth &&
			m_iHeight == other.m_iHeight &&
			memcmp(m_pData, other.m_pData, DataSize(m_iBitsPerPixel, m_iWidth, m_iHeight)) == 0);
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	size_t BitmapFont::Char::DataSize(uint8_t BitsPerPixel, uint32_t width, uint32_t height)
	{
		if (BitsPerPixel > 24)
			return 0;

		const uint32_t iColBytes = (size_t)height / 8 + (height % 8 ? 1 : 0);
		return (size_t)iColBytes * width * BitsPerPixel;
	}

	bool BitmapFont::Char::create(uint8_t BitsPerPixel, uint16_t width, uint16_t height)
	{
		destroy();
		const size_t size = DataSize(BitsPerPixel, width, height);
		if (size == 0)
			return false;

		m_pData = new uint8_t[size];
		memset(m_pData, 0, size);

		m_iBitsPerPixel = BitsPerPixel;
		m_iWidth = width;
		m_iHeight = height;

		return true;
	}

	void BitmapFont::Char::destroy()
	{
		if (!hasData())
			return;

		delete[] m_pData;
		m_pData = nullptr;
		m_iBitsPerPixel = 0;
		m_iWidth = 0;
		m_iHeight = 0;
	}

	bool BitmapFont::Char::hasData() const { return m_pData != nullptr; }

	bool BitmapFont::Char::isEmpty() const
	{
		checkData();

		const size_t iSize = DataSize(m_iBitsPerPixel, m_iWidth, m_iHeight);

		for (size_t i = 0; i < iSize; i++)
		{
			if (m_pData[i] != 0)
				return false;
		}
		return true;
	}

	size_t BitmapFont::Char::getDataSize() const
	{
		checkData();

		const uint32_t iColBytes = m_iHeight / 8 + (m_iHeight % 8 ? 1 : 0);
		return (size_t)iColBytes * m_iWidth * m_iBitsPerPixel;
	}

	uint32_t BitmapFont::Char::getPixel(uint16_t x, uint16_t y) const
	{
		checkData();
		checkPos(x, y);

		const uint16_t iColBytes = m_iHeight / 8 + (m_iHeight % 8 ? 1 : 0);
		const size_t iBitPlaneSize = (size_t)iColBytes * m_iWidth;
		const size_t iCurrentByte = (size_t)x * iColBytes + y / 8;
		const size_t iCurrentBit = y % 8;

		uint32_t iResult = 0;


		for (uint16_t i = 0; i < m_iBitsPerPixel; i++)
		{
			uint8_t iByte = m_pData[iBitPlaneSize * i + iCurrentByte];
			iByte >>= 7 - iCurrentBit;
			iByte &= 1;
			iResult |= (uint32_t)iByte << i;
		}

		return iResult;
	}

	void BitmapFont::Char::setPixel(uint16_t x, uint16_t y, uint32_t val)
	{
		checkData();
		checkPos(x, y);

		const uint16_t iColBytes = m_iHeight / 8 + (m_iHeight % 8 ? 1 : 0);
		const size_t iBitPlaneSize = (size_t)iColBytes * m_iWidth;
		const size_t iCurrentByte = (size_t)x * iColBytes + y / 8;
		const size_t iCurrentBit = y % 8;


		for (uint16_t i = 0; i < m_iBitsPerPixel; i++)
		{
			uint8_t iByte = m_pData[iBitPlaneSize * i + iCurrentByte];
			iByte &= ~(1 << (7 - iCurrentBit)); // mask out old value
			if (val)
				iByte |= 1 << (7 - iCurrentBit); // set new value
			m_pData[iBitPlaneSize * i + iCurrentByte] = iByte;
		}
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void BitmapFont::Char::checkPos(uint16_t iX, uint16_t iY) const
	{
		if (iX >= m_iWidth || iY >= m_iHeight)
			throw std::exception("rl::BitmapFont::Char: Invalid position");
	}

	void BitmapFont::Char::checkData() const
	{
		if (m_pData == nullptr)
			throw std::exception("rl::BitmapFont::Char: Character has no data");
	}

	bool BitmapFont::Char::create(uint8_t iBitsPerPixel, uint16_t iWidth, uint16_t iHeight,
		const uint8_t* pData)
	{
		destroy();
		const size_t size = DataSize(iBitsPerPixel, iWidth, iHeight);
		if (size == 0)
			return false;

		m_pData = new uint8_t[size];
		memcpy_s(m_pData, size, pData, size);

		m_iBitsPerPixel = iBitsPerPixel;
		m_iWidth = iWidth;
		m_iHeight = iHeight;

		return true;
	}










	/***********************************************************************************************
	 class BitmapFont::Face
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	BitmapFont::Face& BitmapFont::Face::operator=(const Face& other)
	{
		destroy();
		if (!other.m_bData) // other object has no data --> Assignment = destruction; no error
			return *this;

		create(other.m_iFixedWidth, other.m_iHeight, other.m_iBitsPerPixel, other.m_iPoints,
			other.m_iWeight, other.m_iFlags, other.m_sFamilyName.c_str(), other.m_sFaceName.c_str(),
			other.m_sCopyright.c_str());

		m_iFallback = other.m_iFallback;

		m_oChars = other.m_oChars; // copy character data

		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	BitmapFont::Face::FileStatus BitmapFont::Face::validate(const wchar_t* szFilename)
	{
		HANDLE hFile = CreateFileW(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			NULL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FileStatus::FileDoesntExist;


		try // "throw [...]" = something was wrong with the file
		{
			DWORD dwRead = 0;
			LARGE_INTEGER liFilePtr;
#define READ(p, size)														\
			if (!ReadFile(hFile, p, size, &dwRead, NULL) || dwRead != size)	\
				throw FileStatus::UnexpectedEOF
#define READVAR(var) READ(&var, sizeof(var))
#define SAVEFILEPTR() SetFilePointerEx(hFile, {0}, &liFilePtr, FILE_CURRENT)


			LARGE_INTEGER liFilesize;
			GetFileSizeEx(hFile, &liFilesize);

			// at minimum, the file must have the full header
			if (liFilesize.QuadPart < sizeof(FontFaceHeader))
				throw FileStatus::UnexpectedEOF; // file too small

			// read file header
			FontFaceHeader ffh = {};
			READVAR(ffh);

			if (memcmp(ffh.sMagicNo, "rlFONTFACE", 10) != 0)
				throw FileStatus::WrongMagicNo; // wrong magic number

			// check filetype version
			switch (ffh.iFiletypeVersion[0])
			{
			case 1: // 1.X
				switch (ffh.iFiletypeVersion[1])
				{
				case 0: // 1.0
				{
					// offsets checked later

					if (ffh.iBitsPerPixel == 0 || ffh.iBitsPerPixel > 24)
						throw FileStatus::InvalidBPP;

					if (ffh.iCharHeight == 0)
						throw FileStatus::ZeroHeight;

					if ((ffh.iWeight > 0 && ffh.iWeight < 10) || ffh.iWeight > 99)
						throw FileStatus::InvalidWeight;

					if (ffh.iFlags & 0xFC)
						throw FileStatus::UnknownFlags;

					SAVEFILEPTR();
					if (liFilesize.QuadPart - liFilePtr.QuadPart < sizeof(uint32_t) + 3)
						throw FileStatus::UnexpectedEOF;

					uint32_t iCharCount = 0;
					READVAR(iCharCount);

					std::vector<uint32_t> oCodepoints;

					uint32_t iPrevCodepoint = 0;
					uint32_t iCodepoint;
					uint32_t iCharWidth = ffh.iFixedCharWidth;
					bool bFallbackFound = false;
					for (uint64_t i = 0; i < iCharCount; i++)
					{
						READVAR(iCodepoint);

						if (std::find(oCodepoints.begin(), oCodepoints.end(), iCodepoint) !=
							oCodepoints.end())
							throw FileStatus::DuplicateChar;

						if (iCodepoint < iPrevCodepoint)
							throw FileStatus::NotAscending;

						if (!bFallbackFound && iCodepoint == ffh.iFallback)
							bFallbackFound = true;

						iPrevCodepoint = iCodepoint;

						oCodepoints.push_back(iCodepoint);

						if (ffh.iFixedCharWidth == 0)
							READVAR(iCharWidth);

						SAVEFILEPTR();
						size_t iDataSize = Char::DataSize(ffh.iBitsPerPixel, iCharWidth,
							ffh.iCharHeight);
						if ((uint64_t)liFilesize.QuadPart - (uint64_t)liFilePtr.QuadPart <
							iDataSize + 3)
							throw FileStatus::UnexpectedEOF;

						liFilePtr.QuadPart = iDataSize;
						SetFilePointerEx(hFile, liFilePtr, NULL, FILE_CURRENT); // skip data
					}

					if (!bFallbackFound)
						throw FileStatus::InvalidFallback;

					uint8_t ch = 0;

					// check string: FamilyName
					SAVEFILEPTR();
					if (liFilePtr.QuadPart != ffh.iFamilyNameOffset)
						throw FileStatus::InvalidOffset_Family;
					do
					{
						READVAR(ch);
					} while (ch != 0);

					// check string: FaceName
					SAVEFILEPTR();
					if (liFilePtr.QuadPart != ffh.iFaceNameOffset)
						throw FileStatus::InvalidOffset_Face;
					do
					{
						READVAR(ch);
					} while (ch != 0);

					// check string: Copyright
					SAVEFILEPTR();
					if (liFilePtr.QuadPart != ffh.iCopyrightOffset)
						throw FileStatus::InvalidOffset_Copyright;
					do
					{
						READVAR(ch);
					} while (ch != 0);
				}
				break;

				default:
					throw FileStatus::UnknownFiletypeVer; // unknown minor filetype version
				}
				break;

			default:
				throw FileStatus::UnknownFiletypeVer; // unknown major filetype version
			}


			CloseHandle(hFile);
			return FileStatus::OK;

#undef SAVEFILEPTR
#undef READVAR
#undef READ
		}
		catch (FileStatus status)
		{
			CloseHandle(hFile);
			return status;
		}
	}

	bool BitmapFont::Face::create(uint16_t fixedwidth, uint16_t height, uint8_t BitsPerPixel,
		uint16_t iPoints, uint8_t weight, uint8_t iFlags, const char* szFamilyName,
		const char* szFaceName, const char* szCopyright)
	{
		destroy();

		if (height == 0 || (weight > 0 && weight < 10) || weight > 99)
			return false;


		m_iBitsPerPixel = BitsPerPixel;
		m_iFixedWidth = fixedwidth;
		m_iFlags = iFlags;
		m_iHeight = height;
		m_iPoints = iPoints;
		m_iWeight = weight;

		if (szFamilyName != nullptr)
			m_sFamilyName = szFamilyName;
		if (szFaceName != nullptr)
			m_sFaceName = szFaceName;
		if (szCopyright != nullptr)
			m_sCopyright = szCopyright;

		m_bData = true;

		return true;
	}

	void BitmapFont::Face::destroy()
	{
		if (!m_bData)
			return;

		m_oChars.clear();

		m_iBitsPerPixel = 0;
		m_iFallback = 0;
		m_iFixedWidth = 0;
		m_iFlags = 0;
		m_iHeight = 0;
		m_iPoints = 0;
		m_iWeight = 0;
		memset(m_oFileVersion, 0, 4);

		m_sCopyright.clear();
		m_sFaceName.clear();
		m_sFamilyName.clear();

		m_bData = false;
	}

	bool BitmapFont::Face::saveToFile(const wchar_t* szFilename) const
	{
		if (!m_bData)
			return false;

		// fallback character must be present
		if (m_oChars.find(m_iFallback) == m_oChars.end())
			return false;

		HANDLE hFile = CreateFileW(szFilename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		FontFaceHeader ffh;

		memcpy_s(ffh.iFileVersion, 4, m_oFileVersion, 4);
		ffh.iBitsPerPixel = m_iBitsPerPixel;
		ffh.iFixedCharWidth = m_iFixedWidth;
		ffh.iCharHeight = m_iHeight;
		ffh.iPoints = m_iPoints;
		ffh.iWeight = m_iWeight;
		ffh.iFallback = m_iFallback;
		ffh.iFlags = m_iFlags;

		try
		{
			DWORD dwWritten = 0;

			LARGE_INTEGER liDummy;
			liDummy.QuadPart = 0;
			LARGE_INTEGER liFilePtr;
			liFilePtr.QuadPart = 0;

#define WRITE(p, size) \
			if (!WriteFile(hFile, p, (DWORD)size, &dwWritten, NULL)) \
				throw 0
#define WRITEVAR(var) WRITE(&var, sizeof(var))

#define SAVEFILEPOINTER() SetFilePointerEx(hFile, liDummy, &liFilePtr, FILE_CURRENT)



			WRITEVAR(ffh); // write the header
			const uint32_t iCharCount = (uint32_t)m_oChars.size();
			WRITEVAR(iCharCount);

			// write the character data
			for (auto& o : m_oChars)
			{
				WRITEVAR(o.first);
				if (m_iFixedWidth == 0)
				{
					auto i = o.second.getWidth();
					WRITEVAR(i);
				}
				WRITE(o.second.getData(), o.second.getDataSize());
			}



			// Save the null-terminated strings

			SAVEFILEPOINTER();
			const uint64_t iFamilyNameOffset = liFilePtr.QuadPart;
			WRITE(m_sFamilyName.c_str(), m_sFamilyName.length() + 1);

			SAVEFILEPOINTER();
			const uint64_t iFaceNameOffset = liFilePtr.QuadPart;
			WRITE(m_sFaceName.c_str(), m_sFaceName.length() + 1);

			SAVEFILEPOINTER();
			const uint64_t iCopyrightOffset = liFilePtr.QuadPart;
			WRITE(m_sCopyright.c_str(), m_sCopyright.length() + 1);



			// save the string offsets

			SetFilePointer(hFile, uint32_t((uint64_t)&ffh.iFamilyNameOffset - (uint64_t)&ffh),
				NULL, FILE_BEGIN);
			WRITEVAR(iFamilyNameOffset);

			SetFilePointer(hFile, uint32_t((uint64_t)&ffh.iFaceNameOffset - (uint64_t)&ffh),
				NULL, FILE_BEGIN);
			WRITEVAR(iFaceNameOffset);

			SetFilePointer(hFile, uint32_t((uint64_t)&ffh.iCopyrightOffset - (uint64_t)&ffh),
				NULL, FILE_BEGIN);
			WRITEVAR(iCopyrightOffset);



#undef WRITEVAR
#undef WRITE

			CloseHandle(hFile);
			return true;
		}
		catch (...)
		{
			CloseHandle(hFile);
			DeleteFileW(szFilename); // delete the incomplete file
			return false;
		}
	}

	bool BitmapFont::Face::loadFromFile(const wchar_t* szFilename)
	{
		destroy();
		FontFaceHeader ffh;
		HANDLE hFile = CreateFileW(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		try
		{
			m_bData = true;
			LARGE_INTEGER liFileSize;
			liFileSize.QuadPart = 0;
			GetFileSizeEx(hFile, &liFileSize);
			if (liFileSize.QuadPart <= sizeof(ffh))
				throw 0;

			DWORD dwRead = 0;
#define READ(p, size)										\
			if (!ReadFile(hFile, p, size, &dwRead, NULL))	\
				throw 0
#define READVAR(var) READ(&var, (DWORD)sizeof(var))



			READVAR(ffh);

			// check magic number
			if (memcmp(ffh.sMagicNo, sBitmapFontFaceMagicNumber, 10) != 0)
			{
				MessageBoxA(NULL, "rl::BitmapFont::Face: Wrong magic number in rlFNT file",
					"rlFNT file error", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
				throw 0;
			}

			// copy data
			m_iBitsPerPixel = ffh.iBitsPerPixel;
			m_iFallback = ffh.iFallback;
			m_iFixedWidth = ffh.iFixedCharWidth;
			m_iFlags = ffh.iFlags;
			m_iHeight = ffh.iCharHeight;
			m_iPoints = ffh.iPoints;
			m_iWeight = ffh.iWeight;
			memcpy_s(m_oFileVersion, 4, ffh.iFileVersion, 4);

			LARGE_INTEGER liFilePtr;
			SetFilePointerEx(hFile, { 0 }, &liFilePtr, FILE_CURRENT); // save current file pointer

			// READ STRINGS ========================================================================
			LARGE_INTEGER liJump;

			// family name
			liJump.QuadPart = ffh.iFamilyNameOffset;
			SetFilePointerEx(hFile, liJump, NULL, FILE_BEGIN);
			if (!FileReadASCIISZ(hFile, m_sFamilyName))
				throw 0;

			// face name
			liJump.QuadPart = ffh.iFaceNameOffset;
			SetFilePointerEx(hFile, liJump, NULL, FILE_BEGIN);
			if (!FileReadASCIISZ(hFile, m_sFaceName))
				throw 0;

			// copyright
			liJump.QuadPart = ffh.iCopyrightOffset;
			SetFilePointerEx(hFile, liJump, NULL, FILE_BEGIN);
			if (!FileReadASCIISZ(hFile, m_sCopyright))
				throw 0;


			SetFilePointerEx(hFile, liFilePtr, NULL, FILE_BEGIN); // reset file pointer



			// READ DATA ===========================================================================
			switch (ffh.iFiletypeVersion[0])
			{
			case 1: // 1.X
				switch (ffh.iFiletypeVersion[1])
				{
				case 0: // 1.0
				{
					uint32_t iCharCount = 0;
					READVAR(iCharCount);

					uint32_t iCodepoint = 0;
					uint32_t iCharWidth = ffh.iFixedCharWidth;
					Char oCh;
					size_t size = (iCharWidth > 0 ?
						Char::DataSize(ffh.iBitsPerPixel, iCharWidth, ffh.iCharHeight) : 0);
					uint8_t* pBuf = (iCharWidth > 0 ? new uint8_t[size] : nullptr);
					for (uint64_t i = 0; i < iCharCount; i++)
					{
						READVAR(iCodepoint);

						// check if codepoint is already in font face
						if (m_oChars.find(iCodepoint) != m_oChars.end())
							throw 0;

						// do stuff for non-fixed-width faces
						if (ffh.iFixedCharWidth == 0)
						{
							delete[] pBuf;
							READVAR(iCharWidth);
							size = Char::DataSize(ffh.iBitsPerPixel, iCharWidth, ffh.iCharHeight);
							pBuf = new uint8_t[size];
						}

						READ(pBuf, (DWORD)size);

						oCh.create(ffh.iBitsPerPixel, iCharWidth, ffh.iCharHeight, pBuf);

						m_oChars.emplace(iCodepoint, oCh);
					}
					delete[] pBuf; // delete last/only buffer
				}
				break;



				default:
					MessageBoxA(NULL, "rl::BitmapFont::Face: Unknown minor version of rlFNT file",
						"rlFNT file error", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
					throw 0;
				}
				break;



			default:
				MessageBoxA(NULL, "rl::BitmapFont::Face: Unknown major version of rlFNT file",
					"rlFNT file error", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
				throw 0;
			}



#undef READVAR
#undef READ

			CloseHandle(hFile);

			if (m_oChars.find(m_iFallback) == m_oChars.end())
				return false;

			return true;
		}
		catch (...)
		{
			CloseHandle(hFile);
			destroy();
			return false;
		}
	}

	BitmapFont::Char& BitmapFont::Face::add(uint32_t codepoint, uint16_t width)
	{
		checkData();
		checkCharFree(codepoint);

		m_oChars.emplace(codepoint,
			Char(m_iBitsPerPixel, (m_iFixedWidth ? m_iFixedWidth : width), m_iHeight));
		return m_oChars[codepoint];
	}

	void BitmapFont::Face::set(uint32_t codepoint, const Char& data)
	{
		checkData();
		data.checkData();

		if ((m_iFixedWidth && data.m_iWidth != m_iFixedWidth) || m_iHeight != data.m_iHeight)
			throw std::exception("rl::BitmapFont::Face: Char is incompatible with this face");

		iterator it = m_oChars.find(codepoint);
		if (it != m_oChars.end())
			m_oChars[codepoint] = data;
		else
			m_oChars.emplace(codepoint, data);
	}

	void BitmapFont::Face::setFallback(uint32_t NewFallback)
	{
		checkCharSet(NewFallback);
		m_iFallback = NewFallback;
	}

	void BitmapFont::Face::setFamilyName(const char* szFamilyName)
	{
		if (!m_bData)
			return;

		m_sFamilyName = szFamilyName;
	}

	void BitmapFont::Face::setFaceName(const char* szFaceName)
	{
		if (!m_bData)
			return;

		m_sFaceName = szFaceName;
	}

	void BitmapFont::Face::setCopyright(const char* szCopyright)
	{
		if (!m_bData)
			return;

		m_sCopyright = szCopyright;
	}

	void BitmapFont::Face::setPoints(uint32_t points)
	{
		if (!m_bData)
			return;

		m_iPoints = points;
	}

	void BitmapFont::Face::setVersion(uint8_t part1, uint8_t part2, uint8_t part3, uint8_t part4)
	{
		checkData();

		m_oFileVersion[0] = part1;
		m_oFileVersion[1] = part2;
		m_oFileVersion[2] = part3;
		m_oFileVersion[3] = part4;
	}

	void BitmapFont::Face::setFlags(uint8_t flags)
	{
		if (flags & 0x02)
			throw std::exception("rl::BitmapFont::Face: Tried to set unknown flags");

		m_iFlags = flags;
	}

	void BitmapFont::Face::getVersion(uint8_t(&dest)[4]) const
	{
		if (!m_bData)
			return;

		memcpy(dest, m_oFileVersion, 4);
	}

	void BitmapFont::Face::remove(uint32_t codepoint)
	{
		checkData();

		iterator it = m_oChars.find(codepoint);
		if (it != m_oChars.end())
			m_oChars.erase(codepoint);
	}

	bool BitmapFont::Face::containsChar(uint32_t codepoint) const
	{
		return m_oChars.find(codepoint) != m_oChars.end();
	}

	const BitmapFont::Char& BitmapFont::Face::getChar(uint32_t codepoint) const
	{
		checkData();
		checkCharSet(codepoint);

		return m_oChars.at(codepoint);
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void BitmapFont::Face::checkData() const
	{
		if (!m_bData)
			throw std::exception("rl::BitmapFont::Face: Font face was not initialized");
	}

	void BitmapFont::Face::checkCharFree(uint32_t iCodepoint) const
	{
		if (m_oChars.find(iCodepoint) != m_oChars.end())
			throw std::exception("rl::BitmapFont::Face: The codepoint was already occupied");
	}

	void BitmapFont::Face::checkCharSet(uint32_t iCodepoint) const
	{
		if (m_oChars.find(iCodepoint) == m_oChars.end())
			throw std::exception("rl::BitmapFont::Face: The codepoint was not occupied");
	}










	/***********************************************************************************************
	 class BitmapFont
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}