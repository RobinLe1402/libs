#include "OnlineFile.hpp"

#include <Windows.h>
#include <atlbase.h> // CComPtr
#include <WinInet.h>

#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Wininet.lib")





size_t OnlineFile::GetFileSize(const wchar_t *szURL)
{
	HINTERNET hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL)
		return 0;

	HINTERNET hInternetURL =
		InternetOpenUrlW(hInternet, szURL, NULL, NULL,
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, NULL);
	if (hInternetURL == NULL)
	{
		InternetCloseHandle(hInternet);
		return 0;
	}

	DWORD dwStatus  = 0;
	DWORD dwSize    = 0;
	DWORD dwBufSize = sizeof(DWORD);
	if (HttpQueryInfoW(hInternetURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
		&dwStatus, &dwBufSize, NULL))
	{
		if (dwStatus == HTTP_STATUS_OK)
		{
			if (!HttpQueryInfoW(hInternetURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
				&dwSize, &dwBufSize, NULL))
				dwSize = 0;
		}
	}

	InternetCloseHandle(hInternetURL);
	InternetCloseHandle(hInternet);

	return dwSize;
}

OnlineFile OnlineFile::DownloadFileToMemory(const wchar_t *szURL)
{
	OnlineFile oResult;

	if (FAILED(CoInitialize(NULL)))
		return oResult;

	const auto iFileSize = GetFileSize(szURL);
	if (iFileSize == 0)
		return oResult;

	oResult.allocate(iFileSize);

	CComPtr<IStream> pStream;
	DeleteUrlCacheEntryW(szURL);
	if (FAILED(URLOpenBlockingStreamW(NULL, szURL, &pStream, 0, NULL)))
		oResult.clear();
	else
	{
		HRESULT  hr         = ERROR_SUCCESS;
		size_t   iOffset    = 0;
		ULONG    iBytesRead = 0;
		do
		{
			iOffset += iBytesRead;
			hr = pStream->Read(oResult.data() + iOffset, ULONG(oResult.size() - iOffset),
				&iBytesRead);
		} while (SUCCEEDED(hr) && iOffset + iBytesRead < oResult.size() && hr != S_FALSE);

		if (FAILED(hr) || iOffset + iBytesRead != oResult.size())
			oResult.clear();
	}

	CoUninitialize();

	return oResult;
}

bool OnlineFile::DownloadFile(const wchar_t *szURL, const wchar_t *szLocalPath)
{
	const auto hr = URLDownloadToFileW(NULL, szURL, szLocalPath, NULL, NULL);
	return SUCCEEDED(hr);
}



bool OnlineFile::allocate(size_t iSize, bool bInitToZero)
{
	if (iSize == 0)
		return false;

	m_upData = std::make_unique<uint8_t[]>(iSize);
	m_iSize  = iSize;

	if (bInitToZero)
		memset(m_upData.get(), 0, iSize);

	return true;
}

void OnlineFile::clear()
{
	m_upData = nullptr;
	m_iSize  = 0;
}

OnlineFile::PosInt OnlineFile::seekPos(OnlineFile::SeekPos ePos, OnlineFile::OffsetInt iOffset)
	const
{
	if (m_iSize == 0)
		return 0;

	switch (ePos)
	{
	case SeekPos::Begin:
		if (iOffset < 0)
			m_iOffset = 0;
		else if ((size_t)iOffset <= m_iSize)
			m_iOffset = iOffset;
		else
			m_iOffset = m_iSize;
		break;

	case SeekPos::Current:
		if (iOffset >= 0)
		{
			if (m_iOffset + iOffset <= m_iSize)
				m_iOffset += iOffset;
			else
				m_iOffset = m_iSize;
		}
		else
		{
			if (m_iOffset + iOffset >= 0)
				m_iOffset += iOffset;
			else
				m_iOffset = 0;
		}
		break;

	case SeekPos::End:
		if (iOffset >= 0)
			m_iOffset = m_iSize;
		else
		{
			if (m_iSize + iOffset >= 0)
				m_iOffset = m_iSize + iOffset;
			else
				m_iOffset = 0;
		}
		break;
	}

	return m_iOffset;
}
