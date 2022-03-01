#include <string>
#include <Windows.h>

#include "../src/RLGF/rlgf.form.hpp"
#include <rl/visualstyles.h>




class formMain : public rl::Form
{
public: // methods

	formMain(const rl::FormInitData& InitData) :
		rl::Form(L"TformMain", InitData) {}
	formMain(const wchar_t* szClassName, const rl::FormInitData& InitData) :
		rl::Form(szClassName, InitData) {}


protected: // methods

	static const size_t iFontCount = 9;
	const std::wstring oFonts[iFontCount] =
	{
		L"Bauhaus 93",
		L"Brush Script MT",
		L"Calibri",
		L"Cascadia Mono",
		L"Comic Sans MS",
		L"Impact",
		L"Nes Controller",
		L"OCR A Extended",
		L"Times New Roman"
	};

	void OnBtn1()
	{
		static size_t index = 0;
		static rl::Font fnt;

		++index;
		index %= iFontCount;

		rl::FontConfig cfg;
		cfg.iHeight = 25;
		cfg.sFontName = oFonts[index];
		fnt.setConfig(cfg);
		SetWindowTextW(getComponents()[0]->getHandle(), oFonts[index].c_str());
		SendMessage(getComponents()[0]->getHandle(), WM_SETFONT, (WPARAM)fnt.getHandle(), 1);
	}

	void OnHover()
	{
		SetWindowTextA(getComponents()[1]->getHandle(), ";)");
	}
	void OnLeave()
	{
		SetWindowTextA(getComponents()[1]->getHandle(), ":)");
	}

	void OnCreate() override
	{
		rl::Form::OnCreate();

		const int iWinPadding = 15;
		const int iCompPadding = 10;

		rl::ButtonInitData init;
		init.iLeft = iWinPadding;
		init.iTop = iWinPadding;
		init.iWidth = 250;
		auto btn = new rl::Button(this, init);
		btn->OnClick = [=]() { MessageBox(m_hWnd, TEXT("BtnClick"), TEXT("LOL"), MB_ICONINFORMATION); };
		this->OnDblClick =
			[&]()
		{
			MessageBoxA(NULL, "TEST", "", MB_ICONINFORMATION);
		};
		addControl(btn);

		init.iLeft = init.iLeft + init.iWidth + iCompPadding;
		init.iWidth = init.iHeight;
		init.sCaption = L":)";
		btn = new rl::Button(this, init);
		btn->OnClick = [&]() { OnBtn1(); };
		btn->OnMouseEnter = [&]() { OnHover(); };
		btn->OnMouseLeave = [&]() { OnLeave(); };
		addControl(btn);

		init.sCaption = L"Hello World!";
		init.iTop += init.iHeight + iCompPadding;
		init.iLeft = iWinPadding;
		init.iWidth = 100;
		auto lb = new rl::Label(this, init);
		addControl(lb);
	}

	void AfterConstruction() override
	{
		rl::Form::AfterConstruction();

		// write your test code
	}

	void OnDestroy() override
	{
		for (auto p : getControls())
			delete p;
	}
};





int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	auto& App = rl::Application::GetInstance();

	rl::FormInitData fid;
	fid.szCaption = L"Test form";
	fid.bVisible = true;
	//fid.oBrushBackground.setColor(RGB(6, 77, 111));

	formMain form(fid);

	rl::FontConfig cfg;
	rl::Font form1(cfg);
	rl::Font form2(cfg);

	App.CreateForm(form);
	App.Run();

	return 0;
}
