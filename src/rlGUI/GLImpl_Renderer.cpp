#include "include/GLImpl_Renderer.hpp"

#include <cassert>

#include <gl/GL.h>



GLImpl_Renderer::GLImpl_Renderer(RenderEvent fnRendering) :
	m_fnRendering(fnRendering)
{
	assert(fnRendering != nullptr);
}

void GLImpl_Renderer::OnCreate()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLImpl_Renderer::OnUpdate(const void* pGraph)
{
	m_fnRendering(*static_cast<const lib::GUIGraph*>(pGraph));
}
