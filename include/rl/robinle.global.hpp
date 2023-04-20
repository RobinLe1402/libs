/***************************************************************************************************
 FILE:	robinle.global.hpp
 CPP:	robinle.global.cpp
 DESCR:	Some global definitions for the RobinLe infrastructure
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GLOBAL
#define ROBINLE_GLOBAL





namespace rl
{
	namespace global
	{

		constexpr wchar_t szREGISTRY_VALUE_APPDIR[] = L"AppDir";
		constexpr wchar_t szREGISTRY_VALUE_DLLDIR[] = L"DllPath";

		bool Initialize();

	}
}





#endif // ROBINLE_GLOBAL