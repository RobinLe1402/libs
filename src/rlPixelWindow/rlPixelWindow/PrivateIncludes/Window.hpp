#pragma once
#ifndef ROBINLE_DLL_PIXEL_WINDOW__WINDOW
#define ROBINLE_DLL_PIXEL_WINDOW__WINDOW



#include <rl/dll/rlPixelWindow/Core.h>
#include <rl/dll/rlPixelWindow/Types.h>

#include <functional>



namespace internal
{



	class Window
	{
	private: // static variables

		static Window s_oInstance;


	public: // static methods

		static Window &instance() { return s_oInstance; }


	public: // methods

		operator bool() const { return m_bInitialized; }

		void create(PixelWindowProc fnCallback, const PixelWindowCreateParams* pParams);
		void destroy();


	private: // variables

		bool m_bInitialized = false;
		std::function<PixelWindowRes(PixelWindow, PixelWindowMsg, PixelWindowArg, PixelWindowArg)>
			m_fnCallback;

	};



}



#endif // ROBINLE_DLL_PIXEL_WINDOW__WINDOW