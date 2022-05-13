#pragma once
#ifndef ROBINLE_CONSOLE_CORE
#define ROBINLE_CONSOLE_CORE





#include "Base.h"



typedef uint32_t		rlConsole_Color;

typedef uint64_t		rlConsole_HINSTANCE;
typedef unsigned int	rlConsole_UINT;
typedef uint64_t		rlConsole_WPARAM;
typedef uint64_t		rlConsole_LPARAM;



typedef struct tag_rlConsole_CharInfo
{
	uint32_t c;
	rlConsole_Color clForeground, clBackground;
} rlConsole_CharInfo, *rlConsole_CharBuffer;

typedef struct tag_rlOpaquePtr
{
	int unused;
} *HrlConsole;



typedef bool(__stdcall *rlConsole_OnMessage)(rlConsole_UINT uMsg, rlConsole_WPARAM wParam,
	rlConsole_LPARAM lParam);
typedef bool (__stdcall *rlConsole_OnUpdate)(float fElapsedTime, HrlConsole hConsole);



struct tag_rlConsole_Font; // forward declaration for "Font.h"

#define RLC_SC_CURSOR		0x01
#define RLC_SC_MAXIMIZEBTN	0x02

typedef struct tag_rlConsole_StartupConfig
{
	uint16_t					iScale;
	uint16_t					iFlags;
	uint16_t					iWidth;
	uint16_t					iHeight;
	uint16_t					iMinWidth;
	uint16_t					iMinHeight;
	uint16_t					iMaxWidth;
	uint16_t					iMaxHeight;
	rlConsole_Color				clBackground;
	rlConsole_Color				clForeground;
	rlConsole_OnMessage			fnOnMessage;
	rlConsole_OnUpdate			fnOnUpdate;
	struct tag_rlConsole_Font	*pFont;
	const wchar_t				*szTitle;
	const wchar_t				*szIconResID;
	rlConsole_HINSTANCE			hInstance;
} rlConsole_StartupConfig;

const rlConsole_StartupConfig rlConsole_StartupConfig_Default =
{
	.iScale = 1,
	.iFlags = RLC_SC_CURSOR | RLC_SC_MAXIMIZEBTN,
	.iWidth = 80,
	.iHeight = 25,
	.iMinWidth = 0,
	.iMinHeight = 0,
	.iMaxWidth = 0,
	.iMaxHeight = 0,
	.clBackground = 0x000000,
	.clForeground = 0xFFFFFF,
	.fnOnMessage = 0,
	.fnOnUpdate = 0,
	.pFont = 0,
	.szTitle = 0,
	.szIconResID = 0,
	.hInstance = 0
};





RLCONSOLE_API HrlConsole __stdcall rlConsole_create(const rlConsole_StartupConfig* pConfig);
RLCONSOLE_API void __stdcall rlConsole_destroy(HrlConsole con);
RLCONSOLE_API bool __stdcall rlConsole_execute(HrlConsole con);

RLCONSOLE_API unsigned __stdcall rlConsole_getBufWidth(HrlConsole con);
RLCONSOLE_API unsigned __stdcall rlConsole_getBufHeight(HrlConsole con);
RLCONSOLE_API bool __stdcall rlConsole_getBuf(HrlConsole con, rlConsole_CharBuffer* pBuf);





#endif // ROBINLE_CONSOLE_CORE