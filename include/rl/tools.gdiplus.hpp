/***************************************************************************************************
 FILE:	tools.gdiplus.hpp
 CPP:	n/a
 DESCR:	Helper class for automatic call to Gdiplus::GdiplusShutdown
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TOOLS_GDIPLUS
#define ROBINLE_TOOLS_GDIPLUS





#undef min
#undef max

#define NOMINMAX
#include <Windows.h>
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#include <gdiplus.h>
#undef min
#undef max

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





#endif // ROBINLE_TOOLS_GDIPLUS