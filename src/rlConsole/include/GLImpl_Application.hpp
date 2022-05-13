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

	void handleResize(unsigned iColumns, unsigned iRows);


private: // methods

	bool OnStart() override;
	bool OnUpdate(float fElapsedTime) override;
	void OnResize(LONG& iWidth, LONG& iHeight) override;
	bool OnStop() override;

	void createGraph(void** pGraph) override;
	void copyGraph(void* pDest, const void* pSource) override;
	void destroyGraph(void* pGraph) override;


private: // variables

	impl::rlConsole* m_pConsole;
	bool m_bRunning = false;

};





#endif // ROBINLE_CONSOLE_GLIMPL_APPLICATION