//#include <rl/dll/rlPixelWindow/Core.h>
#include "PrivateIncludes/Window.hpp"

internal::Window internal::Window::s_oInstance;


void internal::Window::create(PixelWindowProc fnCallback, const PixelWindowCreateParams *pParams)
{
	if (m_bInitialized)
		destroy();



	m_fnCallback = fnCallback;
	// TODO: process pParams


	m_bInitialized = true;
}

void internal::Window::destroy()
{
	if (!m_bInitialized)
		return;



	// todo
	
	
	m_bInitialized = false;
}
