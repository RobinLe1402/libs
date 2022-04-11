#include <Windows.h>

#include "rl/lib/OpenGLWin.hpp"
#include <gl/GL.h>

#include "resource.h"

#pragma comment(lib, "OpenGLWin.lib")

class GameRenderer : public rl::OpenGLWin::IRenderer
{
public: // methods

	using rl::OpenGLWin::IRenderer::IRenderer;


private: // methods

	static constexpr uint8_t s_iTexChess[4 * 4] =
	{
		0x00, 0x00, 0x00, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,		0x00, 0x00, 0x00, 0xFF
	};

	GLuint m_iTexID = 0;

	void OnCreate() override
	{
		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &m_iTexID);
		glBindTexture(GL_TEXTURE_2D, m_iTexID);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_iTexChess);
	}

	void OnUpdate() override
	{
		const float fWidth = width() * 0.5f;
		const float fHeight = height() * 0.5f;

		glBindTexture(GL_TEXTURE_2D, m_iTexID);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 1.0);		glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0, 0.0);			glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0, 0.0);		glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0, 1.0);	glVertex3f(1.0f, -1.0f, 0.0f);
		}
		glEnd();
	}

	void OnDestroy() override
	{
		glDeleteTextures(1, &m_iTexID);
	}
};

class Game : public rl::OpenGLWin::IApplication
{
public: // methods

	using rl::OpenGLWin::IApplication::IApplication;


private: // methods

	bool OnUpdate(float fElapsedTime) override { return true; }

	/*void OnResize(LONG& iWidth, LONG& iHeight) override
	{
		iWidth -= iWidth % 8;
		iHeight -= iHeight % 8;
	}*/

};


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	Game oGame;
	rl::OpenGLWin::Window oWindow(oGame);
	GameRenderer oRenderer(oGame);

	rl::OpenGLWin::OpenGLWinConfig cfg;
	cfg.renderer.bVSync = true;
	cfg.window.bResizable = true;
	cfg.window.iMinWidth = 256;
	cfg.window.iMinHeight = 240;
	cfg.window.iMaxWidth = 500;
	cfg.window.iMaxHeight = 480;
	cfg.window.sTitle = L"OpenGLWin Test";
	cfg.window.hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));
	cfg.window.hIconSmall = cfg.window.hIconBig;
	cfg.window.bFullscreen = false;

	oGame.execute(cfg, oWindow, oRenderer);

	return 0;
}
