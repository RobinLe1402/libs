/***************************************************************************************************
 FILE:	OpenGLWin.hpp
 LIB:	OpenGLWin.lib
 DESCR:	The basic framework for a OpenGL-based window
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_OPENGLWIN
#define ROBINLE_LIB_OPENGLWIN





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint32_t = unsigned;


//--------------------------------------------------------------------------------------------------
// <gl/GL.h>
using GLint = int;
using GLuint = unsigned;
using GLfloat = float;


#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGLWin
	{
		class Window;
		class IRenderer;



		struct WindowConfig
		{
			std::wstring sTitle = L"RobinLe OpenGLWin";
			bool bResizable = false;
			bool bFullscreen = false;
			unsigned iWidth = 500, iHeight = 300;
			unsigned iMinWidth = 0, iMinHeight = 0;
			unsigned iMaxWidth = 0, iMaxHeight = 0;
			HICON hIconSmall = NULL, hIconBig = NULL;
			HMONITOR hMonintorFullscreen = NULL;
		};

		struct RendererConfig
		{
			bool bVSync = false;
		};

		struct OpenGLWinConfig
		{
			WindowConfig window;
			RendererConfig renderer;
		};


		struct WindowMessage
		{
			UINT uMsg;
			WPARAM wParam;
			LPARAM lParam;
		};



		/// <summary>
		/// An interface for the logic behind an OpenGL-based application
		/// </summary>
		class IApplication
		{
			friend class Window;
			friend class IRenderer;


		private: // static variables

			static bool s_bRunning;


		protected: // events

			virtual bool OnStart() { return true; }
			virtual bool OnUpdate(float fElapsedTime) = 0;
			virtual bool OnStop() { return true; }

			virtual void OnResize(LONG& iWidth, LONG& iHeight) { }

			virtual void OnMinize() {}
			virtual void OnRestore() {}
			virtual void OnGainFocus() {}
			virtual void OnLoseFocus() {}


		public: // methods

			IApplication() = default;
			IApplication(const IApplication& other) = delete;
			IApplication(IApplication&& rval) = delete;
			virtual ~IApplication() = default;

			bool execute(OpenGLWinConfig& cfg, Window& oWindow, IRenderer& oRenderer);


		protected: // methods

			// for use in the IApplication derivative and the IRenderer base class only
			auto window() { return m_pWindow; }
			auto renderer() { return m_pRenderer; }

			/// <summary>
			/// [Use by <c>Window</c>]
			/// Inform the application thread that the user is attempting to close the window,
			/// wait for a reply
			/// </summary>
			/// <returns>Did the application thread accept the closing request?</returns>
			bool winClose();

			/// <summary>
			/// [Use by <c>Window</c>]
			/// Send a message to the application thread and wait for it to handle it
			/// </summary>
			void winMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

			/// <summary>
			/// [Use by <c>IApplication</c>]
			/// Handle a message from the window if there is one
			/// </summary>
			bool handleMessage();


		private: // variables

			Window* m_pWindow = nullptr;
			IRenderer* m_pRenderer = nullptr;
			std::atomic_bool m_bAtomRunning = false;

			std::mutex m_muxWindow;
			std::condition_variable m_cvWinClose;
			std::condition_variable m_cvWinMsg;
			std::condition_variable m_cvMinimized;
			bool m_bSleeping = false;
			bool m_bMessageByApp = false;

			const WindowMessage* m_pMessage = nullptr;

		};



		/// <summary>
		/// A window managing class
		/// </summary>
		class Window
		{
		protected: // events

			/// <summary>
			/// Event handler: Window has been created but is not visible yet
			/// </summary>
			virtual void OnCreate() {}
			/// <summary>
			/// Message handler
			/// </summary>
			/// <returns>Was the message handled?</returns>
			virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }
			/// <summary>
			/// Event handler: Window was destroyed
			/// </summary>
			virtual void OnDestroy() {}


		private: // static methods

			static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


		private: // static variables

			static Window* s_pInstance;


		public: // methods

			Window(IApplication& oApplication, const wchar_t* szClassName = L"RobinLeOpenGLWin");
			Window(const Window& other) = delete;
			Window(Window&& rval) = delete;
			~Window();

			bool create(const WindowConfig& cfg);
			void destroy();

			bool running() { return m_bMessageLoop; }
			auto handle() { return m_hWnd; }
			bool minimized() { return m_bMinimized; }
			bool maximized() { return m_bMaximized; }
			bool fullscreen() { return m_bFullscreen; }
			auto width() { return m_iWidth; }
			auto height() { return m_iHeight; }
			auto minWidth() { return m_iMinWidth; }
			auto minHeigth() { return m_iMinHeight; }
			auto maxWidth() { return m_iMaxWidth; }
			auto maxHeigth() { return m_iMaxHeight; }
			auto nativeWidth() { return m_iNativeWidth; }
			auto nativeHeight() { return m_iNativeHeight; }
			const std::wstring& title() { return m_sTitle; }

			void show();
			void setTitle(const wchar_t* szTitle);
			void minimize();
			void setSize(unsigned iWidth, unsigned iHeight);
			void setFullscreen(bool bFullscreen, HMONITOR hMon = NULL);

			void windowToClient(unsigned& iWidth, unsigned& iHeight)
			{
				iWidth -= m_iClientToScreenX;
				iHeight -= m_iClientToScreenY;
			}
			void clientToWindow(unsigned& iWidth, unsigned& iHeight)
			{
				iWidth += m_iClientToScreenX;
				iHeight += m_iClientToScreenY;
			}


		private: // methods

			void messageLoop(WindowConfig cfg);
			void clear();
			/// <summary>
			/// Re-generate the <c>dwStyle</c> value based on the current state of the <c>Window</c>
			/// class and update the Client-To-Window size values
			/// </summary>
			DWORD refreshStyle();


		private: // variables

			IApplication& m_oApplication;
			std::thread m_trdMessageLoop;

			const std::wstring m_sClassName;

			std::mutex m_muxState;
			std::condition_variable m_cvState; // on create() and destroy()

			HWND m_hWnd = NULL;
			bool m_bAppClose = false, m_bWinClose = false; // sender of a recent close request
			bool m_bMessageLoop = false; // does the message loop currently run?
			bool m_bThreadRunning = false; // is the message thread currently running?



			// window size data
			unsigned m_iWidth = 0, m_iHeight = 0; // current client size
			unsigned m_iNativeWidth = 0, m_iNativeHeight = 0; // window size, including the border
			int m_iClientToScreenX = 0, m_iClientToScreenY = 0;
			unsigned m_iMinWidth = 0, m_iMinHeight = 0; // minimum (windowed) client size
			unsigned m_iMaxWidth = 0, m_iMaxHeight = 0; // maximum (windowed) client size
			unsigned m_iRestoredWidth = 0, m_iRestoredHeight = 0; // last restored window size

			// window position data
			int m_iWindowX = 0, m_iWindowY = 0;  // last restored position window position
			int m_iMaximizedX = 0, m_iMaximizedY = 0;
			
			// window state data
			std::wstring m_sTitle = L"";
			HMONITOR m_hMonitorFullscreen = NULL;
			bool m_bResizable = false;
			bool m_bMinimized = false;
			bool m_bMaximized = false;
			bool m_bFullscreen = false;


		};

		// ToDo: improve windowToClient, complete fullscreen functionality



		/// <summary>
		/// An interface for OpenGL graphics
		/// </summary>
		class IRenderer
		{
			friend class IApplication;

		protected: // events

			virtual void OnCreate() {}
			virtual void OnUpdate() = 0;
			virtual void OnDestroy() {}


		public: // methods

			IRenderer(IApplication& oApplication);
			IRenderer(const IRenderer& other) = delete;
			IRenderer(IRenderer&& rval) = delete;
			virtual ~IRenderer();


			/// <summary>
			/// create the renderer
			/// </summary>
			/// <returns>Could the renderer be created?</returns>
			bool create(HDC hDC, unsigned iWidth, unsigned iHeight, bool bVSync);

			/// <summary>
			/// Destroy the renderer
			/// </summary>
			void destroy();

			/// <summary>
			/// Update the viewport
			/// </summary>
			void update();

			/// <summary>
			/// Resize the viewport
			/// </summary>
			void resize(unsigned iWidth, unsigned iHeight);

			void setVSync(bool bVSync);
			bool getVSync() { return m_bVSync; }


		protected: // methods

			auto width() { return m_iWidth; }
			auto height() { return m_iHeight; }


		protected: // variables

			IApplication& m_oApplication;
			HDC m_hDC = NULL;
			HGLRC m_hGLRC = NULL;

			bool m_bVSync = false;


		private: // variables

			unsigned m_iWidth = 0, m_iHeight = 0;

		};



	}
}





// #undef foward declared definitions

#endif // ROBINLE_LIB_OPENGLWIN