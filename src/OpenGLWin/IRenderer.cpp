#include "rl/lib/OpenGLWin/OpenGLWin.hpp"

namespace lib = rl::OpenGLWin;

#include <dwmapi.h>
#include <functional>
#include <gl/GL.h>

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "OpenGL32.lib")



typedef BOOL(WINAPI wglSwapInterval_t)(int interval);
wglSwapInterval_t* wglSwapInterval = nullptr;

lib::IRenderer::IRenderer(IApplication& oApplication) :
	m_oApplication(oApplication) { }

lib::IRenderer::~IRenderer()
{
	destroy();
}

bool lib::IRenderer::create(HDC hDC, unsigned iWidth, unsigned iHeight, bool bVSync)
{
	destroy();

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	int pf = ChoosePixelFormat(hDC, &pfd);
	if (!SetPixelFormat(hDC, pf, &pfd))
		return false;

	m_hGLRC = wglCreateContext(hDC);
	if (m_hGLRC == NULL)
		return false;
	if (!wglMakeCurrent(hDC, m_hGLRC))
	{
		wglDeleteContext(m_hGLRC);
		m_hGLRC = 0;
		return false;
	}

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	glViewport(0, 0, m_iWidth, m_iHeight);
	m_hDC = hDC;

	wglSwapInterval = (wglSwapInterval_t*)wglGetProcAddress("wglSwapIntervalEXT");

	m_bVSync = !bVSync;
	setVSync(bVSync);

	
	return true;
}

void lib::IRenderer::destroy()
{
	if (m_hGLRC == NULL)
		return;

	OnDestroy();

	wglDeleteContext(m_hGLRC);
	m_hGLRC = NULL;
	m_hDC = NULL;
}

void lib::IRenderer::update()
{
	if (m_hGLRC == NULL)
		return;

	OnUpdate();
	SwapBuffers(m_hDC); // refresh display
	if (m_bVSync)
		DwmFlush(); // wait for full redraw
}

void lib::IRenderer::resize(unsigned iWidth, unsigned iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	glViewport(0, 0, iWidth, iHeight);
}

void lib::IRenderer::setVSync(bool bVSync)
{
	if (m_hGLRC == NULL || (bVSync && m_bVSync) || (!bVSync && !m_bVSync))
		return;

	if (bVSync)
		wglSwapInterval(1);
	else
		wglSwapInterval(0);
}
