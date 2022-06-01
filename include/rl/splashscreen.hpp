/***************************************************************************************************
 FILE:	splashscreen.hpp
 CPP:	splashscreen.cpp
 DESCR:	Splash screen for application/games
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_SPLASHSCREEN
#define ROBINLE_SPLASHSCREEN





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


#include <condition_variable>
#include <mutex>

#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// SplashScreen data
	/// </summary>
	struct SplashScreen_Config
	{
		bool bAlwaysOnTop = false; // should the splashscreen be shown on top of every other window?
		bool bDropShadow = false; // should a drop shadow be added?
		HMONITOR hMonitor = NULL; // monitor to show the splashscreen on. NULL --> default monitor
		uint32_t iBackgroundColor = 0xFFFFFF; // background color for transparent images.
	};



	/// <summary>
	/// Bitmap-based splashscreen<para/>
	/// Displays a bitmap resource
	/// </summary>
	class SplashScreen
	{
	public: // static methods

		/// <summary>
		/// Show a splashscreen with a BITMAP resource.<para/>
		/// Does nothing if a splashscreen is already shown.<para/>
		/// Initially sets <c>GetLastError()</c> to 0 to keep track of it's own state.
		/// </summary>
		/// <param name="iBitmapID">Resource ID of the BITMAP resource to show.</param>
		/// <returns>Could the splashscreen be shown?</returns>
		static bool ShowBitmap(const SplashScreen_Config& cfg, int iBitmapID);

		/// <summary>
		/// Show a splashscreen with an image resource.<para/>
		/// Does nothing if a splashscreen is already shown.<para/>
		/// Initially sets <c>GetLastError()</c> to 0 to keep track of it's own state.
		/// </summary>
		/// <param name="ResType">Name/ID of the resource type.</param>
		/// <param name="iResID">
		/// ID of the image resource to show.<para />
		/// The resource data must be of a type supported by GDI+ (BMP/GIF/ICO/JPEG/PNG/TIFF).
		/// </param>
		/// <returns>Could the splashscreen be shown?</returns>
		static bool ShowImage(const SplashScreen_Config& cfg, const wchar_t* ResType, int iResID);

		/// <summary>
		/// Close the splashscreen
		/// </summary>
		static void Close();


	private: // static methods

		static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // static variables

		static const wchar_t s_szWinClassName[]; // window class name
		static SplashScreen s_oSplash; // singleton for guaranteed GDI+ shutdown


	private: // methods

		SplashScreen() {} // no need for creating an instance
		~SplashScreen(); // for guaranteed call to Gdiplus::shutdown

		void threadFunc();
		bool showInternal();
		void closeInternal();


	private: // variables

		SplashScreen_Config m_oCfg;
		unsigned m_iWidth = 0, m_iHeight = 0;

		bool m_bVisible = false; // is the splashscreen currently shown?
		std::mutex m_mux;
		std::condition_variable m_cv;
		bool m_bRunning = false;
		std::thread m_trd; // window thread
		HBITMAP m_hBmp = NULL;

	};

}





#endif // ROBINLE_SPLASHSCREEN