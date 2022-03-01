/***************************************************************************************************
 FILE:	converter.hpp
 CPP:	converter.cpp
 DESCR:	Convert FON files to rlFON data/files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_FONT_CONVERTER
#define ROBINLE_FONT_CONVERTER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#include <string>
#pragma comment(lib, "version.lib")



//==================================================================================================
// DECLARATION
namespace rl
{
	/// <summary>
	/// Struct representation of a VERSIONINFO resource
	/// </summary>
	struct VERSION_INFO
	{
		uint64_t iFileVersion;
		uint64_t iProductVersion;
		uint32_t iFileFlagsMask;
		uint32_t iFileFlags;
		uint32_t iFileOS;
		uint32_t iFileType;
		uint32_t iFileSubtype;

		std::wstring sComments;
		std::wstring sCompanyName;
		std::wstring sFileDescription;
		std::wstring sFileVersion;
		std::wstring sInternalName;
		std::wstring sLegalCopyright;
		std::wstring sOriginalFilename;
		std::wstring sProductName;
		std::wstring sProductVersion;
	};





	/// <summary>
	/// Class representation of a Microsoft FON file
	/// </summary>
	class MicrosoftFON
	{
	public: // types

		/// <summary>
		/// Metadata of a single font face
		/// </summary>
		struct FaceMeta
		{
			char szCopyright[61] = {};
			bool bRaster;
			// TODO: continue
		};

		struct Meta
		{
			
		};


	public: // methods

		/// <summary>
		/// Read the version information of a FON file
		/// </summary>
		static bool GetVersionInfo(const wchar_t* szFON, VERSION_INFO& ver);

		static bool ReadMetadata(const wchar_t* szFON, Meta& meta);

	};
	
}





// #undef foward declared definitions

#endif // ROBINLE_FONT_CONVERTER