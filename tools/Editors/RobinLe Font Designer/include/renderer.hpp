#pragma once
#ifndef ROBINLE_FONT_DESIGNER__RENDERER
#define ROBINLE_FONT_DESIGNER__RENDERER





// RobinLe includes
#include "rl/lib/rlOpenGL/Core.hpp"
#include "rl/lib/rlOpenGL/Texture.hpp"

// project includes
#include "constants.hpp"
#include "graph.hpp"
#include "font.hpp"


namespace gl = rl::OpenGL;



class ButtonTextures
{
public: // methods

	ButtonTextures(unsigned iWidth, unsigned iHeight);
	~ButtonTextures() = default;

	void upload();

	const auto& getNormal() const { return m_oTexNormal; }
	const auto& getHover() const { return m_oTexHover; }
	const auto& getClick() const { return m_oTexClick; }

	auto getWidth() const { return m_iWidth; }
	auto getHeight() const { return m_iHeight; }


private: // variables

	const unsigned m_iWidth, m_iHeight;

	gl::Texture m_oTexNormal;
	gl::Texture m_oTexHover;
	gl::Texture m_oTexClick;

};


class Renderer : public gl::IRenderer
{
private: // types

	using TextureFont = gl::Texture[Font::iCharCount];


private: // methods

	void OnCreate() override;

	void OnUpdate(const void* pGraph) override;

	void createTextureFont(TextureFont& oDest, const Font::BitmapASCIIFont& oSource, gl::Pixel px);

	void drawText(const TextureFont& oFont, const char* szText, unsigned iX, unsigned iY);

	void drawStatusBar();


private: // variables

	const Graph* m_pGraph = nullptr;

	TextureFont m_oFont_Black;
	gl::Texture m_oTex_Footer_BG;
	ButtonTextures m_oBtTex_Footer{ iFooterButtonWidth, iButtonHeight };

	gl::TextureDrawingCoordinates m_oTexCoord_StatusBarBG = { {}, gl::FullTexture };

};





#endif // ROBINLE_FONT_DESIGNER__RENDERER
