/***************************************************************************************************
 FILE:	rlOpenGLWin.hpp
 CPP:	rlOpenGLWin.cpp
 DESCR:	Class for creating an OpenGL (game) window
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_OPENGL_WINDOW_HPP
#define ROBINLE_OPENGL_WINDOW_HPP





//==================================================================================================
// FORWARD DECLARATIONS

#ifdef _WIN64
typedef long long nativeint_t;
typedef unsigned long long nativeuint_t;
#else
typedef long nativeint_t;
typedef unsigned int nativeuint_t;
#endif /* _WIN64 */

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned int uint32_t;

//--------------------------------------------------------------------------------------------------
// <Windows.h>
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
#define NULL 0
#define WINAPI __stdcall
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HMONITOR);
DECLARE_HANDLE(HWND);
typedef int					BOOL;
typedef unsigned long		DWORD;
typedef long				LONG;
typedef nativeint_t			LPARAM;
typedef nativeint_t			LRESULT;
typedef unsigned int		UINT;
typedef nativeuint_t		WPARAM;


#include <atomic>
#include <mutex>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// OpenGL coordinates for placing textures
	/// </summary>
	struct OpenGLCoord
	{
		float x, y;
	};


	/// <summary>
	/// Startup configuration for <c>rl::OpenGLWin</c>
	/// </summary>
	struct OpenGLWin_Config
	{
		uint32_t iWidth = 750, iHeight = 500; // initial client size in windowed mode
		bool bResizable = false; // should the window be resizable in windowed mode?
		bool bInitialFullscreen = false; // should the application start in fullscreen mode?
		bool bVSync = false; // should vsync be enabled by default?
		uint32_t iWidthMin = 0, iHeightMin = 0; // minimum client size in windowed mode
		uint32_t iWidthMax = 0, iHeightMax = 0; // maximum client size in windowed mode
		const wchar_t* szWinClassName = L"rlOpenGLWin"; // process unique window class name
		const wchar_t* szInitialCaption = L"RobinLe OpenGLWin App"; // initial window titlebar text
		HICON hIconBig = NULL; // initial big (taskbar) icon
		HICON hIconSmall = NULL; // initial small (title bar) icon
		HMONITOR hMonitorFullscreen = NULL; // monitor for fullscreen mode (NULL --> primary)
		bool bSysMenuAbout = true; // Should an "About" button be added to the system/window menu?
	};



	/// <summary>
	/// Create and manage a window that's contents get drawn by OpenGL<para/>
	/// Only initializes everything required for OpenGL to run.<para/>
	/// <c>glEnable(GL_TEXTURE_2D)</c>, for instance, isn't called by default.<para/>
	/// This is what the <c>OnCreate()</c> method was made for.
	/// </summary>
	class OpenGLWin
	{
	protected: // virtual methods

		/// <summary>
		/// Called from OpenGL thread after OpenGL was initialized and before the window gets
		/// displayed<para/>
		/// Create/load initial textures here
		/// </summary>
		/// <returns>Should the window loop start?</returns>
		virtual bool OnCreate() { return true; }

		/// <summary>
		/// Called from OpenGL thread on every screen update<para/>
		/// Must be overwritten as the default method always returns false<para/>
		/// Draw textures here
		/// </summary>
		/// <param name="fElapsedTime">= elapsed seconds since last call</param>
		/// <returns>Should the window keep running?</returns>
		virtual bool OnUpdate(float fElapsedTime) { return false; }

		/// <summary>
		/// Called from OpenGL thread before the window closes<para/>
		/// Only called if window loop was actually started -->
		/// <c>OnCreate()</c> must have returned <c>true</c><para/>
		/// Destroy/unload textures here
		/// </summary>
		/// <returns>
		/// Should the window really get destroyed? (Ignored if <c>WM_QUIT</c> is received)<para/>
		/// If <c>false</c> is returned, the window is still refreshed one last time
		/// </returns>
		virtual bool OnDestroy() { return true; }

		/// <summary>
		/// Called from WinAPI thread on every message the window receives
		/// </summary>
		/// <returns>
		/// Was the message processed?<para/>
		/// If <c>true</c>, <c>DefWindowProc</c> isn't called
		/// </returns>
		virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }

		/// <summary>
		/// Called from WinAPI thread right before the window is minimized
		/// </summary>
		virtual void OnMinimize() {}

		/// <summary>
		/// Called from WinAPI thread right before window is restored from being minimized
		/// </summary>
		virtual void OnRestore() {}

		/// <summary>
		/// Called from WinAPI thread when "About" button in the system menu is pressed<para/>
		/// Displays a MessageBox with the <c>rl::OpenGLWin</c> version by default
		/// </summary>
		virtual void OnAbout();


	public: // methods

		OpenGLWin();
		virtual ~OpenGLWin();

		/// <summary>
		/// Run the window<para/>
		/// Fails if a window is already running
		/// </summary>
		void run(OpenGLWin_Config config);


	protected: // methods

		/// <summary>
		/// Quit the execution immediately
		/// </summary>
		inline void quit();



		/// <summary>
		/// Minimize the window immediately (and pause all processing)
		/// </summary>
		void minimize();

		/// <summary>
		/// Is the window currently minimized?
		/// </summary>
		inline bool isMinimized() { return m_bMinimized; }

		/// <summary>
		/// Restores the window from minimization immediately (and continues all processing)
		/// </summary>
		void restore();



		/// <summary>
		/// Enable fullscreen mode/switch fullscreen monitor
		/// </summary>
		/// <param name="monitor">
		/// = monitor to use for fullscreen (<c>NULL</c>--&gt; default)
		/// </param>
		void setFullscreen(HMONITOR monitor = NULL);

		/// <summary>
		/// Is fullscreen mode currently enabled?
		/// </summary>
		inline bool getFullscreen() { return m_bFullscreen; }

		/// <summary>
		/// Set windowed mode
		/// </summary>
		void setWindowed();



		/// <summary>
		/// Set new client size for windowed mode
		/// </summary>
		void setWindowedSize(uint32_t width, uint32_t height);

		/// <summary>
		/// Get current client width<para/>
		/// Uses cached size data from before last <c>OnUpdate()</c> call
		/// </summary>
		inline uint32_t getWidth() { return m_iCachedWidth; }

		/// <summary>
		/// Get current client height<para/>
		/// Uses cached size data from before last <c>OnUpdate()</c> call
		/// </summary>
		inline uint32_t getHeight() { return m_iCachedHeight; }

		/// <summary>
		/// Get current client width<para/>
		/// Uses live size data which might have changed since the last <c>OnUpdate()</c> call
		/// </summary>
		inline uint32_t getLiveWidth() { return m_iWidth; }

		/// <summary>
		/// Get current client height<para/>
		/// Uses live size data which might have changed since the last <c>OnUpdate()</c> call
		/// </summary>
		inline uint32_t getLiveHeight() { return m_iHeight; }



		/// <summary>
		/// Switch VSync on or off
		/// </summary>
		inline void setVSync(bool enabled) { m_bAtomVSync = enabled; }

		/// <summary>
		/// Is VSync currently enabled?
		/// </summary>
		inline bool getVSync() { return m_bAtomVSync; }



		/// <summary>
		/// Get the window's handle
		/// </summary>
		inline HWND getHWND() { return m_hWnd; }

		/// <summary>
		/// Set window title
		/// </summary>
		void setTitle(const wchar_t* title);

		/// <summary>
		/// Set window and taskbar icon
		/// </summary>
		void setIcon(HICON BigIcon, HICON SmallIcon = NULL);



		/// <summary>
		/// Convert pixel coordinates to OpenGL coordinates (for texture placement)<para/>
		/// Uses cached size data from before last <c>OnUpdate()</c> call
		/// </summary>
		OpenGLCoord getPixelCoord(int x, int y);





		/// <summary>
		/// Get the current version of <c>rl::OpenGLWin</c>
		/// </summary>
		void getVersion(uint8_t(&dest)[4]);


	private: // methods

		static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/// <summary>
		/// Set the minimum/maximum size struct on <c>WM_GETMINMAXINFO</c>
		/// </summary>
		void setMinMaxStruct(LPARAM lParam);

		/// <summary>
		/// OpenGL thread function
		/// </summary>
		void OpenGLThread(HDC hDC);

		/// <summary>
		/// Process a minimized fullscreen window
		/// </summary>
		void processMinimize();

		/// <summary>
		/// Process a restored fullscreen window
		/// </summary>
		void processRestore();

		/// <summary>
		/// Notify all threads waiting for end of minimized state
		/// </summary>
		void wakeUpFromMinimized();

		/// <summary>
		/// Write the current size into the cache variables
		/// </summary>
		void cacheSize();

		/// <summary>
		/// Query quitting, wait for thread's answer
		/// </summary>
		/// <returns>thread's answer</returns>
		bool getQuitPermission();


	private: // variables

		HWND m_hWnd = NULL;
		bool m_bResizable = false; // window resizable if in windowed mode?
		uint32_t m_iWinWidth = 0, m_iWinHeight = 0; // client size in windowed mode
		uint32_t m_iWidthMin = 0, m_iHeightMin = 0; // minimum client size in windowed mode
		uint32_t m_iWidthMax = 0, m_iHeightMax = 0; // maximum client size in windowed mode
		bool m_bFullscreen = false; // currently fullscreen?
		wchar_t m_szWinClassName[256 + 1] = {}; // window class name
		HICON m_hIconBig = NULL, m_hIconSmall = NULL; // window icons
		HMONITOR m_hMonitorFullscreen = NULL; // monitor for fullscreen mode

		// variables for skipping minimized state
		std::mutex m_muxMinimize;
		std::condition_variable m_cvMinimize;

		static DWORD m_dwStyleCache; // cached dwStyle. Only used on first WM_GETMINMAXINFO

		int m_iWinPosX = 50, m_iWinPosY = 50; // for when switching to windowed mode

		static OpenGLWin* m_pInstance; // pointer to single instance

		static std::atomic<bool> m_bRunning; // for allowing run() only once
		std::atomic<bool> m_bMinimized = false; // window currently minimized?
		std::atomic<uint32_t> m_iWidth = 0, m_iHeight = 0; // current client size
		std::atomic<uint32_t> m_iCachedWidth = 0, m_iCachedHeight = 0; // cached client size
		std::atomic<bool> m_bAtomVSync = false; // is vsync enabled?
		std::atomic<bool> m_bAtomThreadConfirmRunning = false; // let thread confirm m_bAtomRunning
		std::atomic<bool> m_bAtomRunning = false; // should the window keep running?
		std::atomic<bool> m_bAtomOpenGLThreadRunning = false; // is the OpenGL thread running?
		std::atomic<bool> m_bAtomIdle = false; // are threads currently idle?

		int m_iMenuAbout = 0; // ID of the "About" button in the system menu 

	};



	//----------------------------------------------------------------------------------------------
	// INLINE METHOD DEFINITION

	void OpenGLWin::quit() { m_bAtomRunning = false; }

}





#undef CW_USEDEFAULT
#undef DECLARE_HANDLE
#undef NULL
#undef WINAPI

#endif // ROBINLE_OPENGL_WINDOW_HPP