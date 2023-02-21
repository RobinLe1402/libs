#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__WINDOW
#define ROBINLE_DLL_PIXEL_WINDOW__WINDOW



#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Types.h>

#include <chrono>
#include <functional>

#include <Windows.h>
#include <gl/GL.h>



namespace internal
{



	struct Layer
	{
		std::unique_ptr<PixelWindowPixel[]> upData;
		GLuint iTexID;
		bool bChanged;
	};



	class Window
	{
	private: // static methods

		static LRESULT CALLBACK GlobalWindowProc(
			HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // static variables

		static Window s_oInstance;


	public: // static methods

		static Window &instance() { return s_oInstance; }


	public: // methods

		operator bool() const { return m_bInitialized; }

		void create(PixelWindowProc fnCallback, const PixelWindowCreateParams *pParams);
		void destroy();

		void run();

		void update(uint8_t iReason);

		void draw(const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight,
			uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iFlags,
			uint8_t iAlphaMode);

		void setBackgroundColor(PixelWindowPixel px);


		auto intfPtr() { return reinterpret_cast<PixelWindow>(this); }


	private: // methods

		Window();
		~Window();

		LRESULT localWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		float getElapsedTime(bool bAdvance = false);


	private: // variables

		bool m_bInitialized = false;
		bool m_bRunning     = false;
		std::function<PixelWindowRes(PixelWindow, PixelWindowMsg, PixelWindowArg, PixelWindowArg)>
			m_fnCallback;
		std::function<void(PixelWindow, HWND, UINT, WPARAM, LPARAM)> m_fnOSCallback;

		PixelWindowSize      m_iWidth      = 0, m_iHeight      = 0;
		PixelWindowPixelSize m_iPixelWidth = 0, m_iPixelHeight = 0;
		bool     m_bResizable   = false;
		bool     m_bMaximizable = false;

		PixelWindowPixel m_pxBackground = PXWIN_COLOR_BLACK;
		uint32_t m_iExtraLayers = 0;
		std::vector<Layer> m_oLayers;

		HWND  m_hWnd  = NULL;
		HGLRC m_hGLRC = NULL;

		std::chrono::time_point<std::chrono::system_clock> m_tpLastUpdate;
		std::chrono::time_point<std::chrono::system_clock> m_tpNow;

	};



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__WINDOW