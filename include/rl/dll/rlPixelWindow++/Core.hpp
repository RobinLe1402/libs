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
		inline auto DefMsgHandler(PixelWindow p, PixelWindowMsg msg,
			PixelWindowArg arg1, PixelWindowArg arg2)
		{
			return rlPixelWindow_DefMsgHandler(p, msg, arg1, arg2);
		}





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
				uint32_t iExtraLayers = 0, bool bMaximizable = false, bool bResizable = false)
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

				m_oIntfObj = rlPixelWindow_Create(&GlobalMessageProc, &cp);

				if (!m_oIntfObj)
					return false;

				return true;
			}

			void run()
			{
				rlPixelWindow_Run(m_oIntfObj);
			}

			auto intfObj() { return m_oIntfObj; }


		protected: // interface methods

			virtual PixelWindowRes MessageProc(PixelWindowMsg msg,
				PixelWindowArg arg1, PixelWindowArg arg2)
			{
				return DefMsgHandler(m_oIntfObj, msg, arg1, arg2);
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
				}

				if (it != s_oInstances.end())
					return it->second->MessageProc(msg, arg1, arg2);
				else
					throw std::exception("rl::PixelWindowDLL: No instance to handle message");
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