#include "PrivateIncludes/Window.hpp"
#include <rl/dll/rlPixelWindow/Definitions.h>
#include <rl/dll/rlPixelWindow/Core.h>
#include "PrivateCore.hpp"
#include "DllResources.h"

#include <gl/GL.h>

namespace
{
	constexpr wchar_t szWNDCLASSNAME[] = L"rlPixelWindow";
}


LRESULT CALLBACK internal::Window::GlobalWindowProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (instance().m_hWnd == NULL)
		instance().m_hWnd = hWnd;

	return instance().localWindowProc(uMsg, wParam, lParam);
}

void internal::Window::create(PixelWindowProc fnCallback, const PixelWindowCreateParams *pParams)
{
	ResetError();
	if (m_bInitialized)
		destroy(); // destroy previous instance

	// check parameters
	if (!fnCallback ||
		pParams->iWidth == 0 || pParams->iHeight == 0 ||
		pParams->iPixelWidth == 0 || pParams->iPixelHeight == 0)
	{
		SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}



	// copy parameters
	m_fnCallback   = fnCallback;
	m_fnOSCallback = pParams->fnOSCallback;
	m_iWidth       = pParams->iWidth;
	m_iHeight      = pParams->iHeight;
	m_iPixelWidth  = pParams->iPixelWidth;
	m_iPixelHeight = pParams->iPixelHeight;
	m_oLayers.reserve((size_t)pParams->iExtraLayers + 1);
	m_bResizable   = (pParams->iFlags & PXWIN_CREATE_RESIZABLE) != 0;
	m_bMaximizable = (pParams->iFlags & PXWIN_CREATE_MAXIMIZABLE) != 0;
	m_pxBackground = pParams->pxBackground;
	m_pxBackground.alpha = 0xFF;
	m_iExtraLayers = pParams->iExtraLayers;


	// create actual window
	DWORD dwStyle   = WS_OVERLAPPEDWINDOW;
	if (!m_bMaximizable)
		dwStyle &= ~WS_MAXIMIZEBOX;
	if (!m_bResizable)
		dwStyle &= ~WS_SIZEBOX;
	DWORD dwExStyle = NULL;
	RECT rect
	{
		.left   = 0,
		.top    = 0,
		.right  = m_iWidth  * m_iPixelWidth,
		.bottom = m_iHeight * m_iPixelHeight
	};
	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
	m_hWnd = CreateWindowExW(dwExStyle, szWNDCLASSNAME, L"RobinLe PixelWindow", dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, NULL, (LPVOID)pParams->iUserData);

	if (m_hWnd == NULL)
	{
		SetError(PXWIN_ERROR_OSERROR);
		return; // todo: reset fields?
	}


	m_bInitialized = true;
}

void internal::Window::destroy()
{
	ResetError();
	if (!m_bInitialized)
		return;

	DestroyWindow(m_hWnd);
	m_oLayers.clear(); // !!!! don't call destroy() when layers are still used!

	m_bInitialized = false;
}

void internal::Window::run()
{
	ResetError();
	if (!m_bInitialized)
	{
		SetError(PXWIN_ERROR_NOINIT);
		return;
	}
	if (m_bRunning)
	{
		SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}

	m_bRunning = true;

	ShowWindow(m_hWnd, SW_SHOWNORMAL);


	// message loop
	MSG msg{};
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			do
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				if (msg.message == WM_QUIT)
					break; // do...while
			} while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE));
			if (msg.message == WM_QUIT)
				break; // while
		}

		update(PXWIN_UPDATEREASON_REGULAR);
	}

	m_bRunning     = false;
	m_bInitialized = false;
}

void internal::Window::update(uint8_t iReason)
{
	PixelWindowUpdateParams params{};
	params.fElapsedTime = getElapsedTime(true);
	params.iUpdateReason = iReason;

	if (!m_fnCallback(intfPtr(), PXWINMSG_UPDATE, (PixelWindowArg)&params, 0))
	{
		destroy();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	for (size_t i = 0; i < m_oLayers.size(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_oLayers[i].iTexID);

		if (m_oLayers[i].bChanged)
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGBA, GL_UNSIGNED_BYTE,
				m_oLayers[i].upData.get());

		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 1.0);	glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0, 0.0);	glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0, 0.0);	glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0, 1.0);	glVertex3f(1.0f, -1.0f, 0.0f);
		}
		glEnd();
	}

	SwapBuffers(GetDC(m_hWnd));

}

void internal::Window::draw(
	const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight,
	uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iFlags, uint8_t iAlphaMode)
{
	if (iLayer >= m_oLayers.size() || iX >= m_iWidth || iY >= m_iHeight)
		return;

	const bool bFlip = (iFlags & PXWIN_DRAW_VERTFLIP) != 0;

	switch (iAlphaMode)
	{
		// overwrite all pixels
	case PXWIN_DRAWALPHA_OVERRIDE:
		for (PixelWindowPos iRelX = 0, iAbsX = iX; iRelX < iWidth && iAbsX < m_iWidth;
			++iRelX, ++iAbsX)
		{
			for (PixelWindowPos iRelY = 0, iAbsY = iY; iRelY < iHeight && iAbsY < m_iHeight;
				++iRelY, ++iAbsY)
			{
				const auto px = pData[(bFlip ? iRelY : iHeight - 1 - iRelY) * iWidth + iRelX];

				m_oLayers[iLayer].upData[(size_t)iAbsY * m_iWidth + iAbsX] = px;
			}
		}
		break;


		// ignore pixels that are not fully opaque
	case PXWIN_DRAWALPHA_BINARY:
		for (PixelWindowPos iRelX = 0, iAbsX = iX; iRelX < iWidth && iAbsX < m_iWidth;
			++iRelX, ++iAbsX)
		{
			for (PixelWindowPos iRelY = 0, iAbsY = iY; iRelY < iHeight && iAbsY < m_iHeight;
				++iRelY, ++iAbsY)
			{
				const auto px = pData[(bFlip ? iRelY : iHeight - 1 - iRelY) * iWidth + iRelX];

				if (px.alpha == 0xFF)
					m_oLayers[iLayer].upData[(size_t)iAbsY * m_iWidth + iAbsX] = px;
			}
		}
		break;


		// consider alpha values
	case PXWIN_DRAWALPHA_ADD:
		for (PixelWindowPos iRelX = 0, iAbsX = iX; iRelX < iWidth && iAbsX < m_iWidth; ++iRelX, ++iAbsX)
		{
			for (PixelWindowPos iRelY = 0, iAbsY = iY; iRelY < iHeight && iAbsY < m_iHeight;
				++iRelY, ++iAbsY)
			{
				const auto px = pData[(bFlip ? iRelY : iHeight - 1 - iRelY) * iWidth + iRelX];

				switch (px.alpha)
				{
				case 0: // fully transparent
					// do nothing
					break;

				case 0xFF: // fully opaque
					m_oLayers[iLayer].upData[(size_t)iAbsY * m_iWidth + iAbsX] = px;
					break;

				default: // partly transparent
				{
					const auto &pxOld = m_oLayers[iLayer].upData[(size_t)iAbsY * m_iWidth + iAbsX];
					PixelWindowPixel pxNew;

					if (pxOld.alpha == 0)
						pxNew = px;
					else if (px.alpha == 0)
						pxNew = pxOld;
					else
					{
						// Porter-Duff algorithm

						const float fAlphaOld = pxOld.alpha / 255.0f;
						const float fAlphaAdd = px.alpha / 255.0f;

						float fAlphaNew = fAlphaAdd + (1.0f - fAlphaAdd) * fAlphaOld;
						if (fAlphaNew > 1.0f)
							fAlphaNew = 1.0f;
						pxNew.alpha = uint8_t(fAlphaNew * 255);

						const float fFactorGlobal = 1 / fAlphaNew;
						const float fFactorOld = (1 - fAlphaAdd) * fAlphaOld;
						pxNew.r = uint8_t(fFactorGlobal * (fAlphaAdd * px.r + fFactorOld * pxOld.r));
						pxNew.g = uint8_t(fFactorGlobal * (fAlphaAdd * px.g + fFactorOld * pxOld.g));
						pxNew.b = uint8_t(fFactorGlobal * (fAlphaAdd * px.b + fFactorOld * pxOld.b));
					}

					m_oLayers[iLayer].upData[(size_t)iAbsY * m_iWidth + iAbsX] = pxNew;
				}
				}
			}
		}
		break;

	default:
		SetError(PXWIN_ERROR_INVALID_PARAM);
		return;
	}

	m_oLayers[iLayer].bChanged = true;
}

void internal::Window::setBackgroundColor(PixelWindowPixel px)
{
	glClearColor(px.r / 255.0f, px.g / 255.0f, px.b / 255.0f, 1.0f);
}

internal::Window::Window()
{
	WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };

	const HINSTANCE  hInst          = GetModuleHandle(NULL);
	const auto       iIconSizeBig   = GetSystemMetrics(SM_CXICON);
	const auto       iIconSizeSmall = GetSystemMetrics(SM_CXSMICON);

	wc.lpszClassName = szWNDCLASSNAME;
	wc.lpfnWndProc   = &internal::Window::GlobalWindowProc;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = (HICON)LoadImage(internal::GetDLLHandle(), MAKEINTRESOURCE(IDI_ROBINLE),
		IMAGE_ICON, iIconSizeBig, iIconSizeBig, LR_SHARED);
	wc.hIconSm       = (HICON)LoadImage(internal::GetDLLHandle(), MAKEINTRESOURCE(IDI_ROBINLE),
		IMAGE_ICON, iIconSizeSmall, iIconSizeSmall, LR_SHARED);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	if (!RegisterClassExW(&wc))
	{
		constexpr wchar_t szMessagePraefix[] = L"Couldn't register window class \"";
		constexpr wchar_t szMessageSuffix[] = L"\" for the RobinLe PixelWindow API.";

		const size_t len =
			std::wcslen(szMessagePraefix) +
			std::wcslen(szWNDCLASSNAME) +
			std::wcslen(szMessageSuffix) + 1;

		wchar_t *szMessage = new wchar_t[len];
		szMessage[0] = 0; // terminating zero

		wcscat_s(szMessage, len, szMessagePraefix);
		wcscat_s(szMessage, len, szWNDCLASSNAME);
		wcscat_s(szMessage, len, szMessageSuffix);

		MessageBoxW(NULL, szMessage, L"rlPixelWindow Error", MB_ICONERROR | MB_SYSTEMMODAL);
		exit(1);
	}
}

internal::Window::~Window()
{
	UnregisterClassW(szWNDCLASSNAME, NULL);
}

LRESULT internal::Window::localWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_fnOSCallback)
		m_fnOSCallback(intfPtr(), m_hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_CREATE:
	{
		try
		{
			const HDC hDC = GetWindowDC(m_hWnd);

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
			DestroyWindow(m_hWnd);
			return 0;
		}

		glViewport(0, 0, m_iWidth * m_iPixelWidth, m_iHeight * m_iPixelHeight);
		glClearColor(
			m_pxBackground.r / 255.0f,
			m_pxBackground.g / 255.0f,
			m_pxBackground.b / 255.0f,
			1.0f);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		// create layers
		const size_t iPixelCount = (size_t)m_iWidth * m_iHeight;
		const size_t iLayerSize  = iPixelCount * sizeof(PixelWindowPixel);
		for (size_t i = 0; i < (size_t)m_iExtraLayers + 1; i++)
		{
			m_oLayers.push_back({});
			auto &oLayer = m_oLayers.back();
			oLayer.upData = std::make_unique<PixelWindowPixel[]>((size_t)m_iWidth * m_iHeight);
			memset(oLayer.upData.get(), 0, iLayerSize); // 0x00000000 = blank

			glGenTextures(1, &oLayer.iTexID);
			glBindTexture(GL_TEXTURE_2D, oLayer.iTexID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, oLayer.upData.get());
		}

		auto &cs = *reinterpret_cast<LPCREATESTRUCTW>(lParam);
		m_fnCallback(intfPtr(), PXWINMSG_CREATE, (PixelWindowArg)cs.lpCreateParams, 0);
		break;
	}

	case WM_CLOSE:
		if (!m_fnCallback(intfPtr(), PXWINMSG_TRYDESTROY, 0, 0))
			return NULL;
		DestroyWindow(m_hWnd);
		break;

	case WM_DESTROY:
		m_fnCallback(intfPtr(), PXWINMSG_DESTROY, 0, 0);
		for (auto &o : m_oLayers)
			glDeleteTextures(1, &o.iTexID);
		m_oLayers.clear();
		wglDeleteContext(m_hGLRC);
		m_hGLRC = NULL;
		PostQuitMessage(0);
		break;



	case WM_SIZING:
		// TODO
		break;

	case WM_SHOWWINDOW:
		update(PXWIN_UPDATEREASON_START); // todo: find correct place to put 1st update
		break;
	}

	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

float internal::Window::getElapsedTime(bool bAdvance)
{
	m_tpNow = std::chrono::system_clock::now();
	std::chrono::duration<float> duration = m_tpNow - m_tpLastUpdate;

	if (bAdvance)
		m_tpLastUpdate = m_tpNow;

	return duration.count();
}