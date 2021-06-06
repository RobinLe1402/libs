/***************************************************************************************************
 FILE:	exe16parser.hpp
 CPP:	exe16parser.cpp
 DESCR:	A resource parser for 16-bit NE executables
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_EXEPARSER_16BIT
#define ROBINLE_BINPARSER_16BIT





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

//--------------------------------------------------------------------------------------------------
// <Windows.h>
typedef long long LONG_PTR;
typedef const char* LPCSTR;
typedef char* LPSTR;


#include <string>
#include <functional>
#include <vector>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// An identifier for a NE (16-bit) executable resource<para/>
	/// If <c>bIsInt</c> is <c>true</c>, <c>ID_iID</c> is valid.
	/// Otherwise, <c>ID_szName</c> is valid.
	/// </summary>
	struct NEResID
	{
		bool bIsInt;

		uint16_t ID_iID; // integer identifier
		const char* ID_szName; // ASCII text identifier

		
		bool operator==(const NEResID& other) const;
		bool operator!=(const NEResID& other) const;
	};


#define RL_X16RESPARSER_E_NOERROR		0 // no error occured
#define RL_X16RESPARSER_E_FILEERROR		1 // file doesn't exist/couldn't be read
#define RL_X16RESPARSER_E_NOEXE			2 // file was no executable
#define RL_X16RESPARSER_E_NOEXE16		3 // file was no 16-bit executable


	/// <summary>
	/// A parser for a 16-bit NE executable's resources<para/>
	/// Not thread-safe
	/// </summary>
	class x16ResourceParser
	{
	public: // methods

		x16ResourceParser() {}
		~x16ResourceParser();

		/// <summary>
		/// Parse the resource metadata of a 16-bit executable<para/>
		/// This method doesn't really load any of the resources. The name is just for simplicity.
		/// </summary>
		/// <returns>
		/// Did the method succeed?<para/>
		/// For extended error information, call method <c>getError()</c>
		/// </returns>
		bool load(const wchar_t* szPath);

		/// <summary>
		/// Clear the currently loaded data
		/// </summary>
		void clear();

		/// <summary>
		/// If a previous call to <c>load()</c> failed, this method returns the error's ID.
		/// </summary>
		/// <returns>One of the <c>RL_X16RESPARSER_E_</c> defines</returns>
		inline uint8_t getError() { return m_iError; }

		/// <summary>
		/// Was an EXE parsed?<para/>
		/// Determines if data can be requested via <c>getResCount()</c>, <c>getRes()</c> etc.
		/// TODO: final method names
		/// </summary>
		/// <returns></returns>
		inline bool hasInfo() { return m_bParsed; }

		/// <summary>
		/// Enumerate all resource types
		/// </summary>
		bool enumResourceTypes(std::function<bool(x16ResourceParser& oModule, LPCSTR lpType,
			LONG_PTR lParam)> lpEnumFunc, LONG_PTR lParam);

		bool enumResourceNames(LPCSTR lpType, std::function<bool(x16ResourceParser& oModule,
			LPCSTR lpType, LPCSTR lpName, LONG_PTR lParam)> lpEnumFunc, LONG_PTR lParam);

		/// <summary>
		/// Loads a resource into memory
		/// </summary>
		/// <returns>Could the resource be loaded?</returns>
		bool loadResource(LPCSTR lpType, LPSTR lpName, const uint8_t** pData, size_t* iSize);

		/// <summary>
		/// Delete a resource from memory that was previously loaded via <c>loadResource</c><para/>
		/// Currently only supports default resources, via <c>RT_</c> constant
		/// </summary>
		void unloadResource(LPCSTR lpType, LPSTR lpName);

		/// <summary>
		/// Unload all loaded resources
		/// </summary>
		void unloadResources();


	private: // types

		/// <summary>
		/// Struct representation of a NE (16-bit) executable resource
		/// </summary>
		struct NERes
		{
			NEResID oID; // resource identifier

			size_t iOffset; // offset to resource start, from EXE file start
			size_t iSize; // resource size in bytes

			uint8_t* pData; // if resource was loaded via loadResource(), this pointer is valid
		};

		/// <summary>
		/// Struct representation of a NE (16-bit) executable resource type
		/// </summary>
		struct NEResType
		{
			NEResID oID; // resource type identifier

			std::vector<NERes> oResources; // all resources of this type
		};


	private: // methods

		bool resTypeIndexByName(const char szTypeName);


	private: // variables

		uint8_t m_iError = RL_X16RESPARSER_E_NOERROR; // error container

		bool m_bParsed = false; // does m_oTypes contain valid data?
		std::wstring m_sFilepath; // parsed file/s path
		std::vector<NEResType> m_oTypes; // all resource types in the parsed executable
		std::vector<char*> m_oIDStrings; // all ID strings pointed to by any item in m_oTypes

	};

}





#endif // ROBINLE_EXEPARSER_16BIT