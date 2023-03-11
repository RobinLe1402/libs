#include "MainActions.hpp"
#include "Constants.hpp"

#include <rl/commandline.hpp>


#include <conio.h>
#include <cstdio>
#include <exception>

#include <Windows.h>



class WinConsoleHelper final
{
public:
	WinConsoleHelper()
	{
		if (!AllocConsole())
			throw std::exception("Error allocating console");

		FILE *fDummy;
		freopen_s(&fDummy, "CONIN$",  "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
	}
	~WinConsoleHelper()
	{
		FreeConsole();
	}
};

class MutexHelper final
{
private:

	HANDLE m_hMutex = NULL;
	static char s_szERROR[];


public:

	MutexHelper() = default;

	~MutexHelper()
	{
		if (m_hMutex)
			ReleaseMutex(m_hMutex);
	}

	void lock()
	{
		m_hMutex = CreateMutex(NULL, TRUE, szMUTEXNAME);
		if (m_hMutex == NULL)
			throw std::exception(s_szERROR);
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			const auto dwWaitResult = WaitForSingleObject(m_hMutex, INFINITE);

			switch (dwWaitResult)
			{
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				break;

			default:
				throw std::exception(s_szERROR);
			}
		}
		else if (GetLastError() != ERROR_SUCCESS)
			throw std::exception(s_szERROR);
	}

};
char MutexHelper::s_szERROR[] = "Couldn't create mutex.";


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	MutexHelper mh;

	try
	{
		mh.lock();
	}
	catch (const std::exception &e)
	{
		MessageBoxA(NULL, e.what(), "RobinLe App Launcher", MB_SYSTEMMODAL | MB_ICONERROR);
		return 1;
	}
	

	auto &cmd = rl::Commandline::Instance();

	// app to start
	const auto itApp = cmd.find(szARG_APP, false);
	const bool bApp = itApp != cmd.end();

	// offline mode?
	auto itArg = cmd.find(szARG_OFFLINEMODE, false);
	if (itArg == cmd.end())
	{
		// --> no offline mode --> try updating

		// update launcher
		itArg = cmd.find(szARG_SKIPLAUNCHERUPDATE, false);
		if (itArg == cmd.end())
			UpdateLauncher();

		// update libraries
		itArg = cmd.find(szARG_SKIPLIBUPDATE, false);
		if (itArg == cmd.end())
			UpdateLibraries();

		// update app
		if (bApp)
		{
			itArg = cmd.find(szARG_SKIPAPPUPDATE, false);
			if (itArg == cmd.end())
				UpdateApp(itApp->value());
		}
	}

	if (bApp)
	{
		ExecuteApp(itApp->value());
		return 0;
	}

	

	itArg = cmd.find(szARG_SILENTMODE, false);
	if (itArg != cmd.end())
		return 0;

	WinConsoleHelper wch;

	ShowAppList();

	return 0;
}

