#include "rlAudioStream.hpp"

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










	SoundVoice::SoundVoice(std::mutex& mux, std::condition_variable& cv,
		WaveformData& data, float volume) : m_mux(mux), m_cv(cv)
	{
		uint8_t i = data.iBitsPerSample / 8;
		if (i == 0 || i > 4 || data.iBitsPerSample % 8 > 0)
			throw std::exception("rl::SoundVoice: Invalid bitrate");

		WAVEFORMATEX wavformat{};
		wavformat.cbSize = 0;
		wavformat.nChannels = data.iChannelCount;
		wavformat.nSamplesPerSec = data.iSampleRate;
		wavformat.wBitsPerSample = data.iBitsPerSample;
		wavformat.wFormatTag = (wavformat.wBitsPerSample < 32 ?
			WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT);

		wavformat.nBlockAlign = wavformat.wBitsPerSample / 8 * wavformat.nChannels;
		wavformat.nAvgBytesPerSec = wavformat.nBlockAlign * wavformat.nSamplesPerSec;

		if (FAILED(AudioEngine::getEnginePtr()->CreateSourceVoice(&m_pVoice, &wavformat, 0,
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

	SoundVoice::~SoundVoice()
	{
		m_pVoice->DestroyVoice();
	}

	void SoundVoice::OnStreamEnd()
	{
		stop();
	}

	void SoundVoice::pause()
	{
		if (m_bPaused)
			return;

		m_pVoice->Stop();
		m_bPaused = true;
	}

	void SoundVoice::resume()
	{
		if (!m_bPaused)
			return;

		m_pVoice->Start();
		m_bPaused = false;
	}

	void SoundVoice::stop()
	{
		m_pVoice->Stop();
		m_pVoice->FlushSourceBuffers();

		std::unique_lock<std::mutex> lm(m_mux);
		m_cv.notify_one();
		m_mux.unlock();
		lm.release();
	}

	
	
	Sound::Sound(WaveformData data, bool ManageData)
	{
		m_oData = data;
		m_bManage = ManageData;
	}

	Sound::~Sound()
	{
		stopAll();
		while (m_iThreadCount > 0); // wait for all threads to finish
		if (m_bManage)
			delete[] m_oData.pData;
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
		SoundVoice voice(mux, cv, m_oData, volume);

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


		void OnBufferEnd(void* pBufferContext) override {}
		void OnBufferStart(void* pBufferContext) override {}
		void OnLoopEnd(void* pBufferContext) override {}
		void OnStreamEnd() override
		{
			m_pVoice->Stop();
			m_pVoice->FlushSourceBuffers();

			std::unique_lock<std::mutex> lm(*m_mux);
			m_cv->notify_one();
		}
		void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
		void OnVoiceProcessingPassEnd() override {}
		void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
	};


	//==============================================================================================
	// STATIC VARIABLES

	IXAudio2* AudioEngine::m_pXAudio2 = nullptr;
	IXAudio2MasteringVoice* AudioEngine::m_pMaster = nullptr;
	std::atomic<bool> AudioEngine::m_bRunning = false;





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
		size_t BufBlockCount, size_t BufSamples)
	{
		destroy();

		HRESULT hr = m_pXAudio2->CreateMasteringVoice(&m_pMaster, ChannelCount, samplerate, 0,
			szDeviceID);


		m_bRunning = true;

		return true;
	}

	void AudioEngine::destroy()
	{
		if (!m_bRunning)
			return;



		if (m_pMaster != nullptr)
		{
			m_pMaster->DestroyVoice();
			m_pMaster = nullptr;
		}
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}