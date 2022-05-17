#include "rl/lib/rlOpenGL/Core.hpp"

namespace lib = rl::OpenGL;

#include <functional>
#include <gl/GL.h>

#pragma comment(lib, "OpenGL32.lib")



typedef BOOL(WINAPI wglSwapInterval_t)(int interval);
wglSwapInterval_t* wglSwapInterval = nullptr;



lib::IRenderer::~IRenderer()
{
	destroy();
}

bool lib::IRenderer::create(HDC hDC, unsigned iWidth, unsigned iHeight, const RendererConfig& cfg)
{
	destroy();

	std::unique_lock lm(m_mux);
	m_trdRenderer = std::thread(&IRenderer::threadFunc, this, hDC, iWidth, iHeight, cfg);
	m_cv.wait(lm); // wait until thread updates m_bRunning

	return m_bRunning;
}

void lib::IRenderer::destroy()
{
	if (!m_bRunning)
		return;

	m_mux.lock(); // wait for thread to finish rendering the current graph
	m_mux.unlock();
	m_bRunning = false;
	m_cv.notify_one();

	if (m_trdRenderer.joinable())
		m_trdRenderer.join();
}

bool bWorking = false;

void lib::IRenderer::waitForFinishedFrame()
{
	if (!m_bRunning)
		return;

	std::unique_lock lm(m_mux);
}

size_t iCountUpdate = 0;
size_t iCountRedraw = 0;

void lib::IRenderer::update(const void* pGraph)
{
	if (!m_bRunning)
		return;

	++iCountUpdate;

	std::unique_lock lm(m_mux); // wait for thread to finish drawing current frame
	m_bWorking = false;
	m_pGraph = pGraph;
	lm.unlock();
	m_cv.notify_one(); // continue thread
	while (!m_bWorking) {}
}

void lib::IRenderer::resize(unsigned iWidth, unsigned iHeight)
{
	if (!m_bRunning)
		return;

	std::unique_lock lm(m_mux);
	m_iNewWidth = iWidth;
	m_iNewHeight = iHeight;
}

void lib::IRenderer::setVSync(bool bVSync)
{
	if (!m_bRunning)
		return;

	std::unique_lock lm(m_mux);
	m_bNewVSync = bVSync ? 1 : 0;
}

void lib::IRenderer::threadFunc(HDC hDC, unsigned iWidth, unsigned iHeight,
	const RendererConfig& cfg)
{
	// try to initialize OpenGL
	// on failure --> return instantly, with m_bRunning still set to false
	try
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR), 1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			PFD_MAIN_PLANE, 0, 0, 0, 0
		};
		int pf = ChoosePixelFormat(hDC, &pfd);
		if (!SetPixelFormat(hDC, pf, &pfd))
			throw 0;

		m_hGLRC = wglCreateContext(hDC);
		if (m_hGLRC == NULL)
			throw 0;
		if (!wglMakeCurrent(hDC, m_hGLRC))
		{
			wglDeleteContext(m_hGLRC);
			m_hGLRC = 0;
			throw 0;
		}
	}
	catch (...)
	{
		m_cv.notify_one(); // continue application thread in create()
		return;
	}



	m_iWidth = m_iNewWidth = iWidth;
	m_iHeight = m_iNewHeight = iHeight;
	glViewport(0, 0, m_iWidth, m_iHeight);
	m_hDC = hDC;

	wglSwapInterval = (wglSwapInterval_t*)wglGetProcAddress("wglSwapIntervalEXT");

	m_bVSync = cfg.bVSync ? 1 : 0;
	wglSwapInterval(m_bVSync);
	m_bNewVSync = m_bVSync;


	OnCreate();
	std::unique_lock lm(m_mux);

	m_bRunning = true;
	m_cv.notify_one(); // continue application thread in create()

	while (true)
	{
		++iCountRedraw;
		m_cv.wait(lm); // wait for new graph
		if (!m_bRunning)
			break; // destruction was requested
		m_bWorking = true;
		m_bNewVSync = m_bNewVSync ? 1 : 0;
		if (m_bNewVSync != m_bVSync)
		{
			m_bVSync = m_bNewVSync;
			wglSwapInterval(m_bVSync);
		}
		if (m_iNewWidth != m_iWidth || m_iNewHeight != m_iHeight)
		{
			m_iWidth = m_iNewWidth;
			m_iHeight = m_iNewHeight;
			glViewport(0, 0, m_iWidth, m_iHeight);
			OnResize();
		}

		OnUpdate(m_pGraph);
		SwapBuffers(m_hDC); // refresh display
	}
	OnDestroy();

	wglDeleteContext(m_hGLRC);
	m_hGLRC = NULL;
	m_hDC = NULL;
	m_bRunning = false;
}
