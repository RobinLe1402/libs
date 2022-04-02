#include "include/DataReader.hpp"

#include <fstream>


using BinStream = std::basic_ifstream<uint8_t>;

bool LoadFileToMemory_Internal(BinStream& oStream, std::vector<uint8_t>& oDest)
{
	if (!oStream)
		return false;

	oDest.clear();

	oStream.seekg(0, std::ios::end);
	oDest.resize(oStream.tellg());
	
	oStream.seekg(0);
	oStream.read(&oDest[0], oDest.size());

	return true;
}

bool LoadFileToMemory(const wchar_t* szFilepath, std::vector<uint8_t>& oDest)
{
	BinStream oStream(szFilepath, std::ios::binary);
	return LoadFileToMemory_Internal(oStream, oDest);
}

bool LoadFileToMemory(const char* szFilepath, std::vector<uint8_t>& oDest)
{
	BinStream oStream(szFilepath, std::ios::binary);
	return LoadFileToMemory_Internal(oStream, oDest);
}





DataReader::DataReader(const void* pData, size_t iSize) : m_pData(pData), m_iSize(iSize) { }

bool DataReader::seekPos(size_t iOffset)
{
	if (iOffset > m_iSize)
	{
		if (!m_bExceptions)
			return false;

		throw DataReaderException();
	}

	m_iPos = iOffset;
	return true;
}

bool DataReader::seekPos(ptrdiff_t iOffset, DataSeekMethod eMethod)
{
	switch (eMethod)
	{
	case DataSeekMethod::Current:
		iOffset += (ptrdiff_t)m_iPos;
		break;
	case DataSeekMethod::End:
		iOffset += (ptrdiff_t)m_iSize;
		break;
	}

	if (iOffset < 0 || iOffset > (ptrdiff_t)m_iSize)
	{
		if (!m_bExceptions)
			return false;

		throw DataReaderException();
	}

	m_iPos = (size_t)iOffset;
	return true;
}

bool DataReader::read(void* pDest, size_t iSize)
{
	if (remainingBytes() < iSize)
	{
		if (!m_bExceptions)
			return false;

		else
			throw DataReaderException();
	}

	memcpy_s(pDest, iSize, (const uint8_t*)m_pData + m_iPos, iSize);
	m_iPos += iSize;
	return true;
}
