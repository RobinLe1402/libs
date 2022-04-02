#pragma once
#ifndef ROBINLE_RASTERFONTREADER_DATAREADER
#define ROBINLE_RASTERFONTREADER_DATAREADER


#include <cstdint>
#include <exception>
#include <vector>


bool LoadFileToMemory(const wchar_t* szFilepath, std::vector<uint8_t>& oDest);
bool LoadFileToMemory(const char* szFilepath, std::vector<uint8_t>& oDest);




enum class DataSeekMethod
{
	Begin,
	Current,
	End
};

struct DataReaderException {};


/// <summary>A class for reading from memory byte by byte.</summary>
class DataReader
{
public: // types

	using iterator = const uint8_t*;


public: // methods

	/// <param name="pData">The data to be read - will not be freed on destruction.</param>
	/// <param name="iSize">The size, in bytes, of the buffer pointed to by <c>pData</c>.</param>
	DataReader(const void* pData, size_t iSize);

	/// <summary>Has the end of the data been reached?</summary>
	bool eof() const { return m_iPos == m_iSize; }

	/// <summary>Does this object throw <c>std::exception</c>s on failure?</summary>
	bool throwsExceptions() { return m_bExceptions; }
	/// <summary>Set if this class should throw <c>std::exceptions</c>s.</summary>
	void setExceptions(bool b) { m_bExceptions = b; }

	/// <summary>Go to an absolute offset in the data.</summary>
	bool seekPos(size_t iOffset);
	/// <summary>Go to an offset in the data based on a certain base position.</summary>
	bool seekPos(ptrdiff_t iOffset, DataSeekMethod eMethod);

	/// <summary>Get the current read position in the data.</summary>
	size_t tellPos() const { return m_iPos; }

	/// <summary>The count of bytes remaining available to read.</summary>
	size_t remainingBytes() const { return m_iSize - m_iPos; };

	/// <summary>Read data</summary>
	bool read(void* pDest, size_t iSize);
	/// <summary>Read a variable</summary>
	template <typename T> bool readVar(T& dest) { return read(&dest, sizeof(T)); }

	/// <summary>Get a pointer to the current position in the data</summary>
	iterator current() { return reinterpret_cast<const uint8_t*>(m_pData) + m_iPos; }

	/// <summary>Get an iterator (void pointer) to the start of the data</summary>
	iterator begin() { return reinterpret_cast<const uint8_t*>(m_pData); }
	iterator end() { return reinterpret_cast<const uint8_t*>(m_pData) + m_iSize + 1; }


private: // variables

	// constants
	const void* const m_pData;
	const size_t m_iSize;

	// variables
	size_t m_iPos = 0;
	bool m_bExceptions = false;

};


#endif // ROBINLE_RASTERFONTREADER_DATAREADER