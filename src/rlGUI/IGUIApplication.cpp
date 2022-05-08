#include "rl/lib/rlGUI/Core.hpp"

#include "rl/lib/rlOpenGL/Core.hpp"
#include "include/PrivateImpl.hpp"

#pragma comment(lib, "rlOpenGL.lib")

namespace lib = rl::GLGUI;
namespace GL = rl::OpenGL;



lib::IGUIApplication::IGUIApplication(GUIWindow& oWindow, GUIRenderer& oRenderer) :
	m_oWindow(oWindow), m_oRenderer(oRenderer),
	m_pPrivateImpl(new PrivateImpl(*this, oRenderer, oWindow))
{

}

lib::IGUIApplication::~IGUIApplication()
{
	delete m_pPrivateImpl;

	for (auto p : m_oControls)
	{
		delete p;
	}
}

bool lib::IGUIApplication::execute(const GUIConfig& config)
{
	GL::AppConfig cfgGL;
	cfgGL.renderer.bVSync = true;
	cfgGL.window.bResizable = true;
	cfgGL.window.bFullscreen = false;
	cfgGL.window.iWidth = config.iWidth;
	cfgGL.window.iHeight = config.iHeight;
	cfgGL.window.iMinWidth = config.iMinWidth;
	cfgGL.window.iMinHeight = config.iMinHeight;
	cfgGL.window.sTitle = config.sTitle;

	return m_pPrivateImpl->getGLApplication().execute(cfgGL);
}

bool lib::IGUIApplication::OnUpdate(float fElapsedTime)
{
	auto& oGraph = getGraph();
	oGraph.clear();

	for (auto p : m_oControls)
	{
		p->addToGraph(oGraph);
	}

	return true;
}

void lib::IGUIApplication::OnResize(unsigned iWidth, unsigned iHeight)
{
	for (auto& oCtrl : m_oControls)
	{
		oCtrl->OnParentResize(iWidth, iHeight);
	}
}

unsigned lib::IGUIApplication::getWidth() const { return m_pPrivateImpl->getGLWindow().width(); }

unsigned lib::IGUIApplication::getHeight() const { return m_pPrivateImpl->getGLWindow().height(); }

lib::GUIGraph& lib::IGUIApplication::getGraph()
{
	return m_pPrivateImpl->getGraph();
}
