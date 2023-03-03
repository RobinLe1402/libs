/***************************************************************************************************
 FILE:	rlPixelWindow++/Core.hpp
 DLL:	rlPixelWindow.dll
 DESCR:	C++ wrapper around the core functionality of the PixelWindow DLL.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DLL_PIXELWINDOW_CPP
#define ROBINLE_DLL_PIXELWINDOW_CPP





#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Definitions.h>
#include <rl/dll/rlPixelWindow/Types.h>
#include <map>

namespace rl
{
	namespace PixelWindowDLL
	{



		inline auto GetVersion() { return rlPixelWindow_GetVersion(); }
		inline auto GetError() { return rlPixelWindow_GetError(); }
		inline auto GetErrorMsg(PixelWindowRes iErrorCode)
		{
			return rlPixelWindow_GetErrorMsg(iErrorCode);
		}
		inline auto GetMinSize(PixelWindowPixelSizeStruct oPixelSize,
			PixelWindowBool bResizable, PixelWindowBool bMaximizable)
		{
			return rlPixelWindow_GetMinSize(oPixelSize, bResizable, bMaximizable);
		}
		inline auto DefMsgHandler(PixelWindow p, PixelWindowMsg msg,
			PixelWindowArg arg1, PixelWindowArg arg2)
		{
			return rlPixelWindow_DefMsgHandler(p, msg, arg1, arg2);
		}
		inline auto MakeARGB(uint32_t iARGB) { return rlPixelWindow_ARGB(iARGB); }
		inline auto MakeRGB(uint32_t iRGB) { return rlPixelWindow_RGB(iRGB); }





		class Window
		{
		public: // methods

			Window()
			{
				const auto iVersion = GetVersion();
				if (PXWIN_VERSION_GET_MAJOR(iVersion) != 1 ||
					PXWIN_VERSION_GET_MINOR(iVersion) != 0)
					throw std::exception("Unsupported DLL version");
			}
			Window(const Window &) = delete;
			virtual ~Window() = default;

			Window &operator=(const Window &) = delete;
			operator bool() const { return m_oIntfObj != nullptr; }

			bool create(const PixelWindowSizeStruct &oSize,
				const PixelWindowSizeStruct &oMinSize = {},
				const PixelWindowSizeStruct &oMaxSize = {},
				PixelWindowPixelSizeStruct oPixelSize = { 1, 1 },
				uint32_t iExtraLayers = 0, const wchar_t *szTitle = nullptr,
				bool bMaximizable = false, bool bResizable = false)
			{
				PixelWindowCreateParams cp{};

				cp.iUserData    = (intptr_t)this;
				cp.iExtraLayers = iExtraLayers;

				if (bMaximizable)
					cp.iFlags   |= PXWIN_CREATE_MAXIMIZABLE;
				if (bResizable)
					cp.iFlags   |= PXWIN_CREATE_RESIZABLE;

				cp.oCanvasSize  = oSize;
				cp.oMinSize     = oMinSize;
				cp.oMaxSize     = oMaxSize;
				cp.oPixelSize   = oPixelSize;

				cp.szTitle      = szTitle;
				
				cp.fnOSCallback = GlobalOSMessageProc;

				m_oIntfObj      = rlPixelWindow_Create(&GlobalMessageProc, &cp);

				if (!m_oIntfObj)
					return false;

				return true;
			}

			void run()
			{
				rlPixelWindow_Run(intfObj());
			}

			auto getSize() const
			{
				return rlPixelWindow_GetSize(intfObj());
			}

			auto getPixelSize() const
			{
				return rlPixelWindow_GetPixelSize(intfObj());
			}

			auto getLayerCount() const
			{
				return rlPixelWindow_GetLayerCount(intfObj());
			}

			void clearLayer(uint32_t iLayerID)
			{
				rlPixelWindow_ClearLayer(intfObj(), iLayerID);
			}

			void draw(const PixelWindowPixel *pData,
				PixelWindowSize iWidth, PixelWindowSize iHeight,
				uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iAlphaMode)
			{
				rlPixelWindow_Draw(intfObj(), pData, iWidth, iHeight, iLayer, iX, iY, iAlphaMode);
			}

			auto getBackgroundColor() const
			{
				return rlPixelWindow_GetBackgroundColor(intfObj());
			}

			void setBackgroundColor(PixelWindowPixel px)
			{
				rlPixelWindow_SetBackgroundColor(intfObj(), px);
			}

			auto getTitle() const
			{
				return rlPixelWindow_GetTitle(intfObj());
			}

			void setTitle(const wchar_t *szTitle)
			{
				rlPixelWindow_SetTitle(intfObj(), szTitle);
			}

			PixelWindow intfObj() const { return m_oIntfObj; }


		protected: // interface methods

			virtual PixelWindowRes MessageProc(PixelWindowMsg msg,
				PixelWindowArg arg1, PixelWindowArg arg2)
			{
				return DefMsgHandler(intfObj(), msg, arg1, arg2);
			}

			virtual void OSMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{

			}


		private: // static methods

			static PixelWindowRes PXWIN_CONV GlobalMessageProc(
				PixelWindow win, PixelWindowMsg msg, PixelWindowArg arg1, PixelWindowArg arg2)
			{
				auto it = s_oInstances.find(win);

				if (it == s_oInstances.end() && msg == PXWINMSG_CREATE)
				{
					if (arg1 == 0)
						throw std::exception("rl::PixelWindowDLL: Initialization error");

					s_oInstances.emplace(win, (Window *)arg1);
					it = s_oInstances.find(win);
					it->second->m_oIntfObj = it->first;
				}

				if (it != s_oInstances.end())
					return it->second->MessageProc(msg, arg1, arg2);
				else
					throw std::exception("rl::PixelWindowDLL: No instance to handle message");
			}

			static void PXWIN_CONV GlobalOSMessageProc(PixelWindow p,
				HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				auto it = s_oInstances.find(p);
				if (it == s_oInstances.end())
					return;

				it->second->OSMessageProc(hWnd, uMsg, wParam, lParam);
			}


		private: // variables

			volatile PixelWindow m_oIntfObj = nullptr;


		private: // static variables
			
			static std::map<PixelWindow, Window *> s_oInstances;

		};

		std::map<PixelWindow, Window *> Window::s_oInstances;



	}
}





#endif // ROBINLE_DLL_PIXELWINDOW_CPP