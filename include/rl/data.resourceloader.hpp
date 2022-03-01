/***************************************************************************************************
 FILE:	data.resourceloader.hpp
 CPP:	data.resourceloader.cpp
 DESCR:	Functions for simple binary resource handling
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_RESOURCELOADER
#define ROBINLE_DATA_RESOURCELOADER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;


#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Get a pointer to the data of a resource 
	/// </summary>
	/// <param name="lpType">
	/// = either the type's name as text, or the result of the <c>MAKEINTRESOURCE</c> macro with
	/// the type's ID (<c>RT_[...]</c>)
	/// </param>
	/// <param name="lpName">
	/// = either the resource's name as text, or the result of the <c>MAKEINTRESOURCE</c> macro with
	/// the resource's ID
	/// </param>
	/// <param name="size">(return value) = the size (in bytes) of the heap array</param>
	/// <returns>
	/// If the function succeeded, the return value is a pointer to the resource's data.<para/>
	/// If the function failed, the return value is <c>nullptr</c>.</returns>
	const uint8_t* GetResource(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, size_t& size);

}





#endif // ROBINLE_DATA_RESOURCELOADER