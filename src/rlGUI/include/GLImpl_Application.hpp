#pragma once
#ifndef ROBINLE_LIB_GUI_GLIMPL_APPLICATION
#define ROBINLE_LIB_GUI_GLIMPL_APPLICATION





#include "rl/lib/rlGUI/Core.hpp"
#include "rl/lib/rlOpenGL/Core.hpp"

namespace lib = rl::GLGUI;
namespace GL = rl::OpenGL;



class GLImpl_Application : public GL::IApplication
{
public: // methods

	GLImpl_Application(lib::PrivateImpl* pPImpl,
		GL::Window& oWindow, GL::IRenderer& oRenderer)
		:
		GL::IApplication::IApplication(oWindow, oRenderer),
		m_pPrivateImpl(pPImpl)
	{}
	GLImpl_Application(const GLImpl_Application& other) = delete;
	GLImpl_Application(GLImpl_Application&& rval) = delete;
	virtual ~GLImpl_Application() = default;

	lib::GUIGraph& getGraph() { return *static_cast<lib::GUIGraph*>(graph()); }


private: // methods

	bool OnStart() override;
	bool OnUpdate(float fElapsedTime) override;
	bool OnStop() override;

	void OnResize(LONG& iWidth, LONG& iHeight) override;

	void createGraph(void** pGraph) override;
	void copyGraph(void* pDest, const void* pSource) override;
	void destroyGraph(void* pGraph) override;


private: // variables

	lib::PrivateImpl* m_pPrivateImpl;

};





#endif // ROBINLE_LIB_GUI_GLIMPL_APPLICATION
