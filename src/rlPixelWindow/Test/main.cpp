#include <rl/dll/rlPixelWindow++/Core.hpp>

#include <cstdint>
#include <cstdio>


namespace DLL = rl::PixelWindowDLL;

class WinImpl : public DLL::Window
{
public:

	using DLL::Window::Window;

protected:
	PixelWindowRes MessageProc(PixelWindowMsg msg,
		PixelWindowArg arg1, PixelWindowArg arg2) override
	{
		return DLL::DefMsgHandler(intfObj(), msg, arg1, arg2);
	}
};

void PrintError(PixelWindowRes iErrorCode);

int main()
{
	std::printf(
		"TEST APPLICATION FOR THE rlPixelWindow DLL\n"
		"==========================================\n"
		"2022 Robin Lemanska\n"
		"\n"
		"\n"
	);

	std::printf("DLL version: ");
	const PixelWindowVersion iVersionNumber = DLL::GetVersion();
	std::printf("%u.%u.%u.%u\n\n",
		PXWIN_VERSION_GET_MAJOR(iVersionNumber),
		PXWIN_VERSION_GET_MINOR(iVersionNumber),
		PXWIN_VERSION_GET_PATCH(iVersionNumber),
		PXWIN_VERSION_GET_BUILD(iVersionNumber));

	std::printf("Last error at startup: ");
	PrintError(DLL::GetError());
	std::printf("\n\n");

	WinImpl win;
	std::printf("Creating window...\n");
	if (!win.create(500, 250))
	{
		std::printf("Window creation failed.\n");
		return 1;
	}
	std::printf(
		"Window creation succeeded.\n"
		"\n"
		"Running ...\n"
	);
	win.run();

	std::printf("\nLast error at shutdown: ");
	PrintError(DLL::GetError());
	std::printf("\n\n");


	return 0;
}

void PrintError(PixelWindowRes iErrorCode)
{
	constexpr char szUnknown[] = "unknown error code";

	const char *szErrorName = szUnknown;

	const auto pMsg = DLL::GetErrorMsg(iErrorCode);
	if (pMsg)
		szErrorName = pMsg->szDefName;
		
	std::printf("%llu (%s)", iErrorCode, szErrorName);
}
