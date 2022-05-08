#pragma once
#ifndef ROBINLE_LIB_GUI__PRIVATEIMPL
#define ROBINLE_LIB_GUI__PRIVATEIMPL





#include "rl/lib/rlGUI/Core.hpp"
#include "include/GLImpl_Application.hpp"
#include "include/GLImpl_Renderer.hpp"
#include "include/GLImpl_Window.hpp"

namespace lib = rl::GLGUI;



namespace rl
{
	namespace GLGUI
	{



		class PrivateImpl
		{
		public: // methods

			PrivateImpl(IGUIApplication& oApplication, GUIRenderer& oRenderer, GUIWindow& oWindow);

			GUIGraph& getGraph() { return m_oGLApplication.getGraph(); }

			auto& getGUIApp() { return m_oApplication; }
			auto& getGUIRenderer() { return m_oRenderer; }
			auto& getGUIWindow() { return m_oWindow; }

			auto& getGLWindow() { return m_oGLWindow; }
			auto& getGLRenderer() { return m_oGLRenderer; }
			auto& getGLApplication() { return m_oGLApplication; }


		private: // variables

			IGUIApplication& m_oApplication;
			GUIRenderer& m_oRenderer;
			GUIWindow& m_oWindow;

			GLImpl_Window m_oGLWindow;
			GLImpl_Renderer m_oGLRenderer;
			GLImpl_Application m_oGLApplication;

		};



	}
}





#endif // ROBINLE_LIB_GUI__PRIVATEIMPL