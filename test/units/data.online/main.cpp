#include <rl/data.online.hpp>
#include <stdio.h>

int main(int argc, char* argv[])
{
	uint8_t* pData = nullptr;
	size_t len = 0;
	if (!rl::DownloadToMemory("https://www.robinle.de/index.html", &pData, &len))
		printf("Error.\n");
	else
	{
		printf("Success.\n");
		delete[] pData;
	}

	return 0;
}
