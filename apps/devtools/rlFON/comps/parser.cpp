#include "parser.hpp"

#include <stdint.h>





namespace rl
{

	bool CharSetToCodePage(uint8_t charset, uint16_t& codepage)
	{
		switch (charset)
		{
#define CHARSET(ID, CP) case ID: codepage = CP; break

			CHARSET(ANSI_CHARSET, 1252);
			CHARSET(ARABIC_CHARSET, 1256);
			CHARSET(BALTIC_CHARSET, 1257);
			CHARSET(CHINESEBIG5_CHARSET, 950); // traditional chinese
			CHARSET(EASTEUROPE_CHARSET, 1250);
			CHARSET(GB2312_CHARSET, 936); // simple chinese
			CHARSET(GREEK_CHARSET, 1253);
			CHARSET(HANGEUL_CHARSET, 949);
			CHARSET(HEBREW_CHARSET, 1255);
			CHARSET(JOHAB_CHARSET, 1361);
			CHARSET(RUSSIAN_CHARSET, 1251);
			CHARSET(THAI_CHARSET, 874);
			CHARSET(SHIFTJIS_CHARSET, 932);
			CHARSET(TURKISH_CHARSET, 1254);

#undef CHARSET
		default:
			codepage = RL_CP_UNKNOWN;
			return false;
		}
		return true;
	}

	bool CharSetIDToCodePoint(uint8_t ch, uint8_t charset, uint32_t& codepoint)
	{
		uint16_t cp = RL_CP_UNKNOWN;
		if (!CharSetToCodePage(charset, cp))
			return false;

		wchar_t sz[2] = {};
		if (!MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, (char*)&ch, 1, sz, 2))
			return false;



		// UTF-16 decoding

		if ((sz[0] & 0xFC00) != 0xD800) // no surrogates
			codepoint = sz[0];
		else // surrogates
		{
			uint32_t iTMP = 0;

			iTMP = sz[1] & 0x3FFF;
			iTMP |= uint32_t(sz[0] & 0x3FFF) << 10;
			iTMP += 0x10000;

			codepoint = iTMP;
		}

		return true;
	}

	bool CodePointToCharSetID(uint32_t codepoint, uint8_t charset, uint8_t& ch)
	{
		uint16_t cp = RL_CP_UNKNOWN;
		if (!CharSetToCodePage(charset, cp))
			return false;

		wchar_t sz[3];
		memset(&sz, 0, 3 * sizeof(wchar_t));
		if (codepoint < 0xFFFF)
			sz[0] = (wchar_t)codepoint;
		else // encode to UTF-16
		{
			// write surrogates
			sz[0] = 0xD800;
			sz[1] = 0xDC00;

			// write value
			const uint32_t iVal = codepoint - 0x10000;

			sz[1] |= iVal & 0x3F;
			sz[0] |= (iVal >> 10) & 0x3F;
		}

		uint8_t iTMP = 0;
		BOOL bReplaced;
		char cReplacement = 0;
		if (!WideCharToMultiByte(cp, WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS, sz, -1,
			(char*)&iTMP, 1, &cReplacement, &bReplaced) || bReplaced)
			return false;

		ch = iTMP;
		return true;
	}


	bool LANGANDCODEPAGE::operator<(const LANGANDCODEPAGE& other) const
	{
		return ((uint32_t(this->wLanguage) << 16) | this->wCodePage) <
			((uint32_t(other.wLanguage) << 16) | other.wCodePage);
	}

	VERSIONINFO_STRINGS::VERSIONINFO_STRINGS()
	{
		Comments.resize(1);
		CompanyName.resize(1);
		FileDescription.resize(1);
		FileVersion.resize(1);
		InternalName.resize(1);
		LegalCopyright.resize(1);
		LegalTrademarks.resize(1);
		OriginalFilename.resize(1);
		ProductName.resize(1);
		ProductVersion.resize(1);
		PrivateBuild.resize(1);
		SpecialBuild.resize(1);
	}





	/***********************************************************************************************
	 class MicrosoftRasterChar
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	MicrosoftRasterChar::MicrosoftRasterChar(uint16_t iWidth, uint16_t iHeight, uint8_t* pData,
		size_t len)
	{
		m_iDataSize = len;
		m_pData = new uint8_t[len];
		memcpy_s(m_pData, len, pData, len);
		m_iCharWidth = iWidth;
		m_iCharHeight = iHeight;
	}

	MicrosoftRasterChar::MicrosoftRasterChar(const MicrosoftRasterChar& other) { *this = other; }

	MicrosoftRasterChar::~MicrosoftRasterChar() { clear(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void MicrosoftRasterChar::clear()
	{
		if (!hasData())
			return;

		delete[] m_pData;
		m_pData = nullptr;
		m_iDataSize = 0;
		m_iCharHeight = 0;
		m_iCharWidth = 0;
	}

	bool MicrosoftRasterChar::hasData() const { return m_pData != nullptr; }

	bool MicrosoftRasterChar::getPixel(uint16_t x, uint16_t y) const
	{
		if (!hasData())
			return false;

		if (x >= m_iCharWidth || y >= m_iCharHeight)
			return false;



		size_t iByte = (size_t)m_iCharHeight * (x / 8) + y;
		uint8_t iData = m_pData[iByte];
		return (iData >> (7 - (x % 8))) & 1;
	}

	MicrosoftRasterChar& MicrosoftRasterChar::operator=(const MicrosoftRasterChar& other)
	{
		clear();
		if (!other.hasData())
			return *this;


		m_pData = new uint8_t[other.m_iDataSize];
		m_iDataSize = other.m_iDataSize;
		memcpy_s(m_pData, m_iDataSize, other.m_pData, other.m_iDataSize);
		m_iCharHeight = other.m_iCharHeight;
		m_iCharWidth = other.m_iCharWidth;

		return *this;
	}










	/***********************************************************************************************
	 class MicrosoftRasterFont
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	MicrosoftRasterFont::MicrosoftRasterFont(const uint8_t* pData, size_t size)
	{
		create(pData, size);
	}

	MicrosoftRasterFont::MicrosoftRasterFont(const MicrosoftRasterFont& other) { *this = other; }

	MicrosoftRasterFont::~MicrosoftRasterFont() { clear(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool MicrosoftRasterFont::create(const uint8_t* pData, size_t size)
	{
		clear();

		size_t iOffset = 0;

#define READ(pDest, len)									\
		{													\
			if (size - iOffset < len)						\
			{												\
				m_iError = RL_FONPARSER_E_FILEERROR;		\
				return false;								\
			}												\
			memcpy_s(pDest, len, pData + iOffset, len);	\
			iOffset += len;									\
		}
#define READVAR(var) READ(&var, sizeof(var))


		// copy header
		READVAR(m_oHeader.oHeader);

		// cancel if vector font
		if (m_oHeader.oHeader.dfType & 1)
		{
			m_iError = RL_FONPARSER_E_NORASTERFONTFILE;
			return false;
		}

		m_iDataSize = m_oHeader.oHeader.dfSize;
		m_pData = new uint8_t[m_iDataSize];
		memcpy_s(m_pData, m_iDataSize, pData, m_iDataSize);


		size_t len;
		const char* sz;

		switch (m_oHeader.oHeader.dfVersion)
		{
		case 0x0200: // FNT 2.0
		{
			// copy header strings
			sz = (const char*)m_pData + m_oHeader.oHeader.dfDevice;
			len = strlen(sz);
			if (len > 0)
			{
				m_oHeader.sDeviceName.resize(len);
				strcpy_s(&m_oHeader.sDeviceName[0], len, sz);
			}

			sz = (const char*)m_pData + m_oHeader.oHeader.dfFace;
			len = strlen(sz);
			m_oHeader.sFaceName.resize(len);
			strcpy_s(&m_oHeader.sFaceName[0], len + 1, sz);

#pragma pack(push, 1) // disable padding
			struct FONTHDR_EX
			{
				DWORD dfBitsOffset;
				BYTE  dfReserved;
			};
#pragma pack(pop)
			FONTHDR_EX oHdrEx = {};
			READVAR(oHdrEx);


			// holds the data of one element in the char table of a FONT resource
			struct FONTCHARENTRY
			{
				WORD  dfCharWidth;
				WORD dfBitmapOffset;
			};

			// "+ 1" because data is zero-based
			const uint16_t iCharCount = m_oHeader.oHeader.dfLastChar -
				m_oHeader.oHeader.dfFirstChar + 1;

			for (uint16_t i = 0; i < iCharCount; i++)
			{
				FONTCHARENTRY oChar = {};
				READVAR(oChar);
				const size_t len = (size_t)ceil(oChar.dfCharWidth / 8.0f) *
					m_oHeader.oHeader.dfPixHeight;
				uint8_t* pTMP = new uint8_t[len];
				memcpy_s(pTMP, len, m_pData + oChar.dfBitmapOffset, len);
				m_oChars.emplace(m_oHeader.oHeader.dfFirstChar + i,
					MicrosoftRasterChar(oChar.dfCharWidth, m_oHeader.oHeader.dfPixHeight, pTMP,
						len));
				delete[] pTMP;
			}
		}
		break;





		case 0x0300: // FNT 3.0
		{
			// copy header strings
			sz = (const char*)m_pData + m_oHeader.oHeader.dfDevice;
			len = strlen(sz);
			if (len > 0)
			{
				m_oHeader.sDeviceName.resize(len);
				strcpy_s(&m_oHeader.sDeviceName[0], len, sz);
			}

			sz = (const char*)m_pData + m_oHeader.oHeader.dfFace;
			len = strlen(sz);
			m_oHeader.sFaceName.resize(len);
			strcpy_s(&m_oHeader.sFaceName[0], len + 1, sz);


#pragma pack(push, 1) // disable padding
			struct FONTHDR_EX
			{
				DWORD dfBitsOffset;
				BYTE  dfReserved;
				DWORD dfFlags;
				WORD  dfAspace;
				WORD  dfBspace;
				WORD  dfCspace;
				DWORD dfColorPointer;
				BYTE  dfReserved1[16];
			};

			// holds the data of one element in the char table of a FONT resource
			struct FONTCHARENTRY
			{
				WORD  dfCharWidth;
				DWORD dfBitmapOffset;
			};
#pragma pack(pop)
			FONTHDR_EX oHdrEx = {};
			READVAR(oHdrEx);

			// "+ 1" because data is zero-based
			const uint16_t iCharCount = m_oHeader.oHeader.dfLastChar -
				m_oHeader.oHeader.dfFirstChar + 1;

			for (uint16_t i = 0; i < iCharCount; i++)
			{
				FONTCHARENTRY oChar = {};
				READVAR(oChar);
				const size_t len = (size_t)ceil(oChar.dfCharWidth / 8.0f) * m_oHeader.oHeader.dfPixHeight;
				uint8_t* pTMP = new uint8_t[len];
				memcpy_s(pTMP, len, m_pData + oChar.dfBitmapOffset, len);
				m_oChars.emplace(m_oHeader.oHeader.dfFirstChar + i, MicrosoftRasterChar(oChar.dfCharWidth,
					m_oHeader.oHeader.dfPixHeight, pTMP, len));
				delete[] pTMP;
			}
		}
		break;





		default: // unknown version
			m_iError = RL_FONPARSER_E_UNKNOWNVERSION;
			return false;
		}

#undef READVAR
#undef READ


		m_bData = true;
		return true;
	}

	void MicrosoftRasterFont::clear()
	{
		if (m_pData == nullptr)
			return;


		m_bData = false;
		m_oHeader = {};
		m_oChars.clear();
		m_iHeight = 0;
	}

	bool MicrosoftRasterFont::hasData() const { return (m_pData != nullptr); }

	bool MicrosoftRasterFont::getData(const uint8_t*& ptr, size_t& size) const
	{
		if (m_pData == nullptr)
			return false;


		ptr = m_pData;
		size = m_iDataSize;

		return true;
	}

	bool MicrosoftRasterFont::getChar(uint8_t ch, MicrosoftRasterChar& dest) const
	{
		if (m_pData == nullptr || !containsChar(ch))
			return false;

		dest = m_oChars.at(ch);
		return true;
	}

	bool MicrosoftRasterFont::getHeader(FONTHDR_STRINGS& dest) const
	{
		if (m_pData == nullptr)
			return false;

		dest = m_oHeader;
		return true;
	}

	bool MicrosoftRasterFont::containsChar(uint8_t ch) const
	{
		if (m_pData == nullptr)
			return false;

		return m_oChars.find(ch) != m_oChars.end();
	}

	MicrosoftRasterFont& MicrosoftRasterFont::operator=(const MicrosoftRasterFont& other)
	{
		clear();
		if (other.m_pData == nullptr)
			return *this;

		create(other.m_pData, other.m_iDataSize);

		return *this;
	}










	/***********************************************************************************************
	 class MicrosoftFONParser
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	MicrosoftFONParser::~MicrosoftFONParser() { clear(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool MicrosoftFONParser::parse(const wchar_t* szPath)
	{
		clear();
		std::unique_lock<std::mutex> lm(m_mux);

		// does path exist as a file?
		const DWORD dwFileAttribs = GetFileAttributesW(szPath);
		if (dwFileAttribs == INVALID_FILE_ATTRIBUTES || dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY)
		{
			m_iError = RL_FONPARSER_E_FILEERROR;
			return false;
		}

		// 16-bit executable?
		DWORD dwBinType = 0;
		if (!GetBinaryTypeW(szPath, &dwBinType) || dwBinType != SCS_WOW_BINARY)
		{
			m_iError = RL_FONPARSER_E_NOFONTFILE;
			return false;
		}





		HANDLE hFile = CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			m_iError = RL_FONPARSER_E_FILEERROR;
			return false;
		}

		DWORD dwRead = 0;

#define READ(pDest, len)													\
		if (!ReadFile(hFile, pDest, len, &dwRead, NULL) || dwRead != len)	\
		{																	\
			m_iError = RL_FONPARSER_E_FILEERROR;							\
			CloseHandle(hFile);												\
			return false;													\
		}
#define READVAR(iDest) READ(&iDest, sizeof(iDest))



		// jump to and read offset to segmented .EXE header
		SetFilePointer(hFile, 0x3C, NULL, FILE_BEGIN); // jump to segmented .EXE header offset value
		WORD wOffset_NEHeader = 0;
		READVAR(wOffset_NEHeader);

		// jump to and read offset from segmented .EXE header to resource table
		SetFilePointer(hFile, wOffset_NEHeader + 0x24, NULL, FILE_BEGIN);
		WORD wOffset_ResTab = 0;
		READVAR(wOffset_ResTab);
		wOffset_ResTab += wOffset_NEHeader; // making wResTabOffset an absolute offset

		// jump to resource table
		SetFilePointer(hFile, wOffset_ResTab, NULL, FILE_BEGIN);

		WORD wAlignShift = 0; // alignment shift count for resource data
		READVAR(wAlignShift);



		struct NEResID
		{
			bool bInt;

			uint16_t	ID_i;
			std::string	ID_s;

			bool operator==(const NEResID& other)
			{
				return ((bInt && other.bInt && ID_i == other.ID_i) ||
					(!bInt && !other.bInt && ID_s == other.ID_s));
			}
		};
		struct NERes
		{
			NEResID oID;
			WORD wOffset; // has to be shifted left wAlignShift times
			WORD wSize; // has to be shifted left wAlignShift times
		};
		struct NEResType
		{
			WORD iID;
			std::vector<NERes> oResources; // all resources of this type
		};


		NEResType oTypeVERSIONINFO = {};
		oTypeVERSIONINFO.iID = 16; // VERSIONINFO = 16
		NEResType oTypeFONT = {};
		oTypeFONT.iID = 8; // FONT = 8



		// go through all resources, extract FONT and VERSIONINFO data
		WORD wTypeID = 0;
		READVAR(wTypeID); // read first resource type
		constexpr LONG iResTableEntrySize = 4 * sizeof(WORD) + sizeof(DWORD);
		while (wTypeID != 0)
		{
			bool bRead = wTypeID & 0x8000; // ID must be numeric (--> standard types)
			WORD wID = 0;
			if (bRead)
			{
				wID = wTypeID & ~0x8000;
				bRead = wID == oTypeVERSIONINFO.iID || wID == oTypeFONT.iID;
			}

			WORD wResCount = 0;
			READVAR(wResCount);
			SetFilePointer(hFile, sizeof(DWORD), NULL, FILE_CURRENT); // skip reserved DWORD

			if (!bRead) // type is of no interest --> skip it's resources
				SetFilePointer(hFile, wResCount * iResTableEntrySize, NULL, FILE_CURRENT);
			else // read this type's resources
			{
				NEResType& oType = (wID == oTypeVERSIONINFO.iID ? oTypeVERSIONINFO : oTypeFONT);

				for (uint32_t i = 0; i < wResCount; i++)
				{
					NERes oRes = {};

					READVAR(oRes.wOffset);
					READVAR(oRes.wSize);

					// skip flag WORD (of no interest)
					SetFilePointer(hFile, sizeof(WORD), NULL, FILE_CURRENT);

					// read + process resource ID
					WORD wResID = 0;
					READVAR(wResID);
					if (wResID & 0x8000) // integer type --> save without high-order bit
					{
						oRes.oID.bInt = true;
						oRes.oID.ID_i = wResID & ~0x8000;
					}
					else // string pointer --> read string
					{
						// save current file pointer
						const DWORD dwPrev = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

						SetFilePointer(hFile, wOffset_ResTab + wResID, NULL, FILE_BEGIN);
						BYTE len = 0;
						READVAR(len);
						oRes.oID.ID_s.resize((size_t)len + 1);
						READ(&oRes.oID.ID_s[0], len);

						// jump back to resource table
						SetFilePointer(hFile, dwPrev, NULL, FILE_BEGIN);
					}
					SetFilePointer(hFile, sizeof(DWORD), NULL, FILE_CURRENT); // skip reserved DWORD

					// add resource to its type's resource list
					oType.oResources.push_back(oRes);
				}
			}

			READVAR(wTypeID); // read type of next resource
		}



		// process VERSIONINFO resource
		if (oTypeVERSIONINFO.oResources.size() == 0)
			m_iWarnings |= RL_FONPARSER_W_NOVERSIONINFO;
		else
		{
			// read VERSIONINFO resource to memory
			auto& oRes = oTypeVERSIONINFO.oResources[0];
			uint8_t* pData = new uint8_t[(size_t)oRes.wSize << wAlignShift];
			SetFilePointer(hFile, (size_t)oRes.wOffset << wAlignShift, NULL, FILE_BEGIN);
			READ(pData, (size_t)oRes.wSize << wAlignShift);

			struct VERSIONNODEHDR
			{
				WORD cbNode;
				WORD cbData;
			};

			VERSIONNODEHDR oNode_VERSIONINFO = *(VERSIONNODEHDR*)pData;
			const char* szVERSIONINFOName = (const char*)(pData + 2 * sizeof(WORD));
			if (strcmp(szVERSIONINFOName, "VS_VERSION_INFO") == 0)
			{
				size_t iOffset = 2 * sizeof(WORD) + strlen(szVERSIONINFOName) + 1;
				if (oNode_VERSIONINFO.cbData == sizeof(VS_FIXEDFILEINFO))
				{
					VS_FIXEDFILEINFO oFI = *(VS_FIXEDFILEINFO*)(pData + iOffset);
					iOffset += sizeof(VS_FIXEDFILEINFO);

					m_oVI.FileVersion = (uint64_t(oFI.dwFileVersionMS) << (sizeof(DWORD) * 8)) |
						oFI.dwFileVersionLS;
					m_oVI.ProductVersion = (uint64_t(oFI.dwProductVersionMS) << (sizeof(DWORD) * 8)) |
						oFI.dwProductVersionLS;
					m_oVI.FileFlagsMask = oFI.dwFileFlagsMask;
					m_oVI.FileFlags = oFI.dwFileFlags;
					m_oVI.FileOS = oFI.dwFileOS;
					m_oVI.FileType = oFI.dwFileType;
					m_oVI.FileSubtype = oFI.dwFileSubtype;
					m_oVI.FileDate = (uint64_t(oFI.dwFileDateMS) << (sizeof(DWORD) * 8)) |
						oFI.dwFileDateLS;

					while (iOffset < oNode_VERSIONINFO.cbNode)
					{
						uint8_t iPadding = 0;

#define ALIGNOFFSET() \
						{												\
							iPadding = iOffset % sizeof(DWORD);			\
							if (iPadding > 0)							\
								iOffset += sizeof(DWORD) - iPadding;	\
						}

						size_t iOffset_Header = iOffset;
						VERSIONNODEHDR oNodeHeader = *(VERSIONNODEHDR*)(pData + iOffset);
						iOffset += sizeof(VERSIONNODEHDR);
						const char* szNodeName = (const char*)(pData + iOffset);
						iOffset += strlen(szNodeName) + 1;
						if (strcmp(szNodeName, "StringFileInfo") != 0)
						{
							iOffset = iOffset_Header + oNodeHeader.cbNode;
							ALIGNOFFSET();

							continue;
						}

						ALIGNOFFSET();

						while (iOffset < iOffset_Header + oNodeHeader.cbNode)
						{
							VERSIONNODEHDR oLangHeader = *(VERSIONNODEHDR*)(pData + iOffset);
							iOffset += sizeof(VERSIONNODEHDR);
							const char* szLangAndCP = (const char*)pData + iOffset;
							iOffset += strlen(szLangAndCP) + 4;

							uint32_t iLANGANDCP = strtol(szLangAndCP, nullptr, 16);
							LANGANDCODEPAGE langcp =
							{ (WORD)(iLANGANDCP >> sizeof(WORD)), (WORD)iLANGANDCP };

							VERSIONINFO_STRINGS oStrings;
							const size_t iOffset_Prev = iOffset;

							while (iOffset < iOffset_Prev + oLangHeader.cbNode)
							{
								VERSIONNODEHDR oStrHeader = *(VERSIONNODEHDR*)(pData + iOffset);
								iOffset += sizeof(VERSIONNODEHDR);
								const char* szStrName = (const char*)(pData + iOffset);
								iOffset += strlen(szStrName) + 1;
								ALIGNOFFSET();
								const char* szStrVal = (const char*)(pData + iOffset);
								iOffset += strlen(szStrVal) + 1;
								ALIGNOFFSET();

								const size_t len = MultiByteToWideChar(langcp.wCodePage,
									MB_ERR_INVALID_CHARS, szStrVal, -1, 0, 0);

								wchar_t* pWideStrVal = new wchar_t[len];
								MultiByteToWideChar(langcp.wCodePage, MB_ERR_INVALID_CHARS,
									szStrVal, -1, pWideStrVal, (int)len);

								if (strcmp(szStrName, "Comments") == 0)
									oStrings.Comments = pWideStrVal;
								else if (strcmp(szStrName, "CompanyName") == 0)
									oStrings.CompanyName = pWideStrVal;
								else if (strcmp(szStrName, "FileDescription") == 0)
									oStrings.FileDescription = pWideStrVal;
								else if (strcmp(szStrName, "FileVersion") == 0)
									oStrings.FileVersion = pWideStrVal;
								else if (strcmp(szStrName, "InternalName") == 0)
									oStrings.InternalName = pWideStrVal;
								else if (strcmp(szStrName, "LegalCopyright") == 0)
									oStrings.LegalCopyright = pWideStrVal;
								else if (strcmp(szStrName, "LegalTrademarks") == 0)
									oStrings.LegalTrademarks = pWideStrVal;
								else if (strcmp(szStrName, "OriginalFilename") == 0)
									oStrings.OriginalFilename = pWideStrVal;
								else if (strcmp(szStrName, "ProductName") == 0)
									oStrings.ProductName = pWideStrVal;
								else if (strcmp(szStrName, "ProductVersion") == 0)
									oStrings.ProductVersion = pWideStrVal;
								else if (strcmp(szStrName, "PrivateBuild") == 0)
									oStrings.PrivateBuild = pWideStrVal;
								else if (strcmp(szStrName, "SpecialBuild") == 0)
									oStrings.SpecialBuild = pWideStrVal;

								delete[] pWideStrVal;
							}

							ALIGNOFFSET();


							m_oVI.LangStrings.emplace(langcp, oStrings);

							//const char* szStrNodeName
						}

						break; // only interested in StringFileInfo. No more information needed.
					}
				}
				else
					m_iWarnings |= RL_FONPARSER_W_NOVERSIONINFO;
			}
			else
				m_iWarnings |= RL_FONPARSER_W_NOVERSIONINFO;

			delete[] pData;
		}

		if (oTypeFONT.oResources.size() == 0) // no FONT resource --> cancel
		{
			m_iError = RL_FONPARSER_E_NOFONTRESOURCE;
			CloseHandle(hFile);
			m_oVI = {};
			return false;
		}
		else // FONT resource(s) --> create MicrosoftRasterFont objects
		{
			for (auto& oRes : oTypeFONT.oResources)
			{
				const size_t len = (size_t)oRes.wSize << wAlignShift;
				uint8_t* pData = new uint8_t[len];
				SetFilePointer(hFile, (size_t)oRes.wOffset << wAlignShift, NULL, FILE_BEGIN);
				READ(pData, (DWORD)len);

				// mainly for the MSVC compiler, else: Warning C6385 - "Reading invalid data"
				if (len < 67)
				{
					m_iWarnings |= RL_FONPARSER_W_INVALIDDATA;
					delete[] pData;
					continue;
				}

				uint8_t dfTypeByte1 = pData[66]; // only first byte of WORD dfType

				if ((dfTypeByte1 & 1) == 0) // only process bitmap fonts
				{
					if (!oRes.oID.bInt) // a FONT's resource ID must be numeric(?) TODO: check!
						m_iWarnings |= RL_FONPARSER_W_INVALIDDATA;

					MicrosoftRasterFont font(pData, len);

					// font data couldn't be parsed --> add warning
					if (!font.hasData())
					{
						const uint8_t iError = font.getError();
						if (iError == RL_FONPARSER_E_UNKNOWNVERSION
							|| iError == RL_FONPARSER_E_NOFONTRESOURCE)
						{
							m_iError = iError;
							delete[] pData;
							return false;
						}
						else
							m_iWarnings = RL_FONPARSER_W_INVALIDDATA;
					}
					else // font data could be parsed --> add to font list
						m_oFonts.emplace(oRes.oID.ID_i, font);
				}

				delete[] pData;
			}
		}



#undef READVAR
#undef READ




		m_bParsed = true;

		if (m_oFonts.size() == 0) // no font resource --> error
		{
			m_iError = RL_FONPARSER_E_NORASTERFONTRESOURCE;
			return false;
		}

		return true;
	}

	void MicrosoftFONParser::clear()
	{
		std::unique_lock<std::mutex> lm(m_mux);
		if (!m_bParsed)
			return;

		m_bParsed = false;
		m_iError = RL_FONPARSER_E_NOERROR;
		m_iWarnings = 0;
		m_oVI = {};
		m_oFonts.clear();
	}

	bool MicrosoftFONParser::containsData() const
	{
		std::unique_lock<std::mutex> lm(m_mux);
		return m_bParsed;
	}

	bool MicrosoftFONParser::getVersionInfo(VERSIONINFO& dest) const
	{
		std::unique_lock<std::mutex> lm(m_mux);

		if (!m_bParsed)
			return false;

		dest = m_oVI;
		return true;
	}

	bool MicrosoftFONParser::getFontDir(std::vector<FONTDIRENTRY>& dest) const
	{
		std::unique_lock<std::mutex> lm(m_mux);
		if (!m_bParsed)
			return false;

		dest.clear();
		FONTDIRENTRY o = {};
		for (const auto& oFont : m_oFonts)
		{
			o.fontOrdinal = oFont.first;


			FONTHDR_STRINGS hdr = {};

			oFont.second.getHeader(hdr);
			o.oHeader = hdr.oHeader;
			o.sDeviceName = hdr.sDeviceName;
			o.sFaceName = hdr.sFaceName;


			dest.push_back(o);
		}
		return true;
	}

	bool MicrosoftFONParser::getFont(WORD fontOrdinal, MicrosoftRasterFont& dest) const
	{
		std::unique_lock<std::mutex> lm(m_mux);

		if (m_oFonts.find(fontOrdinal) == m_oFonts.end())
			return false;

		dest = m_oFonts.at(fontOrdinal);
		return true;
	}

	uint8_t MicrosoftFONParser::getParseError()
	{
		std::unique_lock<std::mutex> lm(m_mux);
		return m_iError;
	}

	uint8_t MicrosoftFONParser::getWarnings()
	{
		std::unique_lock<std::mutex> lm(m_mux);
		return m_iWarnings;
	}

}