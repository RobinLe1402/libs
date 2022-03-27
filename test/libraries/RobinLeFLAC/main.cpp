#include "rl/dll/RobinLeFLAC.hpp"

int main(int argc, char* argv[])
{
	namespace dll = rl::RobinLeFLACDLL;

	dll::FLACDecoder flac;
	flac.open(LR"(N:\Musik\alt-J\01 An Awesome Wave\03 - Tessellate.flac)");
	
	if (!flac.isOpened())
	{
		printf("Couldn't be opened\n");
		return 1;
	}
	const size_t iMaxOutput = 50;
	size_t iCurrentSample = 0;
	while (!flac.eof())
	{
		if (iCurrentSample == iMaxOutput)
			break;
		uint32_t iDest = 0;
		flac.getSample(iDest, 0);
		printf("%u", iDest);

		++iCurrentSample;

		flac.nextSample();
	}

	return 0;
}
