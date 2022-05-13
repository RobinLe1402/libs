#pragma once
#ifndef ROBINLE_CONSOLE_GLIMPL_APPLICATION
#define ROBINLE_CONSOLE_GLIMPL_APPLICATION





#include "rl/lib/rlOpenGL/Core.hpp"
#include "Graph.hpp"

namespace GL = rl::OpenGL;

namespace impl
{
	class rlConsole; // forward declaration for "PImpl.hpp"
}

class ConsoleApplication : public GL::IApplication
{
public: // methods

	ConsoleApplication(impl::rlConsole* pConsole);
	virtual ~ConsoleApplication();

	ConsoleGraph* getGraph() { return static_cast<ConsoleGraph*>(graph()); }

	bool isRunning() { return m_bRunning; }


private: // methods

	bool OnStart() override;
	bool OnUpdate(float fElapsedTime) override;
	void OnResizing(LONG& iWidth, LONG& iHeight) override;
	void OnResized(unsigned iWidth, unsigned iHeight) override;
	bool OnStop() override;

	void createGraph(void** pGraph) override;
	void copyGraph(void* pDest, const void* pSource) override;
	void destroyGraph(void* pGraph) override;

	void handleResize(unsigned iColumns, unsigned iRows);


private: // variables

	impl::rlConsole* m_pConsole;
	bool m_bRunning = false;

};





#endif // ROBINLE_CONSOLE_GLIMPL_APPLICATION