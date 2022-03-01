#include "rl/data.online.hpp"

#include <Windows.h>
#include <atlbase.h> // CComPtr

#include <memory>

#pragma comment(lib, "Urlmon.lib")





namespace rl
{

	bool DownloadToMemory(const char* szURL, uint8_t** pDest, size_t* pLen)
	{
		if (FAILED(CoInitialize(NULL)))
			return false;

		CComPtr<IStream> pStream;
		if (FAILED(URLOpenBlockingStreamA(NULL, szURL, &pStream, 0, NULL)))
			return false;

		const size_t iBlockSize = 4096;
		size_t& len = *pLen;
		len = 0;
		size_t lenOld = 0;
		uint8_t* pBuf = nullptr;
		uint8_t* pBufCurrent = pBuf;

		HRESULT hr = ERROR_SUCCESS;
		DWORD dwBytesRead = 0;
		do
		{
			lenOld = len;
			len += iBlockSize;
			uint8_t* pBufOld = pBuf;
			pBuf = new uint8_t[len];
			memcpy_s(pBuf, len, pBufOld, lenOld);
			delete[] pBufOld;
			pBufCurrent = pBuf + lenOld;


			hr = pStream->Read(pBufCurrent, iBlockSize, &dwBytesRead);
		} while (SUCCEEDED(hr) && dwBytesRead == iBlockSize && hr != S_FALSE);

		if (FAILED(hr))
		{
			delete[] pBuf;
			CoUninitialize();
			return false;
		}

		if (dwBytesRead != iBlockSize)
		{
			uint8_t* pBufOld = pBuf;

			len = len - iBlockSize + dwBytesRead;
			pBuf = new uint8_t[len];
			memcpy_s(pBuf, len, pBufOld, len);
			delete[] pBufOld;
		}

		*pDest = pBuf;
		CoUninitialize();
		return true;
	}

	
	/***********************************************************************************************
	 class XXX
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
	// OPERATORS
	
	// operators
	
	
	
	
	
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