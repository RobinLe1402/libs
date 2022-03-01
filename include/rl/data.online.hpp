/***************************************************************************************************
 FILE:	data.online.hpp
 CPP:	data.online.cpp
 DESCR:	Functionality for interacting with online data
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TEMPLATE
#define ROBINLE_TEMPLATE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;


// #includes



//==================================================================================================
// DECLARATION
namespace rl
{
	
	bool DownloadToMemory(const char* szURL, uint8_t** pDest, size_t* pLen);
	
}





// #undef foward declared definitions

#endif // ROBINLE_TEMPLATE