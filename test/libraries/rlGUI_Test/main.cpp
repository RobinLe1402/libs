#include <Windows.h>

#include "rl/lib/rlGUI/Core.hpp"
#include "rl/lib/rlGUI/CommonControls.hpp"

namespace lib = rl::GLGUI;



class ApplicationImpl : public lib::IGUIApplication
{
public: // methods

	// inherited constructors
	using lib::IGUIApplication::IGUIApplication;
	virtual ~ApplicationImpl() = default;

	bool OnCreate() override
	{
		m_oControls.push_back(new lib::Panel(true, 0, 0, 250, 25, GL::Pixel::ByRGB(0xFF0000)));
		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		lib::IGUIApplication::OnUpdate(fElapsedTime);
		return true;
	}

	bool OnDestroy() override
	{
		return true;
	}

};


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	lib::GUIWindow oWindow;
	lib::GUIRenderer oRenderer;
	ApplicationImpl oApplication(oWindow, oRenderer);

	rl::GLGUI::GUIConfig cfg;
	cfg.sTitle = L"Hello World!";
	oApplication.execute(cfg);

	return 0;
}

