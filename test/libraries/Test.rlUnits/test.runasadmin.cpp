#include "tests.hpp"

#include <rl/runasadmin.hpp>

// STL
#include <cstdio>
#include <cstdlib>



bool UnitTest_runasadmin()
{
	const auto eResult = rl::ElevateSelf(false, false);

	switch (eResult)
	{
	case rl::ElevationResult::Success:
		std::exit(0);

	case rl::ElevationResult::AlreadyElevated:
		std::printf("Already elevated.\n");
		return true;

	case rl::ElevationResult::Denied:
		std::printf("Administration privileges denied.\n");
		return true;

	default:
		return false;
	}
}
