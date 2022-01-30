#include <rl/audio.engine.v2.hpp>
#include "resource.h"

int main(int argc, char* argv[])
{
	auto& engine = rl::AudioEngine::GetInstance();

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

	rl::Sound* pSound = rl::Sound::FromResource(NULL, MAKEINTRESOURCE(IDW_TEST));
	if (pSound)
		printf("Successfully loaded WAV file (%zu samples)\n", pSound->getSampleCount());
	else
	{
		printf("Couldn't load WAV file\n");
		return 1;
	}

	auto pInstance = pSound->play3D(rl::Audio3DPos::Left, 0.05f);
	Sleep(1000);
	pInstance->set3DPos(rl::Audio3DPos::Center);
	Sleep(1000);
	pInstance->set3DPos(rl::Audio3DPos::Right);
	Sleep(1000);
	pInstance->stop();
	delete pInstance;

	/*const uint8_t iLoopCount = 5;
	for (uint8_t i = 0; i < iLoopCount; ++i)
	{
		auto pInstance = pSound->play(0.05f);
		printf("Playing loop %u/%u\n", i + 1, iLoopCount);
		pInstance->waitForEnd();
		delete pInstance;
	}*/
	printf("Stopped playing\n");
	delete pSound;


	return 0;
}

// TODO:
/*
- overthink AudioEngine::SourceVoice::Callback (--> initialization)
- remove debug output from AudioEngine::MessageLoop()
*/
