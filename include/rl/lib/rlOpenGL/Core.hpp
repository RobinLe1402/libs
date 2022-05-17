/***************************************************************************************************
 FILE:	lib/rlOpenGL/Core.hpp
 LIB:	rlOpenGL.lib
 DESCR:	The basic framework for a OpenGL-based window
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_OPENGL__CORE
#define ROBINLE_LIB_OPENGL__CORE





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
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGL
	{
		class Window;
		class IRenderer;



		/// <summary>
		/// The startup configuration for a <c>Window</c> object
		/// </summary>
		struct WindowConfig
		{
			std::wstring sTitle = L"RobinLe OpenGLWin";
			bool bResizable = false;
			bool bFullscreen = false;
			unsigned iWidth = 500, iHeight = 300;
			unsigned iMinWidth = 0, iMinHeight = 0;
			unsigned iMaxWidth = 0, iMaxHeight = 0;
			HICON hIconSmall = NULL, hIconBig = NULL;
			HMONITOR hMonintorFullscreen = NULL; // if NULL --> Default monitor
		};



		/// <summary>
		/// A window managing class
		/// </summary>
		class Window
		{
		public: // types

			using MessageCallback = std::function<void(UINT uMsg, WPARAM wParam, LPARAM lParam)>;
			using VoidCallback = std::function<void()>;


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


		public: // static methods

			/// <summary>
			/// Get the minimum client size of a window in the current OS.
			/// </summary>
			/// <param name="bResizable">Is the window resizable?</param>
			/// <param name="iX">[Result] The minimum client width for a window.</param>
			/// <param name="iY">[Result] The minimum client height for a window.</param>
			/// <returns></returns>
			static void GetOSMinWindowedSize(bool bResizable, unsigned& iX, unsigned& iY);


		private: // static methods

			static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


		private: // static variables

			static Window* s_pInstance;


		public: // methods

			Window(const wchar_t* szClassName = L"RobinLeOpenGLWin");
			Window(const Window& other) = delete;
			Window(Window&& rval) = delete;
			virtual ~Window() = default;

			// both create() and destroy() are only called by IApplication
			bool create(const WindowConfig& cfg,
				MessageCallback fnOnMessage,
				VoidCallback fnOnMinimize, VoidCallback fnOnRestore);
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
			const std::wstring& title() { return m_sTitle; }

			void show();
			void setTitle(const wchar_t* szTitle);
			void setTitle(const char* szTitle); // ASCII only --> Non-ASCII gets replaced by "?"
			void minimize();
			void setSize(unsigned iWidth, unsigned iHeight);
			void setFullscreen(bool bFullscreen, HMONITOR hMon = NULL);

			void windowToClient(unsigned& iWidth, unsigned& iHeight)
			{
				iWidth -= m_iBorderWidth;
				iHeight -= m_iBorderHeight;
			}
			void clientToWindow(unsigned& iWidth, unsigned& iHeight)
			{
				iWidth += m_iBorderWidth;
				iHeight += m_iBorderHeight;
			}

			/// <summary>Get the minimum client width allowed by the OS</summary>
			auto getOSMinWidth() const { return m_iOSMinWidth; }
			/// <summary>Get the minimum client height allowed by the OS</summary>
			auto getOSMinHeight() const { return m_iOSMinHeight; }

			bool getCloseRequested() const { return m_bCloseRequested; }
			void clearCloseRequest() { m_bCloseRequested = false; }


		protected: // methods

			// invoke a synchronous, window-related function that might get called from the
			// application thread (if the calling thread is the application thread, a temporary
			// thread is created and detached)

			/// <summary>
			/// Invoke a synchronous, window-related function.<para />
			/// Use if the current thread might be the application thread
			/// (--> "raw call" might lead to a softlock).
			/// </summary>
			template <typename TResult, typename... TArgsFn, typename... TArgsCall>
			inline void invoke(TResult(*fn)(TArgsFn...), TArgsCall... args);


		private: // methods

			// the main thread function, containing the message loop
			void threadFunction(WindowConfig cfg);

			// Re-generate the dwStyle value based on the current state of the Window class
			// and update the Client-To-Window size values as well as the system's size minimum
			DWORD refreshStyle();

			// reset the member variables
			void clear();


		private: // variables

			std::thread m_trdMessageLoop;
			std::thread::id m_trdidApplication;

			const std::wstring m_sClassName;

			std::mutex m_muxState;
			std::condition_variable m_cvState; // on create() and destroy()

			HWND m_hWnd = NULL;
			bool m_bMessageLoop = false; // does the message loop currently run?
			bool m_bThreadRunning = false; // is the message thread currently running?
			std::atomic_bool m_bCloseRequested = false;

			MessageCallback m_fnOnMessage = nullptr;
			VoidCallback m_fnOnMinimize = nullptr;
			VoidCallback m_fnOnRestore = nullptr;



			// window size data
			unsigned m_iWidth = 0, m_iHeight = 0; // current client size
			unsigned m_iBorderWidth = 0, m_iBorderHeight = 0;
			unsigned m_iOSMinWidth = 0, m_iOSMinHeight = 0;
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





		/// <summary>
		/// The startup configuration for a <c>IRenderer</c> derived object
		/// </summary>
		struct RendererConfig
		{
			bool bVSync = false;
		};


		/// <summary>
		/// An interface for an OpenGL frame renderer
		/// </summary>
		class IRenderer
		{
		protected: // events

			virtual void OnCreate() {}
			virtual void OnUpdate(const void* pGraph) = 0;
			virtual void OnDestroy() {}

			virtual void OnResize() {}


		public: // methods

			IRenderer() = default;
			IRenderer(const IRenderer& other) = delete;
			IRenderer(IRenderer&& rval) = delete;
			virtual ~IRenderer();


			/// <summary>Initialize the renderer</summary>
			/// <returns>Did the renderer creation succeed?</returns>
			bool create(HDC hDC, unsigned iWidth, unsigned iHeight, const RendererConfig& cfg);

			/// <summary>Destroy the renderer</summary>
			void destroy();

			/// <summary>Wait for the thread to finish drawing the last frame</summary>
			void waitForFinishedFrame();

			/// <summary>Re-draw the graphics</summary>
			void update(const void* pGraph);

			/// <summary>Resize the viewport</summary>
			void resize(unsigned iWidth, unsigned iHeight);

			void setVSync(bool bVSync);
			bool getVSync() { return m_bVSync; }


		protected: // methods

			auto width() { return m_iWidth; }
			auto height() { return m_iHeight; }


		private: // methods

			void threadFunc(HDC hDC, unsigned iWidth, unsigned iHeight, const RendererConfig& cfg);


		private: // variables

			unsigned m_iWidth = 0, m_iHeight = 0;
			unsigned m_iNewWidth = 0, m_iNewHeight = 0;
			std::thread m_trdRenderer;
			std::mutex m_mux;
			std::condition_variable m_cv;
			bool m_bRunning = false;
			bool m_bWorking = false;

			HDC m_hDC = NULL;
			HGLRC m_hGLRC = NULL;

			// both booleans are always exactly either 1 or 0 for faster comparisons
			bool m_bVSync = false;
			bool m_bNewVSync = false;

			const void* m_pGraph = nullptr;


		};







		/// <summary>
		/// The startup configuration for a <c>IApplication</c> derived object
		/// </summary>
		struct AppConfig
		{
			WindowConfig window;
			RendererConfig renderer;
		};


		/// <summary>
		/// An interface for the logic behind an OpenGL-based application
		/// </summary>
		class IApplication
		{
		private: // types

			struct WindowMessage
			{
				UINT uMsg;
				WPARAM wParam;
				LPARAM lParam;
			};


		private: // static variables

			static bool s_bRunning; // only one object can run at once


		protected: // events

			virtual bool OnStart() { return true; }
			virtual bool OnUpdate(float fElapsedTime) = 0;
			virtual void OnStop() { }

			virtual bool OnUserCloseQuery() { return true; }

			virtual void OnResizing(unsigned& iWidth, unsigned& iHeight) {}
			virtual void OnResized(unsigned iWidth, unsigned iHeight) {}

			virtual void OnMinize() {}
			virtual void OnRestore() {}
			virtual void OnGainFocus() {}
			virtual void OnLoseFocus() {}


			// events for working with graph classes

			virtual void createGraph(void** pGraph) = 0;
			virtual void copyGraph(void* pDest, const void* pSource) = 0;
			virtual void destroyGraph(void* pGraph) = 0;


		public: // methods

			IApplication(Window& oWindow, IRenderer& oRenderer);
			IApplication(const IApplication& other) = delete;
			IApplication(IApplication&& rval) = delete;
			virtual ~IApplication() = default;

			bool execute(AppConfig& cfg);


		protected: // methods

			unsigned getWidth() { return m_oWindow.width(); }
			unsigned getHeight() { return m_oWindow.height(); }

			auto& window() { return m_oWindow; }
			auto& renderer() { return m_oRenderer; }
			auto graph() { return m_pLiveGraph; }


		private: // methods

			// copy the current frame graph to the cache graph (for while the renderer is working)
			void cacheGraph() { copyGraph(m_pGraphForRenderer, m_pLiveGraph); }

			/// <summary>
			/// [Use by <c>Window</c>]
			/// Inform the application thread that the window has been minimized.
			/// </summary>
			void winMinimized();
			/// <summary>
			/// [Use by <c>Window</c>]
			/// Inform the application thread that the window has been restored from minimization.
			/// </summary>
			void winRestored();

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

			/// <summary>
			/// Submit a new graph to the renderer
			/// </summary>
			void updateRenderer();


		private: // variables

			Window& m_oWindow;
			IRenderer& m_oRenderer;
			std::atomic_bool m_bAtomRunning = false;

			std::mutex m_muxWindow;
			std::condition_variable m_cvWinMsg;
			std::condition_variable m_cvMinimized;
			bool m_bSleeping = false;
			bool m_bMessageByApp = false;

			const WindowMessage* m_pMessage = nullptr;

			void* m_pLiveGraph = nullptr;
			void* m_pGraphForRenderer = nullptr;

		};



	}
}





#endif // ROBINLE_LIB_OPENGL__CORE