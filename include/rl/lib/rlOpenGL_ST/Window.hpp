#pragma once
#ifndef ROBINLE_LIB_OPENGL_ST__WINDOW
#define ROBINLE_LIB_OPENGL_ST__WINDOW





//==================================================================================================
// INCLUDES

#include <string>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace OpenGL_ST
	{

		struct WindowStartupConfig
		{
			bool bVisible = false;
			int iWidth = 800;
			int iHeight = 500;
		};



		class Window
		{
		public: // methods

			Window(const wchar_t* szClassName, const WindowStartupConfig& config);
			virtual ~Window();

			void show();
			void hide();
			void close();
			auto getHandle() const { return m_hWnd; }
			auto getAtom() const { return m_iAtom; }


		private: // variables

			WNDCLASSW m_oWC{};
			ATOM m_iAtom{};
			HWND m_hWnd;
			int m_iWidth;
			int m_iHeight;


		private: // constants

			const std::wstring m_sClassName;
		};

	}
}





#endif // ROBINLE_LIB_OPENGL_ST__WINDOW