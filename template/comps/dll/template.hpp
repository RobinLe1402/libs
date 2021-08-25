/***************************************************************************************************
 FILE:	template.hpp
 DLL:	template.dll
 DESCR:	Template for new library header files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TEMPLATE
#define ROBINLE_TEMPLATE

#ifdef LIBRARY_EXPORTS
#    define DLL_API __declspec(dllexport)
#else
#    define DLL_API __declspec(dllimport)
#endif





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <headername>
// forward declarations


// #includes



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace Template_dll
	{

		// declarations

	}
}





// #undef foward declared definitions

#endif // ROBINLE_TEMPLATE