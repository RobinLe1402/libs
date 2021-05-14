/***************************************************************************************************
 FILE:	rlSplashScreen.hpp
 CPP:	rlSplashScreen.cpp
 DESCR:	Splash screen for application/games
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_BOOTSCREEN
#define ROBINLE_BOOTSCREEN





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <atomic>
namespace std
{
	template <class _Ty>
	struct atomic;
}


//--------------------------------------------------------------------------------------------------
// <thread>
namespace std
{
	class thread;
}

//--------------------------------------------------------------------------------------------------
// <Windows.h>
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
#define WINAPI __stdcall
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HMONITOR);
DECLARE_HANDLE(HWND);
typedef unsigned int UINT;

#ifdef _WIN64
typedef long long LPARAM;
typedef long long LRESULT;
typedef unsigned long long WPARAM;
#else
typedef long LPARAM;
typedef long LRESULT;
typedef unsigned int WPARAM;
#endif // _WIN64


#include <atomic>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// SplashScreen data
	/// </summary>
	struct SplashScreen_Config
	{
		int iBitmapID; // resource ID of the bitmap to show
		bool bAlwaysOnTop = false; // should the splashscreen be shown on top of every other window?
		bool bDropShadow = false; // should a drop shadow be added?
		HMONITOR hMonitor = NULL; // monito to show the splashscreen on. NULL --> default monitor

		SplashScreen_Config(int iBitmapID) : iBitmapID(iBitmapID) {}
	};



	/// <summary>
	/// Bitmap-based splashscreen<para/>
	/// Displays a bitmap resource
	/// </summary>
	class SplashScreen
	{
	public: // methods

		/// <summary>
		/// Show the splashscreen<para/>
		/// Does nothing if a splashscreen is already shown<para/>
		/// Calls <c>GetLastError()</c> until it returns 0 to keep track of it's own state
		/// </summary>
		/// <returns>Could the splashscreen be shown?</returns>
		static bool Show(SplashScreen_Config config);

		/// <summary>
		/// Close the splashscreen
		/// </summary>
		static void Close();


	private: // methods

		SplashScreen() {} // no need for creating an instance
		~SplashScreen(); // for guaranteed call to Gdiplus::shutdown

		static void threadFunc(SplashScreen_Config config, unsigned int iWidth, unsigned int iHeight);
		static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // variables

		static bool m_bShowing; // is a splashscreen currently shown?
		static std::atomic<bool> m_bAtomShow; // should the window be shown?
		static std::thread m_trd; // window thread
		static HBITMAP m_hBmp;
		static const wchar_t m_szWinClassName[]; // window class name
		static bool m_bDropShadow;

		static SplashScreen m_bs; // singleton for guaranteed GDI+ shutdown

	};

}





#undef DECLARE_HANDLE
#undef WINAPI

#endif // ROBINLE_BOOTSCREEN