#include "include/GLImpl_Renderer.hpp"

#include "include/PImpl.hpp"
#include "include/Graph.hpp"
#include "rl/dll/rlConsole/Font.h"

#include <gl/GL.h>



void ConsoleRenderer::OnCreate()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ConsoleRenderer::OnUpdate(const void* pGraph)
{
	auto& oGraph = *static_cast<const ConsoleGraph*>(pGraph);
	if (!m_oTex || m_oTex.width() != width() || m_oTex.height() != height())
	{
		m_oTex.create(width(), height(), GL::Pixel::ByRGB(m_pConsole->getBackground()));
		m_oTex.setScalingMethod(GL::TextureScalingMethod::NearestNeighbor);
		m_oTex.setTransparency(false);
	}

	size_t iBaseY = 0;
	for (size_t iY = 0; iY < oGraph.iRows; ++iY)
	{
		size_t iBaseX = 0;
		for (size_t iX = 0; iX < oGraph.iColumns; ++iX)
		{
			auto& oCharInfo = oGraph.pBuf[iY * oGraph.iColumns + iX];
			auto& oChar = *m_pConsole->getFont().getChar(oCharInfo.c);

			for (size_t iCharY = 0; iCharY < rlConsole_Font_Char_Height; ++iCharY)
			{
				rlConsole_Font_Char_Row iRow = oChar.iData[iCharY];

				for (size_t iCharX = rlConsole_Font_Char_Width; iCharX > 0; --iCharX)
				{
					rlConsole_Color cl;
					if (iRow & 1)
						cl = oCharInfo.clForeground;
					else
						cl = oCharInfo.clBackground;

					m_oTex.setPixel(GLuint(iBaseX + iCharX - 1), GLuint(iBaseY + iCharY),
						GL::Pixel::ByRGB(cl));

					iRow >>= 1;
				}
			}

			iBaseX += rlConsole_Font_Char_Width;
		}

		iBaseY += rlConsole_Font_Char_Height;
	}

	m_oTex.upload();
	m_oTex.draw({ GL::FullScreen, GL::FullTexture });
}
