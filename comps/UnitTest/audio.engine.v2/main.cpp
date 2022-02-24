#include <rl/audio.engine.v2.hpp>
#include "resource.h"

class ExampleStream : public rl::IAudioStream
{
public: // methods

	inline void start(const rl::WaveFormat& format, float volume = 1.0f, size_t BufferBlockCount = 8,
		size_t SamplesPerBufferBlock = 512)
	{
		rl::IAudioStream::internalStart(format, volume, BufferBlockCount, SamplesPerBufferBlock);
	}


protected: // methods
	bool nextSample(float fElapsedTime, rl::MultiChannelAudioSample& dest) noexcept override
	{
		if (dest.iBitsPerSample == 16)
		{
			for (uint8_t iChannel = 0; iChannel < dest.iChannelCount; ++iChannel)
			{
				dest.val.p16[iChannel] = (int16_t)rand();
			}
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	auto& engine = rl::AudioEngine::GetInstance();

	if (!engine.create())
	{
		printf("Couldn't create AudioEngine\n");
		return 1;
	}

	//----------------------------------------------------------------------------------------------
	// 1. White noise

	printf("Test 1: Playing white noise...\n");
	ExampleStream stream;
	rl::WaveFormat wfmt;
	wfmt.iChannelCount = 1;
	stream.start(wfmt, 0.125f);
	Sleep(1000);
	stream.stop();
	printf("\n\n");



	//----------------------------------------------------------------------------------------------
	// 2. WAV resource

	const float fVolumeWAV = 0.25f;

	printf("Test 2: WAV data\n");
	rl::Sound* pSound = rl::Sound::FromResource(NULL, MAKEINTRESOURCE(IDW_TEST));
	if (pSound)
		printf("Successfully loaded WAV file (%zu samples)\n", pSound->getSampleCount());
	else
	{
		printf("Couldn't load WAV file\n");
		return 1;
	}

	// 2a: Primitive loop
	printf("\nTest 2a: Primitive loop\n");

	const uint8_t iLoopCount = 3;
	for (uint8_t i = 0; i < iLoopCount; ++i)
	{
		auto pInstance = pSound->play(fVolumeWAV);
		printf("Playing loop %u/%u\n", i + 1, iLoopCount);
		pInstance->waitForEnd();
		delete pInstance;
	}


	// 2b: 3D Sound
	printf("\nTest 2b: 3D audio\n");
	printf("LEFT CENTER RIGHT\n");
	printf(" *");
	auto pInstance = pSound->play3D(rl::Audio3DPos::Left, fVolumeWAV);
	Sleep(1000);
	printf("\b      *");
	pInstance->set3DPos(rl::Audio3DPos::Center);
	Sleep(1000);
	printf("\b       *");
	pInstance->set3DPos(rl::Audio3DPos::Right);
	Sleep(1000);
	pInstance->stop();
	printf("\b \n\n");
	delete pInstance;

	delete pSound;

	printf("All tests done.\n");


	return 0;
}
