/***************************************************************************************************
 FILE:	data.filecontainer.hpp
 CPP:	data.filecontainer.cpp
 DESCR:	An interface for .rlPAK files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_FILECONTAINER
#define ROBINLE_DATA_FILECONTAINER





//==================================================================================================
// INCLUDES


#include <cstdint>
#include <map>
#include <memory>
#include <string>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// A virtual file.
	/// </summary>
	class File final
	{
	public: // methods

		File() = default;
		File(const File &other);
		File(File &&) = default;
		~File() = default;

		File &operator=(const File &other);
		File &operator=(File &&rval) = default;


		bool load(const wchar_t *szPath);
		bool save(const wchar_t *szPath) const;

		void create(size_t iBytes, bool bInitToZero = true);
		void clear();

		uint8_t *data() { return m_upData.get(); }
		const uint8_t *data() const { return m_upData.get(); }

		auto size() const { return m_iSize; }


	private: // variables

		size_t m_iSize = 0;
		std::unique_ptr<uint8_t[]> m_upData;

	};
	


	/// <summary>
	/// A container for virtual files. Can be saved to and loaded from <c>.rlPAK</c> files.
	/// </summary>
	class FileContainer
	{
	public: // methods

		void clear();

		bool load(const wchar_t *szPath);
		bool save(const wchar_t *szPath) const;

		bool addFile(const wchar_t *szPath, const wchar_t *szName); // overwrites previous file
		bool removeFile(const wchar_t *szName);

		bool addDirectory(const wchar_t *szDir, const wchar_t *szMask = L"*.*",
			const wchar_t *szNamePrefix = L"", bool bRecursive = true);
		bool extractAllFiles(const wchar_t *szDir) const;

		auto &files() const { return m_oFiles; }


	private: // variables

		std::map<std::wstring, File> m_oFiles;

	};
	
}





#endif // ROBINLE_DATA_FILECONTAINER