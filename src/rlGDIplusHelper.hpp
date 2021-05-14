/***************************************************************************************************
 FILE:	rlGDIPlusHelper.hpp
 CPP:	n/a
 DESCR:	Helper class for automatic call to Gdiplus::GdiplusShutdown
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GDIPLUSHELPER
#define ROBINLE_GDIPLUSHELPER





#include <Windows.h>
#include <gdiplus.h>

namespace rl
{
	
	/// <summary>
	/// Helper class for GDI+<para/>
	/// Automatically starts GDI+ on creation and shuts it down on destruction
	/// </summary>
	class GDIPlus
	{
	public:

		GDIPlus()
		{
			Gdiplus::GdiplusStartupInput gpSI;
			Gdiplus::GdiplusStartup(&m_gpToken, &gpSI, NULL);
		}

		~GDIPlus()
		{
			Gdiplus::GdiplusShutdown(m_gpToken);
		}


	private:

		ULONG_PTR m_gpToken = NULL;
	};

}





#endif // ROBINLE_GDIPLUSHELPER