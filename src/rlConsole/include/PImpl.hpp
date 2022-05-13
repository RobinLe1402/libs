#pragma once
#ifndef ROBINLE_CONSOLE_PIMPL
#define ROBINLE_CONSOLE_PIMPL





#include "rl/dll/rlConsole/Core.h"
#include "FontWrapper.hpp"
#include "rl/lib/rlOpenGL/Core.hpp"
#include <functional>

namespace GL = rl::OpenGL;

#pragma comment(lib, "rlOpenGL.lib")



class ConsoleWindow;
class ConsoleRenderer;
class ConsoleApplication;

namespace impl
{
	bool ConfigIsValid(const rlConsole_StartupConfig& config);



	class rlConsole
	{
	public: // methods

		rlConsole(const rlConsole_StartupConfig& config);
		~rlConsole();

		bool execute();

		const auto& getFont() { return m_oFont; }

		auto getWindow() { return m_pWindow; }
		auto getRenderer() { return m_pRenderer; }
		auto getApplication() { return m_pApplication; }

		const std::function<bool(rlConsole_UINT uMsg, rlConsole_WPARAM wParam,
			rlConsole_LPARAM lParam)> OnMessage;
		const std::function<bool(float fElapsedTime, HrlConsole hConsole)> OnUpdate;



		bool isRunning();

		auto getScale() const { return m_iScale; }


		auto getColumns() const { return m_iColumns; }
		void setColumns(unsigned iColumns) { m_iColumns = iColumns; }

		auto getRows() const { return m_iRows; }
		void setRows(unsigned iRows) { m_iRows = iRows; }

		auto getForeground() const { return m_clForeground; }
		auto getBackground() const { return m_clBackground; }

		rlConsole_CharBuffer getBuffer();


	private: // variables

		bool m_bRunning = false;

		ConsoleWindow* m_pWindow;
		ConsoleRenderer* m_pRenderer;
		ConsoleApplication* m_pApplication;

		GL::AppConfig m_oConfig = {};

		FontWrapper m_oFont;

		unsigned m_iScale;
		unsigned m_iColumns;
		unsigned m_iRows;

		rlConsole_Color m_clForeground;
		rlConsole_Color m_clBackground;

	};
}





#endif ROBINLE_CONSOLE_PIMPL
