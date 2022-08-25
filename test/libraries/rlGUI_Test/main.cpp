#include <Windows.h>

#include "rl/lib/rlGUI/Application.hpp"
#include "rl/lib/rlGUI/Window.hpp"
#include "rl/lib/rlGUI/Texture.hpp"

#include <cwctype> // std::towupper

#pragma comment(lib, "rlGUI.lib")


using namespace rl::GUI;

class Panel : public IControl
{
public: // methods

	// use inherited constructors
	using IControl::IControl;

	void addChild(IControl& oControl)
	{
		m_oChildControls.push_back(&oControl);
	}

};


class CustomWin : public Window
{
public: // methods

	using Window::Window; // use parent constructors and destructors


protected: // methods

	void onTryResize(unsigned& iWidth, unsigned& iHeight, bool& bAccept) override
	{
		iWidth -= iWidth % 8;
		iHeight -= iHeight % 8;
	}

	void onCreate() override
	{
		m_oTex.setPixel(0, 0, Colors::Black);
		m_oTex.setPixel(0, 1, Colors::Black);
		m_oTex.setPixel(0, 2, Colors::Black);
		m_oTex.setPixel(0, 3, Colors::Black);
		m_oTex.setPixel(1, 3, Colors::Black);
		m_oTex.setPixel(2, 3, Colors::Black);
		m_oTex.setPixel(3, 3, Colors::Black);
		m_oTex.setPixel(1, 2, Colors::Black);
		m_oTex.setPixel(2, 1, Colors::Black);
		m_oTex.setPixel(3, 0, Colors::Black);
		m_oTex.upload();
	}

	bool onMessage(LRESULT& result, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		{
			Window& win = *getChildWindows()[0];
			if (win.isVisible())
				win.hide();
			else
				win.show();

			break;
		}

		default:
			return false;
		}

		return true;
	}

	void onPaint() override
	{
		m_oTex.draw(1, 1);
		m_oTex.drawStretched(10, 10, 100, 100);
		m_oTex.draw(getWidth() - 1 - m_oTex.getWidth(), getHeight() - 1 - m_oTex.getHeight());
	}

	void onFileDrag(int32_t iX, int32_t iY, DropEffect& eEffect,
		DWORD grfKeyState) override
	{
		if (iX < 0 || iY < 0 || iX > 10 || iY > 10)
			return;

		auto& oFilenames = getDragDropFilenames();
		if (oFilenames.size() != 1)
			return;

		const auto& s = oFilenames[0];
		const auto idx = s.find_last_of(L'.');
		if (idx == std::wstring::npos)
			return;

		auto sv = std::wstring_view(s.c_str() + idx + 1);
		std::wstring sUpper;
		sUpper.resize(sv.length());
		for (size_t i = 0; i < sv.length(); ++i)
		{
			sUpper[i] = std::towupper(sv[i]);
		}


		if (sUpper == L"DLL")
			eEffect = DropEffect::Move;
	}


private: // variables

	Texture m_oTex = Texture(4, 4, TextureScalingMethod::NearestNeighbor);
	Panel m_oPn = Panel(nullptr, this, 0, 5, 50, 50, Color::ByARGB(0x80FF00FF));
	//Panel m_oPn2 = Panel(nullptr, &m_oPn, 2, 2, 60, 60, Color::ByRGB(0x00FF00));

};



int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	(void)hPrevInstance;
	(void)szCmdLine;
	(void)iCmdShow;

	CustomWin wnd(L"MainWnd", nullptr, nullptr, 0, 0, 500, 250, Colors::White);
	Window wnd2(L"PopupWnd", nullptr, &wnd, 100, 100, 250, 250, Colors::Black);

	wnd2.setAcceptsFiles(true);

	//wnd.setTitleASCII("ASDF");
	wnd.setClosesApp(true);
	wnd.show();

	ThisApp.run();

	return 0;
}
