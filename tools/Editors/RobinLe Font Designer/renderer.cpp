#include "include/renderer.hpp"

// RobinLe includes
#include <gl/GL.h>

// STL includes
#include <cassert>
#include <cstdint>



ButtonTextures::ButtonTextures(unsigned iWidth, unsigned iHeight) :
	m_iWidth(iWidth), m_iHeight(iHeight)
{
	assert(iWidth > 2 && iHeight > 2);

	m_oTexNormal.create(iWidth, iHeight, Color::ButtonFace);
	m_oTexHover.create(iWidth, iHeight, Color::ButtonFace_Hover);
	m_oTexClick.create(iWidth, iHeight, Color::ButtonFace_Click);

	// draw top and bottom border
	for (unsigned iX = 0; iX < iWidth; ++iX)
	{
		m_oTexNormal.setPixel(iX, 0, Color::ButtonBorder);
		m_oTexHover.setPixel(iX, 0, Color::ButtonBorder_Hover);
		m_oTexClick.setPixel(iX, 0, Color::ButtonBorder_Click);

		m_oTexNormal.setPixel(iX, iHeight - 1, Color::ButtonBorder);
		m_oTexHover.setPixel(iX, iHeight - 1, Color::ButtonBorder_Hover);
		m_oTexClick.setPixel(iX, iHeight - 1, Color::ButtonBorder_Click);
	}
	// draw left and right border
	for (unsigned iY = 1; iY < iHeight - 1; ++iY)
	{
		m_oTexNormal.setPixel(0, iY, Color::ButtonBorder);
		m_oTexHover.setPixel(0, iY, Color::ButtonBorder_Hover);
		m_oTexClick.setPixel(0, iY, Color::ButtonBorder_Click);

		m_oTexNormal.setPixel(iWidth - 1, iY, Color::ButtonBorder);
		m_oTexHover.setPixel(iWidth - 1, iY, Color::ButtonBorder_Hover);
		m_oTexClick.setPixel(iWidth - 1, iY, Color::ButtonBorder_Click);
	}
}

void ButtonTextures::upload()
{
	m_oTexNormal.upload();
	m_oTexHover.upload();
	m_oTexClick.upload();
}



void Renderer::OnCreate()
{
	// initialize OpenGL
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(
		Color::Background.r / 255.0f, // R
		Color::Background.g / 255.0f, // G
		Color::Background.b / 255.0f, // B
		1.0f // alpha
	);





	// create the font textures
	createTextureFont(m_oFont_Black, Font::Font, gl::Color::Black);



	// create other textures

	m_oTex_Footer_BG.create(1, iFooterHeight, Color::FooterClient);
	m_oTex_Footer_BG.setScalingMethod(gl::TextureScalingMethod::NearestNeighbor);
	for (int i = 0; i < iFooterBorderHeight; ++i)
		m_oTex_Footer_BG.setPixel(0, i, Color::FooterBorder);
	m_oTex_Footer_BG.upload();

	// initialize constant coordinates of the status bar
	{
		auto& screen = m_oTexCoord_StatusBarBG.screen;
		screen.topLeft.fX = screen.bottomLeft.fX = -1.0f; // left
		screen.topRight.fX = screen.bottomRight.fX = 1.0f; // right
		screen.bottomLeft.fY = screen.bottomRight.fY = -1.0f; // bottom
	}

	m_oBtTex_Footer.upload();
}

void Renderer::OnUpdate(const void* pGraph)
{
	glClear(GL_COLOR_BUFFER_BIT);

	m_pGraph = static_cast<const Graph*>(pGraph);

	drawText(m_oFont_Black, "Hold on... if love is the answer you're home <3", 0, 0);

	drawStatusBar();
}

void Renderer::createTextureFont(TextureFont& oDest, const Font::BitmapASCIIFont& oSource,
	gl::Pixel px)
{
	for (size_t iChar = 0; iChar < Font::iCharCount; ++iChar)
	{
		auto& oChar = oDest[iChar];

		oChar.create(Font::iCharWidth, Font::iCharHeight);
		oChar.setScalingMethod(gl::TextureScalingMethod::NearestNeighbor);
		oChar.setWrapMethod(gl::TextureWrapMethod::Clamp);

		for (uint8_t iY = 0; iY < Font::iCharHeight; ++iY)
		{
			Font::BitmapCharRow iRow = oSource[iChar][iY];

			for (int8_t iX = Font::iCharWidth - 1; iX >= 0; --iX)
			{
				if (iRow & 1)
					oChar.setPixel(iX, iY, px);
				iRow >>= 1;
			}
		}

		oChar.upload();
	}
}

void Renderer::drawText(const TextureFont& oFont, const char* szText, unsigned iX, unsigned iY)
{
	if (iY >= height())
		return; // text is not visible

	gl::TextureDrawingCoordinates oCoords = { {}, gl::FullTexture };
	oCoords.screen.topLeft.fY = oCoords.screen.topRight.fY = gl::PixelToViewport_Y(iY, height());
	oCoords.screen.bottomLeft.fY = oCoords.screen.bottomRight.fY =
		gl::PixelToViewport_Y(iY + Font::iCharHeight, height());

	const size_t len = strlen(szText);
	for (size_t i = 0; i < len; ++i)
	{
		if (iX >= width())
			break; // remaining characters are not visible

		oCoords.screen.topLeft.fX = oCoords.screen.bottomLeft.fX =
			gl::PixelToViewport_X(iX, width());
		oCoords.screen.topRight.fX = oCoords.screen.bottomRight.fX =
			gl::PixelToViewport_X(iX + Font::iCharWidth, width());

		unsigned char cRenderedChar = szText[i];
		if (cRenderedChar > Font::iCharEnd || cRenderedChar < Font::iCharBegin)
			cRenderedChar = '?';

		oFont[cRenderedChar - Font::iCharBegin].draw(oCoords);

		iX += Font::iCharWidth;
	}
}

void Renderer::drawStatusBar()
{
	auto& screen = m_oTexCoord_StatusBarBG.screen;
	screen.topLeft.fY = screen.topRight.fY =
		gl::PixelToViewport_Y(
			height() - (m_oTex_Footer_BG.height() * m_pGraph->iPixelScale),
			height());


	// draw the status bar background
	m_oTex_Footer_BG.draw(m_oTexCoord_StatusBarBG);

	// draw the status text
	drawText(m_oFont_Black, m_pGraph->sStatus.c_str(), iFooterStatusX,
		height() - iFooterHeight + iFooterStatusY);
}
