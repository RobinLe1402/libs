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

		uint8_t *data() noexcept { return m_upData.get(); }
		const uint8_t *data() const noexcept { return m_upData.get(); }

		auto size() const noexcept { return m_iSize; }


	private: // variables

		size_t m_iSize = 0;
		std::unique_ptr<uint8_t[]> m_upData;

	};
	


	/// <summary>
	/// A container for virtual files. Can be saved to and loaded from <c>.rlPAK</c> files.
	/// </summary>
	class FileContainer final
	{
	public: // types

		/// <summary>
		/// A virtual directory.
		/// </summary>
		class Directory final
		{
			friend class FileContainer;
		public: // methods

			bool addDirectoryContents(const wchar_t *szDirPath, bool bRecursive);

			void clear() noexcept;

			auto &files() noexcept { return m_oFiles; }
			auto &files() const noexcept { return m_oFiles; }

			auto &directories() noexcept { return m_oSubDirectories; }
			auto &directories() const noexcept { return m_oSubDirectories; }


		private: // methods

			size_t totalFileCount() const noexcept;
			size_t totalDirectoryCount() const noexcept;

			bool saveable(bool bUnicode) const noexcept;


		private: // variables

			std::map<std::wstring, File> m_oFiles;
			std::map<std::wstring, Directory> m_oSubDirectories;

		};


	public: // methods

		bool load(const wchar_t *szPath);
		bool save(const wchar_t *szPath, bool bUnicode) const;

		auto &data()       { return m_oRootDir; }
		auto &data() const { return m_oRootDir; }


	private: // variables

		Directory m_oRootDir;

	};
	
}





#endif // ROBINLE_DATA_FILECONTAINER