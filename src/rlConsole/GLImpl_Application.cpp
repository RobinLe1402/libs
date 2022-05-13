#include "include/GLImpl_Application.hpp"

#include "include/GLImpl_Renderer.hpp"
#include "include/GLImpl_Window.hpp"
#include "include/Graph.hpp"
#include "include/PImpl.hpp"


ConsoleApplication::ConsoleApplication(impl::rlConsole* pConsole) :
	m_pConsole(pConsole),
	GL::IApplication(*pConsole->getWindow(), *pConsole->getRenderer())
{
	// ToDo: ConsoleApplication constructor?
}

ConsoleApplication::~ConsoleApplication()
{
	// ToDo: ConsoleApplication destructor?
}

// todo: fix weird behaviour --> OnQueryResize and OnResize in IApplication?
void ConsoleApplication::handleResize(unsigned iColumns, unsigned iRows)
{
	const auto iScale = m_pConsole->getScale();
	const auto iCharWidth = ((rlConsole_Font_Char_Width)*iScale);
	const auto iCharHeight = ((rlConsole_Font_Char_Height)*iScale);

	const auto iNewX = iColumns;
	const auto iNewY = iRows;

	const auto iOldX = m_pConsole->getColumns();
	const auto iOldY = m_pConsole->getRows();
	if (iNewX != iOldX || iNewY != iOldY)
	{
		m_pConsole->setColumns(iNewX);
		m_pConsole->setRows(iNewY);
		const size_t iOldCharCount = (size_t)iOldX * iOldY;
		const size_t iNewCharCount = (size_t)iNewX * iNewY;

		auto& oGraph = *getGraph();
		auto pOldData = oGraph.pBuf;

		oGraph.iColumns = iNewX;
		oGraph.iRows = iNewY;
		oGraph.pBuf = new rlConsole_CharInfo[iNewCharCount];
		auto pNewData = oGraph.pBuf;

		const size_t iCopyableDataSize =
			(iOldCharCount < iNewCharCount ? iOldCharCount : iNewCharCount) *
			sizeof(rlConsole_CharInfo);

		memcpy_s(pNewData, iCopyableDataSize, pOldData, iCopyableDataSize);
		delete[] pOldData;

		if (iNewCharCount > iOldCharCount)
		{
			const rlConsole_CharInfo ciDefault =
			{
				.c = ' ',
				.clForeground = m_pConsole->getForeground(),
				.clBackground = m_pConsole->getBackground()
			};

			for (size_t i = iOldCharCount; i < iNewCharCount; ++i)
			{
				pNewData[i] = ciDefault;
			}
		}
	}
}

bool ConsoleApplication::OnStart()
{
	auto& oGraph = *getGraph();

	const size_t iCharCount = (size_t)m_pConsole->getColumns() * m_pConsole->getRows();
	const size_t iDataSize = iCharCount * sizeof(rlConsole_CharInfo);

	oGraph.iColumns = m_pConsole->getColumns();
	oGraph.iRows = m_pConsole->getRows();
	oGraph.pBuf = new rlConsole_CharInfo[iCharCount];

	const rlConsole_CharInfo ciDefault =
	{
		.c = ' ',
		.clForeground = m_pConsole->getForeground(),
		.clBackground = m_pConsole->getBackground()
	};
	for (size_t i = 0; i < iCharCount; ++i)
		oGraph.pBuf[i] = ciDefault;

	m_bRunning = true;
	return true;
}

bool ConsoleApplication::OnUpdate(float fElapsedTime)
{
	return m_pConsole->OnUpdate(fElapsedTime, reinterpret_cast<HrlConsole>(m_pConsole));
}

void ConsoleApplication::OnResize(LONG& iWidth, LONG& iHeight)
{
	const auto iScale = m_pConsole->getScale();

	iWidth -= (iWidth % (rlConsole_Font_Char_Width * iScale));
	iHeight -= (iHeight % (rlConsole_Font_Char_Height * iScale));
}

bool ConsoleApplication::OnStop()
{
	m_bRunning = false;
	return true;
}

void ConsoleApplication::createGraph(void** pGraph) { *pGraph = new ConsoleGraph{}; }

void ConsoleApplication::copyGraph(void* pDest, const void* pSource)
{
	auto& oDest = *static_cast<ConsoleGraph*>(pDest);
	auto& oSrc = *static_cast<const ConsoleGraph*>(pSource);

	const size_t iCharCount = (size_t)oSrc.iColumns * oSrc.iRows;
	const size_t iSize = iCharCount * sizeof(rlConsole_CharInfo);
	if (oDest.iColumns != oSrc.iColumns || oDest.iRows != oSrc.iRows)
	{
		delete[] oDest.pBuf;

		oDest.iColumns = oSrc.iColumns;
		oDest.iRows = oSrc.iRows;

		oDest.pBuf = new rlConsole_CharInfo[iCharCount];
	}

	memcpy_s(oDest.pBuf, iSize, oSrc.pBuf, iSize);
}

void ConsoleApplication::destroyGraph(void* pGraph)
{
	auto pConGraph = static_cast<ConsoleGraph*>(pGraph);
	delete[] pConGraph->pBuf;
	delete pConGraph;
}
