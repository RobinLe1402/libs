#pragma once
#ifndef ROBINLE_LIB_GUI_APPLICATION
#define ROBINLE_LIB_GUI_APPLICATION





#include <string>
#include <set>
#include <Windows.h>



namespace rl
{
	namespace GUI
	{

		class Application final
		{
		public: // static methods

			static Application& GetInstance() { return s_oInstance; }


		public: // methods

			bool addClass(const wchar_t* szClassName);
			void removeClass(const wchar_t* szClassName);

			bool hasClass(const wchar_t* szClassName) const;

			auto getHandle() const { return m_hInstance; }

			auto getDefaultCursor() const { return m_hCurArrow; }

			void run();


		private: // methods

			Application(); // --> singleton
			~Application();


		private: // variables

			const HINSTANCE m_hInstance;
			const HCURSOR m_hCurArrow;

			std::set<std::wstring> m_oClasses;


		private: // static variables

			static Application s_oInstance;

		};

		extern Application& ThisApp;

	}
}





#endif // ROBINLE_LIB_GUI_APPLICATION