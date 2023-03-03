#include <rl/dll/rlPixelWindow++/Core.hpp>

#include <rl/input.keyboard.hpp>
#include <rl/input.mouse.hpp>

#include <cstdint>
#include <cstdio>

#include <string>

// todo: handle maximum window size!!!!


namespace DLL = rl::PixelWindowDLL;

void OpenFileDialog()
{
	TCHAR lptstrBuf[MAX_PATH + 1]{};


	OPENFILENAME ofn{sizeof(ofn)};
	ofn.hwndOwner = NULL;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFile = lptstrBuf;
	ofn.nMaxFile  = sizeof(lptstrBuf) / sizeof(TCHAR);

	GetOpenFileName(&ofn);
}

class WinImpl : public DLL::Window
{
public:

	using DLL::Window::Window;

protected:
	PixelWindowRes MessageProc(PixelWindowMsg msg,
		PixelWindowArg arg1, PixelWindowArg arg2) override
	{
		auto &oKeyboard = rl::Keyboard::getInstance();
		oKeyboard.processInput();

		auto &oMouse = rl::Mouse::getInstance();
		oMouse.processInput();
		const auto &oMouseState = oMouse.getState();

		const auto oPixelSize = getPixelSize();

		std::wstring sTitle = L"Test Window";

		const auto iMouseX = oMouseState.x / oPixelSize.iWidth;
		const auto iMouseY = oMouseState.y / oPixelSize.iHeight;

		if (!oMouseState.bOnClient)
			sTitle += L" [no mouse]";
		else
			sTitle += L" [" + std::to_wstring(iMouseX) + L'|' + std::to_wstring(iMouseY) + L']';


		const auto actionBtnState = oKeyboard.getKey(VK_SPACE);
		if (actionBtnState.held())
			sTitle += L" (SPACE)";

		setTitle(sTitle.c_str());


		clearLayer(0);
		if (oMouseState.bOnClient)
		{
			PixelWindowPixel px = PXWIN_COLOR_GREEN;
			clearLayer(0);
			draw(&px, 1, 1, 0, iMouseX, iMouseY, 0);
		}


		switch (msg)
		{
		case PXWINMSG_CREATE:
		{
			constexpr uint8_t iText[] =
			{
				0b10101010,
				0b10100010,
				0b11101010,
				0b10101000,
				0b10101010
			};

			PixelWindowPixel imgText[sizeof(iText) * 8]{};
			for (size_t iY = 0; iY < sizeof(iText); ++iY)
			{
				auto iRow = iText[iY];

				uint8_t iX = 8;
				while (iRow)
				{
					--iX;
					if (iRow & 1)
						imgText[iY * 8 + iX] = PXWIN_COLOR_WHITE;

					iRow >>= 1;
				}
			}

			draw(imgText, 8, sizeof(iText), 0, 1, 1, 0);
		}
		break;

		case PXWINMSG_UPDATE:
		{
			/*if (oMouseState.left.bPressed)
				OpenFileDialog();*/


			if (actionBtnState.pressed())
				setBackgroundColor(DLL::MakeRGB(0x646464));
			else if (actionBtnState.released())
				setBackgroundColor(PXWIN_COLOR_BLACK);

			PixelWindowPixel px = PXWIN_COLOR_BLACK;
			if (actionBtnState.held())
				px = PXWIN_COLOR_WHITE;
			draw(&px, 1, 1, 1, 5, 2, 0);

			px = DLL::MakeRGB(0x9F07F7);
			if (oMouseState.bOnClient)
			{
				if (oMouseState.left.bHeld)
					draw(&px, 1, 1, 2, iMouseX, iMouseY, 0);
				else if (oMouseState.right.bHeld)
				{
					px = PXWIN_COLOR_BLANK;
					draw(&px, 1, 1, 2, iMouseX, iMouseY, PXWIN_DRAWALPHA_OVERRIDE);
				}
			}

			if (oKeyboard.getKey(VK_SPACE).pressed())
				clearLayer(2);

			if (oKeyboard.getKey(VK_CONTROL).held())
			{
				if (oKeyboard.getKey(VK_OEM_PLUS).pressed())
				{
					auto oPixelSize = getPixelSize();
					oPixelSize.iWidth++;
					oPixelSize.iHeight++;
					setPixelSize(oPixelSize);
				}

				else if (oKeyboard.getKey(VK_OEM_MINUS).pressed())
				{
					auto oPixelSize = getPixelSize();
					oPixelSize.iWidth--;
					oPixelSize.iHeight--;
					setPixelSize(oPixelSize);
				}
			}


			const auto &params = *reinterpret_cast<PixelWindowUpdateParams *>(arg1);

			if (params.iUpdateReason == PXWIN_UPDATEREASON_RESIZE)
			{
				px = PXWIN_COLOR_RED;
				const auto oSize = getSize();

				clearLayer(1);
				this->draw(&px, 1, 1, 1, oSize.iWidth - 1, oSize.iHeight - 1, 0);
			}
		}
		break;
		}

		return DLL::DefMsgHandler(intfObj(), msg, arg1, arg2);
	}

	void OSMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		rl::Keyboard::getInstance().update(uMsg, wParam, lParam);
		rl::Mouse::getInstance().update(uMsg, wParam, lParam);


		switch (uMsg)
		{
		case WM_CREATE:
			rl::Mouse::getInstance().setHWND(hWnd);
			break;
		}
	}
};

void PrintError(PixelWindowRes iErrorCode);

int main()
{
	std::printf(
		"TEST APPLICATION FOR THE rlPixelWindow DLL\n"
		"==========================================\n"
		"2022 Robin Lemanska\n"
		"\n"
		"\n"
	);

	std::printf("DLL version: ");
	const PixelWindowVersion iVersionNumber = DLL::GetVersion();
	std::printf("%u.%u.%u.%u\n\n",
		PXWIN_VERSION_GET_MAJOR(iVersionNumber),
		PXWIN_VERSION_GET_MINOR(iVersionNumber),
		PXWIN_VERSION_GET_PATCH(iVersionNumber),
		PXWIN_VERSION_GET_BUILD(iVersionNumber));

	std::printf("Last error at startup: ");
	PrintError(DLL::GetError());
	std::printf("\n\n");

	constexpr PixelWindowPixelSize iPixelSize   = 5;
	constexpr bool                 bResizable   = true;
	constexpr bool                 bMaximizable = true;

	const auto oMinSize = DLL::GetMinSize({ iPixelSize, iPixelSize }, bResizable, bMaximizable);

	WinImpl win;
	std::printf("Creating window...\n");
	if (!win.create(
		{ .iWidth = 100, .iHeight = 50 },
		{ .iWidth = 30,  .iHeight = 10 },
		{                              },
		{ iPixelSize, iPixelSize }, 2, L"rlPixelWindow Test Application", bMaximizable, bResizable))
	{
		std::printf("Window creation failed.\n");
		std::printf("Error: ");
		PrintError(DLL::GetError());
		std::printf("\n\n");
		return 1;
	}
	std::printf(
		"Window creation succeeded.\n"
		"\n"
		"Running ...\n"
	);
	win.run();

	std::printf("\nLast error at shutdown: ");
	PrintError(DLL::GetError());
	std::printf("\n\n");


	return 0;
}

void PrintError(PixelWindowRes iErrorCode)
{
	constexpr char szUnknown[] = "unknown error code";

	const char *szErrorName = szUnknown;

	const auto pMsg = DLL::GetErrorMsg(iErrorCode);
	if (pMsg)
		szErrorName = pMsg->szDefName;

	std::printf("%llu (%s)", iErrorCode, szErrorName);
}
