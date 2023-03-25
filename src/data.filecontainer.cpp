#include "rl/data.filecontainer.hpp"

#include <cctype>
#include <fstream>
#include <Windows.h>





namespace
{
	constexpr char szMagicNumber[]       = "rlFILECONTAINER";
	constexpr uint8_t iCurrentVersion[2] = { 1, 0 };
}

namespace rl
{
	
	/***********************************************************************************************
	 class File
	***********************************************************************************************/
	
	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS
	
	File::File(const File &other)
	{
		this->m_iSize  = other.m_iSize;
		if (m_iSize == 0)
			return;

		this->m_upData = std::make_unique<uint8_t[]>(this->m_iSize);
		memcpy_s(m_upData.get(), this->m_iSize, other.m_upData.get(), other.m_iSize);
	}
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// OPERATORS

	File &File::operator=(const File &other)
	{
		this->m_iSize  = other.m_iSize;
		if (m_iSize == 0)
		{
			m_upData = nullptr;
			return;
		}

		this->m_upData = std::make_unique<uint8_t[]>(this->m_iSize);
		memcpy_s(m_upData.get(), this->m_iSize, other.m_upData.get(), other.m_iSize);

		return *this;
	}
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS
	
	bool File::load(const wchar_t *szPath)
	{
		clear();

		std::ifstream oFile(szPath, std::ios::binary);
		if (!oFile)
			return false;

		oFile.seekg(0, std::ios::end);
		m_iSize = oFile.tellg();
		if (m_iSize == 0)
			return true; // empty file

		oFile.seekg(0, std::ios::beg);
		m_upData = std::make_unique<uint8_t[]>(m_iSize);
		oFile.read(reinterpret_cast<char*>(m_upData.get()), m_iSize);

		oFile.close();
		return true;
	}

	bool File::save(const wchar_t *szPath) const
	{
		std::ofstream oFile(szPath, std::ios::binary);
		if (!oFile)
			return false;

		if (m_iSize > 0)
			oFile.write(reinterpret_cast<const char*>(m_upData.get()), m_iSize);

		oFile.close();
		return true;
	}

	void File::create(size_t iBytes, bool bInitToZero)
	{
		clear();

		m_iSize = iBytes;

		if (m_iSize == 0)
			return;

		m_upData = std::make_unique<uint8_t[]>(m_iSize);
		if (bInitToZero)
			memset(m_upData.get(), 0, m_iSize);
	}

	void File::clear()
	{
		m_iSize = 0;
		m_upData = nullptr;
	}










	/***********************************************************************************************
	 class FileContainer
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void FileContainer::clear()
	{
		m_oFiles.clear();
	}

	bool FileContainer::load(const wchar_t *szPath)
	{
		clear();

		std::ifstream oFile(szPath, std::ios::binary);
		if (!oFile)
			return false;
		oFile.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);

		try
		{
			// read magic number
			char szFileMagicNumber[sizeof(szMagicNumber)];
			oFile.read(szFileMagicNumber, sizeof(szMagicNumber) - 1);
			if (memcmp(szFileMagicNumber, szMagicNumber, sizeof(szMagicNumber) - 1) != 0)
				return false; // wrong magic number

			// read version
			uint8_t iFileVersion[2]{};
			oFile.read(reinterpret_cast<char *>(iFileVersion), sizeof(iFileVersion));

			switch (iFileVersion[0]) // major version
			{
			case 1: // 1.X

				switch (iFileVersion[1]) // minor version
				{
				case 0: // 1.0
				{
					uint64_t iFileCount;
					oFile >> iFileCount;

					for (uint64_t i = 0; i < iFileCount; ++i)
					{
						// read file name
						uint64_t iFileNameLength;
						oFile >> iFileNameLength;
						std::wstring sFileName;
						sFileName.resize(iFileNameLength);
						oFile.read(reinterpret_cast<char*>(sFileName.data()),
							iFileNameLength * sizeof(wchar_t));

						uint64_t iFileSize;
						oFile >> iFileSize;
						uint64_t iOffset;
						oFile >> iOffset;

						File oVFile;
						if (iFileSize > 0)
						{
							const auto iOldOffset = oFile.tellg();

							oFile.seekg(iOffset, std::ios::beg); // jump to data
							oVFile.create(iFileSize, false);
							oFile.read(reinterpret_cast<char*>(oVFile.data()), iFileSize);

							oFile.seekg(iOldOffset); // jump back to file table
						}

						if (m_oFiles.contains(sFileName))
						{
							clear();
							return false; // duplicate file name
						}
						m_oFiles.emplace(sFileName, oVFile);
						
					}

					break;
				}

				default: // unknown minor version
					return false;
				}

			default: // unknown major version
				return false;
			}
		}
		catch (...)
		{
			clear();
			return false;
		}

		return true;
	}

	bool FileContainer::save(const wchar_t *szPath) const
	{
		std::ofstream oFile(szPath, std::ios::binary | std::ios::trunc);
		if (!oFile)
			return false;

		// write magic number
		oFile.write(szMagicNumber, sizeof(szMagicNumber) - 1);
		// write version
		oFile << iCurrentVersion[0] << iCurrentVersion[1];

		// write file count
		oFile << (uint64_t)m_oFiles.size();

		auto up_iOffsetOffsets = std::make_unique<std::streamoff[]>(m_oFiles.size());
		auto up_iOffsets = std::make_unique<uint64_t[]>(m_oFiles.size());
		
		// write file table
		size_t i = 0;
		for (auto &it : m_oFiles)
		{
			// write file name
			const auto &sFileName          = it.first;
			const uint64_t iFileNameLength = sFileName.length();
			oFile << iFileNameLength;
			oFile.write(reinterpret_cast<const char*>(sFileName.data()),
								iFileNameLength * sizeof(wchar_t));

			// write file size
			const auto &oVFile = it.second;
			const auto iFileSize = oVFile.size();
			oFile << (uint64_t)iFileSize;
			// write file offset
			up_iOffsetOffsets[i] = oFile.tellp();
			oFile << (uint64_t)0;
			++i;
		}

		// write file data
		i = 0;
		for (auto &it : m_oFiles)
		{
			const auto &oVFile   = it.second;
			const auto iFileSize = oVFile.size();

			up_iOffsets[i] = oFile.tellp();

			if (iFileSize > 0)
				oFile.write(reinterpret_cast<const char*>(oVFile.data()), iFileSize);

			++i;
		}

		// write file offsets
		for (i = 0; i < m_oFiles.size(); ++i)
		{
			oFile.seekp(up_iOffsetOffsets[i], std::ios::beg);
			oFile << up_iOffsets[i];
		}

		oFile.close();
		return true;
	}

	bool FileContainer::addFile(const wchar_t *szPath, const wchar_t *szName)
	{
		File oFile;
		if (!oFile.load(szPath))
			return false;

		m_oFiles[szPath] = std::move(oFile);

		return true;
	}

	bool FileContainer::removeFile(const wchar_t *szName)
	{
		if (!m_oFiles.contains(szName))
			return false;

		m_oFiles.erase(szName);
		return true;
	}

	bool FileContainer::addDirectory(const wchar_t *szDir, const wchar_t *szMask,
		const wchar_t *szNamePrefix, bool bRecursive)
	{
		std::wstring sSearchPath;
		sSearchPath.reserve(wcslen(szDir) + 1 + wcslen(szMask));
		sSearchPath  = szDir;
		sSearchPath += L'\\';
		sSearchPath += szMask;

		WIN32_FIND_DATAW oFindData;
		auto hFind = FindFirstFileW(sSearchPath.c_str(), &oFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue; // skip directories

				std::wstring sFileName;
				sFileName.reserve(wcslen(szDir) + 1 + wcslen(oFindData.cFileName));
				sFileName  = szDir;
				sFileName += L'\\';
				sFileName += oFindData.cFileName;

				std::wstring sName;
				sName.reserve(wcslen(szNamePrefix) + wcslen(oFindData.cFileName));
				sName  = szNamePrefix;
				sName += oFindData.cFileName;

				File oFile;
				oFile.load(sFileName.c_str());
				m_oFiles[sName] = std::move(oFile);
			} while (FindNextFileW(hFind, &oFindData));

			FindClose(hFind);
		}

		if (bRecursive)
		{
			sSearchPath  = szDir;
			sSearchPath += L"\\*";
			hFind = FindFirstFileW(sSearchPath.c_str(), &oFindData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!(oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
						continue; // skip files
					if (wcscmp(oFindData.cFileName, L".") == 0 ||
						wcscmp(oFindData.cFileName, L"..") == 0)
						continue; // skip "." and ".."

					std::wstring sDirName;
					sDirName.reserve(wcslen(szDir) + 1 + wcslen(oFindData.cFileName));
					sDirName  = szDir;
					sDirName += L'\\';
					sDirName += oFindData.cFileName;
					
					std::wstring sNamePrefix;
					sNamePrefix.reserve(wcslen(szNamePrefix) + wcslen(oFindData.cFileName) + 1);
					sNamePrefix  = szNamePrefix;
					sNamePrefix += oFindData.cFileName;
					sNamePrefix += L'\\';

					addDirectory(sDirName.c_str(), szMask, sNamePrefix.c_str(), true);
				} while (FindNextFileW(hFind, &oFindData));
				FindClose(hFind);
			}
		}

	}

	bool FileContainer::extractAllFiles(const wchar_t *szDir) const
	{
		// TODO
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods
	
}