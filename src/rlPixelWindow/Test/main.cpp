#include <rl/dll/rlPixelWindow++/Core.hpp>

#include <rl/input.keyboard.hpp>

#include <cstdint>
#include <cstdio>


namespace DLL = rl::PixelWindowDLL;

class WinImpl : public DLL::Window
{
public:

	using DLL::Window::Window;

protected:
	PixelWindowRes MessageProc(PixelWindowMsg msg,
		PixelWindowArg arg1, PixelWindowArg arg2) override
	{
		rl::Keyboard::getInstance().processInput();

		const auto actionBtnState = rl::Keyboard::getInstance().getKey(VK_SPACE);
		if (actionBtnState.pressed())
			setTitle(L"Test Window [SPACE]");
		else if (actionBtnState.released())
			setTitle(L"Test Window");


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
			static bool bFirstDraw = true;

			if (actionBtnState.pressed())
				setBackgroundColor(DLL::MakeRGB(0x646464));
			else if (actionBtnState.released())
				setBackgroundColor(PXWIN_COLOR_BLACK);

			PixelWindowPixel px = PXWIN_COLOR_BLACK;
			if (actionBtnState.held())
				px = PXWIN_COLOR_WHITE;
			draw(&px, 1, 1, 1, 5, 2, 0);


			const auto &params = *reinterpret_cast<PixelWindowUpdateParams *>(arg1);

			if (params.iUpdateReason == PXWIN_UPDATEREASON_RESIZE)
			{
				px = PXWIN_COLOR_RED;
				const auto oSize = getSize();

				clearLayer(1);
				this->draw(&px, 1, 1, 1, oSize.iWidth - 1, oSize.iHeight - 1, 0);
			}

			if (!bFirstDraw)
				break;

			bFirstDraw = false;
			PixelWindowPixel imgTest[9]{};
			imgTest[4] = PXWIN_COLOR_RED;
			imgTest[4].alpha = 128;
			draw(imgTest, 3, 3, 0, 0, 0, PXWIN_DRAWALPHA_ADD);

			PixelWindowPixel img = PXWIN_COLOR_RED;
			draw(&img, 1, 1, 1, 1, 2, PXWIN_DRAWALPHA_OVERRIDE);
		}
		break;
		}

		return DLL::DefMsgHandler(intfObj(), msg, arg1, arg2);
	}

	void OSMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		rl::Keyboard::getInstance().update(uMsg, wParam, lParam);
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
	constexpr bool                 bMaximizable = false;

	const auto oMinSize = DLL::GetMinSize(iPixelSize, iPixelSize, bResizable, bMaximizable);

	WinImpl win;
	std::printf("Creating window...\n");
	if (!win.create(oMinSize, { .iWidth = 30, .iHeight = 10 }, { .iHeight = 10 }, iPixelSize, iPixelSize, 1,
		L"rlPixelWindow Test Application", false, true))
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
