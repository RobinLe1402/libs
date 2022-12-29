#pragma once
#ifndef ROBINLE_LIB_OPENGL_ST__APPLICATION
#define ROBINLE_LIB_OPENGL_ST__APPLICATION





//==================================================================================================
// INCLUDES

#include <vector>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGL_ST
	{
		// forward declarations
		class Window;



		class Application
		{
		public: // static methods

			static Application& GetInstance() { return s_oInstance; }

			static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				return s_oInstance.processMessage(hWnd, uMsg, wParam, lParam);
			}


		private: // static variables

			static Application s_oInstance;


		public: // methods


			void run();
			void addWindow(Window& oWindow) { m_oWindows.push_back(&oWindow); }
			void processMessages();
			Window* getMainWindow();


		private: // methods

			// --> singleton
			Application() = default;
			~Application() = default;

			LRESULT processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


		private: // variables

			std::vector<Window*> m_oWindows;
		};

		extern Application& ThisApplication;

	}
}





#endif // ROBINLE_LIB_OPENGL_ST__APPLICATION