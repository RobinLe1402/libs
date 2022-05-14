#include <Windows.h>

#include "rl/lib/rlOpenGL/Core.hpp"
#include "rl/lib/rlOpenGL/Texture.hpp"
#include "rl/visualstyles.h"
#include <gl/GL.h>
#include <format>

#include "resource.h"

#pragma comment(lib, "rlOpenGL.lib")

using rl::OpenGL::Pixel;
namespace Color = rl::OpenGL::Color;

constexpr auto pxFuchsia = Pixel::ByRGB(0xFF00FF);



struct GameGraph
{
	bool bPixelWhite;
};

class GameRenderer : public rl::OpenGL::IRenderer
{
public: // methods

	using rl::OpenGL::IRenderer::IRenderer;


private: // methods

	static constexpr uint8_t s_iTexChess[4 * 4] =
	{
		0x00, 0x00, 0x00, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,		0x00, 0x00, 0x00, 0xFF
	};

	void OnCreate() override
	{
		glEnable(GL_TEXTURE_2D);

		/*m_oTex.create(2, 2);

		m_oTex.setPixel(0, 0, pxFuchsia);
		m_oTex.setPixel(1, 0, rl::OpenGL::Color::Black);
		m_oTex.setPixel(0, 1, rl::OpenGL::Color::Black);
		m_oTex.setPixel(1, 1, pxFuchsia);*/

		m_oTex.create(6, 6);

		using namespace rl::OpenGL::Color;

		/*
		+------+
		|      |
		| X XX |
		|      |
		| X  X |
		|  XX  |
		|      |
		+------+
		*/

		m_oTex.setPixel(0, 0, White);
		m_oTex.setPixel(1, 0, White);
		m_oTex.setPixel(2, 0, White);
		m_oTex.setPixel(3, 0, White);
		m_oTex.setPixel(4, 0, White);
		m_oTex.setPixel(5, 0, White);

		m_oTex.setPixel(0, 1, White);
		m_oTex.setPixel(1, 1, Black);
		m_oTex.setPixel(2, 1, White);
		m_oTex.setPixel(3, 1, Black);
		m_oTex.setPixel(4, 1, Black);
		m_oTex.setPixel(5, 1, White);

		m_oTex.setPixel(0, 2, White);
		m_oTex.setPixel(1, 2, White);
		m_oTex.setPixel(2, 2, White);
		m_oTex.setPixel(3, 2, White);
		m_oTex.setPixel(4, 2, White);
		m_oTex.setPixel(5, 2, White);

		m_oTex.setPixel(0, 3, White);
		m_oTex.setPixel(1, 3, Black);
		m_oTex.setPixel(2, 3, White);
		m_oTex.setPixel(3, 3, White);
		m_oTex.setPixel(4, 3, Black);
		m_oTex.setPixel(5, 3, White);

		m_oTex.setPixel(0, 4, White);
		m_oTex.setPixel(1, 4, White);
		m_oTex.setPixel(2, 4, Black);
		m_oTex.setPixel(3, 4, Black);
		m_oTex.setPixel(4, 4, White);
		m_oTex.setPixel(5, 4, White);

		m_oTex.setPixel(0, 5, White);
		m_oTex.setPixel(1, 5, White);
		m_oTex.setPixel(2, 5, White);
		m_oTex.setPixel(3, 5, White);
		m_oTex.setPixel(4, 5, White);
		m_oTex.setPixel(5, 5, White);



		m_oTex.setUpscalingMethod(rl::OpenGL::TextureScalingMethod::NearestNeighbor);
		m_oTex.setTransparency(false);
		//m_oTex.setWrapMethod(rl::OpenGL::TextureWrapMethod::Clamp);
		m_oTex.upload();
	}

	rl::OpenGL::Texture m_oTex;

	void OnUpdate(const void* pGraph) override
	{
		const GameGraph& oGraph = *reinterpret_cast<const GameGraph*>(pGraph);


		//m_oTex.draw({ rl::OpenGL::FullScreen, rl::OpenGL::FullTexture });
		/*const auto pos1 = m_oTex.coordsUnscaled({ 0, 0, (int)width(), (int)height() },
			0, 0, { width(), height() });*/

		if (oGraph.bPixelWhite)
			m_oTex.setPixel(3, 1, Color::White);
		else
			m_oTex.setPixel(3, 1, Color::Black);
		m_oTex.upload();

		const auto pos2 = m_oTex.coordsScaled({ 0, 0, (int)width() / 8, (int)height() / 16 },
			{ 0, 0, (int)width(), (int)height() }, { width(), height() });
		m_oTex.draw(pos2);

	}

	/*void OnDestroy() override
	{

	}*/
};

class Game : public rl::OpenGL::IApplication
{
public: // methods

	using rl::OpenGL::IApplication::IApplication;


private: // methods

	bool OnUpdate(float fElapsedTime) override
	{
		GameGraph& oGraph = *reinterpret_cast<GameGraph*>(graph());

		oGraph.bPixelWhite = !oGraph.bPixelWhite;
		return true;
	}

	void OnResizing(unsigned& iWidth, unsigned& iHeight) override
	{
		iWidth -= iWidth % 8;
		iHeight -= iHeight % 16;
	}

	char szSize[20] = {};

	void OnResized(unsigned iWidth, unsigned iHeight) override
	{
		szSize[0] = 0;
		_itoa_s(iWidth, szSize, 10);
		strcat_s(szSize, "x");
		_itoa_s(iHeight, szSize + strlen(szSize), 20 - strlen(szSize), 10);
		window().setTitle(szSize);
	}

	void createGraph(void** pGraph) override { *pGraph = new GameGraph; }

	void copyGraph(void* pDest, const void* pSource) override
	{
		memcpy_s(pDest, sizeof(GameGraph), pSource, sizeof(GameGraph));
	}

	void destroyGraph(void* pGraph) override { delete pGraph; }

};

class GameWindow : public rl::OpenGL::Window
{
public: // methods

	using rl::OpenGL::Window::Window; // use same constructors


private: // methods

	static const UINT_PTR s_iMenuAbout = 0;

	void OnCreate() override
	{
		HMENU hMenu = GetSystemMenu(handle(), FALSE);
		InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 255, NULL);
		InsertMenuW(hMenu, 0, MF_BYPOSITION, s_iMenuAbout, L"About");
	}

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_RBUTTONDOWN:
			setFullscreen(!fullscreen());
			break;

		case WM_MBUTTONDOWN:
		{
			std::string sStatus = std::format(
				"Size: {{ {}, {} }}\n"
				"Minimized: {}\n"
				"Maximized: {}\n",
				width(), height(), minimized(), maximized());

			OutputDebugStringA(sStatus.c_str());

			break;
		}

		case WM_SYSCOMMAND:
			if (wParam == s_iMenuAbout)
				MessageBoxA(handle(), "ABOUT!!!", "About", MB_ICONINFORMATION | MB_APPLMODAL);
			break;
		}


		return false;
	}
};


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)szCmdLine;
	(void)iCmdShow;

	GameWindow oWindow(L"OpenGLWin_Test");
	GameRenderer oRenderer;
	Game oGame(oWindow, oRenderer);

	rl::OpenGL::AppConfig cfg;
	cfg.renderer.bVSync = true;
	cfg.window.bResizable = true;
	cfg.window.iMinWidth = 256;
	cfg.window.iMinHeight = 240;
	cfg.window.sTitle = L"OpenGLWin Test";
	cfg.window.hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));
	cfg.window.hIconSmall = cfg.window.hIconBig;
	cfg.window.iWidth = 512;
	cfg.window.iHeight = 480;
	//cfg.window.bFullscreen = true;

	if (MessageBoxA(NULL,
		"The following window is a test for the RobinLe OpenGLWin library.\n"
		"\n"
		"CONTROLS\n"
		"  RMB = Toggle fullscreen\n"
		"  MMB = Write info text to debug text stream\n",
		"OpenGLWin_Test", MB_OKCANCEL | MB_ICONINFORMATION | MB_SYSTEMMODAL) != IDOK)
		return 0; // user cancelled test
	oGame.execute(cfg);

	return 0;
}
