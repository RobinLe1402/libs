#pragma once
#ifndef ROBINLE_FONT_DESIGNER__APPLICATION
#define ROBINLE_FONT_DESIGNER__APPLICATION





// RobinLe includes
#include "rl/lib/rlOpenGL/Core.hpp"
#include "rl/splashscreen.hpp"

// project includes
#include "graph.hpp"


namespace gl = rl::OpenGL;



class Application : public gl::IApplication
{
public: // methods

	// use same constructors
	using gl::IApplication::IApplication;


private: // methods

	bool OnStart() override;

	bool OnUpdate(float fElapsedTime) override;

	void OnResize(LONG& iWidth, LONG& iHeight) override;

	void createGraph(void** pGraph) override { *pGraph = new Graph{}; }

	void copyGraph(void* pDest, const void* pSource) override
	{
		auto& oDest = *static_cast<Graph*>(pDest);
		auto& oSrc = *static_cast<const Graph*>(pSource);

		oDest = oSrc;
	}

	void destroyGraph(void* pGraph) override { delete static_cast<Graph*>(pGraph); }

};





#endif // ROBINLE_FONT_DESIGNER__APPLICATION
