#include "resource.h"

#include "rl/graphics.opengl.gui.hpp"
#include "rl/graphics.opengl.texture.hpp"
#include "rl/tools.gdiplus.hpp"

#include <Windows.h>
#include <gl/GL.h>



#include "rl/graphics.opengl.window.hpp"
class UI : public rl::OpenGLWin
{
private: // variables

	rl::OpenGLTexture tex;

private: // methods

	bool OnCreate() override
	{
		glEnable(GL_TEXTURE_2D);

		rl::GDIPlus gdip;
		Gdiplus::Bitmap* pBMP =
			Gdiplus::Bitmap::FromFile(LR"(E:\Bilder\Anderes\736038-cc7cfb94a7d1c64a5172c72eec94668c.jpg)");
		rl::OpenGLTexture_Config cfg;
		tex.createFromGDIPlusBitmap(pBMP, cfg);
		delete pBMP;
		tex.upload();

		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		tex.drawToScreen(rl::OpenGL::FullScreen);

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

	UI ui;

	rl::OpenGLWin_Config cfg;
	cfg.szInitialCaption = L"FontFace GUI";
	//cfg.hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROBINLE));
	cfg.bResizable = true;

	ui.run(cfg);

	return 0;
}
