#include "audio.engine.hpp"

#include "tools.hresult.hpp"

#undef min
#undef max
#include <exception>
#include <mutex>
#include <objbase.h>
#include <stdint.h>
#include <string>
#include <xaudio2.h>





namespace rl
{

	/***********************************************************************************************
	struct int24_t
	***********************************************************************************************/

	bool BigEndian()
	{
		union
		{
			uint16_t i;
			uint8_t b[2];
		} o = { 0x00FF };

		return o.b[0] == 0x00;
	}

	const bool bBigEndian = BigEndian();



	int32_t int24_t::asInt32()
	{
		int32_t i = 0;
		memcpy((uint8_t*)&i + (bBigEndian ? 1 : 0), this, 3);
		if (i & 0x00800000)
			i |= 0xFF000000;
		return i;
	}

	void int24_t::assign(int32_t i)
	{
		if (i > 0x007FFFFFi32)
			throw std::exception("int24_t: integer overflow");
		if (i < 0xFF800000i32)
			throw std::exception("int24_t: integer underflow");

		memcpy(this, (uint8_t*)&i + (bBigEndian ? 1 : 0), 3);
	}










	WAVEFORMATEX CreateWaveFmt(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate)
	{
		if (BitsPerSample < 8 || BitsPerSample % 8 > 0 || BitsPerSample > 32)
			throw std::exception("CreateWaveFmt: Invalid bits per sample");
		if (ChannelCount < 1 || ChannelCount > XAUDIO2_MAX_AUDIO_CHANNELS)
			throw std::exception("CreateWaveFmt: Invalid audio channel count");
		if (SampleRate < XAUDIO2_MIN_SAMPLE_RATE || SampleRate > XAUDIO2_MAX_SAMPLE_RATE ||
			SampleRate % XAUDIO2_QUANTUM_DENOMINATOR > 0)
			throw std::exception("CreateWaveFmt: Invalid samplerate");

		WAVEFORMATEX result = { sizeof(result) };

		result.wBitsPerSample = BitsPerSample;
		result.nChannels = ChannelCount;
		result.nSamplesPerSec = SampleRate;

		result.wFormatTag = (result.wBitsPerSample < 32 ? WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT);
		result.nBlockAlign = result.wBitsPerSample / 8 * result.nChannels;
		result.nAvgBytesPerSec = result.nBlockAlign * result.nSamplesPerSec;

		return result;
	}

	bool ValidWaveFmt(WAVEFORMATEX& fmt)
	{
		if (fmt.nAvgBytesPerSec != fmt.nBlockAlign * fmt.nSamplesPerSec ||
			fmt.nBlockAlign != fmt.wBitsPerSample / 8 * fmt.nChannels ||
			fmt.wFormatTag !=
			(fmt.wBitsPerSample < 32 ? WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT) ||
			fmt.nSamplesPerSec < XAUDIO2_MIN_SAMPLE_RATE ||
			fmt.nSamplesPerSec > XAUDIO2_MAX_SAMPLE_RATE ||
			fmt.nSamplesPerSec % XAUDIO2_QUANTUM_DENOMINATOR ||
			fmt.nChannels < 1 || fmt.nChannels > XAUDIO2_MAX_AUDIO_CHANNELS ||
			fmt.wBitsPerSample < 8 || fmt.wBitsPerSample % 8 > 0 || fmt.wBitsPerSample > 32)
			return false;

		return true;
	}










	/***********************************************************************************************
	 class IAudio
	***********************************************************************************************/


	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IAudio::~IAudio() { unregisterAudio(); }


	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void IAudio::registerAudio() { m_oEngine.registerAudio(this); }

	void IAudio::unregisterAudio() { m_oEngine.unregisterAudio(this); }










	/***********************************************************************************************
	class AudioEngine
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::AudioEngine() : m_oCallback(*this)
	{
		createEngine();
	}

	AudioEngine::~AudioEngine()
	{
		destroy();
		destroyEngine();

		if (m_trdEngine.joinable())
			m_trdEngine.join();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	AudioEngine& AudioEngine::getInstance()
	{
		static AudioEngine engine;
		return engine;
	}

	bool AudioEngine::create(const wchar_t* DeviceID, uint8_t ChannelCount, uint32_t samplerate)
	{
		destroy();

		if (m_trdEngine.joinable())
			m_trdEngine.join();

		if (!m_bEngineExists)
			createEngine();

		HRESULT hr = m_pEngine->CreateMasteringVoice(&m_pMaster, ChannelCount, samplerate, 0,
			DeviceID);

		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 mastering voice:\n");

		m_bError = false;

		// refresh output of 3D audio
		for (IAudio* audio : m_pAudio)
		{
			if (audio->is3D())
				audio->refresh3DOutput();
		}

		m_iChannelCount = ChannelCount;
		m_bRunning = true;

		m_trdEngine = std::thread(&rl::AudioEngine::threadFunc, this);

		return true;
	}

	void AudioEngine::destroy()
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();
	}

	void AudioEngine::stopAllAudio()
	{
		for (IAudio* pAudio : m_pAudio)
			pAudio->stop();
	}

	void AudioEngine::pauseAllAudio()
	{
		for (IAudio* pAudio : m_pAudio)
			pAudio->pause();
	}

	void AudioEngine::resumeAllAudio()
	{
		for (IAudio* pAudio : m_pAudio)
			pAudio->resume();
	}

	IXAudio2* AudioEngine::getEngine()
	{
		if (m_bRunning)
			return m_pEngine;
		else
			return nullptr;
	}

	void AudioEngine::get3DOutputVolume(Audio3DPos pos, float(&OutputMatrix)[8])
	{
		// references for simplification
		float& fFrontLeft = OutputMatrix[0];
		float& fFrontRight = OutputMatrix[1];
		float& fFrontCenter = OutputMatrix[2];
		float& fLFE = OutputMatrix[3];
		float& fBackLeft = OutputMatrix[4];
		float& fBackRight = OutputMatrix[5];
		float& fSideLeft = OutputMatrix[6];
		float& fSideRight = OutputMatrix[7];



		// relative volume
		float fRelLeft, fRelCenter, fRelRight, fRelFront, fRelSide, fRelBack;

		fRelLeft = 1.0f - std::min(1.0f, abs((pos.x + 1.0f) / pos.radius));
		fRelCenter = 1.0f - std::min(1.0f, abs(pos.x) / pos.radius);
		fRelRight = 1.0f - std::min(1.0f, abs((pos.x - 1.0f) / pos.radius));
		fRelFront = 1.0f - std::min(1.0f, abs(pos.z) / pos.radius);
		fRelSide = 1.0f - std::min(1.0f, abs(pos.z - 1.0f) / pos.radius);
		fRelBack = 1.0f - std::min(1.0f, abs(pos.z - 2.0f) / pos.radius);



		// value calculation
		fFrontLeft = fRelFront * fRelLeft;
		fFrontRight = fRelFront * fRelRight;
		fFrontCenter = fRelFront * fRelCenter;
		fLFE = 0.0f;
		fBackLeft = fRelBack * fRelLeft;
		fBackRight = fRelBack * fRelRight;
		fSideLeft = fRelSide * fRelLeft;
		fSideRight = fRelSide * fRelRight;
	}

	void AudioEngine::getVersion(uint8_t(&dest)[4])
	{
		static uint8_t version[4] = { 0, 5, 0, 0 };

		memcpy_s(dest, sizeof(dest), version, sizeof(version));
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void AudioEngine::registerAudio(IAudio* snd)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxAudio);
		m_pAudio.push_back(snd);
		lm.unlock();
	}

	void AudioEngine::unregisterAudio(IAudio* audio)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxAudio);

		auto it = std::find(m_pAudio.begin(), m_pAudio.end(), audio);
		if (it != m_pAudio.end())
			m_pAudio.erase(it);

		lm.unlock();
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void AudioEngine::createEngine()
	{
		HRESULT hr;
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't initialize COM:\n");

		hr = XAudio2Create(&m_pEngine);

		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 object:\n");

		m_pEngine->RegisterForCallbacks(&m_oCallback);
		m_bEngineExists = true;
	}

	void AudioEngine::destroyEngine()
	{
		if (!m_bEngineExists)
			return;


		if (m_pEngine != nullptr)
			m_pEngine->Release();

		CoUninitialize();

		m_bEngineExists = false;
	}

	void AudioEngine::threadFunc()
	{
		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.wait(lm);

		stopAllAudio();
		if (m_pMaster != nullptr)
		{
			m_pMaster->DestroyVoice();
			m_pMaster = nullptr;
		}

		m_bRunning = false;

		if (m_bError)
		{
			destroyEngine();
			m_bError = false;
		}
	}










	/***********************************************************************************************
	class AudioEngine::Callback
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioEngine::Callback::OnCriticalError(HRESULT Error)
	{
		/*std::string sError;
		GenerateHResultString(sError, Error, "Critical error in rl::AudioEngine:\n");
		MessageBoxA(NULL, sError.c_str(), "Exception", MB_ICONERROR | MB_SYSTEMMODAL);*/

		m_oEngine.m_bError = true;

		m_oEngine.destroy();
	}










	/***********************************************************************************************
	class IAudio
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	bool IAudio::createVoices(WAVEFORMATEX wavfmt, const char* szClassName)
	{
		if (!m_b3D) // regular playback
		{
			HRESULT hr = m_oEngine.getEngine()->CreateSourceVoice(&m_pVoice, &wavfmt, 0,
				XAUDIO2_DEFAULT_FREQ_RATIO, m_pCallback);

			if (FAILED(hr))
			{
				std::string sMsg;
				GenerateHResultString(sMsg, hr, "rl::SoundInstance: Couldn't create source voice:\n");
				MessageBoxA(NULL, sMsg.c_str(),
					"Exception", MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}
		}
		else // 3D mode
		{
			auto p = m_oEngine.getEngine();
			XAUDIO2_SEND_DESCRIPTOR send = { 0 };
			XAUDIO2_VOICE_SENDS sendlist = { 1, &send };

			HRESULT hr;

			// 7.1 surround submix
			hr = p->CreateSubmixVoice(&m_pVoiceSurround, 8, wavfmt.nSamplesPerSec, 0, 1);
			if (FAILED(hr))
			{
				std::string sMsg;
				if (szClassName != nullptr)
				{
					sMsg.reserve(strlen(szClassName) + 2);
					sMsg = szClassName;
					sMsg += ": ";
				}
				GenerateHResultString(sMsg, hr,
					(sMsg + "Couldn't create surround submix voice:\n").c_str());
				MessageBoxA(NULL, sMsg.c_str(),
					"Error", MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}

			// mono submix
			send.pOutputVoice = m_pVoiceSurround;
			hr = p->CreateSubmixVoice(&m_pVoiceMono, 1, wavfmt.nSamplesPerSec, 0, 0, &sendlist);
			if (FAILED(hr))
			{
				m_pVoiceSurround->DestroyVoice();
				m_pVoiceSurround = nullptr;

				std::string sMsg;
				if (szClassName != nullptr)
				{
					sMsg.reserve(strlen(szClassName) + 2);
					sMsg = szClassName;
					sMsg += ": ";
				}
				GenerateHResultString(sMsg, hr,
					(sMsg + "Couldn't create mono submix voice:\n").c_str());
				MessageBoxA(NULL, sMsg.c_str(),
					"Error", MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}

			// source input
			send.pOutputVoice = m_pVoiceMono;
			hr = p->CreateSourceVoice(&m_pVoice, &wavfmt, 0,
				XAUDIO2_DEFAULT_FREQ_RATIO, m_pCallback, &sendlist);
			if (FAILED(hr))
			{
				m_pVoiceMono->DestroyVoice();
				m_pVoiceMono = nullptr;
				m_pVoiceSurround->DestroyVoice();
				m_pVoiceSurround = nullptr;

				std::string sMsg;
				if (szClassName != nullptr)
				{
					sMsg.reserve(strlen(szClassName) + 2);
					sMsg = szClassName;
					sMsg += ": ";
				}
				GenerateHResultString(sMsg, hr,
					(sMsg + "Couldn't create mono source voice:\n").c_str());
				MessageBoxA(NULL, sMsg.c_str(),
					"Error", MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}

			refresh3DOutput();
		}
		return true;
	}

	void IAudio::destroyVoices()
	{
		if (!m_oEngine.error())
		{
			m_bRunning = false;
			m_pVoice->Stop();
			m_pVoice->FlushSourceBuffers();
			m_pVoice->DestroyVoice();

			if (m_b3D)
			{
				m_pVoiceMono->DestroyVoice();
				m_pVoiceSurround->DestroyVoice();

				m_b3D = false;
			}
		}

		m_pVoice = nullptr;
		m_pVoiceMono = nullptr;
		m_pVoiceSurround = nullptr;

		m_bRunning = false;
	}

	void IAudio::refresh3DOutput()
	{
		if (!m_b3D)
			return;

		m_oEngine.get3DOutputVolume(m_oPos, m_f3DVolume);
		m_pVoiceMono->SetOutputMatrix(m_pVoiceSurround, 1, 8, m_f3DVolume);
	}

	void IAudio::setPos(Audio3DPos pos, float volume)
	{
		if (!m_bRunning || !m_b3D)
			return;

		m_oPos = pos;
		refresh3DOutput();
		m_pVoice->SetVolume(volume);
	}

	void IAudio::setVolume(float volume)
	{
		if (!m_bRunning)
			return;

		m_pVoice->SetVolume(volume);
	}










	/***********************************************************************************************
	class SoundInstance
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	SoundInstance::~SoundInstance()
	{
		stop();
		if (m_trd.joinable())
			m_trd.join();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void SoundInstance::play(WaveformData& data, float volume)
	{
		stop();

		m_b3D = false;
		playInternal(data, volume);
	}

	void SoundInstance::play3D(WaveformData& data, Audio3DPos& pos, float volume)
	{
		stop();

		m_b3D = true;
		m_oPos = pos;

		playInternal(data, volume);
	}

	void SoundInstance::pause()
	{
		if (!m_bRunning || m_bPaused)
			return;

		m_pVoice->Stop();
		m_bPaused = true;
	}

	void SoundInstance::resume()
	{
		if (!m_bRunning || !m_bPaused)
			return;

		m_pVoice->Start();
		m_bPaused = false;
	}

	void SoundInstance::stop()
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();

		unregisterAudio();
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void SoundInstance::playInternal(WaveformData& data, float fVolume)
	{
		WAVEFORMATEX wavfmt = CreateWaveFmt(data.iBitsPerSample, data.iChannelCount,
			data.iSampleRate);
		m_pCallback = &m_oCallback;
		if (!createVoices(wavfmt, "rl::SoundInstance"))
			return;

		registerAudio();
		if (fVolume != 1.0)
			m_pVoice->SetVolume(fVolume);

		XAUDIO2_BUFFER buf{};
		buf.AudioBytes = (UINT32)data.bytes;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.pAudioData = (BYTE*)data.pData;
		m_pVoice->SubmitSourceBuffer(&buf);
		m_pVoice->Start();

		m_trd = std::thread(&rl::SoundInstance::threadFunc, this);
		m_bRunning = true;
	}

	void SoundInstance::threadFunc()
	{
		std::unique_lock<std::mutex> lm(m_mux);

		m_cv.wait(lm); // wait for stop command

		destroyVoices();
	}










	/***********************************************************************************************
	class SoundInstance::Callback
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void SoundInstance::Callback::OnStreamEnd() { m_oSound.stop(); }










	/***********************************************************************************************
	class IAudioStream
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IAudioStream::~IAudioStream()
	{
		stop();

		if (m_trdStream.joinable())
			m_trdStream.join(); // wait for thread to finish
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void IAudioStream::play(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate,
		float volume, size_t BufferBlocks, size_t BufferBlockSamples)
	{
		if (!m_oEngine.isRunning())
			return;


		stop();

		if (m_trdStream.joinable())
			m_trdStream.join(); // wait for thread to finish


		m_b3D = false;
		playInternal(BitsPerSample, ChannelCount, SampleRate, volume, BufferBlocks,
			BufferBlockSamples);
	}

	void IAudioStream::play3D(Audio3DPos pos, uint8_t BitsPerSample, uint8_t ChannelCount,
		uint32_t SampleRate, float volume, size_t BufferBlocks, size_t BufferBlockSamples)
	{
		if (!m_oEngine.isRunning())
			return;

		stop();
		m_b3D = true;
		m_oPos = pos;
		playInternal(BitsPerSample, ChannelCount, SampleRate, volume, BufferBlocks,
			BufferBlockSamples);
	}

	void IAudioStream::pause()
	{
		if (!m_bRunning || m_bPaused)
			return;

		m_pVoice->Stop();
		m_bPaused = true;
	}

	void IAudioStream::resume()
	{
		if (!m_bRunning || !m_bPaused)
			return;

		m_bPaused = false;
		m_pVoice->Start();
	}

	void IAudioStream::stop()
	{
		if (!m_bRunning)
			return;

		m_bRunning = false;

		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();

		m_bPaused = false;
		m_b3D = false;

		unregisterAudio();
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void IAudioStream::threadFunc(uint8_t iBitsPerSample, uint8_t iChannelCount,
		uint32_t iSampleRate, float fVolume)
	{
		WAVEFORMATEX wavfmt = CreateWaveFmt(iBitsPerSample, iChannelCount, iSampleRate);
		m_pCallback = &m_oCallback;
		if (!createVoices(wavfmt, "rl::IAudioStream"))
			return;

		m_bRunning = true;

		if (fVolume != 1.0)
			m_pVoice->SetVolume(fVolume);

		const float fTimeBetweenCalls = 1.0f / m_iSampleRate;
		const size_t iBytesPerSampleGroup = (size_t)m_iBitsPerSample / 8 * m_iChannelCount;
		XAUDIO2_BUFFER buf = {};

		m_pBufferData = new uint8_t[m_iBlockSize * m_iBufferBlocks];

		OnStartup();

		// fill buffer first time
		for (size_t i = 0; i < m_iBufferBlocks; i++)
		{
			buf = {};
			for (size_t j = 0; j < m_iBufferBlockSamples; j++)
			{
				if (!OnUpdate(fTimeBetweenCalls, m_pBufferData +
					i * m_iBlockSize + j * iBytesPerSampleGroup))
				{
					buf.Flags = XAUDIO2_END_OF_STREAM;
					m_iBlocksFree = 1;
					break;
				}

				buf.AudioBytes += m_iChannelCount * ((size_t)m_iBitsPerSample / 8);
			}

			buf.pAudioData = m_pBufferData + i * m_iBlockSize;
			m_pVoice->SubmitSourceBuffer(&buf);

			if (buf.Flags == XAUDIO2_END_OF_STREAM)
				break;
		}
		m_iBufferCurrentBlock = 0;
		m_iBlocksFree = 0;

		m_pVoice->Start();

		while (m_bRunning)
		{
			// wait while buffer is filled
			if (m_iBlocksFree == 0)
			{
				std::unique_lock<std::mutex> lm(m_mux);
				m_cv.wait(lm);
			}

			// fill up buffer
			while (m_iBlocksFree > 0)
			{
				buf = {};
				for (size_t i = 0; i < m_iBufferBlockSamples; i++)
				{
					if (!OnUpdate(fTimeBetweenCalls, m_pBufferData +
						m_iBufferCurrentBlock * m_iBlockSize + i * iBytesPerSampleGroup))
					{
						buf.Flags = XAUDIO2_END_OF_STREAM;
						break;
					}

					buf.AudioBytes += m_iChannelCount * ((size_t)m_iBitsPerSample / 8);
				}

				buf.pAudioData = m_pBufferData + m_iBufferCurrentBlock * m_iBlockSize;
				m_pVoice->SubmitSourceBuffer(&buf);

				m_iBlocksFree--;
				m_iBufferCurrentBlock++;
				m_iBufferCurrentBlock %= m_iBufferBlocks;

				if (buf.Flags == XAUDIO2_END_OF_STREAM)
					break;
			}
		}

		destroyVoices();

		delete[] m_pBufferData;
		m_pBufferData = nullptr;

		OnShutdown();
	}

	void IAudioStream::playInternal(uint8_t BitsPerSample, uint8_t ChannelCount,
		uint32_t SampleRate, float volume, size_t BufferBlocks, size_t BufferBlockSamples)
	{
		registerAudio();

		if (BitsPerSample == 0 || BitsPerSample % 8 > 1 || BitsPerSample > 32 ||
			ChannelCount == 0 || ChannelCount > XAUDIO2_MAX_AUDIO_CHANNELS ||
			SampleRate % XAUDIO2_QUANTUM_DENOMINATOR > 0 ||
			SampleRate < XAUDIO2_MIN_SAMPLE_RATE || SampleRate > XAUDIO2_MAX_SAMPLE_RATE ||
			BufferBlocks < 2 || BufferBlockSamples < 1)
			throw std::exception("rl::IAudioStream: Invalid launch parameters");

		m_iBitsPerSample = BitsPerSample;
		m_iChannelCount = ChannelCount;
		m_iSampleRate = SampleRate;

		m_iBufferBlocks = BufferBlocks;
		m_iBufferBlockSamples = BufferBlockSamples;
		m_iBlocksFree = BufferBlocks;

		m_iBlockSize = m_iBufferBlocks * m_iBufferBlockSamples * m_iChannelCount *
			(m_iBitsPerSample / 8);

		if (m_trdStream.joinable())
			m_trdStream.join();
		m_trdStream = std::thread(&rl::IAudioStream::threadFunc, this, BitsPerSample, ChannelCount,
			SampleRate, volume);
	}










	/***********************************************************************************************
	class IAudioStream::Callback
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void IAudioStream::Callback::OnBufferEnd(void* pBufferContext)
	{
		m_pStream->m_iBlocksFree++;
		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();
	}

	void IAudioStream::Callback::OnStreamEnd() { m_pStream->stop(); }


}