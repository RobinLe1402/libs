#include "rl/console.hpp"

#include <Windows.h>





namespace rl
{

	//==============================================================================================
	// STATIC VARIABLES

	std::vector<uint8_t> Console::m_oColorStack;
	uint8_t Console::m_iStartupColor = 0;
	UINT Console::m_iStartupCP = 0;

	Console Console::m_oInstance;
	HANDLE Console::m_hOut = 0;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Console::Console()
	{
		m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(m_hOut, &csbi);
		m_iStartupColor = (uint8_t)csbi.wAttributes;

		m_iStartupCP = GetConsoleCP();
	}

	Console::~Console()
	{
		ResetColor();
		ResetCodepage();
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	void Console::PushColor(uint8_t color)
	{
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(m_hOut, &csbi);
			m_oColorStack.push_back((uint8_t)csbi.wAttributes);

		setColor(color);
	}

	void Console::PopColor()
	{
		if (m_oColorStack.size() == 0)
			return;

		setColor(m_oColorStack[m_oColorStack.size() - 1]);
		m_oColorStack.pop_back();
	}

	void Console::ResetColor()
	{
		setColor(m_iStartupColor);
		m_oColorStack.clear();
	}

	void Console::SetCodepage(UINT cp)
	{
		SetConsoleCP(cp);
		SetConsoleOutputCP(cp);
	}

	void Console::ResetCodepage()
	{
		SetConsoleCP(m_iStartupCP);
		SetConsoleOutputCP(m_iStartupCP);
	}

	void Console::setColor(uint8_t color)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(m_hOut, &csbi);

		WORD attribs = csbi.wAttributes & 0xFF00;
		attribs |= color;
		SetConsoleTextAttribute(m_hOut, attribs);
	}

}