#include "include/PImpl.hpp"

#include "include/GLImpl_Application.hpp"
#include "include/GLImpl_Window.hpp"
#include "include/GLImpl_Renderer.hpp"



bool impl::ConfigIsValid(const rlConsole_StartupConfig& config)
{
	// basic checks
	if (config.iScale == 0 ||
		config.iWidth == 0 || config.iHeight == 0 ||
		config.fnOnMessage == 0 ||
		config.fnOnUpdate == 0 ||
		config.pFont == 0) // ToDo: default font
		return false;

	// check size constraint integrity
	if ((config.iMinWidth && config.iMaxWidth && config.iMinWidth > config.iMaxWidth) ||
		(config.iMinHeight && config.iMaxHeight && config.iMinHeight > config.iMaxHeight))
		return false;

	// check size constraint conformance
	if ((config.iMinWidth && config.iWidth < config.iMinWidth) ||
		(config.iMinHeight && config.iHeight < config.iMinHeight) ||
		(config.iMaxWidth && config.iWidth > config.iMaxWidth) ||
		(config.iMaxHeight && config.iHeight > config.iMaxHeight))
		return false;


	return true;
}



impl::rlConsole::rlConsole(const rlConsole_StartupConfig& config) :
	OnMessage(config.fnOnMessage),
	OnUpdate(config.fnOnUpdate),
	m_pWindow(new ConsoleWindow(this)),
	m_pRenderer(new ConsoleRenderer(this)),
	m_pApplication(new ConsoleApplication(this)),
	m_oFont(*config.pFont), // ToDo: default font
	m_iScale(config.iScale),
	m_iColumns(config.iWidth),
	m_iRows(config.iHeight),
	m_clForeground(config.clForeground),
	m_clBackground(config.clBackground)
{
	m_oConfig.renderer.bVSync = true;

	if (config.szTitle)
		m_oConfig.window.sTitle = config.szTitle;
	else
		m_oConfig.window.sTitle = L"RobinLe Console App";
	m_oConfig.window.bFullscreen = false;
	m_oConfig.window.bResizable = true;
	if (config.szIconResID)
	{
		m_oConfig.window.hIconBig = LoadIconW((HINSTANCE)config.hInstance, config.szIconResID);
		m_oConfig.window.hIconSmall = m_oConfig.window.hIconBig;
	}
	// ToDo: Default icon

	const unsigned iCharWidth = rlConsole_Font_Char_Width * config.iScale;
	const unsigned iCharHeight = rlConsole_Font_Char_Height * config.iScale;

	m_oConfig.window.iWidth = config.iWidth * iCharWidth;
	m_oConfig.window.iHeight = config.iHeight * iCharHeight;
	m_oConfig.window.iMinWidth = config.iMinWidth * iCharWidth;
	m_oConfig.window.iMinHeight = config.iMinHeight * iCharHeight;
	m_oConfig.window.iMaxWidth = config.iMaxWidth * iCharWidth;
	m_oConfig.window.iMaxHeight = config.iMaxHeight * iCharHeight;
}

impl::rlConsole::~rlConsole()
{
	DestroyIcon(m_oConfig.window.hIconBig);
	DestroyIcon(m_oConfig.window.hIconSmall);
}

bool impl::rlConsole::execute()
{
	return m_pApplication->execute(m_oConfig); // ToDo: find reason for wrong window size
}

bool impl::rlConsole::isRunning() { return m_pApplication->isRunning(); }

rlConsole_CharBuffer impl::rlConsole::getBuffer()
{
	return m_pApplication->getGraph()->pBuf;
}

void impl::rlConsole::notifyOfResize(unsigned iColumns, unsigned iRows)
{
	m_pApplication->handleResize(iColumns, iRows);
}
