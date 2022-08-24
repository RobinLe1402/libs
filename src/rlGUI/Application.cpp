#include "rl/lib/rlGUI/Application.hpp"


namespace rl
{
	namespace GUI
	{

		Application Application::s_oInstance;

		Application& ThisApp = Application::GetInstance();



		bool Application::addClass(const wchar_t* szClassName)
		{
			if (m_oClasses.find(szClassName) != m_oClasses.end())
				return false;

			m_oClasses.insert(szClassName);
			return true;
		}

		void Application::removeClass(const wchar_t* szClassName)
		{
			m_oClasses.erase(szClassName);
		}

		bool Application::hasClass(const wchar_t* szClassName) const
		{
			return m_oClasses.find(szClassName) != m_oClasses.end();
		}

		void Application::run()
		{
			MSG msg{};
			while (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Application::Application() :
			m_hInstance(GetModuleHandle(NULL)), m_hCurArrow(LoadCursor(NULL, IDC_ARROW)) {}

		Application::~Application() { }

	}
}
