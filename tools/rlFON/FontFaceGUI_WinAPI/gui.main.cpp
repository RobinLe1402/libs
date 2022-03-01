#include "gui.main.hpp"
#include "resource.h"
#include "constants.h"

#include <cwchar>

#include <CommCtrl.h>
#pragma comment(lib, "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\um\\x64\\ComCtl32.Lib")

#pragma comment(linker,"\"/manifestdependency:type                  = 'win32' \
                                              name                  = 'Microsoft.Windows.Common-Controls' \
                                              version               = '6.0.0.0' \
                                              processorArchitecture = '*' \
                                              publicKeyToken        = '6595b64144ccf1df' \
                                              language              = '*'\"")

HMENU hMenu = NULL;
HMENU hMenuFile = NULL;
HMENU hMenuEdit = NULL;
HMENU hMenuFont = NULL;
HMENU hMenuChar = NULL;
HMENU hMenuView = NULL;
HMENU hMenuHelp = NULL;

HWND hWndMain = NULL;
HWND hWndNew = NULL;





void CreateMenus(HWND hWndMain)
{
	hMenu = CreateMenu();



	hMenuFile = CreateMenu();

	AppendMenuW(hMenuFile, MF_STRING, MENU_FILE_NEW, L"&New\tCtrl+N");
	AppendMenuW(hMenuFile, MF_STRING, MENU_FILE_OPEN, L"&Open...\tCtrl+O");
	AppendMenuW(hMenuFile, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hMenuFile, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FILE_CLOSE, L"&Close");
	AppendMenuW(hMenuFile, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FILE_SAVE, L"&Save\tCtrl+S");
	AppendMenuW(hMenuFile, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FILE_SAVEAS, L"Save &As...\tCtrl+Shift+S");
	AppendMenuW(hMenuFile, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hMenuFile, MF_STRING, MENU_FILE_EXIT, L"&Exit\tAlt+F4");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuFile, L"&File");



	hMenuEdit = CreateMenu();

	AppendMenuW(hMenuEdit, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_EDIT_UNDO, L"&Undo\tCtrl+Z");
	AppendMenuW(hMenuEdit, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_EDIT_REDO, L"&Redo\tCtrl+Shift+Z");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuEdit, L"&Edit");

	

	hMenuFont = CreateMenu();

	AppendMenuW(hMenuFont, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FONT_PROPERTIES, L"&Properties");
	AppendMenuW(hMenuFont, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FONT_PREVIEW, L"Pre&view");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuFont, L"F&ont");



	hMenuChar = CreateMenu();

	AppendMenuW(hMenuChar, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_CHAR_ADD, L"&Add...");
	AppendMenuW(hMenuChar, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_CHAR_ADDCOPY, L"Add by &copy...");
	AppendMenuW(hMenuChar, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hMenuChar, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_CHAR_SETWIDTH, L"Set &width...");
	AppendMenuW(hMenuChar, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hMenuChar, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_CHAR_DELETE, L"&Delete");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuChar, L"&Char");



	hMenuView = CreateMenu();

	AppendMenuW(hMenuView, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_VIEW_100PERCENT, L"&100%");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuView, L"&View");



	hMenuHelp = CreateMenu();

	AppendMenuW(hMenuHelp, MF_STRING, MENU_HELP_ABOUT, L"&About");
	AppendMenuW(hMenuHelp, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hMenuHelp, MF_STRING, MENU_HELP_WEBSITE, L"RobinLe &Website");
	AppendMenuW(hMenuHelp, MF_STRING, MENU_HELP_GITHUB, L"RobinLe at &GitHub");

	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuHelp, L"&Help");



	SetMenu(hWndMain, hMenu);


	HWND hWndUpDown = CreateWindowEx(NULL, UPDOWN_CLASS, L"TestUpDown", NULL, 0, 0, 50, 25, hWndMain, NULL, NULL, NULL);
	// https://docs.microsoft.com/en-us/windows/win32/controls/create-an-up-down-control
}

void New()
{
	RECT rectMain = {};
	GetWindowRect(hWndMain, &rectMain);

	const int iX = rectMain.left + ((rectMain.right - rectMain.left) / 2) - (WIDTH_FORM_NEW / 2);
	const int iY = rectMain.top + ((rectMain.bottom - rectMain.top) / 2) - (HEIGHT_FORM_NEW / 2);


	hWndNew = CreateWindowW(szCLASS_FORM_NEW, L"Create a new FontFace", WS_DLGFRAME | WS_SYSMENU,
		iX, iY, WIDTH_FORM_NEW, HEIGHT_FORM_NEW, hWndMain, NULL,
		GetModuleHandle(NULL), NULL);

	ShowWindow(hWndNew, SW_SHOWNORMAL);
}

void Open()
{
	const wchar_t szFILTER[] =
		L"RobinLe FontFace Files (*.rlFNT)\0*.rlFNT\0"
		L"All Files (*.*)\0*.*\0\0";
	wchar_t szPath[MAX_PATH + 1] = { 0 };

	OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.hwndOwner = hWndMain;
	ofn.lpstrFilter = szFILTER;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH + 1;

	if (!GetOpenFileNameW(&ofn))
		return;

	MessageBoxW(hWndMain, szPath, L"File selected", MB_ICONINFORMATION);
}

bool IsMenuItemEnabled(HMENU hMenu, UINT uId)
{
	UINT state = GetMenuState(hMenu, uId, MF_BYCOMMAND);
	return !(state & (MF_DISABLED | MF_GRAYED));
}

constexpr int MENUCMD = 0x10000;

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case MENUCMD | ID_ACCEL_NEW:
			if (hWndNew)
				break;
		case MENU_FILE_NEW:
			New();
			break;



		case MENUCMD | ID_ACCEL_OPEN:
		case MENU_FILE_OPEN:
			Open();
			break;



		case MENUCMD | ID_ACCEL_SAVE:
			if (!IsMenuItemEnabled(hMenuFile, MENU_FILE_SAVE))
				break;
		case MENU_FILE_SAVE:
			MessageBoxA(hWnd, "Not implemented", "NOTIMPL", MB_ICONERROR);
			break;



		case MENUCMD | ID_ACCEL_SAVEAS:
			if (!IsMenuItemEnabled(hMenuFile, MENU_FILE_SAVEAS))
				break;
		case MENU_FILE_SAVEAS:
			MessageBoxA(hWnd, "Not implemented", "NOTIMPL", MB_ICONERROR);
			break;



		case MENUCMD | ID_ACCEL_UNDO:
			if (!IsMenuItemEnabled(hMenuEdit, MENU_EDIT_UNDO))
				break;
		case MENU_EDIT_UNDO:
			MessageBoxA(hWnd, "Not implemented", "NOTIMPL", MB_ICONERROR);
			break;



		case MENUCMD | ID_ACCEL_REDO:
			if (!IsMenuItemEnabled(hMenuEdit, MENU_EDIT_REDO))
				break;
		case MENU_EDIT_REDO:
			MessageBoxA(hWnd, "Not implemented", "NOTIMPL", MB_ICONERROR);
			break;



		case MENU_FILE_EXIT:
			DestroyWindow(hWnd);
			break;



		case MENU_HELP_ABOUT:
		{
			wchar_t szMsg1[] = L"RobinLe FontFace Designer v";
			wchar_t szMsgVer[16] = { };
			wchar_t szMsg2[] = L"\n\n\u00A9 Robin Lemanska\n\nwww.robinle.de";

			swprintf(szMsgVer, 16, L"%u.%u.%u.%u",
				iPROG_VER[0], iPROG_VER[1], iPROG_VER[2], iPROG_VER[3]);

			const size_t len = wcslen(szMsg1) + wcslen(szMsgVer) + wcslen(szMsg2) + 1;
			wchar_t* szMsg = new wchar_t[len]{};

			wcscat_s(szMsg, len, szMsg1);
			wcscat_s(szMsg, len, szMsgVer);
			wcscat_s(szMsg, len, szMsg2);

			MessageBoxW(hWnd, szMsg, L"About RobinLe FontFace Designer",
				MB_APPLMODAL | MB_ICONINFORMATION | MB_OK);

			delete[] szMsg;
			break;
		}

		case MENU_HELP_WEBSITE:
			ShellExecuteW(hWnd, NULL, L"https://www.robinle.de", NULL, NULL, SW_SHOWNORMAL);
			break;

		case MENU_HELP_GITHUB:
			ShellExecuteW(hWnd, NULL, L"https://github.com/RobinLe1402", NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		break;

	case WM_CREATE:
		CreateMenus(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_New(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		// todo
		break;

	case WM_SHOWWINDOW:
		EnableWindow(hWndMain, FALSE);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			SendMessage(hWnd, WM_CLOSE, NULL, NULL);
		break;

	case WM_CLOSE:
		EnableWindow(hWndMain, TRUE);
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		hWndNew = NULL;
		break;

	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

bool CreateGUI(HINSTANCE hInstance, LPSTR szCmdLine, int iCmdShow)
{
	WNDCLASSW wc = {};

	// initialize global values
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	//wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ROBINLE));



	// register main window class

	wc.lpfnWndProc = WndProc_Main;
	wc.lpszClassName = szCLASS_FORM_MAIN;

	RegisterClassW(&wc);



	// register "New" window class

	wc.lpfnWndProc = WndProc_New;
	wc.lpszClassName = szCLASS_FORM_NEW;

	RegisterClassW(&wc);





	hWndMain = CreateWindowW(szCLASS_FORM_MAIN, L"RobinLe FontFace Designer",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
		hInstance, NULL);

	if (hWndMain == NULL)
		return false;

	ShowWindow(hWndMain, iCmdShow);

	return true;
}

void RunGUI()
{

	HACCEL hAccel = LoadAcceleratorsW(NULL, MAKEINTRESOURCEW(IDR_ACCEL_EDITOR));

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		if (!TranslateAcceleratorW(hWndMain, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}