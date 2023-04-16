#include "rl/data.filecontainer.hpp"

// STL
#include <cctype>
#include <fstream>
#include <vector>

// Win32
#include <Windows.h>





namespace
{
	constexpr char szMagicNumber[]       = "rlFILECONTAINER";
	constexpr uint8_t iCurrentVersion[2] ={ 1, 0 };

	constexpr uint16_t iFlag_Unicode = 0x01;

#pragma pack(push, 1)

	struct FileHeader
	{
		char     szMagicNo[16];
		uint8_t  iFormatVersion[2];
		uint16_t iFlags;
		uint64_t iDirCount;
		uint64_t iFileCount;
	};

	constexpr size_t iElemNameLen = 50;

	template <typename TChar>
	struct DirTableEntry
	{
		TChar  szName[iElemNameLen];
		size_t iParentDirID;
	};
	using DirTableEntryA = DirTableEntry<char>;
	using DirTableEntryW = DirTableEntry<wchar_t>;

	template <typename TChar>
	struct FileTableEntry
	{
		TChar  szName[iElemNameLen];
		size_t iParentDirID;
		size_t iDataOffset;
		size_t iDataSize;
	};
	using FileTableEntryA = FileTableEntry<char>;
	using FileTableEntryW = FileTableEntry<wchar_t>;

#pragma pack(pop)


	// ! CHECK Directory::saveable() BEFORE USING!
	template <typename TChar>
	void SaveToList(const rl::FileContainer::Directory &o, uint64_t iParentDirID,
		std::vector<DirTableEntry<TChar>> &oDirs,
		std::vector<FileTableEntry<TChar>> &oFiles,
		std::vector<const rl::FileContainer::File *> &oFileData)
	{
		for (auto &oDir : o.directories())
		{
			DirTableEntry<TChar> oEntry{};

			if constexpr (std::is_same<TChar, wchar_t>::value)
				memcpy_s(oEntry.szName, sizeof(oEntry.szName), oDir.first.c_str(),
					oDir.first.length() * sizeof(TChar));
			else if constexpr (std::is_same<TChar, char>::value)
			{
				for (size_t i = 0; i < oDir.first.length() && i < 50; ++i)
				{
					oEntry.szName[i] = static_cast<char>(oDir.first[i]);
				}
			}

			oEntry.iParentDirID = iParentDirID;

			oDirs.push_back(std::move(oEntry));
			SaveToList<TChar>(oDir.second, oDirs.size(), oDirs, oFiles, oFileData);
		}

		for (auto &oFile : o.files())
		{
			FileTableEntry<TChar> oEntry{};

			if constexpr (std::is_same<TChar, wchar_t>::value)
				memcpy_s(oEntry.szName, sizeof(oEntry.szName), oFile.first.c_str(),
					oFile.first.length() * sizeof(TChar));
			else if constexpr (std::is_same<TChar, char>::value)
			{
				for (size_t i = 0; i < oFile.first.length() && i < 50; ++i)
				{
					oEntry.szName[i] = static_cast<char>(oFile.first[i]);
				}
			}

			oEntry.iParentDirID = iParentDirID;
			oEntry.iDataSize    = oFile.second.size();

			oFiles.push_back(std::move(oEntry));
			oFileData.push_back(&oFile.second);
		}
	}

}

namespace rl
{

	FileContainer::File::File(const File &other)
	{
		this->m_iSize  = other.m_iSize;
		if (m_iSize == 0)
			return;

		this->m_upData = std::make_unique<uint8_t[]>(this->m_iSize);
		memcpy_s(m_upData.get(), this->m_iSize, other.m_upData.get(), other.m_iSize);
	}

	FileContainer::File &FileContainer::File::operator=(const File &other)
	{
		this->m_iSize  = other.m_iSize;
		if (m_iSize == 0)
		{
			m_upData = nullptr;
			return *this;
		}

		this->m_upData = std::make_unique<uint8_t[]>(this->m_iSize);
		memcpy_s(m_upData.get(), this->m_iSize, other.m_upData.get(), other.m_iSize);

		return *this;
	}

	bool FileContainer::File::load(const wchar_t *szPath)
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
		oFile.read(reinterpret_cast<char *>(m_upData.get()), m_iSize);

		oFile.close();
		return true;
	}

	bool FileContainer::File::save(const wchar_t *szPath) const
	{
		std::ofstream oFile(szPath, std::ios::binary);
		if (!oFile)
			return false;

		if (m_iSize > 0)
			oFile.write(reinterpret_cast<const char *>(m_upData.get()), m_iSize);

		oFile.close();
		return true;
	}

	void FileContainer::File::create(size_t iBytes, bool bInitToZero)
	{
		clear();

		m_iSize = iBytes;

		if (m_iSize == 0)
			return;

		m_upData = std::make_unique<uint8_t[]>(m_iSize);
		if (bInitToZero)
			memset(m_upData.get(), 0, m_iSize);
	}

	void FileContainer::File::clear()
	{
		m_iSize  = 0;
		m_upData = nullptr;
	}





	bool FileContainer::Directory::addDirectoryContents(const wchar_t *szDirPath, bool bRecursive)
	{
		if (szDirPath == nullptr)
			return false;

		std::wstring sDir;
		sDir.reserve(wcslen(szDirPath) + 2);
		sDir = szDirPath;
		if (!sDir.ends_with(L'\\') && !sDir.ends_with(L'/'))
			sDir += L"\\";
		const auto sMask = sDir + L"*";


		bool bResult = true;

		// handle subdirectories
		if (bRecursive)
		{
			WIN32_FIND_DATAW fd{};
			auto hFind = FindFirstFileW(sMask.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
						continue; // no directory

					std::wstring_view sv = fd.cFileName;
					if (sv == L"." || sv == L"..")
						continue; // "."/".."

					if (sv.length() > 50)
					{
						bResult = false;
						continue; // next directory
					}

					auto &oDir = m_oSubDirectories[sv.data()];
					std::wstring sMask2 = sDir + sv.data();
					oDir.addDirectoryContents(sMask2.c_str(), true);

				} while (FindNextFileW(hFind, &fd) != 0);

				FindClose(hFind);
			}
		}

		// handle files
		{
			WIN32_FIND_DATAW fd{};
			auto hFind = FindFirstFileW(sMask.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
						continue; // directory

					std::wstring_view sv = fd.cFileName;

					if (sv.length() > 50)
					{
						bResult = false;
						continue; // next file
					}

					auto &oFile = m_oFiles[sv.data()];

					std::wstring sFile = sDir + sv.data();
					if (!oFile.load(sFile.c_str()))
					{
						bResult = false;
						m_oFiles.erase(sv.data());
					}

				} while (FindNextFileW(hFind, &fd) != 0);

				FindClose(hFind);
			}
		}

		return bResult;
	}

	void FileContainer::Directory::clear() noexcept
	{
		m_oFiles.clear();
		m_oSubDirectories.clear();
	}

	size_t FileContainer::Directory::totalFileCount() const noexcept
	{
		size_t iResult = m_oFiles.size();
		for (auto &oDir : m_oSubDirectories)
			iResult += oDir.second.totalFileCount();

		return iResult;
	}

	size_t FileContainer::Directory::totalDirectoryCount() const noexcept
	{
		size_t iResult = m_oSubDirectories.size();
		for (auto &oDir : m_oSubDirectories)
			iResult += oDir.second.totalDirectoryCount();

		return iResult;
	}

	bool FileContainer::Directory::saveable(bool bUnicode) const noexcept
	{
		for (auto &oDir : m_oSubDirectories)
		{
			const auto &sName = oDir.first;

			if (sName.length() > 50)
				return false;

			if (!bUnicode) // ASCII
			{
				for (auto c : sName)
				{
					if (c & 0xFF80)
						return false; // not ASCII-compatible
				}
			}

			if (!oDir.second.saveable(bUnicode))
				return false;
		}

		for (auto &oFile : m_oFiles)
		{
			const auto &sName = oFile.first;

			if (sName.length() > 50)
				return false;

			if (!bUnicode) // ASCII
			{
				for (auto c : sName)
				{
					if (c & 0xFF80)
						return false; // not ASCII-compatible
				}
			}
		}

		return true;
	}

	/*void FileContainer::Directory::saveToListsW(uint64_t iParentDirID,
				std::vector<DirTableEntryW> &oDirs,
				std::vector<FileTableEntryW> &oFiles,
				std::vector<const File *> &oFileData) const noexcept
	{
		for (auto &oDir : m_oSubDirectories)
		{
			DirTableEntryW oEntry{};
			memcpy_s(oEntry.szName, sizeof(oEntry.szName), oDir.first.c_str(),
				oDir.first.length() * sizeof(wchar_t));
			oEntry.iParentDirID = iParentDirID;

			oDirs.push_back(std::move(oEntry));
			oDir.second.saveToListsW(oDirs.size(), oDirs, oFiles, oFileData);
		}

		for (auto &oFile : m_oFiles)
		{
			FileTableEntryW oEntry{};
			memcpy_s(oEntry.szName, sizeof(oEntry.szName), oFile.first.c_str(),
				oFile.first.length() * sizeof(wchar_t));
			oEntry.iParentDirID = iParentDirID;
			oEntry.iDataSize    = oFile.second.size();

			oFiles.push_back(std::move(oEntry));
			oFileData.push_back(&oFile.second);
		}
	}

	void FileContainer::Directory::saveToListsA(uint64_t iParentDirID,
				std::vector<DirTableEntryA> &oDirs,
				std::vector<FileTableEntryA> &oFiles,
				std::vector<const File *> &oFileData) const noexcept
	{
		for (auto &oDir : m_oSubDirectories)
		{
			DirTableEntryA oEntry{};
			memcpy_s(oEntry.szName, sizeof(oEntry.szName), oDir.first.c_str(),
				oDir.first.length() * sizeof(char));
			oEntry.iParentDirID = iParentDirID;

			oDirs.push_back(std::move(oEntry));
			oDir.second.saveToListsA(oDirs.size(), oDirs, oFiles, oFileData);
		}

		for (auto &oFile : m_oFiles)
		{
			FileTableEntryA oEntry{};
			memcpy_s(oEntry.szName, sizeof(oEntry.szName), oFile.first.c_str(),
				oFile.first.length() * sizeof(char));
			oEntry.iParentDirID = iParentDirID;
			oEntry.iDataSize    = oFile.second.size();

			oFiles.push_back(std::move(oEntry));
			oFileData.push_back(&oFile.second);
		}
	}*/





	bool FileContainer::load(const wchar_t *szPath)
	{
		m_oRootDir.clear();

		std::ifstream in(szPath, std::ios::binary);
		if (!in)
			return false;
		in.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);

		try
		{
#define READVAR(var) in.read(reinterpret_cast<char *>(&var), sizeof(var))
#define READBIN(pDest, iSize) in.read(reinterpret_cast<char *>(pDest), iSize)

			// FILE HEADER
			FileHeader hdr{};
			READVAR(hdr);

			if (memcmp(hdr.iFormatVersion, iCurrentVersion, sizeof(iCurrentVersion)) != 0)
				return false; // unknown file format version


			std::unique_ptr<DirTableEntryW[]> up_oDirs;
			std::unique_ptr<Directory *[]> up_oDirByIndex;
			if (hdr.iDirCount > 0)
			{
				up_oDirs       = std::make_unique<DirTableEntryW[]>(hdr.iDirCount);
				up_oDirByIndex = std::make_unique<Directory * []>(hdr.iDirCount);

				memset(up_oDirByIndex.get(), 0, hdr.iDirCount * sizeof(up_oDirByIndex[0]));
			}

			std::unique_ptr<FileTableEntryW[]> up_oFiles;
			if (hdr.iFileCount > 0)
				up_oFiles = std::make_unique<FileTableEntryW[]>(hdr.iFileCount);

			if (hdr.iFlags & iFlag_Unicode)
			{
				// DIRECTORIES
				if (hdr.iDirCount > 0)
					READBIN(up_oDirs.get(), sizeof(DirTableEntryW) * hdr.iDirCount);

				// FILES
				if (hdr.iFileCount > 0)
					READBIN(up_oFiles.get(), sizeof(FileTableEntryW) * hdr.iFileCount);
			}
			else // ASCII version
			{
				// DIRECTORIES
				if (hdr.iDirCount > 0)
				{
					auto up_oDirsA = std::make_unique<DirTableEntryA[]>(hdr.iDirCount);
					READBIN(up_oDirsA.get(), sizeof(DirTableEntryA) * hdr.iDirCount);

					for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
					{
						auto &oDest = up_oDirs[iDir];
						auto &oSrc  = up_oDirsA[iDir];

						oDest.iParentDirID = oSrc.iParentDirID;
						for (size_t iChar = 0; iChar < iElemNameLen; ++iChar)
						{
							oDest.szName[iChar] = oSrc.szName[iChar];
						}
					}
				}
								
				// FILES
				if (hdr.iFileCount > 0)
				{
					auto up_oFilesA = std::make_unique<FileTableEntryA[]>(hdr.iDirCount);
					READBIN(up_oFilesA.get(), sizeof(FileTableEntryA) * hdr.iDirCount);

					for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
					{
						auto &oDest = up_oFiles[iDir];
						auto &oSrc  = up_oFilesA[iDir];

						oDest.iParentDirID = oSrc.iParentDirID;
						oDest.iDataOffset  = oSrc.iDataOffset;
						oDest.iDataSize    = oSrc.iDataSize;
						for (size_t iChar = 0; iChar < iElemNameLen; ++iChar)
						{
							oDest.szName[iChar] = oSrc.szName[iChar];
						}
					}
				}
			}

			// create directories
			for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
			{
				auto &oDir = up_oDirs[iDir];
				if (oDir.iParentDirID > 0 && oDir.iParentDirID >= iDir)
					return false; // invalid parent directory ID

				wchar_t szName[51]{};
				memcpy_s(szName, sizeof(szName), oDir.szName, sizeof(oDir.szName));

				if (oDir.iParentDirID == 0)
					up_oDirByIndex[iDir] = &m_oRootDir.directories()[szName];
				else
					up_oDirByIndex[iDir] =
					&up_oDirByIndex[oDir.iParentDirID]->directories()[szName];
			}

			// read actual data
			for (size_t iFile = 0; iFile < hdr.iFileCount; ++iFile)
			{
				auto &oFile = up_oFiles[iFile];

				wchar_t szName[51]{};
				memcpy_s(szName, sizeof(szName), oFile.szName, sizeof(oFile.szName));

				File *pFile;

				if (oFile.iParentDirID == 0)
					pFile = &m_oRootDir.files()[szName];
				else
					pFile = &up_oDirByIndex[oFile.iParentDirID - 1]->files()[szName];

				pFile->create(oFile.iDataSize, false);
				READBIN(pFile->data(), pFile->size());
			}

#undef READVAR
#undef READBIN
		}
		catch (...)
		{
			m_oRootDir.clear();
			return false;
		}

		return true;
	}

	bool FileContainer::save(const wchar_t *szPath, bool bUnicode) const
	{
		if (!m_oRootDir.saveable(bUnicode))
			return false;


		std::ofstream out(szPath, std::ios::binary);
		if (!out)
			return false;
		out.exceptions(std::ios::badbit | std::ios::failbit);

		try
		{
#define WRITEVAR(var) out.write(reinterpret_cast<const char*>(&var), sizeof(var))
#define WRITEBIN(pSrc, iSize) out.write(reinterpret_cast<const char*>(pSrc), iSize)


			// file header
			FileHeader hdr{};
			strcpy_s(hdr.szMagicNo, szMagicNumber);
			memcpy_s(hdr.iFormatVersion, sizeof(hdr.iFormatVersion),
				iCurrentVersion, sizeof(iCurrentVersion));
			if (bUnicode)
				hdr.iFlags |= iFlag_Unicode;
			hdr.iDirCount  = m_oRootDir.totalDirectoryCount();
			hdr.iFileCount = m_oRootDir.totalFileCount();
			WRITEVAR(hdr);


			std::vector<const File *> oData;
			std::vector<size_t> oOffsets;


			if (bUnicode)
			{
				size_t iOffsetOffset;
				{
					FileTableEntryW dte{};
					iOffsetOffset = sizeof(dte) - ((intptr_t)&dte.iDataOffset - (intptr_t)&dte);
				}
				std::vector<DirTableEntryW>  oDirs;
				std::vector<FileTableEntryW> oFiles;

				SaveToList<wchar_t>(m_oRootDir, 0, oDirs, oFiles, oData);

				// directory table
				for (auto &oDir : oDirs)
				{
					WRITEVAR(oDir);
				}

				// file table
				for (auto &oFile : oFiles)
				{
					WRITEVAR(oFile);
					oOffsets.push_back(static_cast<size_t>(out.tellp()) - iOffsetOffset);
				}
			}

			else
			{
				size_t iOffsetOffset;
				{
					FileTableEntryA dte{};
					iOffsetOffset = sizeof(dte) - ((intptr_t)&dte.iDataOffset - (intptr_t)&dte);
				}
				std::vector<DirTableEntryA>  oDirs;
				std::vector<FileTableEntryA> oFiles;
				SaveToList<char>(m_oRootDir, 0, oDirs, oFiles, oData);

				// directory table
				for (auto &oDir : oDirs)
				{
					WRITEVAR(oDir);
				}

				// file table
				for (auto &oFile : oFiles)
				{
					WRITEVAR(oFile);
					oOffsets.push_back(static_cast<size_t>(out.tellp()) - iOffsetOffset);
				}
			}


			// file data
			for (size_t i = 0; i < oData.size(); ++i)
			{
				const auto oOffset = out.tellp();
				const auto iOffset = static_cast<size_t>(oOffset);

				// set offset in file table entry
				out.seekp(oOffsets[i], std::ios::beg);
				WRITEVAR(iOffset);

				// write file data
				out.seekp(oOffset);
				WRITEBIN(oData[i]->data(), oData[i]->size());
			}


#undef WRITEVAR
#undef WRITEBIN
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

}