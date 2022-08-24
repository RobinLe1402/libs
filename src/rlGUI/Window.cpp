#include "rl/lib/rlGUI/Window.hpp"
#include "rl/lib/rlGUI/Application.hpp"

#include <functional>
#include <Windows.h>
#include <Shlobj.h>



namespace rl
{
	namespace GUI
	{

		std::map<HWND, Window*> Window::s_oWindows;



		Window::Window(const wchar_t* szClassName, IControl* pOwner, Window* pParent,
			int iLeft, int iTop, unsigned iWidth, unsigned iHeight, Color clBackground) :
			IControl(pOwner, nullptr, iLeft, iTop, iWidth, iHeight, clBackground),
			m_sClassName(szClassName), m_pParentWindow(pParent), m_sTitle(szClassName)
		{
			// disable default background --> glClear() ignores alpha
			clBackground.a = 0;
			setBackgroundColor(clBackground);

			if (!ThisApp.hasClass(szClassName))
			{
				// todo: replace with proper class registration
				// (via another method, maybe called performClassRegistration()?

				WNDCLASSEXW wc{ sizeof(wc) };
				wc.lpszClassName = szClassName;
				wc.hInstance = ThisApp.getHandle();
				wc.lpfnWndProc = GlobalWndProc;
				wc.hCursor = ThisApp.getDefaultCursor();
				wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

				if (!RegisterClassExW(&wc))
					throw std::exception("rl::GUI::Window: Failed to register window class");

				ThisApp.addClass(szClassName);

				// todo: add automatic UnregisterClass()?
			}
		}

		void Window::initialize()
		{
			if (m_bInitialized)
				return;

			m_bInitialized = true;

			HWND hWndParent;
			if (m_pParentWindow != nullptr)
			{
				hWndParent = m_pParentWindow->getHandle();
				m_pParentWindow->m_oChildWindows.push_back(this);
			}
			else
				hWndParent = NULL;
			// todo: check why owned windows don't disappear on minimization


			DWORD dwExStyle = NULL;
			DWORD dwStyle = WS_OVERLAPPEDWINDOW;

			RECT rc = { 0, 0, (LONG)m_iWidth, (LONG)m_iHeight };
			AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
			const auto iNativeWidth = rc.right - rc.left;
			const auto iNativeHeight = rc.bottom - rc.top;

			m_oClientRect.left = m_iX - rc.left;
			m_oClientRect.top = m_iY - rc.top;
			m_oClientRect.right = m_oClientRect.left + m_iWidth;
			m_oClientRect.bottom = m_oClientRect.top + m_iHeight;

			// use temporary to store the handle, as m_hWnd is already being assigned on WM_CREATE.
			HWND hWnd = CreateWindowExW(dwExStyle, m_sClassName.c_str(), m_sClassName.c_str(),
				dwStyle, m_iX, m_iY, (int)iNativeWidth, (int)iNativeHeight, hWndParent, NULL,
				ThisApp.getHandle(), this);
			if (hWnd == NULL)
				throw std::exception("rl::GUI::Window: Failed to create the window");
			m_oDropTarget.initialize();

			const auto clBG = getBackgroundColor();
			glClearColor(
				clBG.r / 255.0f,
				clBG.g / 255.0f,
				clBG.b / 255.0f,
				1.0f // alpha
			);

			// only show window after OpenGL has been set up
			//ShowWindow(hWnd, SW_SHOWNORMAL);
		}

		void Window::repaint()
		{
			checkInitialized();
			InvalidateRect(m_hWnd, NULL, TRUE);
		}

		void Window::show()
		{
			checkInitialized();

			if (m_bVisible)
				return; // already shown

			ShowWindow(m_hWnd, SW_NORMAL);
			m_bVisible = true;
		}

		void Window::hide()
		{
			checkInitialized();
			if (!m_bVisible)
				return; // already hidden

			ShowWindow(m_hWnd, SW_HIDE);
			m_bVisible = false;
		}

		void Window::setTitle(const wchar_t* szTitle)
		{
			checkInitialized();
			m_sTitle = szTitle;
			SetWindowTextW(m_hWnd, szTitle);
		}

		void Window::setTitleASCII(const char* szTitle)
		{
			checkInitialized();
			const size_t len = strlen(szTitle);
			m_sTitle.reserve(len);

			m_sTitle.clear();

			for (size_t i = 0; i < len; ++i)
			{
				char c = szTitle[i];

				// block non-ASCII characters
				if (c & 0x800)
					c = '?';

				m_sTitle += c;
			}
			SetWindowTextW(m_hWnd, m_sTitle.c_str());
		}

		void Window::setAcceptsFiles(bool bAcceptsFiles)
		{
			checkInitialized();
			bAcceptsFiles = bAcceptsFiles ? 1 : 0;

			if (bAcceptsFiles == m_bAcceptsFiles)
				return; // no change

			m_bAcceptsFiles = bAcceptsFiles;

			DWORD dwExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
			if (bAcceptsFiles)
				dwExStyle |= WS_EX_ACCEPTFILES;
			else
				dwExStyle &= ~WS_EX_ACCEPTFILES;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
		}

		void Window::checkInitialized()
		{
			if (!m_bInitialized)
				initialize();
			//throw std::exception("rl::GUI::Window: Object was not initialized");
		}

		LRESULT Window::internalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			bool bHandled = true;
			switch (uMsg)
			{
				// todo: proper OnPaint

			case WM_SHOWWINDOW:
				if (wParam == TRUE) // window is now visible
					break;
				[[fallthrough]]; // redraw on initial show
			case WM_PAINT:
				m_pOpenGL->makeCurrent();
				glClear(GL_COLOR_BUFFER_BIT);
				IControl::repaint();
				SwapBuffers(m_hDC);
				break;

			case WM_SETFOCUS:
				m_bFocused = true;
				onGainFocus();
				break;

			case WM_KILLFOCUS:
				m_bFocused = false;
				onLoseFocus();
				break;

			case WM_WINDOWPOSCHANGED:
			{
				auto& wp = *reinterpret_cast<const WINDOWPOS*>(lParam);
				const bool bRedraw = ~wp.flags & SWP_NOREDRAW;
				const bool bMove = ~wp.flags & SWP_NOMOVE;
				const bool bResize = ~wp.flags & SWP_NOSIZE;

				if (!bRedraw && !bMove && !bResize)
					break; // nothing to do

				const DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
				const DWORD dwExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);

				RECT rc{};
				AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
				if (bMove)
				{
					m_iX = wp.x - rc.left;
					m_iY = wp.y - rc.top;
				}
				if (bResize)
				{
					m_iWidth = wp.cx - (rc.right - rc.left);
					m_iHeight = wp.cy - (rc.bottom - rc.top);

					m_pOpenGL->setSize(m_iWidth, m_iHeight);
					onResized(m_iWidth, m_iHeight);
				}
				m_oClientRect.left = m_iX;
				m_oClientRect.top = m_iY;
				m_oClientRect.right = m_oClientRect.left + m_iWidth;
				m_oClientRect.bottom = m_oClientRect.top + m_iHeight;
				if (bRedraw)
					repaint();

				break;
			}

			case WM_CLOSE:
				// ToDo: handle properly for multi-window apps
				if (m_bWinClosesApp)
					DestroyWindow(m_hWnd);
				else
					hide();
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			case WM_QUIT:
				break;

			default:
				bHandled = false;
				break;
			}

			LRESULT result = 0;
			if (onMessage(result, uMsg, wParam, lParam) && !bHandled)
				return result;
			else if (!bHandled)
				return DefWindowProc(m_hWnd, uMsg, wParam, lParam);

			else
				return 0;
		}

		LRESULT CALLBACK Window::GlobalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			Window* pWin;

			if (uMsg == WM_CREATE)
			{
				pWin = reinterpret_cast<Window*>(
					reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

				pWin->m_hWnd = hWnd;
				s_oWindows.emplace(hWnd, pWin);

				pWin->m_hDC = GetDC(hWnd);
				pWin->m_pOpenGL =
					OpenGL::CreateInstance(pWin->m_hDC, pWin->m_iWidth, pWin->m_iHeight);

				/*glMatrixMode(GL_PROJECTION);
				glOrtho(0.0f, m_iWidth, m_iHeight, 0.0f, 0.0f, 1.0f);*/
			}
			else
			{
				auto it = s_oWindows.find(hWnd);

				if (it == s_oWindows.end())
					return DefWindowProc(hWnd, uMsg, wParam, lParam);

				pWin = it->second;

				if (uMsg == WM_DESTROY)
					OpenGL::DestroyInstance(pWin->m_hDC);
			}

			return pWin->internalWindowProc(uMsg, wParam, lParam);
		}





		//==========================================================================================
		// Window::DropTarget

		// todo: constructor doesn't work here as there is no HWND upon object creation.
		Window::DropTarget::DropTarget(Window& oWindow) : m_oWindow(oWindow)
		{
			if (FAILED(OleInitialize(NULL)))
				throw std::exception("rl::GUI::Window::DropTarget: Failed to initialize OLE");
		}

		Window::DropTarget::~DropTarget()
		{
			OleUninitialize();
		}

		void Window::DropTarget::initialize()
		{
			const HRESULT hr = RegisterDragDrop(m_oWindow.getHandle(), this);
			if (FAILED(hr))
				throw std::exception("rl::GUI::Window::DropTarget: Failed to register drop target");
		}

		STDMETHODIMP Window::DropTarget::QueryInterface(REFIID iid, void** ppv)
		{
			if (iid != IID_IUnknown && iid != IID_IDropTarget)
			{
				*ppv = NULL;
				return ResultFromScode(E_NOINTERFACE);
			}

			*ppv = this;
			AddRef();
			return NOERROR;
		}

		STDMETHODIMP_(ULONG) Window::DropTarget::AddRef()
		{
			return ++m_iRefCount;
		}

		STDMETHODIMP_(ULONG) Window::DropTarget::Release()
		{
			if (--m_iRefCount == 0)
			{
				delete this;
				return 0;
			}
			return m_iRefCount;
		}

		STDMETHODIMP Window::DropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState,
			POINTL pt, DWORD* pdwEffect)
		{
			FORMATETC fmtetc =
			{
				.cfFormat = CF_HDROP,
				.ptd = NULL,
				.dwAspect = DVASPECT_CONTENT,
				.lindex = -1,
				.tymed = TYMED_HGLOBAL
			};

			*pdwEffect = DROPEFFECT_NONE; // default: doesn't accept file

			// check if dragged object provides CF_HDROP
			if (pDataObj->QueryGetData(&fmtetc) != NOERROR)
				return NOERROR; // not a file
			else
			{
				// file

				STGMEDIUM medium;
				HRESULT hr = pDataObj->GetData(&fmtetc, &medium);

				if (FAILED(hr))
					return hr; // couldn't get data

				auto& oDropFiles = *reinterpret_cast<DROPFILES*>(GlobalLock(medium.hGlobal));
				if (oDropFiles.fNC)
					return NOERROR; // the nonclient area doesn't ever accept files

				const void* sz = reinterpret_cast<const uint8_t*>(&oDropFiles) + oDropFiles.pFiles;

				// Unicode
				if (oDropFiles.fWide)
				{
					auto szCurrent = reinterpret_cast<const wchar_t*>(sz);
					do
					{
						const auto len = wcslen(szCurrent);

						std::wstring s;
						s.resize(len);
						memcpy_s(&s[0], sizeof(wchar_t) * (s.length() + 1),
							szCurrent, sizeof(wchar_t) * len);

						m_oFilenames.push_back(std::move(s));

						szCurrent += len + 1;
					} while (szCurrent[0] != 0);
				}

				// Codepage
				else
				{
					auto szCurrent = reinterpret_cast<const char*>(sz);
					size_t len;

					do
					{
						len = 0;
						while (szCurrent[len] != '0') { ++len; }
						const size_t lenWide = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
							szCurrent, (int)len, NULL, NULL);

						std::wstring s;
						s.resize(lenWide - 1);
						MultiByteToWideChar(CP_ACP, NULL, szCurrent, (int)len, &s[0], (int)lenWide);
						m_oFilenames.push_back(std::move(s));

						szCurrent += len + 1;
					} while (szCurrent[0] != 0);
				}


				m_oWindow.onFileDragEnter();
				DropEffect eEffect = DropEffect::None;
				m_oWindow.onFileDrag(pt.x - m_oWindow.m_oClientRect.left,
					pt.y - m_oWindow.m_oClientRect.top, eEffect, grfKeyState);

				switch (eEffect)
				{
				case DropEffect::None:
					*pdwEffect = DROPEFFECT_NONE;
					break;
				case DropEffect::Copy:
					*pdwEffect = DROPEFFECT_COPY;
					break;
				case DropEffect::Move:
					*pdwEffect = DROPEFFECT_MOVE;
					break;
				case DropEffect::Link:
					*pdwEffect = DROPEFFECT_LINK;
					break;

				default:
					*pdwEffect = DROPEFFECT_NONE;
					break;
				}
			}

			return NOERROR;
		}

		STDMETHODIMP Window::DropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
		{
			DropEffect eEffect = DropEffect::None;
			m_oWindow.onFileDrag(pt.x - m_oWindow.m_oClientRect.left,
				pt.y - m_oWindow.m_oClientRect.top, eEffect, grfKeyState);

			switch (eEffect)
			{
			case DropEffect::None:
				*pdwEffect = DROPEFFECT_NONE;
				break;
			case DropEffect::Copy:
				*pdwEffect = DROPEFFECT_COPY;
				break;
			case DropEffect::Move:
				*pdwEffect = DROPEFFECT_MOVE;
				break;
			case DropEffect::Link:
				*pdwEffect = DROPEFFECT_LINK;
				break;

			default:
				*pdwEffect = DROPEFFECT_NONE;
				break;
			}

			return NOERROR;
		}

		STDMETHODIMP Window::DropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt,
			LPDWORD pdwEffect)
		{
			m_oWindow.onFileDrop(pt.x - m_oWindow.m_oClientRect.left,
				pt.y - m_oWindow.m_oClientRect.top, grfKeyState);
			m_oFilenames.clear();

			return NOERROR;
		}

		STDMETHODIMP Window::DropTarget::DragLeave()
		{
			m_oFilenames.clear();
			m_oWindow.onFileDragLeave();

			return NOERROR;
		}

	}
}
