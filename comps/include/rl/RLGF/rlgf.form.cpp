#include "rlgf.form.hpp"

#include <CommCtrl.h> // CLR_NONE

// #includes
// #pragma comment(lib, "template.lib")





namespace rl
{

	/***********************************************************************************************
	 class Brush
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Brush::Brush() noexcept : m_hBrush((HBRUSH)GetStockObject(WHITE_BRUSH)) {}

	Brush::Brush(Color color) noexcept : m_hBrush(CreateSolidBrush(color)) {}

	Brush::Brush(Brush&& rval) noexcept : m_hBrush(rval.m_hBrush) { rval.m_hBrush = NULL; }

	Brush::Brush(const Brush& other) noexcept : Brush(other.getColor()) {}

	Brush::~Brush() { free(); }





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	Brush& Brush::operator=(Brush&& rval) noexcept
	{
		free();
		m_hBrush = rval.m_hBrush;
		rval.m_hBrush = NULL;

		return *this;
	}

	Brush& Brush::operator=(const Brush& other) noexcept
	{
		free();
		m_hBrush = CreateSolidBrush(other.getColor());

		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	Color Brush::getColor() const noexcept
	{
		if (m_hBrush == NULL)
			return CLR_NONE; // no brush

		LOGBRUSH lbr = {};
		GetObject(m_hBrush, sizeof(lbr), &lbr);

		if (lbr.lbStyle != BS_SOLID)
			return CLR_NONE; // not a solid color brush

		return lbr.lbColor;
	}

	void Brush::setColor(Color color) noexcept
	{
		free();
		m_hBrush = CreateSolidBrush(color);
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void Brush::free() noexcept
	{
		if (m_hBrush == NULL)
			return;

		DeleteObject(m_hBrush);
	}





















	/***********************************************************************************************
	 class Font
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	std::map<HFONT, size_t> Font::oHandles;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Font::Font() : m_hFont(NULL) {}

	Font::Font(const FontConfig& config) :
		m_hFont(CreateFontW(config.iHeight, 0, 0, 0, config.iWeight, config.bItalic,
			config.bUnterline, config.bStrikeOut, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, (DWORD)config.eQuality, FF_DONTCARE, config.sFontName.c_str()))
	{
		AddHandle(m_hFont);
	}

	Font::Font(Font&& rval) noexcept : m_hFont(rval.m_hFont) { rval.m_hFont = NULL; }

	Font::Font(const Font& other) : m_hFont(other.m_hFont) { AddHandle(m_hFont); }

	Font::~Font() { free(); }





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	Font& Font::operator=(Font&& rval) noexcept
	{
		free();
		m_hFont = rval.m_hFont;
		rval.m_hFont = NULL;

		return *this;
	}

	Font& Font::operator=(const Font& other)
	{
		free();
		m_hFont = other.m_hFont;
		if (m_hFont != NULL)
			++oHandles[m_hFont];

		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	const Font& Font::GetDefault()
	{
		static Font font;

		if (font.getHandle() == NULL)
		{
			NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
			font.m_hFont = CreateFontIndirect(&ncm.lfMessageFont);
		}

		if (oHandles.find(font.m_hFont) == oHandles.end())
			oHandles.emplace(font.m_hFont, 0);

		++oHandles[font.m_hFont];

		return font;
	}

	void Font::AddHandle(HFONT handle)
	{
		if (handle == NULL)
			return; // NULL is not handled

		if (oHandles.find(handle) == oHandles.end())
			oHandles.emplace(handle, 0);

		++oHandles[handle];
	}

	void Font::RemoveHandle(HFONT handle)
	{
		if (oHandles.find(handle) == oHandles.end())
			return; // handle wasn't added to list

		--oHandles[handle];
		if (oHandles[handle] == 0)
		{
			oHandles.erase(handle);
			DeleteObject(handle);
		}
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Font::setConfig(const FontConfig& config)
	{
		free();
		m_hFont = CreateFontW(config.iHeight, 0, 0, 0, config.iWeight, config.bItalic,
			config.bUnterline, config.bStrikeOut, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, (DWORD)config.eQuality, FF_DONTCARE, config.sFontName.c_str());
	}

	bool Font::getConfig(FontConfig& dest) const
	{
		if (m_hFont == NULL)
			return false;

		LOGFONTW lf = {};
		if (GetObject(m_hFont, sizeof(LOGFONTW), &lf) == 0)
			return false;

		FontConfig cfg = {};
		cfg.sFontName = lf.lfFaceName;
		cfg.iHeight = lf.lfHeight;
		cfg.iWeight = lf.lfWeight;
		cfg.bItalic = lf.lfItalic;
		cfg.bUnterline = lf.lfUnderline;
		cfg.bStrikeOut = lf.lfStrikeOut;
		cfg.eQuality = static_cast<FontQuality>(lf.lfQuality);

		dest = cfg;
		return true;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void Font::free() { RemoveHandle(m_hFont); }










	/***********************************************************************************************
	 class IWinControl
	***********************************************************************************************/

	//==============================================================================================
	// METHODS

	//==============================================================================================
	// STATIC VARIABLES

	std::map<HWND, IWinControl*> IWinControl::oControls;





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	LRESULT CALLBACK IWinControl::GlobalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto it = oControls.find(hWnd);
		if (it == oControls.end() && uMsg == WM_CREATE)
		{
			oControls.emplace(hWnd, (IWinControl*)lParam);
			it = oControls.find(hWnd);
		}

		if (it != oControls.end())
		{
			oControls[hWnd]->processMessage(hWnd, uMsg, wParam, lParam);
			return oControls[hWnd]->m_pOriginalWndProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}





	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IWinControl::IWinControl(Component owner, int left, int top) :
		IComponent(owner), m_iLeft(left), m_iTop(top) {}

	IWinControl::~IWinControl()
	{
		removeHWND(m_hWnd);
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void IWinControl::setParent(Component parent)
	{
		if (m_hWnd != NULL)
			SetParent(m_hWnd, parent->getHandle());

		m_pParent = parent;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	bool IWinControl::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool bHandled = false;
		switch (uMsg)
		{
		case WM_CONTEXTMENU:
			invoke<POINT, bool&>(OnContextPopup, { LOWORD(lParam), HIWORD(lParam) }, bHandled);
			break;

		case WM_LBUTTONDBLCLK:
			invoke(OnDblClick);
			invoke(OnMouseDown, MouseButton::Left, (WORD)(wParam | ShiftButton::Double),
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_LBUTTONDOWN:
			invoke(OnMouseDown, MouseButton::Left, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_LBUTTONUP:
			invoke(OnMouseUp, MouseButton::Left, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_MBUTTONDBLCLK:
			invoke(OnMouseDown, MouseButton::Middle, (WORD)(wParam | ShiftButton::Double),
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_MBUTTONDOWN:
			invoke(OnMouseDown, MouseButton::Middle, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_MBUTTONUP:
			invoke(OnMouseUp, MouseButton::Middle, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_RBUTTONDBLCLK:
			invoke(OnMouseDown, MouseButton::Right, (WORD)(wParam | ShiftButton::Double),
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_RBUTTONDOWN:
			invoke(OnMouseDown, MouseButton::Right, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_RBUTTONUP:
			invoke(OnMouseUp, MouseButton::Right, (WORD)wParam,
				(int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_MOUSEMOVE:
			if (!m_bHovering)
				invoke(OnMouseEnter);
			invoke(OnMouseMove);
			if (!m_bMouseTracking)
			{
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				if (!TrackMouseEvent(&tme))
					throw std::exception("Call to TrackMouseEvent() failed");

				m_bMouseTracking = true;
			}
			break;

		case WM_MOUSELEAVE:
			m_bMouseTracking = false; // must re-call TrackMouseEvent()
			m_bHovering = false;
			invoke(OnMouseLeave);
			break;
		}

		return bHandled;
	}

	template <typename ...FN> void IWinControl::invoke(std::function<void(FN...)> fn,
		FN... args)
	{
		if (fn)
			fn(args...);
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void IWinControl::addHWND(HWND hWnd)
	{
		if (hWnd == NULL)
			throw std::exception("Tried to use a HWND with the value NULL");

		if (oControls.find(hWnd) != oControls.end())
			throw std::exception("Control callback already registered");

		oControls.emplace(hWnd, this);
	}

	void IWinControl::removeHWND(HWND hWnd) { oControls.erase(hWnd); }

	void IWinControl::afterInitialization()
	{
		addHWND(m_hWnd); // register HWND for mouse events
		m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)GlobalWndProc);
	}










	/***********************************************************************************************
	 class Button
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void Button::initialize()
	{
		m_hWnd = CreateWindowW(WC_BUTTONW, m_sCaption.c_str(),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, m_iLeft, m_iTop, m_iWidth,
			m_iHeight, getParentHandle(), NULL, Application::GetInstance().getHandle(), this);

		SendMessage(m_hWnd, WM_SETFONT, (WPARAM)Font::GetDefault().getHandle(), NULL);
	}

	bool Button::processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (rl::IWinControl::processMessage(hWnd, uMsg, wParam, lParam))
			return true;

		bool bHandled = false;

		if (uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
		{
			invoke(OnClick);
			bHandled = true;
		}

		return bHandled;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class Form
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	std::map<HWND, Form*> Form::oForms;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Form::Form(const wchar_t* szClassName, const FormInitData& InitData) :
		IWinControl(nullptr, 0, 0), m_sClassName(szClassName), m_oInitData(InitData),
		m_bVisible(InitData.bVisible) {}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	LRESULT CALLBACK Form::GlobalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto it = oForms.find(hWnd);
		if (it == oForms.end() && uMsg == WM_CREATE)
		{
			oForms.emplace(hWnd, (rl::Form*)((LPCREATESTRUCT)lParam)->lpCreateParams);
			it = oForms.find(hWnd);
			it->second->m_hWnd = hWnd;
		}

		if (it != oForms.end())
			return oForms.at(hWnd)->LocalWndProc(hWnd, uMsg, wParam, lParam);

		//MessageBox(NULL, TEXT("Unknown HWND value"), TEXT("Error"), MB_ICONERROR);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Form::show()
	{
		ShowWindow(m_hWnd, SW_SHOWNORMAL);
	}

	void Form::addComponent(Component comp)
	{
		if (comp == nullptr)
			return;

		if (std::find(m_oComponents.begin(), m_oComponents.end(), comp) == m_oComponents.end())
			m_oComponents.push_back(comp);
	}

	void Form::addControl(WinControl ctrl)
	{
		if (ctrl == nullptr)
			return;

		if (std::find(m_oControls.begin(), m_oControls.end(), ctrl) == m_oControls.end())
			m_oControls.push_back(ctrl);
		addComponent(ctrl);
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void Form::AfterConstruction()
	{
		initializeComponents();
	}

	LRESULT Form::LocalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool bHandled = IWinControl::processMessage(hWnd, uMsg, wParam, lParam);
		
		if (bHandled)
			return ERROR_SUCCESS;

		switch (uMsg)
		{
		case WM_CREATE:
			OnCreate();
			break;

		case WM_COMMAND:
		{
			if (lParam == 0) // menu or accelerator
			{
				const WORD wHigh = HIWORD(wParam);
				/*
				if (wHigh == 0)
					// ToDo: Menu
					;
				else if (wHigh == 1)
					// ToDo: Accelerator
					;
				*/
			}
			else // control
			{
				for (auto ctrl : m_oControls)
				{
					if (ctrl->getHandle() == (HWND)lParam)
						bHandled = ctrl->processMessage(hWnd, uMsg, wParam, lParam);
				}
			}
		}
		break;

		case WM_DESTROY:
			OnDestroy();
			PostQuitMessage(0);
			break;
		}

		LRESULT result;
		if (!bHandled)
			result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			result = ERROR_SUCCESS;

		if (uMsg == WM_CREATE)
			AfterConstruction();

		return result;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void Form::initialize()
	{
		HINSTANCE hInstance = Application::GetInstance().getHandle();

		WNDCLASSW wc = {};
		wc.hInstance = hInstance;
		wc.hbrBackground = m_oInitData.oBrushBackground.getHandle();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = m_oInitData.hIcon;
		wc.lpszClassName = m_sClassName.c_str();
		wc.lpfnWndProc = GlobalWndProc;
		wc.style = CS_DBLCLKS;

		if (!RegisterClassW(&wc))
			throw std::exception("Couldn't register window class");

		// m_hWnd is assigned on WM_CREATE
		CreateWindowExW(WS_EX_CONTROLPARENT, m_sClassName.c_str(), m_oInitData.szCaption,
			WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
			NULL, hInstance, this);

		if (m_hWnd == NULL)
			throw std::exception("Couldn't create window");
	}

	void Form::initializeComponents()
	{
		for (auto ctrl : m_oControls) { ctrl->setParent(this); }
		for (auto comp : m_oComponents) { comp->initialize(); }
		for (auto ctrl : m_oControls) { ctrl->afterInitialization(); }
	}










	/***********************************************************************************************
	 class Application
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	Application Application::m_oInstance;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Application::Application() : m_hInstance(GetModuleHandle(NULL))
	{

	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Application::ProcessMessages()
	{
		MSG msg = {};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Application::CreateForm(Form& form)
	{
		form.initialize();
		m_oForms.push_back(form);

		Form::oForms.emplace(form.getHandle(), &form);
	}

	void Application::Run()
	{
		if (m_oForms.size() == 0)
			return; // no forms

		for (auto& oForm : m_oForms)
		{
			if (oForm.m_bVisible)
				oForm.show();
		}

		MSG msg = {};
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			if (!IsDialogMessage(m_oForms[0].getHandle(), &msg)) // todo: might not be optimal
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class XXX
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}