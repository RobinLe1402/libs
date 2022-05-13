#include "rl/dll/rlConsole/Core.h"

#include "include/PImpl.hpp"



HrlConsole rlConsole_create(const rlConsole_StartupConfig* pConfig)
{
	if (!impl::ConfigIsValid(*pConfig))
		return nullptr;

	return reinterpret_cast<HrlConsole>(new impl::rlConsole(*pConfig));
}

void rlConsole_destroy(HrlConsole con)
{
	delete reinterpret_cast<impl::rlConsole*>(con);
}

bool rlConsole_execute(HrlConsole con)
{
	return reinterpret_cast<impl::rlConsole*>(con)->execute();
}

unsigned rlConsole_getBufWidth(HrlConsole con)
{
	return reinterpret_cast<impl::rlConsole*>(con)->getColumns();
}

unsigned rlConsole_getBufHeight(HrlConsole con)
{
	return reinterpret_cast<impl::rlConsole*>(con)->getRows();
}

bool rlConsole_getBuf(HrlConsole con, rlConsole_CharBuffer* pBuf)
{
	if (con == nullptr || !reinterpret_cast<impl::rlConsole*>(con)->isRunning())
		return false;

	*pBuf = reinterpret_cast<impl::rlConsole*>(con)->getBuffer();
	return true;
}
