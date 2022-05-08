#pragma once
#ifndef ROBINLE_LIB_GUI_GLIMPL_RENDERER
#define ROBINLE_LIB_GUI_GLIMPL_RENDERER





#include "rl/lib/rlGUI/Core.hpp"
#include "rl/lib/rlOpenGL/Core.hpp"

#include <functional>

namespace lib = rl::GLGUI;
namespace GL = rl::OpenGL;



class GLImpl_Renderer : public GL::IRenderer
{
public: // types

	using RenderEvent = std::function<void(const lib::GUIGraph& oGraph)>;


public: // methods

	GLImpl_Renderer(RenderEvent fnRendering);
	virtual ~GLImpl_Renderer() = default;


private: // methods

	void OnCreate() override;

	void OnUpdate(const void* pGraph) override;


private: // variables

	RenderEvent m_fnRendering;

};





#endif // ROBINLE_LIB_GUI_GLIMPL_RENDERER
