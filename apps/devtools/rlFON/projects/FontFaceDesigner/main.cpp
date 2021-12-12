#include <rl/global/robinle_mainicon.h>

#include <rl/graphics.opengl.window.hpp>
#include <rl/graphics.opengl.texture.hpp>
#include <rl/graphics.fonts.bitmap.reader.hpp>
#include <rl/input.mouse.hpp>
#include <rl/input.keyboard.hpp>
#include <rl/splashscreen.hpp>

#include <Windows.h>
#include <gdiplus.h>
#include <gl/GL.h>

#undef min
#undef max
#include <algorithm>
#include <string>
#include <functional>


#include "gui.hpp"
#include "types.hpp"
#include "constants.hpp"
#include "resource.h"




HRESULT GetGdiplusEncoderClsid(const std::wstring& format, GUID* pGuid)
{
	HRESULT hr = S_OK;
	UINT  nEncoders = 0;          // number of image encoders
	UINT  nSize = 0;              // size of the image encoder array in bytes
	std::vector<BYTE> spData;
	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
	Gdiplus::Status status;
	bool found = false;

	if (format.empty() || !pGuid)
	{
		hr = E_INVALIDARG;
	}

	if (SUCCEEDED(hr))
	{
		*pGuid = GUID_NULL;
		status = Gdiplus::GetImageEncodersSize(&nEncoders, &nSize);

		if ((status != Gdiplus::Ok) || (nSize == 0))
		{
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{

		spData.resize(nSize);
		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)&spData.front();
		status = Gdiplus::GetImageEncoders(nEncoders, nSize, pImageCodecInfo);

		if (status != Gdiplus::Ok)
		{
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		for (UINT j = 0; j < nEncoders && !found; j++)
		{
			if (pImageCodecInfo[j].MimeType == format)
			{
				*pGuid = pImageCodecInfo[j].Clsid;
				found = true;
			}
		}

		hr = found ? S_OK : E_FAIL;
	}

	return hr;
}





class GUI : public rl::OpenGLWin
{
private: // variables

	rl::Mouse& m_oMouse = rl::Mouse::getInstance();
	rl::Keyboard& m_oKeyboard = rl::Keyboard::getInstance();
	char32_t m_cFallback = 0;
	uint16_t m_iCharHeight = 0;

	gui::GUI* m_pGUI = nullptr;


protected: // event handlers

	bool OnCreate() override
	{
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_pGUI = new gui::GUI(getHWND());


		rl::SplashScreen::Close();
		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		m_oKeyboard.processInput();
		m_oMouse.processInput();
		const rl::MouseState& oMouseState = m_oMouse.getState();

		m_pGUI->processInput(m_oKeyboard, oMouseState);
		m_pGUI->drawGraphics();

		return true;
	}

	bool OnDestroy() override
	{
		delete m_pGUI;
		m_pGUI = nullptr;

		return true;
	}

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (m_oKeyboard.update(uMsg, wParam, lParam))
			return true;

		if (m_oMouse.update(uMsg, wParam, lParam))
			return true;

		return false;
	}
};



int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	rl::SplashScreen_Config splashcfg(IDB_SPLASH);
#ifndef _DEBUG
	splashcfg.bAlwaysOnTop = true;
#endif // _DEBUG

	splashcfg.bDropShadow = true;
	rl::SplashScreen::Show(splashcfg);


	GUI gui;
	const wchar_t szTitle[] = L"RobinLe Font Face Designer";

	rl::OpenGLWin_Config cfg;
	cfg.iWidth = gui::iWidth * gui::iScalingFactor;
	cfg.iHeight = gui::iHeight * gui::iScalingFactor;
	cfg.szInitialCaption = szTitle;
	cfg.hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));

	gui.run(cfg);

	return 0;
}
