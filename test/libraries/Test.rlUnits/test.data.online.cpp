#include "tests.hpp"

// rl
#include <rl/data.online.hpp>



bool UnitTest_data_online()
{
	uint8_t *pData = nullptr;
	size_t len = 0;
	if (!rl::DownloadToMemory("https://www.robinle.de/index.html", &pData, &len))
		return false;
	
	delete[] pData;
	return true;
}
