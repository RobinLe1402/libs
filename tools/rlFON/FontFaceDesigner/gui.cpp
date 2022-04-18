#include "gui.hpp"
#include "resource.h"

#include <memory>
#include <Windows.h>

#define ROBINLE_OPENGL_FUNCTIONS
#include "rl/graphics.opengl.types.hpp"
#include "rl/unicode.hpp"

#include "constants.hpp"


namespace gui
{

	//==============================================================================================
	// IControl

	void IControl::setPosition(uint16_t x, uint16_t y)
	{
		m_iX = x;
		m_iY = y;

		calculateRect();
	}

	void IControl::click()
	{
		if (!m_bEnabled)
			return;

		OnClick();
	}

	void IControl::setMouseState(ControlMouseState state)
	{
		if (m_eMouseState == state)
			return;

		switch (state)
		{
			// normal ==> mouse left
		case ControlMouseState::Normal:
			OnMouseLeave();
			break;


			// hovering
		case ControlMouseState::Hovering:
			switch (m_eMouseState)
			{
				// normal --> hovering ==> mouse entered
			case ControlMouseState::Normal:
				OnMouseEnter();
				break;

				// clicking --> hovering ==> registered as clicking
			case ControlMouseState::Clicking:
				click();
				break;
			}
			break;
		}


		m_eMouseState = state;
	}

	void IControl::setMouseStateAuto(const rl::MouseState& state)
	{
		bool bOnControl;

		if (!state.bOnClient)
			bOnControl = false;
		else
			bOnControl = state.x >= m_iX && state.x <= m_iX + m_iWidth &&
			state.y >= m_iY && state.y <= m_iY + m_iHeight;


		if (!bOnControl)
			setMouseState(ControlMouseState::Normal);
		else
		{
			if (state.left.bPressed || state.left.bHeld)
				setMouseState(ControlMouseState::Clicking);
			else
				setMouseState(ControlMouseState::Hovering);
		}
	}

	void IControl::calculateRect()
	{
		if (m_iWidth && m_iHeight)
			m_oRect = GetRect(m_iX, m_iY, m_iWidth, m_iHeight);
		else
			m_oRect = { 0, 0, 0, 0 };
	}





	//==============================================================================================
	// Glyph

	void Glyph::drawToScreen()
	{
		if (!m_bVisible || m_iWidth == 0)
			return;


		m_pGlyph->drawToScreen(getRect());
	}

	void Glyph::setGlyph(const rl::OpenGLTexture* glyph)
	{
		m_pGlyph = glyph;

		// no data --> size = { 0, 0 }
		if (!m_pGlyph || !m_pGlyph->isValid())
		{
			m_iWidth = 0;
			m_iHeight = 0;
		}

		// valid data
		else
		{
			m_iWidth = m_pGlyph->getWidth();
			m_iHeight = m_pGlyph->getHeight();
			calculateRect();
		}
	}





	//==============================================================================================
	// GlyphButton

	void GlyphButton::drawToScreen()
	{
		if (!m_bVisible || m_iWidth == 0)
			return;

		// draw outline
		if (m_bEnabled)
		{
			switch (m_eMouseState)
			{
			case ControlMouseState::Normal:
				if (m_bFlat) // flat --> only drawn when hovering or clicking
					break;

				m_oBaseTexNormal.drawToScreen(getRect());
				break;

			case ControlMouseState::Hovering:
				m_oBaseTexNormal.drawToScreen(getRect());
				break;

			case ControlMouseState::Clicking:
				m_oBaseTexClick.drawToScreen(getRect());
				break;
			}
		}

		if (m_bEnabled)
			m_oGlyphTex.drawToScreen(getRect());
		else
			m_oGlyphTex.drawToScreen(getRect(), 0.5f);
	}

	GlyphButton::GlyphButton(int width, int height, bool flat)
	{
		setSize(width, height);
		setFlat(flat);
	}

	void GlyphButton::setGlyph(const rl::OpenGLTexture* glyph)
	{
		m_pGlyph = glyph;

		drawGlyphToClientTex();
	}

	void GlyphButton::setSize(int width, int height)
	{
		m_iWidth = width;
		m_iHeight = height;
		calculateRect();


		// at least one parameter zero?
		if (m_iWidth == 0 || m_iHeight == 0)
		{
			// --> make sure that both variables are zero
			m_iWidth = 0;
			m_iHeight = 0;

			return;
		}



		// recreate base textures

		const rl::OpenGLTexture_Config& cfg = rl::OpenGL::TextureConfig_Pixelart;

		m_oBaseTexNormal.create(m_iWidth, m_iHeight, cfg, colButtonFace);
		m_oBaseTexClick.create(m_iWidth, m_iHeight, cfg, colButtonFaceClicked);

		// edge points
		m_oBaseTexNormal.setPixel(m_iWidth - 1, 0, colButtonOutlineEdge);
		m_oBaseTexNormal.setPixel(0, m_iHeight - 1, colButtonOutlineEdge);
		m_oBaseTexClick.setPixel(m_iWidth - 1, 0, colButtonOutlineEdge);
		m_oBaseTexClick.setPixel(0, m_iHeight - 1, colButtonOutlineEdge);



		// horizontal
		for (int iX = 0; iX < m_iWidth; ++iX)
		{
			if (iX < m_iWidth - 1)
				m_oBaseTexNormal.setPixel(iX, 0, colButtonOutlineBright);
			if (iX > 0)
				m_oBaseTexNormal.setPixel(iX, m_iHeight - 1, colButtonOutlineDark);

			if (iX < m_iWidth - 1)
				m_oBaseTexClick.setPixel(iX, 0, colButtonOutlineDark);
			if (iX > 0)
				m_oBaseTexClick.setPixel(iX, m_iHeight - 1, colButtonOutlineBright);
		}

		// vertical
		for (int iY = 1; iY < m_iHeight - 1; ++iY)
		{
			m_oBaseTexNormal.setPixel(0, iY, colButtonOutlineBright);
			m_oBaseTexNormal.setPixel(m_iWidth - 1, iY, colButtonOutlineDark);

			m_oBaseTexClick.setPixel(0, iY, colButtonOutlineDark);
			m_oBaseTexClick.setPixel(m_iWidth - 1, iY, colButtonOutlineBright);
		}


		m_oBaseTexNormal.upload();
		m_oBaseTexClick.upload();

		m_oGlyphTex.create(m_iWidth, m_iHeight, cfg);
		drawGlyphToClientTex();

	}

	void GlyphButton::drawGlyphToClientTex()
	{
		if (m_iWidth == 0 || !m_pGlyph)
			return;

		GLsizei iX, iY;

		const auto iGlyphWidth = m_pGlyph->getWidth();
		const auto iGlyphHeight = m_pGlyph->getHeight();

		const auto iClientWidth = m_oGlyphTex.getWidth();
		const auto iClientHeight = m_oGlyphTex.getHeight();


		if (iClientWidth == iGlyphWidth)
			iX = 0;
		else
			iX = iClientWidth / 2 - iGlyphWidth / 2;

		if (iClientHeight == iGlyphHeight)
			iY = 0;
		else
			iY = iClientHeight / 2 - iGlyphHeight / 2;

		m_oGlyphTex.clear();
		m_oGlyphTex.draw(*m_pGlyph, iX, iY, true);
		m_oGlyphTex.upload();
	}

	void GlyphButton::OnClick()
	{
		if (m_fnOnClick)
			m_fnOnClick();
	}





	//==============================================================================================
	// TextButton

	void TextButton::drawToScreen()
	{
		if (!m_bVisible || m_iWidth == 0)
			return;

		if (m_bEnabled)
		{
			switch (m_eMouseState)
			{
			case ControlMouseState::Normal:
				m_oBaseTexNormal.drawToScreen(getRect());
				break;

			case ControlMouseState::Hovering:
				m_oBaseTexNormal.drawToScreen(getRect());
				break;

			case ControlMouseState::Clicking:
				m_oBaseTexClick.drawToScreen(getRect());
				break;
			}
		}

		if (m_bEnabled)
			m_oCaptionTex.drawToScreen(getRect());
		else
			m_oCaptionTex.drawToScreen(getRect(), 0.5f);
	}

	TextButton::TextButton(int width, int height, const rl::FontFaceClass* font,
		const wchar_t* szCaption)
	{
		m_iWidth = width;
		m_iHeight = height;
		calculateRect();


		// at least one parameter zero?
		if (m_iWidth == 0 || m_iHeight == 0)
		{
			// --> make sure that both variables are zero
			m_iWidth = 0;
			m_iHeight = 0;

			return;
		}


		m_pFont = font;
		m_sCaption = szCaption;



		// recreate base textures

		const rl::OpenGLTexture_Config& cfg = rl::OpenGL::TextureConfig_Pixelart;

		m_oBaseTexNormal.create(m_iWidth, m_iHeight, cfg, colButtonFace);
		m_oBaseTexClick.create(m_iWidth, m_iHeight, cfg, colButtonFaceClicked);

		// edge points
		m_oBaseTexNormal.setPixel(m_iWidth - 1, 0, colButtonOutlineEdge);
		m_oBaseTexNormal.setPixel(0, m_iHeight - 1, colButtonOutlineEdge);
		m_oBaseTexClick.setPixel(m_iWidth - 1, 0, colButtonOutlineEdge);
		m_oBaseTexClick.setPixel(0, m_iHeight - 1, colButtonOutlineEdge);



		// horizontal
		for (int iX = 0; iX < m_iWidth; ++iX)
		{
			if (iX < m_iWidth - 1)
				m_oBaseTexNormal.setPixel(iX, 0, colButtonOutlineBright);
			if (iX > 0)
				m_oBaseTexNormal.setPixel(iX, m_iHeight - 1, colButtonOutlineDark);

			if (iX < m_iWidth - 1)
				m_oBaseTexClick.setPixel(iX, 0, colButtonOutlineDark);
			if (iX > 0)
				m_oBaseTexClick.setPixel(iX, m_iHeight - 1, colButtonOutlineBright);
		}

		// vertical
		for (int iY = 1; iY < m_iHeight - 1; ++iY)
		{
			m_oBaseTexNormal.setPixel(0, iY, colButtonOutlineBright);
			m_oBaseTexNormal.setPixel(m_iWidth - 1, iY, colButtonOutlineDark);

			m_oBaseTexClick.setPixel(0, iY, colButtonOutlineDark);
			m_oBaseTexClick.setPixel(m_iWidth - 1, iY, colButtonOutlineBright);
		}


		m_oBaseTexNormal.upload();
		m_oBaseTexClick.upload();





		m_oCaptionTex.create(m_iWidth, m_iHeight, cfg);

		auto& oFont = *m_pFont;

		const auto iHeight = oFont.getHeight();
		const auto cFallback = oFont.getFallback();
		const auto iWidthFallback = oFont.getCharWidth(cFallback);

		auto up_szCaption32 = std::make_unique<char32_t[]>((size_t)m_sCaption.length() + 1);
		auto szCaption32 = up_szCaption32.get();
		memset(szCaption32, 0, (m_sCaption.length() + 1) * sizeof(char32_t));
	}

	void TextButton::OnClick()
	{
		if (m_fnOnClick)
			m_fnOnClick();
	}





	//==============================================================================================
	// Label

	void Label::drawToScreen()
	{
		if (!m_oTex.isValid())
			return;

		if (!m_oTex.uploaded())
			m_oTex.upload();
		m_oTex.drawToScreen(getRect());
	}

	void Label::setFont(const rl::FontFaceClass* font, rl::Pixel color)
	{
		m_pFont = font;
		m_oColorFont = color;

		createTexture();
	}

	void Label::setCaption(const wchar_t* szCaption)
	{
		m_sCaption = szCaption;
		createTexture();
	}

	void Label::setDropShadow(bool DropShadow, rl::Pixel color)
	{
		m_bDropShadow = DropShadow;
		m_oColorDropShadow = color;

		createTexture();
	}

	void Label::setFontScale(uint8_t scale)
	{
		if (scale == 0)
			return;

		m_iFontScale = scale;
		createTexture();
	}

	void Label::createTexture()
	{
		uint16_t iWidthRaw = 0;
		uint16_t iHeightRaw = 0;

		// making sure that m_bDropShadow is either 0 or 1 (is used as integer later on)
		m_bDropShadow = (m_bDropShadow ? 1 : 0);

		m_iWidth = 0;
		m_iHeight = 0;

		if (m_sCaption.empty() || !m_pFont || !m_pFont->hasData())
		{
			m_oTex.destroy(); // making sure nothing will be drawn
			return;
		}

		auto& oFont = *m_pFont;

		iHeightRaw = oFont.getHeight();
		m_iHeight = iHeightRaw;
		if (m_bDropShadow)
			++m_iHeight;
		m_iHeight *= m_iFontScale;

		const auto cFallback = oFont.getFallback();
		const uint16_t iWidthFallback = oFont.getCharWidth(cFallback);

		size_t len = 0;
		auto sp_szCaption32 = rl::UTF16::DecodeString(m_sCaption.c_str(), &len);
		auto szCaption32 = sp_szCaption32.get();


		// calculate the total width
		for (size_t i = 0; i < len; ++i)
		{
			char32_t& c = szCaption32[i];

			auto pCh = oFont.findChar(c);
			if (!pCh)
			{
				iWidthRaw += iWidthFallback;
				c = cFallback;
			}
			else
				iWidthRaw += pCh->iWidth;
		}
		m_iWidth = iWidthRaw;
		if (m_bDropShadow)
			++m_iWidth;
		m_iWidth *= m_iFontScale;

		calculateRect();

		m_oTex.create(iWidthRaw + m_bDropShadow, iHeightRaw + m_bDropShadow,
			rl::OpenGL::TextureConfig_Pixelart);


		char32_t c = szCaption32[0];
		size_t iCh = 0;
		GLsizei iOffset = 0;
		while (c != 0)
		{
			auto& oCh = *oFont.findChar(c);

			// draw drop shadow
			if (m_bDropShadow)
			{
				for (uint16_t iX = 0; iX < oCh.iWidth; ++iX)
				{
					for (uint16_t iY = 0; iY < oFont.getHeight(); ++iY)
					{
						if (oFont.getPixel(c, iX, iY))
							m_oTex.setPixel(iOffset + iX + 1, iY + 1, m_oColorDropShadow);
					}
				}
			}

			// draw main text
			for (uint16_t iX = 0; iX < oCh.iWidth; ++iX)
			{
				for (uint16_t iY = 0; iY < oFont.getHeight(); ++iY)
				{
					if (oFont.getPixel(c, iX, iY))
						m_oTex.setPixel(iOffset + iX, iY, m_oColorFont);
				}
			}


			iOffset += oCh.iWidth;
			++iCh;
			c = szCaption32[iCh];
		}

		// upload --> on demand
	}





	//==============================================================================================
	// GUI

	GUI::GUI(HWND hWnd) : m_hWnd(hWnd)
	{
		// Load the GUI font
		// Must have a bit depth of 1
		if (!m_oGUIFont.loadFromResource(NULL, MAKEINTRESOURCEA(IDF_EDITOR)) ||
			m_oGUIFont.getBitsPerPixel() != 1)
			throw std::exception("There was a problem with the editor font");

		m_oTexBG.create(1, 1, rl::OpenGL::TextureConfig_Pixelart, colWorkspaceBG);
		m_oTexBG.upload();

		m_oTexBlack.create(1, 1, rl::OpenGL::TextureConfig_Pixelart, rl::Color::Black);
		m_oTexBlack.upload();

		m_oTexMenuDivisor.create(1, 16, rl::OpenGL::TextureConfig_Pixelart, colDivisor);
		m_oTexMenuDivisor.upload();

		m_oTexMenuBG.create(1, 25, rl::OpenGL::TextureConfig_Pixelart, colMenuBG);
		m_oTexMenuBG.setPixel(0, 24, colMenuOutline);
		m_oTexMenuBG.upload();

		m_oTexWinBG.create(1, 1, rl::OpenGL::TextureConfig_Pixelart, colMenuBG);
		m_oTexWinBG.upload();



		for (auto& oIcon : m_oIcons)
			oIcon.upload();

		const int iEditorMenuButtonOffsetTopLeft = 1;
		const int iEditorMenuButtonSize = 22;
		const int iEditorMenuButtonOffset = 2;
		const int iEditorMenuButtonGroupOffset = 10;

		int iOffset = iEditorMenuButtonOffsetTopLeft;

#define CREATE_BTN(Name, GlyphID, OnClick, Enabled)	\
			Name = new GlyphButton(iEditorMenuButtonSize, iEditorMenuButtonSize, true);	\
			Name->setPosition(iOffset, iEditorMenuButtonOffsetTopLeft);					\
			Name->setGlyph(m_oIcons + GlyphID);											\
			Name->setOnClick(OnClick);													\
			Name->setEnabled(Enabled);													\
			m_oControls_Editor_Menu.push_back(Name);									\
			iOffset += iEditorMenuButtonSize + iEditorMenuButtonOffset

		CREATE_BTN(btCreate, IconID::Create, [&]() { create(); }, true);

		CREATE_BTN(btOpen, IconID::Open, [&]() { open(); }, true);
		CREATE_BTN(btSave, IconID::Save, [&]() { save(); }, false);
		CREATE_BTN(btSaveAs, IconID::SaveAs, [&]() { saveAs(); }, false);

		iOffset += iEditorMenuButtonGroupOffset;

		CREATE_BTN(btUndo, IconID::Undo, [&]() { undo(); }, false);
		CREATE_BTN(btRedo, IconID::Redo, [&]() { redo(); }, false);

		iOffset += iEditorMenuButtonGroupOffset;

		CREATE_BTN(btProperties, IconID::Properties, [&]() { properties(); }, false);
		CREATE_BTN(btPreview, IconID::Preview, [&]() { preview(); }, false);

		iOffset += iEditorMenuButtonGroupOffset;

		CREATE_BTN(btAddChar, IconID::AddChar, [&]() { addChar(); }, false);
		CREATE_BTN(btDelChar, IconID::DelChar, [&]() { delChar(); }, false);

#undef CREATE_BTN

		lbTest = new Label;
		lbTest->setFont(&m_oGUIFont, rl::Color::White);
		lbTest->setFontScale(2);
		lbTest->setDropShadow(true, colOutlineNormal);
		lbTest->setCaption(L"TEST LABEL");
	}

	GUI::~GUI()
	{
		for (auto* p : m_oControls_Editor_Menu)
			delete p;
		m_oControls_Editor_Menu.clear();


		for (auto* p : m_oControls_Create)
			delete p;
		m_oControls_Create.clear();

		
		for (auto* p : m_oControls_PopupWin)
			delete p;
		m_oControls_PopupWin.clear();

		delete lbTest;
	}

	void GUI::processInput(const rl::Keyboard& kb, const rl::MouseState& mouse)
	{
		rl::MouseState oMouseScaled = mouse;
		oMouseScaled.x /= iScalingFactor;
		oMouseScaled.y /= iScalingFactor;

		switch (m_eWin)
		{
		case CurrentWindow::Editor: // in editor

			for (auto* p : m_oControls_Editor_Menu)
				p->setMouseStateAuto(oMouseScaled);

			// CTRL shortcuts
			if (kb.getKey(VK_CONTROL).bHeld)
			{
				// CTRL+N --> Create
				if (kb.getKey('N').bPressed)
					btCreate->click();

				// CTRL+O --> Open
				else if (kb.getKey('O').bPressed)
					btOpen->click();

				// CTRL+S --> Save/Save As...
				else if (kb.getKey('S').bPressed)
				{
					// CTRL+SHIFT+S --> Save As...
					if (kb.getKey(VK_SHIFT).bHeld)
						btSaveAs->click();
					// CTRL+S --> Save
					else
						btSave->click();
				}

				// CTRL+Z --> Undo
				else if (kb.getKey('Z').bPressed)
					btUndo->click();

				// CTRL+Y --> Redo
				else if (kb.getKey('Y').bPressed)
					btRedo->click();
			}

			break;



		default: // not in editor

			// F4 or ESC --> return to editor
			if (kb.getKey(VK_F4).bPressed || kb.getKey(VK_ESCAPE).bPressed)
			{
				m_eWin = CurrentWindow::Editor;
				break;
			}

			// handle general popup window controls
			for (auto* p : m_oControls_PopupWin)
				p->setMouseStateAuto(oMouseScaled);

			/*switch (m_eWin)
			{
				
			}*/
		}
	}

	void GUI::drawGraphics()
	{
		m_oTexBG.drawToScreen(rl::OpenGL::FullScreen);

		m_oTexMenuBG.drawToScreen(GetRect(0, 0, iWidth, m_oTexMenuBG.getHeight()));

		for (auto* p : m_oControls_Editor_Menu)
			p->drawToScreen();

		// todo: draw editor window

		if (m_eWin != CurrentWindow::Editor)
		{
			// draw shade
			m_oTexBlack.drawToScreen(rl::OpenGL::FullScreen, 0.75f);

			// draw window background
			m_oTexWinBG.drawToScreen(GetRect(iCenterX - iPopupWinCenterX,
				iCenterY - iPopupWinCenterY, iPopupWinWidth, iPopupWinHeight));

			// draw general popup window controls
			for (auto* p : m_oControls_PopupWin)
				p->drawToScreen();



			lbTest->setPosition(iCenterX - iPopupWinCenterX, iCenterY - iPopupWinCenterY);
			lbTest->drawToScreen();


			switch (m_eWin)
			{
			case CurrentWindow::Create:
				for (auto* p : m_oControls_Create)
					p->drawToScreen();
				break;
			}
		}
	}

	void GUI::create()
	{
		m_eWin = CurrentWindow::Create;
		clearEditorMouse();
	}

	void GUI::open()
	{
		wchar_t szPath[MAX_PATH + 1] = {};

		const wchar_t szTitle[] = L"Select font file to import";

		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFilter =
			L"Supported file formats (*.rlFNT;*.sprfnt;*.PXFNT)\0"
			L" *.rlFNT;*.sprfnt;*.PXFNT\0\0";
		ofn.lpstrFile = szPath;
		ofn.nMaxFile = MAX_PATH + 1;
		ofn.lpstrTitle = szTitle;
		ofn.Flags = OFN_FILEMUSTEXIST;

		if (!GetOpenFileNameW(&ofn))
			return;

		MessageBoxW(m_hWnd, szPath, L"Selected path", MB_ICONINFORMATION | MB_SYSTEMMODAL);
		// ToDo:
	}

	void GUI::clearEditorMouse()
	{
		for (auto* p : m_oControls_Editor_Menu)
			p->setMouseState(ControlMouseState::Normal);
	}

}
