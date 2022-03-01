#pragma once
#ifndef ROBINLE_FONTFACEDESIGNER_GUI
#define ROBINLE_FONTFACEDESIGNER_GUI





#include <functional>
#include <string>
#include <Windows.h>



#include <rl/graphics.fonts.bitmap.reader.hpp>
#include <rl/graphics.opengl.texture.hpp>
#define ROBINLE_OPENGL_FUNCTIONS
#include <rl/graphics.opengl.types.hpp>
#include <rl/input.keyboard.hpp>
#include <rl/input.mouse.hpp>

#include "constants.hpp"
#include "types.hpp"





using uint8_t = unsigned char;
using uint16_t = unsigned short;



namespace gui
{
	const uint16_t iWidth = 700;
	const uint16_t iHeight = 400;

	const uint16_t iCenterX = iWidth / 2;
	const uint16_t iCenterY = iHeight / 2;

	const uint8_t iScalingFactor = 2;



	inline rl::OpenGLRect GetRect(int left, int top, int width, int height)
	{
		return rl::OpenGL::GetPixelRect(iWidth, iHeight, left, top, width, height);
	}





	enum class ControlMouseState
	{
		Normal,
		Hovering,
		Clicking
	};



	class IControl
	{
	public: // interface methods

		virtual void drawToScreen() = 0;


	public: // methods

		void setPosition(uint16_t x, uint16_t y);
		inline void setVisible(bool visible) { m_bVisible = visible; }
		inline void setEnabled(bool enabled) { m_bEnabled = enabled; }

		void click();
		void setMouseState(ControlMouseState state);
		void setMouseStateAuto(const rl::MouseState& state);


	protected: // event handlers

		virtual void OnMouseEnter() {}
		virtual void OnMouseLeave() {}
		virtual void OnClick() {}


	protected: // methods

		void calculateRect();

		inline const rl::OpenGLRect& getRect() { return m_oRect; }


	protected: // variables

		uint16_t m_iX = 0;
		uint16_t m_iY = 0;
		uint16_t m_iWidth = 0;
		uint16_t m_iHeight = 0;
		ControlMouseState m_eMouseState = ControlMouseState::Normal;
		bool m_bVisible = true;
		bool m_bEnabled = true;


	private: // variables

		rl::OpenGLRect m_oRect = {};

	};



	/// <summary>
	/// A simple glyph<para />
	/// Always draws the glyph in its original size
	/// </summary>
	class Glyph : public IControl
	{
	public: // interface methods

		void drawToScreen() override;


	public: // methods

		void setGlyph(const rl::OpenGLTexture* glyph);


	private: // variables

		const rl::OpenGLTexture* m_pGlyph = nullptr;

	};



	/// <summary>
	/// A button with no text but a glyph
	/// </summary>
	class GlyphButton : public IControl
	{
	public: // interface methods

		void drawToScreen() override;


	public: // methods

		GlyphButton(int width, int height, bool flat);

		void setGlyph(const rl::OpenGLTexture* glyph);

		inline void setFlat(bool flat) { m_bFlat = flat; }

		void setSize(int width, int height);

		inline void setOnClick(std::function<void()> fnOnClick) { m_fnOnClick = fnOnClick; }


	private: // methods

		void drawGlyphToClientTex();

		void OnClick() override;


	private: // variables

		const rl::OpenGLTexture* m_pGlyph = nullptr;
		bool m_bFlat = false;
		std::function<void()> m_fnOnClick = nullptr;

		rl::OpenGLTexture m_oBaseTexNormal;
		rl::OpenGLTexture m_oBaseTexClick;
		rl::OpenGLTexture m_oGlyphTex; // cropped glyph

	};



	/// <summary>
	/// A button with text
	/// </summary>
	class TextButton : public IControl
	{
	public: // interface methods

		void drawToScreen() override;


	public: // methods

		TextButton(int width, int height, const rl::FontFaceClass* font, const wchar_t* szCaption);

		inline void setOnClick(std::function<void()> fnOnClick) { m_fnOnClick = fnOnClick; }


	private: // methods

		void OnClick() override;


	private: // variables

		std::function<void()> m_fnOnClick = nullptr;

		const rl::FontFaceClass* m_pFont = nullptr;
		std::wstring m_sCaption;

		rl::OpenGLTexture m_oBaseTexNormal;
		rl::OpenGLTexture m_oBaseTexClick;
		rl::OpenGLTexture m_oCaptionTex;
	};



	/// <summary>
	/// A label
	/// </summary>
	class Label : public IControl
	{
	public: // interface methods

		void drawToScreen() override;


	public: // methods

		void setFont(const rl::FontFaceClass* font, rl::Pixel color = rl::Color::White);
		void setCaption(const wchar_t* szCaption);
		void setDropShadow(bool DropShadow, rl::Pixel color = rl::Color::Black);
		void setFontScale(uint8_t scale);


	private: // methods

		void createTexture();


	private: // variables

		const rl::FontFaceClass* m_pFont = nullptr;
		std::wstring m_sCaption;
		rl::Pixel m_oColorFont = rl::Color::White;
		bool m_bDropShadow = false;
		rl::Pixel m_oColorDropShadow = rl::Color::Black;
		uint8_t m_iFontScale = 1;
		rl::OpenGLTexture m_oTex;

	};





	/// <summary>
	/// An enum for all windows accessible from within the RobinLe FontFace Designer
	/// </summary>
	enum class CurrentWindow
	{
		Editor,
		Create,
		Properties,
		Preview
	};


	/// <summary>
	/// A managing class for OpenGL controls
	/// </summary>
	class GUI
	{
	public: // methods

		GUI(HWND hWnd);
		~GUI();

		void processInput(const rl::Keyboard& kb, const rl::MouseState& mouse);
		void drawGraphics();


	private: // types

		enum IconID
		{
			Create,
			Open,
			Save,
			SaveAs,
			Undo,
			Redo,
			Properties,
			Preview,
			AddChar,
			DelChar
		};

		rl::OpenGLTexture m_oIcons[11] =
		{
			HeaderButtonIconToTexture(hmiCreate),
			HeaderButtonIconToTexture(hmiOpen),
			HeaderButtonIconToTexture(hmiSave),
			HeaderButtonIconToTexture(hmiSaveAs),
			HeaderButtonIconToTexture(hmiUndo),
			HeaderButtonIconToTexture(hmiRedo),
			HeaderButtonIconToTexture(hmiProperties),
			HeaderButtonIconToTexture(hmiPreview),
			HeaderButtonIconToTexture(hmiAddChar),
			HeaderButtonIconToTexture(hmiDelChar)
		};


	private: // methods

		void create();
		void open();
		void save() {}
		void saveAs() {}
		void undo() {}
		void redo() {}
		void properties() {}
		void preview() {}
		void addChar() {}
		void delChar() {}


		void clearEditorMouse();


	private: // variables

		rl::FontFaceClass m_oGUIFont;

		rl::OpenGLTexture m_oTexBG;
		rl::OpenGLTexture m_oTexBlack;
		rl::OpenGLTexture m_oTexMenuDivisor;
		rl::OpenGLTexture m_oTexMenuBG;
		rl::OpenGLTexture m_oTexWinBG;

		std::vector<IControl*> m_oControls_Editor_Menu;
		std::vector<IControl*> m_oControls_PopupWin;
		std::vector<IControl*> m_oControls_Create;
		CurrentWindow m_eWin = CurrentWindow::Editor;


		GlyphButton* btCreate = nullptr;
		GlyphButton* btOpen = nullptr;
		GlyphButton* btSave = nullptr;
		GlyphButton* btSaveAs = nullptr;
		GlyphButton* btUndo = nullptr;
		GlyphButton* btRedo = nullptr;
		GlyphButton* btProperties = nullptr;
		GlyphButton* btPreview = nullptr;
		GlyphButton* btAddChar = nullptr;
		GlyphButton* btDelChar = nullptr;

		Label* lbTest = nullptr;


	private: // constants

		const HWND m_hWnd;
	};

}





#endif // ROBINLE_FONTFACEDESIGNER_GUI