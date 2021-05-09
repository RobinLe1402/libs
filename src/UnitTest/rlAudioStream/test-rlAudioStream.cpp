#include "../../rlAudioStream.hpp"

#include <exception>

#define _USE_MATH_DEFINES
#include <math.h>


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	try
	{
		auto& engine = rl::AudioEngine::getInstance();
		engine.create();


		float* fBuf = new float[88200];
		float fTimeBetweenSamples = 1.0f / 44100;
		double dLoopDuration = 1.0 / 130.81;

		float fElapsed = 0;
		for (size_t i = 0; i < 88200; i++)
		{
			fBuf[i] = (float)sin(2.0 * M_PI + 2.0 * M_PI * fElapsed / dLoopDuration);

			fElapsed += fTimeBetweenSamples;
			fElapsed = (float)fmod(fElapsed, dLoopDuration);
		}


		rl::WaveformData snd;
		snd.pData = fBuf;
		snd.bytes = sizeof(float) * 88200;
		snd.iBitsPerSample = sizeof(float) * 8;
		snd.iChannelCount = 1;
		snd.iSampleRate = 44100;
		engine.playSound(snd);
		delete[] fBuf;

	}
	catch (std::exception e)
	{
		MessageBoxA(NULL, e.what(), "Test exception", MB_ICONINFORMATION);
	}

	return 0;
}
