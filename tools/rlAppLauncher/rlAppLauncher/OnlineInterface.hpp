#pragma once
#ifndef ROBINLE_APPLAUNCHER_ONLINEINTERFACE
#define ROBINLE_APPLAUNCHER_ONLINEINTERFACE





#include <memory>
#include <cstdint>



class OnlineFile
{
public: // methods

	operator bool() const { return m_upData.get() != nullptr; }

	bool allocate(size_t iSize, bool bInitToZero = true);
	void clear();

	const uint8_t *data() const { return m_upData.get(); }
	uint8_t *data() { return m_upData.get(); }

	auto size() const { return m_iSize; }

private: // variables

	std::unique_ptr<uint8_t[]> m_upData;
	size_t m_iSize = 0;

};

class OnlineInterface
{
public: // methods

	static size_t     GetFileSize(const wchar_t *szURL);
	static OnlineFile DownloadFileToMemory(const wchar_t *szURL);
	static bool       DownloadFile(const wchar_t *szURL, const wchar_t *szLocalPath);


private: // variables



};





#endif // ROBINLE_APPLAUNCHER_ONLINEINTERFACE