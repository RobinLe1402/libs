#include <rl/dll/rlPixelWindow++/Core.hpp>

#include <cstdint>
#include <cstdio>

namespace DLL = rl::PixelWindowDLL;

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
	uint32_t iVersionNumber = DLL::GetVersion();
	std::printf("%u.%u.%u.%u\n\n",
		uint8_t(iVersionNumber >> 24), // major version
		uint8_t(iVersionNumber >> 16), // minor version
		uint8_t(iVersionNumber >> 8),  // patch level
		uint8_t(iVersionNumber));      // build number

	std::printf("Last error at startup: ");
	auto iError = DLL::GetError();
	std::printf("%llu\n\n", iError);

	return 0;
}