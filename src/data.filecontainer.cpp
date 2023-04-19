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

#pragma pack(push, 1)

	struct FileHeader
	{
		char     szMagicNo[16];
		uint8_t  iFormatVersion[2];
		uint64_t iDirCount;
		uint64_t iFileCount;
	};

	constexpr uint8_t PAK_STRING_UNICODE = 0x01;
	struct StringTableHeader
	{
		uint64_t iStringCount;
		uint64_t iStringTableSize;
		uint8_t  iFlags;
	};

	struct DataBlockHeader
	{
		uint64_t iTotalDataSize;
	};

	struct DirTableEntry
	{
		uint64_t iStringOffset;
		uint64_t iParentDirID;
	};

	struct FileTableEntry
	{
		uint64_t iStringOffset;
		uint64_t iParentDirID;
		uint64_t iDataOffset;
		uint64_t iDataSize;
	};

#pragma pack(pop)


	struct TempDir
	{
		size_t iStringIndex;
		uint64_t iParentDir;
	};

	struct TempFile
	{
		size_t iStringIndex;
		uint64_t iParentDir;
		const rl::FileContainer::File *pFile;
	};

	void GetFileContainerElements(uint64_t iParentDir, const rl::FileContainer::Directory &oSrc,
		std::vector<std::wstring> &oStrings, std::vector<TempDir> &oDirs,
		std::vector<TempFile> &oFiles)
	{
		for (auto &itDir : oSrc.directories())
		{
			TempDir oDir{};
			oDir.iParentDir = iParentDir;

			const auto itString = std::find(oStrings.begin(), oStrings.end(), itDir.first);
			if (itString != oStrings.end())
				oDir.iStringIndex = itString - oStrings.begin();
			else
			{
				oDir.iStringIndex = oStrings.size();
				oStrings.push_back(itDir.first);
			}
			oDirs.push_back(std::move(oDir));
			GetFileContainerElements(oDirs.size(), itDir.second, oStrings, oDirs, oFiles);
		}

		for (auto &itFile : oSrc.files())
		{
			TempFile oFile{};
			oFile.iParentDir = iParentDir;
			oFile.pFile      = &itFile.second;

			const auto itString = std::find(oStrings.begin(), oStrings.end(), itFile.first);
			if (itString != oStrings.end())
				oFile.iStringIndex = itString - oStrings.begin();
			else
			{
				oFile.iStringIndex = oStrings.size();
				oStrings.push_back(itFile.first);
			}
			oFiles.push_back(std::move(oFile));
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
			if (memcmp(hdr.szMagicNo, szMagicNumber, sizeof(szMagicNumber)) != 0)
				return false; // wrong magic number
			if (memcmp(hdr.iFormatVersion, iCurrentVersion, sizeof(iCurrentVersion)) != 0)
				return false; // unknown file format version

			// STRING TABLE
			std::vector<std::wstring> oStrings;
			{
				StringTableHeader sth{};
				READVAR(sth);

				if (sth.iStringCount > 0)
				{
					// Unicode
					if (sth.iFlags & PAK_STRING_UNICODE)
					{
						auto up_szStrings =
							std::make_unique<wchar_t[]>(sth.iStringTableSize / sizeof(wchar_t));
						READBIN(up_szStrings.get(), sth.iStringTableSize);

						// todo: save strings to oStrings
					}

					// ASCII
					else
					{
						auto up_szStrings =
							std::make_unique<char[]>(sth.iStringTableSize);
						READBIN(up_szStrings.get(), sth.iStringTableSize);

						//  todo: save strings to oStrings
					}
				}
			}

			// todo:
			// 1. read dir table
			// 2. read file table

			//			std::unique_ptr<DirTableEntryW[]> up_oDirs;
			//			std::unique_ptr<Directory *[]> up_oDirByIndex;
			//			if (hdr.iDirCount > 0)
			//			{
			//				up_oDirs       = std::make_unique<DirTableEntryW[]>(hdr.iDirCount);
			//				up_oDirByIndex = std::make_unique<Directory * []>(hdr.iDirCount);
			//
			//				memset(up_oDirByIndex.get(), 0, hdr.iDirCount * sizeof(up_oDirByIndex[0]));
			//			}
			//
			//			std::unique_ptr<FileTableEntryW[]> up_oFiles;
			//			if (hdr.iFileCount > 0)
			//				up_oFiles = std::make_unique<FileTableEntryW[]>(hdr.iFileCount);
			//
			//			if (hdr.iFlags & iFlag_Unicode)
			//			{
			//				// DIRECTORIES
			//				if (hdr.iDirCount > 0)
			//					READBIN(up_oDirs.get(), sizeof(DirTableEntryW) * hdr.iDirCount);
			//
			//				// FILES
			//				if (hdr.iFileCount > 0)
			//					READBIN(up_oFiles.get(), sizeof(FileTableEntryW) * hdr.iFileCount);
			//			}
			//			else // ASCII version
			//			{
			//				// DIRECTORIES
			//				if (hdr.iDirCount > 0)
			//				{
			//					auto up_oDirsA = std::make_unique<DirTableEntryA[]>(hdr.iDirCount);
			//					READBIN(up_oDirsA.get(), sizeof(DirTableEntryA) * hdr.iDirCount);
			//
			//					for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
			//					{
			//						auto &oDest = up_oDirs[iDir];
			//						auto &oSrc  = up_oDirsA[iDir];
			//
			//						oDest.iParentDirID = oSrc.iParentDirID;
			//						for (size_t iChar = 0; iChar < iElemNameLen; ++iChar)
			//						{
			//							oDest.szName[iChar] = oSrc.szName[iChar];
			//						}
			//					}
			//				}
			//								
			//				// FILES
			//				if (hdr.iFileCount > 0)
			//				{
			//					auto up_oFilesA = std::make_unique<FileTableEntryA[]>(hdr.iDirCount);
			//					READBIN(up_oFilesA.get(), sizeof(FileTableEntryA) * hdr.iDirCount);
			//
			//					for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
			//					{
			//						auto &oDest = up_oFiles[iDir];
			//						auto &oSrc  = up_oFilesA[iDir];
			//
			//						oDest.iParentDirID = oSrc.iParentDirID;
			//						oDest.iDataOffset  = oSrc.iDataOffset;
			//						oDest.iDataSize    = oSrc.iDataSize;
			//						for (size_t iChar = 0; iChar < iElemNameLen; ++iChar)
			//						{
			//							oDest.szName[iChar] = oSrc.szName[iChar];
			//						}
			//					}
			//				}
			//			}
			//
			//			// create directories
			//			for (size_t iDir = 0; iDir < hdr.iDirCount; ++iDir)
			//			{
			//				auto &oDir = up_oDirs[iDir];
			//				if (oDir.iParentDirID > 0 && oDir.iParentDirID >= iDir)
			//					return false; // invalid parent directory ID
			//
			//				wchar_t szName[51]{};
			//				memcpy_s(szName, sizeof(szName), oDir.szName, sizeof(oDir.szName));
			//
			//				if (oDir.iParentDirID == 0)
			//					up_oDirByIndex[iDir] = &m_oRootDir.directories()[szName];
			//				else
			//					up_oDirByIndex[iDir] =
			//					&up_oDirByIndex[oDir.iParentDirID]->directories()[szName];
			//			}
			//
			//			// read actual data
			//			for (size_t iFile = 0; iFile < hdr.iFileCount; ++iFile)
			//			{
			//				auto &oFile = up_oFiles[iFile];
			//
			//				wchar_t szName[51]{};
			//				memcpy_s(szName, sizeof(szName), oFile.szName, sizeof(oFile.szName));
			//
			//				File *pFile;
			//
			//				if (oFile.iParentDirID == 0)
			//					pFile = &m_oRootDir.files()[szName];
			//				else
			//					pFile = &up_oDirByIndex[oFile.iParentDirID - 1]->files()[szName];
			//
			//				pFile->create(oFile.iDataSize, false);
			//				READBIN(pFile->data(), pFile->size());
			//			}

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
			hdr.iDirCount  = m_oRootDir.totalDirectoryCount();
			hdr.iFileCount = m_oRootDir.totalFileCount();
			WRITEVAR(hdr);


			std::vector<std::wstring> oStrings;
			std::vector<TempDir>      oDirs;
			std::vector<TempFile>     oFiles;
			oDirs.reserve(m_oRootDir.totalDirectoryCount());
			oFiles.reserve(m_oRootDir.totalFileCount());

			GetFileContainerElements(0, m_oRootDir, oStrings, oDirs, oFiles);

			std::vector<size_t> oStringOffsets;
			oStringOffsets.reserve(oStrings.size());
			std::vector<size_t> oFileOffsets;
			oFileOffsets.reserve(oFiles.size());

			// write string table
			StringTableHeader sth{};
			sth.iStringCount = oStrings.size();
			if (bUnicode)
				sth.iFlags |= PAK_STRING_UNICODE;
			// get total string size
			{
				for (size_t i = 0; i < oStrings.size(); ++i)
				{
					sth.iStringTableSize += oStrings[i].length() + 1;
				}
				if (bUnicode)
					sth.iStringTableSize *= sizeof(wchar_t);
			}
			WRITEVAR(sth);
			const size_t posStrings = out.tellp();
			for (auto &s : oStrings)
			{
				oStringOffsets.push_back((size_t)out.tellp() - posStrings);
				if (bUnicode)
					WRITEBIN(s.c_str(), (s.length() + 1) * sizeof(wchar_t));
				else
				{
					auto upString = std::make_unique<char[]>(s.length() + 1);
					for (size_t i = 0; i < s.length(); ++i)
						upString[i] = (char)s[i];
					upString[s.length()] = 0; // terminating zero

					WRITEBIN(upString.get(), s.length() + 1);
				}
			}

			// write binary data
			DataBlockHeader dth{};
			// get total data size
			{
				for (size_t i = 0; i < oFiles.size(); ++i)
				{
					dth.iTotalDataSize += oFiles[i].pFile->size();
				}
			}
			WRITEVAR(dth);
			const size_t posData = out.tellp();
			for (size_t i = 0; i < oFiles.size(); ++i)
			{
				oFileOffsets.push_back((size_t)out.tellp() - posData);
				WRITEBIN(oFiles[i].pFile->data(), oFiles[i].pFile->size());
			}

			// write directory table
			DirTableEntry dte{};
			for (auto &oDir : oDirs)
			{
				dte.iParentDirID  = oDir.iParentDir;
				dte.iStringOffset = oStringOffsets[oDir.iStringIndex];
				WRITEVAR(dte);
			}

			// write file table
			FileTableEntry fte{};
			for (size_t iFile = 0; iFile < oFiles.size(); ++iFile)
			{
				auto &oFile = oFiles[iFile];

				fte.iStringOffset = oStringOffsets[oFile.iStringIndex];
				fte.iParentDirID  = oFile.iParentDir;
				fte.iDataOffset   = oFileOffsets[iFile];
				fte.iDataSize     = oFile.pFile->size();

				WRITEVAR(fte);
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