#ifndef FONTFACEGUI_WINAPI__GUI_MAIN
#define FONTFACEGUI_WINAPI__GUI_MAIN



#include <Windows.h>
#include <stdint.h>

#include <rl/visualstyles.h>


const wchar_t szCLASS_FORM_MAIN[] = L"formMain";
const wchar_t szCLASS_FORM_NEW[] = L"formNew";

const uint8_t iPROG_VER[4] = { 0, 0, 0, 1 };



LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool CreateGUI(HINSTANCE hInstance, LPSTR szCmdLine, int iCmdShow);

void RunGUI();



#endif // FONTFACEGUI_WINAPI__GUI_MAIN