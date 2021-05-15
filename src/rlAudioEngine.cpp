#include "rlAudioEngine.hpp"

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










	Sound::Voice::Voice(AudioEngine& engine, std::mutex& mux, std::condition_variable& cv,
		WaveformData& data, float volume) : m_mux(mux), m_cv(cv)
	{
		uint8_t i = data.iBitsPerSample / 8;
		if (i == 0 || i > 4 || data.iBitsPerSample % 8 > 0)
			throw std::exception("rl::SoundVoice: Invalid bitrate");

		WAVEFORMATEX wavformat =
			CreateWaveFmt(data.iBitsPerSample, data.iChannelCount, data.iSampleRate);

		if (FAILED(engine.getEngine()->CreateSourceVoice(&m_pVoice, &wavformat, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, this)))
			throw std::exception("rl::SoundVoice: Couldn't create source voice");

		if (volume != 1.0)
			m_pVoice->SetVolume(volume);

		XAUDIO2_BUFFER buf{};
		buf.AudioBytes = (UINT32)data.bytes;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.pAudioData = (BYTE*)data.pData;
		m_pVoice->SubmitSourceBuffer(&buf);
		m_pVoice->Start();
	}

	Sound::Voice::~Voice()
	{
		m_pVoice->DestroyVoice();
	}

	void Sound::Voice::OnStreamEnd()
	{
		stop();
	}

	void Sound::Voice::pause()
	{
		if (m_bPaused)
			return;

		m_pVoice->Stop();
		m_bPaused = true;
	}

	void Sound::Voice::resume()
	{
		if (!m_bPaused)
			return;

		m_pVoice->Start();
		m_bPaused = false;
	}

	void Sound::Voice::stop()
	{
		m_pVoice->Stop();
		m_pVoice->FlushSourceBuffers();

		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();
		m_mux.unlock();
		lm.release();
	}



	Sound::Sound(AudioEngine& engine, WaveformData data, bool ManageData) :
		m_oEngine(engine), m_oData(data), m_bManage(ManageData)
	{
		m_oData = data;
		m_bManage = ManageData;
		engine.registerSound(this);
	}

	Sound::~Sound()
	{
		stopAll();
		while (m_iThreadCount > 0); // wait for all threads to finish
		if (m_bManage)
			delete[] m_oData.pData;

		m_oEngine.unregisterSound(this);
	}

	void Sound::play(float volume)
	{
		std::thread trd(&rl::Sound::threadPlay, this, volume);
		trd.detach();
	}

	void Sound::pauseAll()
	{
		std::unique_lock<std::mutex> muxVector(m_muxVector);
		for (auto p : m_oVoices)
			p->pause();
		muxVector.unlock();
	}

	void Sound::resumeAll()
	{
		std::unique_lock<std::mutex> muxVector(m_muxVector);
		for (auto p : m_oVoices)
			p->resume();
		muxVector.unlock();
	}

	void Sound::stopAll()
	{
		std::unique_lock<std::mutex> muxVector(m_muxVector);
		for (auto p : m_oVoices)
			p->stop();
		muxVector.unlock();
	}

	void Sound::threadPlay(float volume)
	{
		m_iThreadCount++;
		std::mutex mux;
		std::unique_lock<std::mutex> lm(mux);
		std::condition_variable cv;
		Voice voice(m_oEngine, mux, cv, m_oData, volume);

		std::unique_lock<std::mutex> muxVector(m_muxVector);
		m_oVoices.push_back(&voice);
		muxVector.unlock();

		cv.wait(lm); // wait for voice to stop

		muxVector.lock();
		m_oVoices.erase(std::find(m_oVoices.begin(), m_oVoices.end(), &voice));
		muxVector.unlock();

		m_iThreadCount--;

		// RAII will destroy the voice
	}










	/***********************************************************************************************
	class IAudioStream
	***********************************************************************************************/

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IAudioStream::IAudioStream(AudioEngine& engine) : m_oEngine(engine),
		m_oCallback(this, m_mux, m_cv)
	{
		m_oEngine.registerStream(this);
	}

	IAudioStream::~IAudioStream()
	{
		stop();
		m_oEngine.unregisterStream(this);
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void IAudioStream::play(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate,
		float volume, size_t BufferBlocks, size_t BufferBlockSamples)
	{
		stop();

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

		m_fVolume = volume;

		m_iBlockSize = m_iBufferBlocks * m_iBufferBlockSamples * m_iChannelCount *
			(m_iBitsPerSample / 8);

		m_trd = std::thread(&rl::IAudioStream::threadFunc, this, BitsPerSample, ChannelCount,
			SampleRate, volume);
	}

	void IAudioStream::play3D(Audio3DPos pos, uint8_t BitsPerSample, uint8_t ChannelCount,
		uint32_t SampleRate, float volume, size_t BufferBlocks, size_t BufferBlockSamples)
	{
		stop();

		m_b3D = true;
		m_oPos = pos;
		play(BitsPerSample, ChannelCount, SampleRate, volume, BufferBlocks, BufferBlockSamples);
	}

	void IAudioStream::setPos(Audio3DPos pos, float volume)
	{
		if (!m_b3D)
			return;

		m_oPos = pos;
		refresh3DOutput();
		setVolume(volume);
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

		if (m_trd.joinable())
			m_trd.join(); // wait for thread to finish

		m_bPaused = false;
		m_b3D = false;
	}

	void IAudioStream::setVolume(float volume)
	{
		if (!m_bRunning)
			return;

		m_pVoice->SetVolume(volume);
	}

	void IAudioStream::refresh3DOutput()
	{
		if (!m_b3D)
			return;

		m_oEngine.get3DOutputVolume(m_oPos, m_f3DVolume);
		m_pVoiceMono->SetOutputMatrix(m_pVoiceSurround, 1, 8, m_f3DVolume);
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void IAudioStream::threadFunc(uint8_t iBitsPerSample, uint8_t iChannelCount,
		uint32_t iSampleRate, float fVolume)
	{
		WAVEFORMATEX wavfmt = CreateWaveFmt(iBitsPerSample, iChannelCount, iSampleRate);
		if (!m_b3D) // regular playback
		{
			m_oEngine.getEngine()->CreateSourceVoice(&m_pVoice, &wavfmt, 0,
				XAUDIO2_DEFAULT_FREQ_RATIO, &m_oCallback);
		}
		else // 3D mode
		{
			auto p = m_oEngine.getEngine();
			XAUDIO2_SEND_DESCRIPTOR send = { 0 };
			XAUDIO2_VOICE_SENDS sendlist = { 1, &send };

			// 7.1 surround submix
			p->CreateSubmixVoice(&m_pVoiceSurround, 8, iSampleRate, 0, 1);

			// mono submix
			send.pOutputVoice = m_pVoiceSurround;
			p->CreateSubmixVoice(&m_pVoiceMono, 1, iSampleRate, 0, 0, &sendlist);

			// source input
			send.pOutputVoice = m_pVoiceMono;
			p->CreateSourceVoice(&m_pVoice, &wavfmt, 0,
				XAUDIO2_DEFAULT_FREQ_RATIO, &m_oCallback, &sendlist);
			refresh3DOutput();
		}

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
		m_bRunning = true;

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

		m_pVoice->Stop();
		m_pVoice->FlushSourceBuffers();
		m_pVoice->DestroyVoice();
		m_pVoice = nullptr;

		if (m_b3D)
		{
			m_pVoiceMono->DestroyVoice();
			m_pVoiceMono = nullptr;
			m_pVoiceSurround->DestroyVoice();
			m_pVoiceSurround = nullptr;
		}

		delete[] m_pBufferData;
		m_pBufferData = nullptr;

		OnShutdown();
	}

	IAudioStream::Callback::Callback(IAudioStream* stream, std::mutex& mux,
		std::condition_variable& cv) : m_pStream(stream), m_mux(mux), m_cv(cv) {}

	void IAudioStream::Callback::OnBufferEnd(void* pBufferContext)
	{
		m_pStream->m_iBlocksFree++;
		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();
	}

	void IAudioStream::Callback::OnStreamEnd() { m_pStream->stop(); }










	/***********************************************************************************************
	class AudioEngine
	***********************************************************************************************/

	/// <summary>
	/// Write the hexadecimal representation of a <c>HRESULT</c> into a c-string
	/// </summary>
	/// <param name="str">= a c-string with a minimum length of 8</param>
	void WriteHResultHexIntoString(HRESULT hr, char* str)
	{
		static char cHEX[] = "0123456789ABCDEF";

		for (uint8_t i = 0; i < sizeof(hr) * 2; i++)
		{
			str[i] = cHEX[(hr >> ((7 - i) * 4)) & 0x0F];
		}
	}

	class SoundCallback : public IXAudio2VoiceCallback
	{
	private: // variables

		IXAudio2SourceVoice* m_pVoice = nullptr;
		std::mutex* m_mux = nullptr;
		std::condition_variable* m_cv = nullptr;


	public: // methods

		void set(IXAudio2SourceVoice* src, std::mutex* mux, std::condition_variable* cv)
		{
			m_pVoice = src;
			m_mux = mux;
			m_cv = cv;
		}


		void __stdcall OnBufferEnd(void* pBufferContext) override {}
		void __stdcall OnBufferStart(void* pBufferContext) override {}
		void __stdcall OnLoopEnd(void* pBufferContext) override {}
		void __stdcall OnStreamEnd() override
		{
			m_pVoice->Stop();
			m_pVoice->FlushSourceBuffers();

			std::unique_lock<std::mutex> lm(*m_mux);
			m_cv->notify_one();
		}
		void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override {}
		void __stdcall OnVoiceProcessingPassEnd() override {}
		void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
	};


	//==============================================================================================
	// STATIC VARIABLES

	IXAudio2* AudioEngine::m_pXAudio2 = nullptr;
	IXAudio2MasteringVoice* AudioEngine::m_pMaster = nullptr;
	std::vector<Sound*> AudioEngine::m_pSounds;
	std::mutex AudioEngine::m_muxSounds;
	std::vector<IAudioStream*> AudioEngine::m_pStreams;
	std::mutex AudioEngine::m_muxStreams;
	std::atomic<bool> AudioEngine::m_bRunning = false;
	uint8_t AudioEngine::m_iChannelCount = 0;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::AudioEngine()
	{
		HRESULT hr;
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			char szException[] = "Couldn't initialize COM (error code: 0x00000000)";
			WriteHResultHexIntoString(hr, &szException[39]);

			throw std::exception(szException);
		}

		hr = XAudio2Create(&m_pXAudio2);
		if (FAILED(hr))
		{
			char szException[] = "Couldn't create XAudio2 object (error code: 0x00000000)";
			WriteHResultHexIntoString(hr, &szException[46]);

			throw std::exception(szException);
		}
	}

	AudioEngine::~AudioEngine()
	{
		destroy();

		if (m_pXAudio2 != nullptr)
			m_pXAudio2->Release();

		CoUninitialize();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	AudioEngine& AudioEngine::getInstance()
	{
		static AudioEngine engine;
		return engine;
	}

	bool AudioEngine::create(const wchar_t* szDeviceID, uint8_t ChannelCount, uint32_t samplerate,
		size_t StreamBufBlockCount, size_t StreamBufSamples)
	{
		destroy();

		HRESULT hr = m_pXAudio2->CreateMasteringVoice(&m_pMaster, ChannelCount, samplerate, 0,
			szDeviceID);

		// refresh 3D output in 3D streams
		for (IAudioStream* stream : m_pStreams)
		{
			if (stream->is3D())
				stream->refresh3DOutput();
		}

		m_iChannelCount = ChannelCount;
		m_bRunning = true;

		return true;
	}

	void AudioEngine::destroy(bool StopSounds)
	{
		if (!m_bRunning)
			return;


		std::unique_lock<std::mutex> lmSounds(m_muxSounds);
		std::unique_lock<std::mutex> lmStreams(m_muxStreams);
		if (StopSounds)
		{
			for (Sound* pSnd : m_pSounds)
				pSnd->stopAll();
			for (IAudioStream* pStream : m_pStreams)
				pStream->stop();
		}
		lmSounds.unlock();
		lmStreams.unlock();


		if (m_pMaster != nullptr)
		{
			m_pMaster->DestroyVoice();
			m_pMaster = nullptr;
		}

		m_iChannelCount = 0;
		m_bRunning = false;
	}

	void AudioEngine::registerSound(Sound* snd)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxSounds);
		m_pSounds.push_back(snd);
		lm.unlock();
	}

	void AudioEngine::unregisterSound(Sound* snd)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxSounds);

		auto it = std::find(m_pSounds.begin(), m_pSounds.end(), snd);
		if (it != m_pSounds.end())
			m_pSounds.erase(it);

		lm.unlock();
	}

	void AudioEngine::registerStream(IAudioStream* stream)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxStreams);
		m_pStreams.push_back(stream);
		lm.unlock();
	}

	void AudioEngine::unregisterStream(IAudioStream* stream)
	{
		if (!m_bRunning)
			return;

		std::unique_lock<std::mutex> lm(m_muxStreams);

		auto it = std::find(m_pStreams.begin(), m_pStreams.end(), stream);
		if (it != m_pStreams.end())
			m_pStreams.erase(it);

		lm.unlock();
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


}