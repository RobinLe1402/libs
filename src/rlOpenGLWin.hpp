/***************************************************************************************************
 FILE:	rlOpenGLWin.hpp
 LIB:	rlOpenGLWin.lib
 DESCR:	Library for creating an OpenGL window
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
DECLARE_HANDLE(HGLRC);
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
#include <string>

typedef BOOL(WINAPI wglSwapInterval)(int interval);



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Startup configuration for <c>rl::OpenGLWin</c>
	/// </summary>
	struct OpenGLWin_Config
	{
		uint32_t iWidth = 0, iHeight = 0; // initial client size in windowed mode
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
		/// Called on every screen update<para/>
		/// Must be overwritten as the default method always returns false
		/// </summary>
		/// <param name="fElapsedTime">= elapsed seconds since last call</param>
		/// <returns>Should the window keep running?</returns>
		virtual bool OnUpdate(float fElapsedTime) { return false; }

		/// <summary>
		/// Called after OpenGL was initialized and before the window gets shown
		/// </summary>
		/// <returns>Should the window loop start?</returns>
		virtual bool OnCreate() { return true; }

		/// <summary>
		/// Called when a <c>WM_CLOSE</c> message is received
		/// </summary>
		/// <returns>Should the window get destroyed?</returns>
		virtual bool OnTryClosing() { return true; }

		/// <summary>
		/// Called before the window closes<para/>
		/// Only called if window loop was actually started -->
		/// <c>OnCreate()</c> must have returned <c>true</c>
		/// </summary>
		/// <returns>
		/// Should the window really get destroyed? (Ignored if <c>WM_QUIT</c> is received)
		/// </returns>
		virtual bool OnDestroy() { return true; }

		/// <summary>
		/// Called on every message the window receives
		/// </summary>
		/// <returns>
		/// Was the message processed?<para/>
		/// If <c>true</c>, <c>DefWindowProc</c> isn't called
		/// </returns>
		virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }

		/// <summary>
		/// Called right before the window is minimized
		/// </summary>
		virtual void OnMinimize() {}

		/// <summary>
		/// Called right before window is restored from being minimized
		/// </summary>
		virtual void OnRestore() {}


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
		inline bool getMinimized() { return m_bMinimized; }

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
		/// Get current client width
		/// </summary>
		inline uint32_t getWidth() { return m_iWidth; }

		/// <summary>
		/// Get current client height
		/// </summary>
		inline uint32_t getHeight() { return m_iHeight; }



		/// <summary>
		/// Switch VSync on or off
		/// </summary>
		void setVSync(bool enabled);

		/// <summary>
		/// Is VSync currently enabled?
		/// </summary>
		inline bool getVSync() { return m_bVSync; }



		/// <summary>
		/// Get the window's handle
		/// </summary>
		inline HWND getHWND() { return m_hWnd; }

		/// <summary>
		/// Get the window's device context handle
		/// </summary>
		/// <returns></returns>
		inline HDC getHDC() { return m_hDC; }

		/// <summary>
		/// Set window title
		/// </summary>
		void setTitle(const wchar_t* title);

		/// <summary>
		/// Set window and taskbar icon
		/// </summary>
		void setIcon(HICON BigIcon, HICON SmallIcon = NULL);


	private: // methods

		static LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/// <summary>
		/// Update OpenGL viewport to current values of <c>m_iWidth</c> and <c>m_iHeight</c>
		/// </summary>
		void updateViewport();

		/// <summary>
		/// Repaint the viewport
		/// </summary>
		/// <param name="bWaitForVSync">= should be waited for VSync? (Only in main loop)</param>
		void repaint(bool bWaitForVSync = false);

		/// <summary>
		/// Update size variables
		/// </summary>
		void processResize(uint32_t iNewClientX, uint32_t iNewClientY);

		/// <summary>
		/// Set the minimum/maximum size struct on <c>WM_GETMINMAXINFO</c>
		/// </summary>
		void setMinMaxStruct(LPARAM lParam);


	private: // variables

		HWND m_hWnd = NULL;
		HDC m_hDC = NULL;
		uint32_t m_iWidth = 0, m_iHeight = 0; // current client size
		bool m_bMinimized = false; // window currently minimized?
		bool m_bResizable = false; // window resizable if in windowed mode?
		bool m_bVSync = false; // is vsync enabled?
		uint32_t m_iWinWidth = 0, m_iWinHeight = 0; // client size in windowed mode
		uint32_t m_iWidthMin = 0, m_iHeightMin = 0; // minimum client size in windowed mode
		uint32_t m_iWidthMax = 0, m_iHeightMax = 0; // maximum client size in windowed mode
		bool m_bFullscreen = false; // currently fullscreen?
		wchar_t m_szWinClassName[256 + 1] = {}; // window class name
		HICON m_hIconBig = NULL, m_hIconSmall = NULL; // window icons
		HMONITOR m_hMonitorFullscreen = NULL; // monitor for fullscreen mode
		HGLRC m_hGLRC = NULL; // OpenGL rendering context

		static DWORD m_dwStyleCache; // cached dwStyle. Only used on first WM_GETMINMAXINFO
		static bool m_bUnknownSize; // is the current exact window size unknown?

		int m_iWinPosX = 50, m_iWinPosY = 50; // for when switching to windowed mode

		bool m_bBlockResize = false; // for while setting fullscreen/windowed mode

		std::atomic<bool> m_bAtomRunning = false; // is the window loop currently running?

		wglSwapInterval* m_wglSwapInterval = nullptr; // for enabling/disabling vsync

		static bool m_bRunning; // is this class currently active?
		static OpenGLWin* m_pInstance;

	};



	//----------------------------------------------------------------------------------------------
	// INLINE METHOD DEFINITION

	void OpenGLWin::quit() { m_bAtomRunning = false; }

}





#undef CW_USEDEFAULT
#undef DECLARE_HANDLE
#undef NULL
#undef WINAPI

#endif /* ROBINLE_OPENGL_WINDOW_HPP */