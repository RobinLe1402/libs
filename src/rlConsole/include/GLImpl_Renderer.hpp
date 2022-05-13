#pragma once
#ifndef ROBINLE_CONSOLE_GLIMPL_RENDERER
#define ROBINLE_CONSOLE_GLIMPL_RENDERER





#include "rl/lib/rlOpenGL/Core.hpp"
#include "rl/lib/rlOpenGL/Texture.hpp"

namespace GL = rl::OpenGL;

namespace impl
{
	class rlConsole; // forward declaration for "PImpl.hpp"
}

class ConsoleRenderer : public GL::IRenderer
{
public: // methods

	ConsoleRenderer(impl::rlConsole* pConsole) : m_pConsole(pConsole) {}
	virtual ~ConsoleRenderer() = default;

	void OnCreate() override;
	void OnUpdate(const void* pGraph) override;


private: // variables

	impl::rlConsole* m_pConsole;
	GL::Texture m_oTex;

};





#endif // ROBINLE_CONSOLE_GLIMPL_RENDERER
