#include "include/GLImpl_Application.hpp"

#include "rl/lib/rlGUI/Core.hpp"
#include "include/PrivateImpl.hpp"

namespace lib = rl::GLGUI;



bool GLImpl_Application::OnStart()
{
	m_pPrivateImpl->getGUIApp().OnCreate();
	return true;
}

bool GLImpl_Application::OnUpdate(float fElapsedTime)
{
	return m_pPrivateImpl->getGUIApp().OnUpdate(fElapsedTime);
}

bool GLImpl_Application::OnStop()
{
	return m_pPrivateImpl->getGUIApp().OnDestroy();
}

void GLImpl_Application::OnResize(LONG& iWidth, LONG& iHeight)
{
	m_pPrivateImpl->getGUIRenderer().OnResize(iWidth, iHeight);
	m_pPrivateImpl->getGUIApp().OnResize(iWidth, iHeight);
}

void GLImpl_Application::createGraph(void** pGraph)
{
	*pGraph = new lib::GUIGraph;
}

void GLImpl_Application::copyGraph(void* pDest, const void* pSource)
{
	auto& oDest = *static_cast<lib::GUIGraph*>(pDest);
	auto& oSrc = *static_cast<const lib::GUIGraph*>(pSource);

	// delete cached data
	for (auto p : oDest)
	{
		delete p;
	}
	oDest.clear();

	// clone current data
	oDest.reserve(oSrc.size());
	for (auto p : oSrc)
	{
		oDest.push_back(p->clone());
	}
}

void GLImpl_Application::destroyGraph(void* pGraph)
{
	delete static_cast<lib::GUIGraph*>(pGraph);
}
