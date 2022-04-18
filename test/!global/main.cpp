#include "resource.h"

#include "rl/splashscreen.hpp"
#include "rl/graphics.opengl.window.hpp"
#include "rl/graphics.opengl.texture.hpp"
#include "rl/input.keyboard.hpp"
#include "rl/input.mouse.hpp"
#include "rl/visualstyles.h"

#include <string>
#include <Windows.h>
#include <gdiplus.h>
#include <gl/GL.h>



rl::OpenGLTexture tex1;

class GLTest : public rl::OpenGLWin
{
private:

	rl::Keyboard& kb = rl::Keyboard::getInstance();
	rl::Mouse& mouse = rl::Mouse::getInstance();

	bool OnCreate() override
	{
		mouse.setHWND(getHWND());

		glEnable(GL_TEXTURE_2D);

		HANDLE hFile = CreateFileW(LR"(logo.png)", GENERIC_READ, 0, NULL,
			OPEN_EXISTING, NULL, NULL);
		if (hFile == NULL)
		{
			MessageBoxA(NULL, "Error loading file", NULL, MB_ICONERROR);
			exit(1);
		}

		LARGE_INTEGER liFilesize;
		if (!GetFileSizeEx(hFile, &liFilesize))
			MessageBoxA(NULL, "Error loading file", NULL, MB_ICONERROR);
		uint8_t* pData = new uint8_t[liFilesize.LowPart];
		if (!ReadFile(hFile, pData, liFilesize.LowPart, NULL, NULL))
			MessageBoxA(NULL, "Error loading file", NULL, MB_ICONERROR);

		rl::OpenGLTexture_Config texcfg;
		tex1.createFromImageData(pData, liFilesize.LowPart, texcfg);
		CloseHandle(hFile);
		delete[] pData;

		tex1.upload();

		//Sleep(5000); // emulate resource loading
		rl::SplashScreen::Close();
		return true;
	}

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return kb.update(uMsg, wParam, lParam) || mouse.update(uMsg, wParam, lParam);
	}

	void OnGainFocus() override { kb.reset(); }

	void OnLoseFocus() override { }

	bool OnUpdate(float fElapsedTime) override
	{
		kb.processInput();
		mouse.processInput();

		if (kb.getKey('A').held())
			MessageBoxA(NULL, "Test", NULL, MB_ICONINFORMATION | MB_SYSTEMMODAL);

		auto& mousestate = mouse.getState();
		if (mousestate.left.bReleased)
		{
			MessageBoxA(getHWND(), ("Clicked at (" + std::to_string(mousestate.x) + "|" + std::to_string(mousestate.y) + ")").c_str(), NULL, MB_ICONINFORMATION);
		}
		if (mousestate.iWheelRotation != 0)
		{
			MessageBoxA(getHWND(), ("Mouse wheel rotation: " + std::to_string(mousestate.iWheelRotation)).c_str(), NULL, MB_ICONINFORMATION);
		}

		glClearColor(0xFF, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		rl::OpenGLCoord coordBR = getPixelCoord(tex1.getWidth(), tex1.getHeight());
		rl::OpenGLRect rect = { -1, 1, coordBR.x, coordBR.y };
		tex1.drawToScreen(rect);

		/*rl::OpenGLCoord coordTMP = getPixelCoord(tex2.getWidth(), tex2.getHeight());
		coordTL = { coordBR.x, 1 };
		coordBR = { coordTL.x + coordTMP.x + 1, coordTMP.y };
		tex2.draw(coordTL.x, coordTL.y, coordBR.x, coordBR.y);*/

		return true;
	}

	bool OnDestroy() override
	{
		tex1.destroy();

		return true;
	}

	void OnAbout() override
	{
		if (kb.getKey(VK_SHIFT).held())
			MessageBoxA(getHWND(), "Secret message :P", "Secret", NULL);
		else
			rl::OpenGLWin::OnAbout();
	}

};


#include <fstream>

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	rl::SplashScreen_Config splashconfig(IDB_SPLASH);
	splashconfig.bDropShadow = true;
	splashconfig.bAlwaysOnTop = true;

	rl::SplashScreen::Show(splashconfig);


	rl::OpenGLWin_Config openglconfig;
	//config.bInitialFullscreen = true;
	//openglconfig.hIconBig = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));
	openglconfig.szInitialCaption = L"General unit test application";

	openglconfig.bResizable = true;

	GLTest gltest;
	gltest.run(openglconfig);


	return 0;
}
