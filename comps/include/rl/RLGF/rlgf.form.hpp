/***************************************************************************************************
 FILE:	rlgf.form.hpp
 CPP:	rlgf.form.cpp
 DESCR:	RobinLe GUI Framework - Forms
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GF_FORM
#define ROBINLE_GF_FORM

// class names: https://docs.microsoft.com/en-us/windows/win32/controls/common-control-window-classes





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
using uint32_t = unsigned int;


#include <functional>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{

	// forward declarations
	class Application;
	class IComponent;




	using Color = COLORREF;

	class Brush final
	{
	public: // operators

		Brush& operator=(Brush&& rval) noexcept;
		Brush& operator=(const Brush& other) noexcept;


	public: // methods

		Brush() noexcept;
		Brush(Color color) noexcept;
		Brush(Brush&& rval) noexcept;
		Brush(const Brush& other) noexcept;

		~Brush();



		inline HBRUSH getHandle() const noexcept { return m_hBrush; }

		Color getColor() const noexcept;

		void setColor(Color color) noexcept;


	private: // methods

		void free() noexcept;


	private: // variables

		HBRUSH m_hBrush;
	};
	// ToDo:
	// Pattern (device dependent/DIB)


	enum class PenStyle
	{
		Solid = PS_SOLID,
		Dash = PS_DASH,
		Dot = PS_DOT,
		DashDot = PS_DASHDOT,
		DashDotDot = PS_DASHDOTDOT
	};

	class Pen
	{
	private: // variables

		HPEN m_hPen;
		PenStyle m_eStyle;
	};
	// ToDo:
	// Implement





	/// <summary>
	/// Interface for Win32 components
	/// </summary>
	class IComponent
	{
		friend class Form;

	public: // methods

		IComponent(IComponent* owner) : m_pOwner(owner) {}

		inline const IComponent* getOwner() noexcept { return m_pOwner; }
		inline HWND getHandle() const noexcept { return m_hWnd; }


	protected: // method prototypes

		virtual void initialize() = 0;


	protected: // variables

		const IComponent* m_pOwner;
		HWND m_hWnd = NULL;

	};
	using Component = IComponent*;





	/// <summary>
	/// Interface for Win32 visual controls
	/// </summary>
	class IWinControl : public IComponent
	{
		friend class Form;
	private: // static methods

		static WORD getControlID();
		static void releaseControlID(WORD controlID);
		static LRESULT CALLBACK GlobalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // static variables

		static std::vector<uint64_t> oControlIDs;
		static std::map<HWND, IWinControl*> oControls;


	public: // methods

		IWinControl(Component owner, int left, int top);
		virtual ~IWinControl();

		inline int getLeft() const noexcept { return m_iLeft; }
		inline int getTop() const noexcept { return m_iTop; }

		void setParent(Component parent);

		std::function<void()> OnMouseEnter = nullptr;
		std::function<void()> OnMouseHover = nullptr;
		std::function<void()> OnMouseLeave = nullptr;


	protected: // methods

		inline HWND getParentHandle() const noexcept
		{
			return m_pParent ? m_pParent->getHandle() : NULL;
		}

		virtual void processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	protected: // variables

		const WORD m_dwControlID;
		int m_iLeft;
		int m_iTop;
		Component m_pParent = nullptr;


	private: // methods

		void addHWND(HWND hWnd);
		void removeHWND(HWND hWnd);

		void afterInitialization();


	private: // variables

		bool m_bMouseTracking = false;
		bool m_bHovering = false;
		WNDPROC m_pOriginalWndProc = nullptr;

	};
	using WinControl = IWinControl*;










	/// <summary>
	/// The output quality of a GDI font.
	/// </summary>
	enum class FontQuality
	{
		/// <summary>
		/// Appearance of thie font does not matter.
		/// </summary>
		Default = DEFAULT_QUALITY,
		/// <summary>
		/// Appearance of the font is less important than when the <c>PROOF_QUALITY</c> value is
		/// used. For GDI raster fonts, scaling is enabled, which means that more font sizes are
		/// available, but the quality may be lower. Bold, italic, underline, and strikeout
		/// fonts are synthesized, if necessary.
		/// </summary>
		Draft = DRAFT_QUALITY,
		/// <summary>
		/// Character quality of the font is more important than exact matching of the logical-font
		/// attributes. For GDI raster fonts, scaling is disabled and the font closest in size is
		/// chosen. Although the chosen font size may not be mapped exactly when
		/// <c>PROOF_QUALITY</c> is used, the quality of the font is high and there is no distortion
		/// of appearance. Bold, italic, underline, and strikeout fonts are synthesized, if
		/// necessary. 
		/// </summary>
		Proof = PROOF_QUALITY,
		/// <summary>
		/// Font is never antialiased, that is, font smoothing is not done.
		/// </summary>
		NonAntialiased = NONANTIALIASED_QUALITY,
		/// <summary>
		/// Font is antialiased, or smoothed, if the font supports it and the size of the font is
		/// not too small or too large. 
		/// </summary>
		Antialiased = ANTIALIASED_QUALITY,
		/// <summary>
		/// If set, text is rendered (when possible) using ClearType antialiasing method.
		/// </summary>
		ClearType = CLEARTYPE_QUALITY
	};
	
	struct FontConfig
	{
		std::wstring sFontName = L"Segoe UI";
		int iHeight = -12;
		int iWeight = FW_REGULAR;
		bool bItalic = false;
		bool bUnterline = false;
		bool bStrikeOut = false;
		FontQuality eQuality = FontQuality::ClearType;
	};
	
	class Font final
	{
	public: // static methods

		static const Font& GetDefault();


	private: // static methods

		void AddHandle(HFONT handle);
		void RemoveHandle(HFONT handle);


	private: // static variables

		static std::map<HFONT, size_t> oHandles;


	public: // operators

		Font& operator=(Font&& rval) noexcept;
		Font& operator=(const Font& other);


	public: // methods

		Font();
		Font(const FontConfig& config);
		Font(Font&& rval) noexcept;
		Font(const Font& other);

		~Font();

		inline HFONT getHandle() const noexcept { return m_hFont; }
		void setConfig(const FontConfig& config);
		bool getConfig(FontConfig& dest) const;


	private: // methods

		void free();


	private: // variables

		HFONT m_hFont;
	};










	struct ButtonInitData
	{
		std::wstring sCaption = L"Button";
		int iLeft = 0;
		int iTop = 0;
		int iWidth = 75;
		int iHeight = 23;
	};
	
	class Button final : public IWinControl
	{
	public: // methods

		Button(Component owner, const ButtonInitData& InitData) :
			IWinControl(owner, InitData.iLeft, InitData.iTop), m_sCaption(InitData.sCaption),
			m_iWidth(InitData.iWidth), m_iHeight(InitData.iHeight) {}

		std::function<void(void)> OnClick = nullptr;


	protected: // IComponent implementation

		void initialize() override;

		virtual void processMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;


	private: // variables

		std::wstring m_sCaption;
		int m_iWidth;
		int m_iHeight;

	};





	struct FormInitData
	{
		bool bVisible = false;
		const wchar_t* szCaption = NULL;
		HICON hIcon = NULL;
		Brush oBrushBackground;
	};

	class Form : public IComponent
	{
		friend class Application;

	private: // static methods

		static LRESULT CALLBACK GlobalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // static variables

		static std::map<HWND, Form*> oForms;


	public: // methods

		Form(const wchar_t* szClassName, const FormInitData& InitData);

		inline HWND getHandle() const noexcept { return m_hWnd; }

		inline const auto& getComponents() const noexcept { return m_oComponents; }
		inline auto& getComponents() noexcept { return m_oComponents; }

		inline const auto& getControls() const noexcept { return m_oControls; }
		inline auto& getControls() noexcept { return m_oControls; }

		void show();


	protected: // methods

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void AfterConstruction();

		virtual LRESULT LocalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	private: // methods

		void initialize() override;
		void initializeComponents();


	private: // variables

		const std::wstring m_sClassName;
		const FormInitData m_oInitData;
		bool m_bVisible;
		std::vector<Component> m_oComponents;
		std::vector<WinControl> m_oControls;

	};



	class Application
	{
	public: // static methods

		static inline Application& getInstance() noexcept { return m_oInstance; }


	private: // static variables

		static Application m_oInstance;


	public: // methods

		inline HINSTANCE getHandle() const noexcept { return m_hInstance; }

		void ProcessMessages();

		void CreateForm(Form& form);

		void Run();


	private: // methods

		Application(); // --> singleton


	private: // variables

		HINSTANCE m_hInstance;
		std::vector<Form> m_oForms;
	};

}





// #undef foward declared definitions

#endif // ROBINLE_GF_FORM