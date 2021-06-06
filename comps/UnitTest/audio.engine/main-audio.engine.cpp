#include <rl/audio.engine.hpp>
#include <rl/audio.devices.hpp>
#include <rl/visualstyles.h>

#include <exception>

#define _USE_MATH_DEFINES
#include <math.h>




class ExampleStream : public rl::IAudioStream
{
public:

	ExampleStream(rl::AudioEngine& engine) : rl::IAudioStream(engine) {}


private: // variables

	float m_fCurrentPos = 0.0f; // percentage of loop completed (0.0 - 1.0)
	float m_fLoopDuration = 1.0f / 250;


private: // methods

	bool OnUpdate(float fElapsedTime, void* pBuffer) override
	{
		m_fCurrentPos += fElapsedTime;
		m_fCurrentPos = fmod(m_fCurrentPos, m_fLoopDuration);

		switch (getBitsPerSample())
		{
		case 32:
			*((float*)pBuffer) = 0.25f * (float)sin(2.0 * M_PI + 2.0 * M_PI * m_fCurrentPos / m_fLoopDuration);
			break;
		default:
			return false;
		}

		return true;
	}
};




int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{
	/*auto& devman = rl::AudioDeviceManager::getInstance();
	std::wstring sMsg;

	do
	{
		auto o = devman.enumerateDevices();
		sMsg.clear();
		for (auto& dev : o)
		{
			sMsg += L"\"" + dev.sFriendlyName + L"\": \"" + dev.sID + L"\" (" +
				std::to_wstring(dev.iChannelCount) + L" channels)\n";
		}
		sMsg.pop_back();
	} while (MessageBoxW(NULL, sMsg.c_str(), L"Audio output devices", MB_SYSTEMMODAL | MB_OKCANCEL) == IDOK);


	return 0;*/

	try
	{
		rl::AudioDevice device = {};
		auto& devmgr = rl::AudioDeviceManager::getInstance();
		devmgr.getDevice(device, 0);

		auto& engine = rl::AudioEngine::getInstance();
		engine.create(device.sID.c_str(), 6);





		ExampleStream stream(engine);

		rl::Audio3DPos pos = { -1.0f, 0.0f };
		pos.radius = 1.0;

		stream.play3D(pos, 32, 1);
		while (true)
		{
			if (!engine.isRunning())
			{
				try
				{
					engine.create(0, 6);
					stream.play3D(pos, 32, 1);
				}
				catch (std::exception e) {}
			}
		}





		/*float* fBuf = new float[88200];
		float fTimeBetweenSamples = 1.0f / 44100;
		float fFrequency = 750;
		double dLoopDuration = 1.0 / fFrequency;

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
		rl::SoundInstance sound(engine);

		int iRhythm[] =
		{
			100,
			100,
			100,
			300,
			300,
			300,
			100,
			100,
			100
		};

		rl::Audio3DPos pos = {};
		pos.x = -1.0f;

		sound.play3D(snd, pos, 0.25f);

		for (int i : iRhythm)
		{
			Sleep(i);
			sound.pause();
			Sleep(100);
			sound.resume();
		}
		sound.stop();*/

	}
	catch (std::exception e)
	{
		MessageBoxA(NULL, e.what(), "std::exception", MB_ICONERROR | MB_SYSTEMMODAL);
	}

	return 0;
}