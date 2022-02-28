#include <rl/audio.decoder.flac.hpp>

int main(int argc, char* argv[])
{
	rl::FLAC_test flac;
	flac.loadFromFile(LR"(N:\Soundtracks\Videospiele\_Nintendo\01 NES\DuckTales\02 Hub.flac)");

	return 0;
}
