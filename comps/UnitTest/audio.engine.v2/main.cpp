#include <rl/audio.engine.v2.hpp>

int main(int argc, char* argv[])
{
	auto& engine = rl::AudioEngine::getInstance();

	if (!engine.create())
	{
		printf("Couldn't create AudioEngine\n");
		return 1;
	}



	WAVEFORMATEX wf = { sizeof(WAVEFORMATEX) };
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.nSamplesPerSec = 44100;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);
	wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;

	rl::AudioEngine::SourceVoice* pVoice;
	if (FAILED(engine.createSourceVoice(&pVoice, &wf)))
	{
		printf("Couldn't create source voice\n");
		return 1;
	}


	return 0;
}

// TODO:
/*
- overthink AudioEngine::SourceVoice::Callback (--> initialization)
*/
