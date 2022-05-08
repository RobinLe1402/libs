#include "rl/lib/rlGUI/Core.hpp"
#include "rl/lib/rlGUI/CommonControls.hpp"

#include "rl/lib/rlOpenGL/Texture.hpp"

#include "include/PrivateImpl.hpp"

namespace lib = rl::GLGUI;
namespace ControlIDs = lib::StdControlTypes;

namespace GL = rl::OpenGL;



bool lib::GUIRenderer::render(const GUIGraph& oGraph)
{
	bool bAllRendered = true;

	for (auto p : oGraph)
	{
		switch (p->iType)
		{
		case ControlIDs::Panel:
		{
			auto& o = *static_cast<const PanelGraphNode*>(p);

			GL::Texture oTex;
			oTex.create(1, 1, o.oColor);
			oTex.setScalingMethod(GL::TextureScalingMethod::NearestNeighbor);
			oTex.upload();
			oTex.draw(
				{
					GL::TextureQuad::Rect(
						{
							GL::PixelToViewport_X(o.iX, getWidth()), // left
							GL::PixelToViewport_Y(o.iY, getHeight()) // top
						},
						{
							GL::PixelToViewport_X(o.iX + o.iWidth, getWidth()), // right
							GL::PixelToViewport_Y(o.iY + o.iHeight, getHeight()) // bottom
						}
					),
					GL::FullTexture
				});
			break;
		}

		default:
			bAllRendered = false;
		}
	}

	return bAllRendered;
}

unsigned lib::GUIRenderer::getWidth()
{
	return m_pPImpl->getGLWindow().width();
}

unsigned lib::GUIRenderer::getHeight()
{
	return m_pPImpl->getGLWindow().height();
}
