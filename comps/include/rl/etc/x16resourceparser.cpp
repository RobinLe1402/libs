#include "x16resourceparser.hpp"

#include <stdint.h>
#include <Windows.h>





namespace rl
{

	bool NEResID::operator==(const NEResID& other) const
	{
		return ((other.bIsInt && this->bIsInt && other.ID_iID == this->ID_iID) ||
			(!other.bIsInt && !this->bIsInt && strcmp(other.ID_szName, this->ID_szName) == 0));
	}

	bool NEResID::operator!=(const NEResID& other) const { return !operator==(other); }



	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	x16ResourceParser::~x16ResourceParser() { clear(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool x16ResourceParser::load(const wchar_t* szPath)
	{
		clear();

		DWORD dwAttribs = GetFileAttributesW(szPath); // valid file?
		if (dwAttribs == INVALID_FILE_ATTRIBUTES || dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
		{
			m_iError = RL_X16RESPARSER_E_FILEERROR;
			return false;
		}

		DWORD dwBinType = NULL;
		if (!GetBinaryTypeW(szPath, &dwBinType)) // executable?
		{
			m_iError = RL_X16RESPARSER_E_NOEXE;
			return false;
		}

		if (dwBinType != SCS_WOW_BINARY) // 16-bit executable?
		{
			m_iError = RL_X16RESPARSER_E_NOEXE16;
			return false;
		}





		HANDLE hFile = CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);
		if (hFile == NULL)
		{
			m_iError = RL_X16RESPARSER_E_FILEERROR;
			return false;
		}

		DWORD dwRead = 0;
#define	READ(pDest, len) if (!ReadFile(hFile, pDest, len, &dwRead, NULL)) \
			{ m_iError = RL_X16RESPARSER_E_FILEERROR; CloseHandle(hFile); return false; }
#define READVAR(iDest) READ(&iDest, sizeof(iDest))



		// jump to and read offset to segmented .EXE header
		SetFilePointer(hFile, 0x3C, NULL, FILE_BEGIN); // jump to segmented .EXE header offset value
		WORD wHeaderOffset = 0;
		READVAR(wHeaderOffset);

		// jump to and read offset from segmented .EXE header to resource table
		SetFilePointer(hFile, wHeaderOffset + 0x24, NULL, FILE_BEGIN);
		WORD wResTabOffset = 0;
		READVAR(wResTabOffset);
		wResTabOffset += wHeaderOffset; // making wResTabOffset an absolute offset

		// jump to resource table
		SetFilePointer(hFile, wResTabOffset, NULL, FILE_BEGIN);

		WORD wAlignShift = 0; // alignment shift count for resource data
		READVAR(wAlignShift);

		uint16_t iStringsOffset = 0; // offset from resource table to first ID string

		// go through all resource types (and the associated resources)
		while (true)
		{
			NEResType oType = {};


			WORD wTypeID = 0;
			READVAR(wTypeID);
			if (wTypeID == 0) // type ID is 0 --> no more types
				break;
			if (wTypeID & 0x8000) // integer type --> disable high-order bit
			{
				wTypeID &= ~0x8000;
				oType.oID.bIsInt = true;
				oType.oID.ID_iID = wTypeID;
			}
			else // string pointer --> copy string
			{
				if (iStringsOffset == 0)
					iStringsOffset = wTypeID;

				const DWORD dwFilePointer = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

				SetFilePointer(hFile, wResTabOffset + wTypeID, NULL, FILE_BEGIN);
				BYTE len = 0;
				READVAR(len);
				char* szName = new char[(size_t)len + 1];
				READ(szName, len);
				szName[len] = 0; // set terminating zero
				m_oIDStrings.push_back(szName);
				oType.oID.ID_szName = szName;

				SetFilePointer(hFile, dwFilePointer, NULL, FILE_BEGIN);
			}

			WORD wResCount = 0;
			READVAR(wResCount);
			SetFilePointer(hFile, 4, NULL, FILE_CURRENT); // skip reserved DWORD

			// go through all resources of this type
			for (uint32_t i = 0; i < wResCount; i++)
			{
				NERes oRes = {};


				WORD wResOffset = 0;
				READVAR(wResOffset);
				oRes.iOffset = size_t(wResOffset) << wAlignShift;
				
				WORD wResSize = 0;
				READVAR(wResSize);
				oRes.iSize = size_t(wResSize) << wAlignShift;

				SetFilePointer(hFile, 2, NULL, FILE_CURRENT); // skip flag WORD (of no interest)

				WORD wResID = 0;
				READVAR(wResID);
				if (wResID & 0x8000) // integer type --> disable high-order bit
				{
					wResID &= ~0x8000;
					oRes.oID.bIsInt = true;
					oRes.oID.ID_iID = wResID;
				}
				else // string pointer --> copy string
				{
					if (iStringsOffset == 0)
						iStringsOffset = wResID;

					const DWORD dwFilePointer = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

					SetFilePointer(hFile, wResTabOffset + wResID, NULL, FILE_BEGIN);
					BYTE len = 0;
					READVAR(len);
					char* szName = new char[(size_t)len + 1];
					READ(szName, len);
					szName[len] = 0; // set terminating zero
					m_oIDStrings.push_back(szName);
					oRes.oID.ID_szName = szName;

					SetFilePointer(hFile, dwFilePointer, NULL, FILE_BEGIN);
				}

				oType.oResources.push_back(oRes);
				SetFilePointer(hFile, 4, NULL, FILE_CURRENT); // skip reserved DWORD
			}

			m_oTypes.push_back(oType);



			// strings reached? --> all resource types read
			if (iStringsOffset > 0)
			{
				DWORD dwFilePointer = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
				if (dwFilePointer - wResTabOffset == iStringsOffset)
					break;
			}
		}



#undef READ





		m_sFilepath = szPath;
		m_bParsed = true;
		return true;
	}

	void x16ResourceParser::clear()
	{
		if (!m_bParsed)
			return;


		m_bParsed = false;
		m_sFilepath.clear();

		// delete ID strings, clear resource list
		for (auto p : m_oIDStrings)
			delete[] p;
		m_oIDStrings.clear();

		unloadResources();

		m_oTypes.clear();

		m_iError = RL_X16RESPARSER_E_NOERROR;
	}

	bool x16ResourceParser::enumResourceTypes(std::function<bool(x16ResourceParser& oModule,
		LPCSTR lpType, LONG_PTR lParam)> lpEnumFunc, LONG_PTR lParam)
	{
		if (!m_bParsed)
			return false;


		for (auto& o : m_oTypes)
		{
			LPCSTR lp;
			if (o.oID.bIsInt)
				lp = MAKEINTRESOURCEA(o.oID.ID_iID);
			else
				lp = o.oID.ID_szName;

			if (!lpEnumFunc(*this, lp, lParam))
				return false;
		}

		return true;
	}

	bool x16ResourceParser::enumResourceNames(LPCSTR lpType,
		std::function<bool(x16ResourceParser&, LPCSTR, LPCSTR, LONG_PTR)> lpEnumFunc,
		LONG_PTR lParam)
	{
		if (!m_bParsed)
			return false;


		NEResID oID = {};
		if (IS_INTRESOURCE(lpType))
		{
			oID.bIsInt = true;
			oID.ID_iID = (uint16_t)lpType;
		}
		else
			oID.ID_szName = lpType;

		for (auto& o : m_oTypes)
		{
			if (o.oID != oID)
				continue;



			for (auto& oRes : o.oResources)
			{
				LPCSTR lp;
				if (oRes.oID.bIsInt)
					lp = MAKEINTRESOURCEA(oRes.oID.ID_iID);
				else
					lp = oRes.oID.ID_szName;

				if (!lpEnumFunc(*this, lpType, lp, lParam))
					return false;
			}
		}

		return true;
	}

	bool x16ResourceParser::loadResource(LPCSTR lpType, LPSTR lpName, const uint8_t** pData,
		size_t* size)
	{
		if (!m_bParsed)
			return false;


		NEResID oTypeID = {};
		if (IS_INTRESOURCE(lpType))
		{
			oTypeID.bIsInt = true;
			oTypeID.ID_iID = (uint16_t)lpType;
		}
		else
			oTypeID.ID_szName = lpType;

		NEResID oResID = {};
		if (IS_INTRESOURCE(lpName))
		{
			oResID.bIsInt = true;
			oResID.ID_iID = (uint16_t)lpType;
		}
		else
			oTypeID.ID_szName = lpName;



		for (auto& oType : m_oTypes)
		{
			if (oType.oID == oTypeID)
			{
				for (auto& oRes : oType.oResources)
				{
					if (oRes.oID == oResID)
					{
						if (oRes.pData == nullptr)
						{
							// load resource

							HANDLE hFile = CreateFileW(m_sFilepath.c_str(), GENERIC_READ,
								FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);

							if (hFile == INVALID_HANDLE_VALUE)
								return false;

							SetFilePointer(hFile, oRes.iOffset, NULL, FILE_BEGIN);
							uint8_t* pBuf = new uint8_t[oRes.iSize];
							DWORD dwRead = 0;
							bool result = ReadFile(hFile, pBuf, oRes.iSize, &dwRead, NULL);
							CloseHandle(hFile);

							if (result)
							{
								oRes.pData = pBuf;
								*pData = oRes.pData;
								*size = oRes.iSize;
							}
							else
							{
								delete[] pBuf;
							}
							return result;
						}
					}
				}
			}
		}

		return false;
	}

	void x16ResourceParser::unloadResource(LPCSTR lpType, LPSTR lpName)
	{
		if (!m_bParsed)
			return;


		NEResID oTypeID = {};
		if (IS_INTRESOURCE(lpType))
		{
			oTypeID.bIsInt = true;
			oTypeID.ID_iID = (uint16_t)lpType;
		}
		else
			oTypeID.ID_szName = lpType;

		NEResID oResID = {};
		if (IS_INTRESOURCE(lpName))
		{
			oResID.bIsInt = true;
			oResID.ID_iID = (uint16_t)lpName;
		}
		else
			oTypeID.ID_szName = lpName;



		for (auto& oType : m_oTypes)
		{
			if (oType.oID == oTypeID)
			{
				for (auto& oRes : oType.oResources)
				{
					if (oRes.oID == oResID)
					{
						if (oRes.pData != nullptr)
						{
							delete[] oRes.pData;
							oRes.pData = nullptr;
						}
						break;
					}
				}
				break;
			}
		}
	}

	void x16ResourceParser::unloadResources()
	{
		if (!m_bParsed)
			return;


		for (auto& oType : m_oTypes)
		{
			for (auto& oRes : oType.oResources)
			{
				if (oRes.pData != nullptr)
				{
					delete[] oRes.pData;
					oRes.pData = nullptr;
				}
			}
		}
	}

}