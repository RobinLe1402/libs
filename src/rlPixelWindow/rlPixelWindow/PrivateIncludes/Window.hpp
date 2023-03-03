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

		static DWORD GetStyle(bool bResizable, bool bMaximizable,
			bool bMinimizable = true, bool bMaximized = false);


	public: // static methods

		static Window &instance()
		{
			static Window oInstance;

			return oInstance;
		}

		static PixelWindowSizeStruct MinSize(PixelWindowPixelSizeStruct oPixelSize,
			bool bResizable, bool bMaximizable, bool bMinimizable = true);



	public: // methods

		operator bool() const { return m_bInitialized; }

		void create(PixelWindowProc fnCallback, const PixelWindowCreateParams *pParams);
		void destroy();

		PixelWindowLayerID getLayerCount() const
		{
			return (PixelWindowLayerID)m_oLayers.size();
		}
		auto getSize() const { return m_oCanvasSize; }

		auto getPixelSize() const { return m_oPixelSize; }

		PixelWindowBool setPixelSize(PixelWindowPixelSizeStruct oPixelSize);

		void run();

		void update(uint8_t iReason);

		void clearLayer(uint32_t iLayerID);

		void draw(const PixelWindowPixel *pData, PixelWindowSize iWidth, PixelWindowSize iHeight,
			uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iAlphaMode);

		PixelWindowPixel getBackgroundColor() const;
		void setBackgroundColor(PixelWindowPixel px);

		const wchar_t *getTitle() const { return m_sTitle.c_str(); }
		void setTitle(const wchar_t *szTitle);


		auto intfPtr() { return reinterpret_cast<PixelWindow>(this); }


	private: // methods

		Window();
		~Window();

		LRESULT localWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		float getElapsedTime(bool bAdvance = false);

		void handleResize(PixelWindowSize iWidth, PixelWindowSize iHeight);

		void calcFrameSize();


	private: // variables

		// fully internal data

		bool m_bInitialized = false;
		bool m_bRunning     = false;

		HWND  m_hWnd  = NULL;
		HGLRC m_hGLRC = NULL;

		std::chrono::time_point<std::chrono::system_clock> m_tpLastUpdate;
		std::chrono::time_point<std::chrono::system_clock> m_tpNow;

		// todo: use (at all?)
		uint32_t m_iWindowFrameWidth  = 0;
		uint32_t m_iWindowFrameHeight = 0;


		// user configurated data
		std::function<PixelWindowRes(PixelWindow, PixelWindowMsg, PixelWindowArg, PixelWindowArg)>
			m_fnCallback;
		std::function<void(PixelWindow, HWND, UINT, WPARAM, LPARAM)> m_fnOSCallback;

		PixelWindowSizeStruct m_oCanvasSize = {};
		PixelWindowSizeStruct m_oCanvasMinSize = {};
		PixelWindowSizeStruct m_oCanvasMaxSize = {};
		PixelWindowPixelSizeStruct m_oPixelSize = {};

		bool m_bResizable   = false;
		bool m_bMaximizable = false;

		PixelWindowPixel m_pxBackground = PXWIN_COLOR_BLACK;
		std::vector<Layer> m_oLayers;

		std::wstring m_sTitle;

	};



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__WINDOW