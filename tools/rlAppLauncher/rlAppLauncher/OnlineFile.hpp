#pragma once
#ifndef ROBINLE_APPLAUNCHER_ONLINEINTERFACE
#define ROBINLE_APPLAUNCHER_ONLINEINTERFACE





#include <memory>
#include <cstdint>



/// <summary>
/// A virtual file downloaded from the internet.
/// </summary>
class OnlineFile
{
public: // static methods

	/// <summary>
	/// Get the size of a file on the internet.
	/// </summary>
	/// <param name="szURL">The URL of the file.</param>
	/// <returns>
	/// If the function suceeds, it returns the size of the requested file, in Bytes.<br />
	/// If the function fails, it returns 0.
	/// </returns>
	static size_t GetFileSize(const wchar_t *szURL);

	/// <summary>
	/// Download a file from the internet without saving it to disk.
	/// </summary>
	/// <param name="szURL">The URL of the file.</param>
	/// <returns>
	/// The downloaded data. <br />
	/// If the function failed, the returned object will evaluate to false.
	/// </returns>
	static OnlineFile DownloadFileToMemory(const wchar_t *szURL);

	/// <summary>
	/// Download a file from the internet and save it to disk.
	/// </summary>
	/// <param name="szURL">The URL of the file.</param>
	/// <param name="szLocalPath">The local path to save the file to.</param>
	/// <returns>Did the download succeed?</returns>
	static bool DownloadFile(const wchar_t *szURL, const wchar_t *szLocalPath);


public: // types

	/// <summary>
	/// Seek base.
	/// </summary>
	enum class SeekPos
	{
		Begin,
		Current,
		End
	};
	using SeekPos::Begin;
	using SeekPos::Current;
	using SeekPos::End;

	using OffsetInt = int64_t;
	using PosInt    = size_t;


public: // methods

	operator bool() const { return m_upData.get() != nullptr; }


	/// <summary>
	/// Reserve data in memory for the virtual file.
	/// </summary>
	/// <param name="iSize">The count of bytes to reserve</param>
	/// <param name="bInitToZero">Should the data be initialized to all-zero?</param>
	/// <returns>Did the allocation succeed?</returns>
	bool allocate(size_t iSize, bool bInitToZero = true);

	/// <summary>
	/// Clear all internal data, freeing the memory.
	/// </summary>
	void clear();


	/// <summary>
	/// Get the data of the virtual file.
	/// </summary>
	const uint8_t *data() const { return m_upData.get(); }

	/// <summary>
	/// Get the data of the virtual file.
	/// </summary>
	uint8_t *data() { return m_upData.get(); }


	/// <summary>
	/// Get the size of the virtual file, in bytes.
	/// </summary>
	auto size() const { return m_iSize; }



	PosInt seekPos(SeekPos ePos, OffsetInt iOffset) const;

	bool eof() const noexcept { return m_iOffset >= m_iSize; }

	bool read(void *pDest, size_t iBytes) const
	{
		if (remainingBytes() < iBytes)
			return false;

		memcpy_s(pDest, iBytes, data() + m_iOffset, iBytes);
		m_iOffset += iBytes;

		return true;
	}

	template <typename T>
	typename std::enable_if<std::is_fundamental<T>::value, T>::type read() const
	{
		T tResult{};

		if (!read(&tResult, sizeof(T)))
			throw std::exception();

		return tResult;
	}

	auto readInt8()  const { return read<int8_t>(); }
	auto readInt16() const { return read<int16_t>(); }
	auto readInt32() const { return read<int32_t>(); }
	auto readInt64() const { return read<int64_t>(); }

	auto readUInt8()  const { return read<uint8_t>(); }
	auto readUInt16() const { return read<uint16_t>(); }
	auto readUInt32() const { return read<uint32_t>(); }
	auto readUInt64() const { return read<uint64_t>(); }

	auto readFloat()  const { return read<float>(); }
	auto readDouble() const { return read<double>(); }


	auto offset() const { return m_iOffset; }
	PosInt remainingBytes() const { return m_iSize - m_iOffset; }

private: // variables

	std::unique_ptr<uint8_t[]> m_upData;
	size_t m_iSize = 0;

	mutable PosInt m_iOffset = 0;

};





#endif // ROBINLE_APPLAUNCHER_ONLINEINTERFACE