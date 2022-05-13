#include "rl/dll/rlConsole/Core.h"
#include "rl/dll/rlConsole/Font.h"

#include <Windows.h>



bool OnMessage(rlConsole_UINT uMsg, rlConsole_WPARAM wParam, rlConsole_LPARAM lParam)
{
	return false;
}

bool bInitialized = false;

bool OnUpdate(float fElapsedTime, HrlConsole hConsole)
{
	if (!bInitialized)
	{
		rlConsole_CharBuffer buf;
		rlConsole_getBuf(hConsole, &buf);

		buf[0].c = 'A';
		buf[1].clBackground = 0xFF00FF;

		bInitialized = true;
	}

	return true;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	rlConsole_StartupConfig cfg = rlConsole_StartupConfig_Default;
	cfg.iMinHeight = 1;
	cfg.iMinWidth = 10;
	cfg.fnOnMessage = OnMessage;
	cfg.fnOnUpdate = OnUpdate;

	const size_t iCharSize = rlConsole_Font_Char_Height * sizeof(rlConsole_Font_Char_Row);
	
	rlConsole_Font font;
	rlConsole_Font_create(&font, 2, ' ');
	font.pData[0].c = ' ';
	/*const rlConsole_Font_Char_Row oCharDataSPACE[rlConsole_Font_Char_Height] =
	{
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	};*/
	font.pData[1].c = 'A';
	const rlConsole_Font_Char_Row oCharDataA[rlConsole_Font_Char_Height] =
	{
		0b00010000,
		0b00101000,
		0b01000100,
		0b10000010,
		0b10000010,
		0b10000010,
		0b11111110,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	}; // ToDo: find out why drawing is incorrect (gap between row 0 and 4
	memcpy_s(font.pData[1].iData, iCharSize, oCharDataA, iCharSize);
	cfg.pFont = &font;

	HrlConsole handle = rlConsole_create(&cfg);
	if (handle == 0)
		return 1;

	rlConsole_execute(handle);
	rlConsole_destroy(handle);

	return 0;
}

