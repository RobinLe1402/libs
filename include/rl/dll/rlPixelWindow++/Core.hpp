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
		inline auto GetMinSize(PixelWindowPixelSize iPixelWidth, PixelWindowPixelSize iPixelHeight,
			PixelWindowBool bResizable, PixelWindowBool bMaximizable)
		{
			return rlPixelWindow_GetMinSize(iPixelWidth, iPixelHeight, bResizable, bMaximizable);
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

			Window() = default;
			Window(const Window &) = delete;
			virtual ~Window() = default;

			Window &operator=(const Window &) = delete;
			operator bool() const { return m_oIntfObj != nullptr; }

			bool create(PixelWindowSize iWidth, PixelWindowSize iHeight,
				PixelWindowPixelSize iPixelWidth = 1, PixelWindowPixelSize iPixelHeight = 1,
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

				cp.iWidth       = iWidth;
				cp.iHeight      = iHeight;
				cp.iPixelWidth  = iPixelWidth;
				cp.iPixelHeight = iPixelHeight;

				cp.szTitle      = szTitle;
				
				cp.fnOSCallback = GlobalOSMessageProc;

				m_oIntfObj = rlPixelWindow_Create(&GlobalMessageProc, &cp);

				if (!m_oIntfObj)
					return false;

				return true;
			}

			void run()
			{
				rlPixelWindow_Run(m_oIntfObj);
			}

			void draw(const PixelWindowPixel *pData,
				PixelWindowSize iWidth, PixelWindowSize iHeight,
				uint32_t iLayer, PixelWindowPos iX, PixelWindowPos iY, uint8_t iAlphaMode)
			{
				rlPixelWindow_Draw(intfObj(), pData, iWidth, iHeight, iLayer, iX, iY, iAlphaMode);
			}

			void setBackgroundColor(PixelWindowPixel px)
			{
				rlPixelWindow_SetBackgroundColor(intfObj(), px);
			}

			PixelWindow intfObj() { return m_oIntfObj; }


		protected: // interface methods

			virtual PixelWindowRes MessageProc(PixelWindowMsg msg,
				PixelWindowArg arg1, PixelWindowArg arg2)
			{
				return DefMsgHandler(m_oIntfObj, msg, arg1, arg2);
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

			PixelWindow m_oIntfObj = nullptr;


		private: // static variables
			
			static std::map<PixelWindow, Window *> s_oInstances;

		};

		std::map<PixelWindow, Window *> Window::s_oInstances;



	}
}





#endif // ROBINLE_DLL_PIXELWINDOW_CPP