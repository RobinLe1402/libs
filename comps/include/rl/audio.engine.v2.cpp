#include "audio.engine.v2.hpp"
#include "tools.hresult.hpp"

#include <fstream> // std::ifstream
#include <memory> // memcpy
#include <stdint.h>
#define NOMINMAX
#include <Windows.h>

#undef min
#undef max





namespace rl
{

	constexpr bool ValidWaveFormat(const WaveFormat& format) noexcept
	{
		return format.iChannelCount > 0 && format.iChannelCount <= XAUDIO2_MAX_AUDIO_CHANNELS &&
			format.iSampleRate >= XAUDIO2_MIN_SAMPLE_RATE &&
			format.iSampleRate <= XAUDIO2_MAX_SAMPLE_RATE;
	}

	constexpr WAVEFORMATEX CreateWaveFormatEx(const WaveFormat& format) noexcept
	{
		WAVEFORMATEX result = { sizeof(WAVEFORMATEX) };

		result.wBitsPerSample = (WORD)format.eBitDepth;
		result.nChannels = format.iChannelCount;
		result.nSamplesPerSec = format.iSampleRate;

		result.wFormatTag = (result.wBitsPerSample < 32 ? WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT);
		result.nBlockAlign = result.wBitsPerSample / 8 * result.nChannels;
		result.nAvgBytesPerSec = result.nBlockAlign * result.nSamplesPerSec;

		return result;
	}

	namespace RIFF
	{
		constexpr FOURCC FourCC(const char(&szString)[5])
		{
			DWORD result = 0;

			for (uint8_t i = 0; i < 4; ++i)
			{
				result |= (DWORD)szString[i] << (i * 8);
			}

			return result;
		}


		struct ChunkHeader
		{
			FOURCC ckID; // Chunk type identifier
			DWORD ckSize; // Chunk size field
		};

		using WaveformHeader = WAVEFORMAT;
	}


	void SurroundStructToFloatMatrix(const Audio3DPos& pos, float(&result)[8])
	{
		// references for simplification
		float& fFrontLeft = result[0];
		float& fFrontRight = result[1];
		float& fFrontCenter = result[2];
		float& fLFE = result[3];
		float& fBackLeft = result[4];
		float& fBackRight = result[5];
		float& fSideLeft = result[6];
		float& fSideRight = result[7];



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

	const Audio3DPos Audio3DPos::Center = { 0.0f, 0.0f };
	const Audio3DPos Audio3DPos::Left = { -1.0f, 0.0f };
	const Audio3DPos Audio3DPos::Right = { +1.0f, 0.0f };





	/***********************************************************************************************
	 struct int24_t
	***********************************************************************************************/


	int32_t int24_t::asInt32()
	{
		int32_t i = 0;
		memcpy((uint8_t*)&i + (BigEndian ? 1 : 0), this, 3);
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

		memcpy(this, (uint8_t*)&i + (BigEndian ? 1 : 0), 3);
	}










	/***********************************************************************************************
	 struct MultiChannelAudioSample
	***********************************************************************************************/

	void MultiChannelAudioSample::free()
	{
		if (!val.p8)
			return;

#define CASE(bits)					\
		case bits:					\
			delete[] val.p##bits;	\
			break

		switch (iBitsPerSample)
		{
			CASE(8);
			CASE(16);
			CASE(24);
			CASE(32);
		}

#undef CASE

		val.p8 = nullptr;
	}










	/***********************************************************************************************
	 class AudioEngine
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	std::thread AudioEngine::trdMsgLoop;
	std::mutex AudioEngine::muxMsgLoop;
	std::condition_variable AudioEngine::cvMsgLoop;
	std::queue<AudioEngine::Message> AudioEngine::oMessageQueue;
	HRESULT AudioEngine::hr;





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::AudioEngine()
	{
		// initialize COM
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't initialize COM:\n");

		// start message loop
		trdMsgLoop = std::thread(MessageLoop);

		createEngine();
	}

	AudioEngine::~AudioEngine()
	{
		destroy();
		destroyEngine();


		// quit message loop
		PostMsg(MessageVal::Quit, nullptr);
		if (trdMsgLoop.joinable())
			trdMsgLoop.join();

		// uninitialize COM
		CoUninitialize();
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	AudioEngine& AudioEngine::GetInstance()
	{
		static AudioEngine oInstance;
		return oInstance;
	}

	void AudioEngine::PostMsg(MessageVal eMsg, void* ptr)
	{
		std::unique_lock lm(muxMsgLoop);
		oMessageQueue.push({ eMsg, ptr });
		lm.unlock();
		cvMsgLoop.notify_one();
	}

	void AudioEngine::MessageLoop()
	{
		std::unique_lock lm(muxMsgLoop);

		while (true)
		{
			while (oMessageQueue.size() > 0)
			{
				auto& msg = oMessageQueue.front();

				switch (msg.eMsg)
				{
				case MessageVal::DestroyVoice:
					reinterpret_cast<IXAudio2Voice*>(msg.ptr)->DestroyVoice();
					break;

				case MessageVal::DestroyEngine:
					reinterpret_cast<IXAudio2*>(msg.ptr)->Release();
					break;

				case MessageVal::Quit:
					oMessageQueue.pop(); // remove quit message
					return; // escape message loop
				}

				oMessageQueue.pop(); // remove handled message
			}
			cvMsgLoop.wait(lm);
		}
	}

	void AudioEngine::ProcessMessages() { cvMsgLoop.notify_one(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool AudioEngine::create(uint8_t ChannelCount, uint32_t SampleRate)
	{
		destroy();

		IXAudio2MasteringVoice* pMasteringVoice = nullptr;
		hr = m_pEngine->CreateMasteringVoice(&pMasteringVoice, ChannelCount, SampleRate,
			0, NULL);

		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 mastering voice:\n");

		std::set<SubmixVoice*> oParents;
		m_pMasteringVoice = new MasteringVoice(pMasteringVoice);

		m_iChannelCount = ChannelCount;

		m_bCreated = true;

		return true;
	}

	void AudioEngine::destroy()
	{
		if (!m_bCreated)
			return;

		delete m_pMasteringVoice;
		m_bCreated = false;
	}

	HRESULT AudioEngine::createSubmixVoice(SubmixVoice** dest, UINT32 InputChannels,
		UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const VoiceSends* sends,
		const XAUDIO2_EFFECT_CHAIN* pEffectChain)
	{
		if (!m_bCreated)
			return ERROR_INIT_STATUS_NEEDED;

		IXAudio2SubmixVoice* pSubmixVoice = nullptr;
		XAUDIO2_VOICE_SENDS SendList{};
		XAUDIO2_VOICE_SENDS* pSendList;
		if (sends)
		{
			pSendList = &SendList;
			SendList.SendCount = sends->iSendCount;
			SendList.pSends = new XAUDIO2_SEND_DESCRIPTOR[sends->iSendCount];

			for (size_t i = 0; i < sends->iSendCount; ++i)
			{
				SendList.pSends[i].Flags = sends->pSends[i].iFlags;
				SendList.pSends[i].pOutputVoice = sends->pSends[i].pOutputVoice->getPtr();
			}
		}
		else
			pSendList = nullptr;

		hr = m_pEngine->CreateSubmixVoice(&pSubmixVoice, InputChannels, InputSampleRate,
			Flags, ProcessingStage, pSendList, pEffectChain);

		if (SUCCEEDED(hr))
		{
			std::set<SubmixVoice*> oSends;
			if (sends)
			{
				for (size_t i = 0; i < sends->iSendCount; ++i)
				{
					oSends.insert(sends->pSends[i].pOutputVoice);
				}
			}
			*dest = new SubmixVoice(pSubmixVoice, oSends);
		}
		else
			*dest = nullptr;

		if (sends)
			delete[] SendList.pSends;

		return hr;
	}

	HRESULT AudioEngine::createSourceVoice(SourceVoice** dest, const WAVEFORMATEX* pSourceFormat,
		UINT32 Flags, float MaxFrequencyRatio, const VoiceSends* sends,
		const XAUDIO2_EFFECT_CHAIN* pEffectChain)
	{
		if (!m_bCreated)
			return ERROR_INIT_STATUS_NEEDED;

		IXAudio2SourceVoice* pSourceVoice = nullptr;
		XAUDIO2_VOICE_SENDS SendList{};
		XAUDIO2_VOICE_SENDS* pSendList;
		if (sends)
		{
			pSendList = &SendList;
			SendList.SendCount = sends->iSendCount;
			SendList.pSends = new XAUDIO2_SEND_DESCRIPTOR[sends->iSendCount];

			for (size_t i = 0; i < sends->iSendCount; ++i)
			{
				SendList.pSends[i].Flags = sends->pSends[i].iFlags;
				SendList.pSends[i].pOutputVoice = sends->pSends[i].pOutputVoice->getPtr();
			}
		}
		else
			pSendList = nullptr;

		auto pCallback = new SourceVoice::Callback;

		hr = m_pEngine->CreateSourceVoice(&pSourceVoice, pSourceFormat, Flags,
			MaxFrequencyRatio, pCallback, pSendList, pEffectChain);

		if (SUCCEEDED(hr))
		{
			std::set<SubmixVoice*> oSends;
			if (sends)
			{
				for (size_t i = 0; i < sends->iSendCount; ++i)
				{
					oSends.insert(sends->pSends[i].pOutputVoice);
				}
			}
			else
				oSends.insert(m_pMasteringVoice);
			*dest = new SourceVoice(pSourceVoice, oSends, pCallback);
		}
		else
		{
			delete pCallback;
			*dest = nullptr;
		}

		if (sends)
			delete[] SendList.pSends;

		return hr;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void AudioEngine::createEngine()
	{
		hr = XAudio2Create(&m_pEngine);
		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 object:\n");

		m_bEngineExists = true;
	}

	void AudioEngine::destroyEngine()
	{
		if (!m_bEngineExists)
			return;

		PostMsg(MessageVal::DestroyEngine, m_pEngine);
		m_pEngine = nullptr;
		m_bEngineExists = false;
	}










	/***********************************************************************************************
	 class AudioEngine::Voice
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::Voice::Voice(IXAudio2Voice* ptr, const std::set<SubmixVoice*>& parents,
		VoiceType type) : m_pVoice(ptr), m_eVoiceType(type)
	{
		std::unique_lock lm(AudioEngine::GetInstance().m_muxVoices);

		if (parents.size() > 0)
		{
			for (auto p : parents)
			{
				m_oParents.insert(p);
				p->m_oSubVoices.insert(this);
			}
		}
		else if (type != VoiceType::MasteringVoice)
		{

			auto pParent = AudioEngine::GetInstance().m_pMasteringVoice;
			m_oParents.insert(pParent);
			reinterpret_cast<SubmixVoice*>(pParent)->m_oSubVoices.insert(this);
		}
	}

	AudioEngine::Voice::~Voice()
	{
		// destroy() must be called in the destructor of the class with the latest overload
		if (m_eVoiceType == VoiceType::SourceVoice)
			destroy();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioEngine::Voice::destroy()
	{
		if (!m_pVoice)
			return;

		for (auto p : m_oParents)
		{
			p->m_oSubVoices.erase(this);
		}
		m_oParents.clear();

		AudioEngine::PostMsg(MessageVal::DestroyVoice, m_pVoice);
		m_pVoice = nullptr;
	}










	/***********************************************************************************************
	 class AudioEngine::SourceVoice
	***********************************************************************************************/

	//==============================================================================================
	// METHODS

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::SourceVoice::SourceVoice(IXAudio2SourceVoice* ptr,
		const std::set<SubmixVoice*>& parents, Callback* pCallback) :
		Voice(ptr, parents, VoiceType::SourceVoice), m_pCallback(pCallback)
	{
		m_pCallback->pOwner = this;
	}

	AudioEngine::SourceVoice::~SourceVoice()
	{
		delete m_pCallback;
	}










	/***********************************************************************************************
	 class AudioEngine::SubmixVoice
	***********************************************************************************************/

	//==============================================================================================
	// METHODS

	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::SubmixVoice::~SubmixVoice() { destroy(); }


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioEngine::SubmixVoice::destroy()
	{
		// destroy subvoices
		while (m_oSubVoices.size() > 0)
		{
			auto pVoice = *m_oSubVoices.begin();

			pVoice->destroy();
			delete pVoice;
		}

		Voice::destroy();
	}










	/***********************************************************************************************
	 class AudioEngine::SourceVoice::Callback
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	template <typename ...Args>
	void AudioEngine::SourceVoice::Callback::invoke(std::function<void(Args...)> fn, Args... args)
	{
		if (fn)
			fn(args...);
	}










	/***********************************************************************************************
	 class AudioEngine::MasteringVoice
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioEngine::MasteringVoice::~MasteringVoice()
	{
		std::unique_lock lm(AudioEngine::GetInstance().m_muxVoices);
		destroy();
	}










	/***********************************************************************************************
	 class Sound
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Sound::Sound() : m_oWavFmt{}, m_iSampleAlign(0), m_iSampleCount(0), m_iDataSize(0),
		m_pData(nullptr) {}

	Sound::Sound(const Sound& other) : m_oWavFmt(other.m_oWavFmt),
		m_iSampleAlign(other.m_iSampleAlign), m_iSampleCount(other.m_iSampleCount),
		m_iDataSize(other.m_iDataSize), m_pData(nullptr)
	{
		if (m_iDataSize == 0)
			return;

		m_pData = new uint8_t[m_iDataSize];
		memcpy_s(m_pData, m_iDataSize, other.m_pData, other.m_iDataSize);
	}

	Sound::Sound(Sound&& rval) noexcept : m_oWavFmt(rval.m_oWavFmt),
		m_iSampleAlign(rval.m_iSampleAlign), m_iSampleCount(rval.m_iSampleCount),
		m_iDataSize(rval.m_iDataSize), m_pData(rval.m_pData)
	{
		rval.m_oWavFmt = {};
		rval.m_iSampleCount = {};
		rval.m_iDataSize = 0;
		rval.m_pData = nullptr;
	}

	Sound::Sound(const WaveFormat& Format, size_t SampleCount) :
		m_oWavFmt(Format), m_iSampleAlign((size_t)Format.eBitDepth / 8 * Format.iChannelCount),
		m_iSampleCount(SampleCount), m_iDataSize(m_iSampleAlign* m_iSampleCount), m_pData(nullptr)
	{
		if (!ValidWaveFormat(Format))
		{
			clear();
			return;
		}

		m_pData = new uint8_t[m_iDataSize];
		memset(m_pData, 0, m_iDataSize);
	}

	Sound::~Sound() { delete[] m_pData; }





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	Sound& Sound::operator=(const Sound& other)
	{
		delete[] m_pData;

		m_oWavFmt = other.m_oWavFmt;
		m_iSampleAlign = other.m_iSampleAlign;
		m_iSampleCount = other.m_iSampleCount;
		m_iDataSize = other.m_iDataSize;

		if (m_iDataSize)
		{
			m_pData = new uint8_t[m_iDataSize];
			memcpy_s(m_pData, m_iDataSize, other.m_pData, other.m_iDataSize);
		}
		else
			m_pData = nullptr;

		return *this;
	}

	Sound& Sound::operator=(Sound&& rval) noexcept
	{
		if (&rval == this)
			return *this;

		delete[] m_pData;

		m_oWavFmt = rval.m_oWavFmt;
		m_iSampleAlign = rval.m_iSampleAlign;
		m_iSampleCount = rval.m_iSampleCount;
		m_iDataSize = rval.m_iDataSize;
		m_pData = rval.m_pData;

		rval.clear();

		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	Sound* Sound::FromFile(const wchar_t* szFileName)
	{
		std::ifstream file(szFileName, std::ios::binary | std::ios::ate);
		if (!file)
			return nullptr; // couldn't open file

		const size_t size = file.tellg();
		auto up_Data = std::make_unique<char[]>(size);
		file.seekg(0, std::ios::beg);
		if (!file.read(up_Data.get(), size))
			return nullptr; // couldn't read file

		return FromMemory(up_Data.get(), size);
	}

	Sound* Sound::FromResource(HMODULE hModule, LPCWSTR lpName)
	{
		HRSRC hRsrc = FindResourceW(hModule, lpName, L"WAVE");
		if (hRsrc == NULL)
			return nullptr; // couldn't find resource
		HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
		if (hGlobal == NULL)
			return nullptr; // couldn't load resource
		LPVOID lpVoid = LockResource(hGlobal);
		if (lpVoid == NULL)
			return nullptr; // couldn't lock resource

		return FromMemory(lpVoid, SizeofResource(hModule, hRsrc));
	}

	Sound* Sound::FromMemory(const void* data, size_t size)
	{
		using namespace RIFF;

		constexpr FOURCC fccData = FourCC("data");

		const uint8_t* pData = static_cast<const uint8_t*>(data);
#define OFFSET (pData - data)
#define REMAINING(cb) (size < OFFSET + cb)

		pData += 0x10; // skip RIFF header, "WAVE" and "fmt "


		const DWORD wHeaderSize = *reinterpret_cast<const DWORD*>(pData);
		pData += sizeof(wHeaderSize);

		const WaveformHeader hdr = *reinterpret_cast<const WaveformHeader*>(pData);
		pData += sizeof(WaveformHeader);

		WORD wBitsPerSample;
		switch (hdr.wFormatTag)
		{
		case (WAVE_FORMAT_PCM):
			wBitsPerSample = *reinterpret_cast<const WORD*>(pData);
			break;

		case WAVE_FORMAT_IEEE_FLOAT:
			wBitsPerSample = sizeof(float) * 8;
			break;

		default:
			return nullptr; // unknown format
		}

		pData += wHeaderSize - sizeof(WaveformHeader);

		ChunkHeader chData = {};
		while (true)
		{
			chData = *reinterpret_cast<const ChunkHeader*>(pData);
			pData += sizeof(ChunkHeader);

			if (chData.ckID != fccData)
				pData += chData.ckSize;
			else
				break;
		}

		WaveFormat wf = {};
		wf.eBitDepth = static_cast<AudioBitDepth>(wBitsPerSample);
		wf.iChannelCount = (uint8_t)hdr.nChannels;
		wf.iSampleRate = hdr.nSamplesPerSec;

		Sound* result = new Sound(wf, chData.ckSize / hdr.nBlockAlign);
		memcpy_s(result->m_pData, result->m_iDataSize, pData, chData.ckSize);
		return result;
#undef OFFSET
	}

	void Sound::clear()
	{
		delete[] m_pData;
		m_pData = nullptr;

		m_oWavFmt = {};
		m_iSampleAlign = 0;
		m_iSampleCount = 0;
		m_iDataSize = 0;
	}

	bool Sound::getSample(MultiChannelAudioSample& dest, size_t iSampleID) const
	{
		if (!m_pData || iSampleID >= m_iSampleCount)
			return false;

#define CASE(bits)																\
		case AudioBitDepth::Audio##bits:										\
			dest.val.p##bits = new audio##bits##_t[m_oWavFmt.iChannelCount];	\
			break

		dest.free();
		switch (m_oWavFmt.eBitDepth)
		{
			CASE(8);
			CASE(16);
			CASE(24);
			CASE(32);
		}

#undef CASE

		memcpy_s(dest.val.p8, m_iSampleAlign, m_pData + iSampleID * m_iSampleAlign, m_iSampleAlign);

		return true;
	}

	SoundInstance* Sound::play(float volume)
	{
		if (volume < 0.0f)
			volume = 0.0f;

		return new SoundInstance(*this, volume);
	}

	SoundInstance3D* Sound::play3D(const Audio3DPos& pos, float volume)
	{
		if (volume < 0.0f)
			volume = 0.0f;

		return new SoundInstance3D(*this, volume, pos);
	}










	/***********************************************************************************************
	 class SoundInstance
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	SoundInstance::SoundInstance(const Sound& sound, float volume) :
		m_eType(SoundInstanceType::Default), m_fVolume(volume)
	{

		auto wf = CreateWaveFormatEx(sound.getWaveFormat());

		if (!createVoices(wf) || !loadSound(sound))
			return; // couldn't create the necessary XAudio2 voices/load the sound data

		m_bPaused = false;
		m_bPlaying = true;

		auto pVoice = m_pSourceVoice->getPtr();

		pVoice->SetVolume(m_fVolume);
		pVoice->Start();
	}

	SoundInstance::~SoundInstance() { stop(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void SoundInstance::pause()
	{
		std::unique_lock lm(m_mux);

		if (!m_bPlaying || m_bPaused)
			return;

		m_pSourceVoice->getPtr()->Stop();
		m_bPaused = true;
	}

	void SoundInstance::resume()
	{
		std::unique_lock lm(m_mux);

		if (!m_bPlaying || !m_bPaused)
			return;

		m_pSourceVoice->getPtr()->Start();
		m_bPaused = false;
	}

	void SoundInstance::stop()
	{
		std::unique_lock lm(m_mux);

		if (!m_bVoiceExists)
			return;

		m_pSourceVoice->getPtr()->Stop();

		destroyVoices();

		m_bPlaying = false;
		m_bPaused = false;
	}

	void SoundInstance::waitForEnd()
	{
		if (!m_bPlaying || m_bPaused)
			return;

		std::unique_lock lm(m_mux);
		m_cv.wait(lm);
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	bool SoundInstance::createVoices(const WAVEFORMATEX& format)
	{
		HRESULT hr = AudioEngine::GetInstance().createSourceVoice(&m_pSourceVoice, &format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO);
		if (FAILED(hr))
			return false;

		m_pSourceVoice->OnStreamEnd = [&]() { onEnd(); };
		m_bVoiceExists = true;
		return true;
	}

	void SoundInstance::destroyVoices()
	{
		delete m_pSourceVoice;
		m_pSourceVoice = nullptr;
		m_bVoiceExists = false;
	}

	bool SoundInstance::loadSound(const Sound& sound)
	{
		XAUDIO2_BUFFER buf = {};
		buf.AudioBytes = (UINT32)sound.getDataSize();
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.pAudioData = (BYTE*)sound.getDataPtr();

		auto pVoice = m_pSourceVoice->getPtr();
		HRESULT hr = pVoice->SubmitSourceBuffer(&buf);

		return SUCCEEDED(hr);
	}

	void SoundInstance::onEnd()
	{
		std::unique_lock lm(m_mux);

		m_bPlaying = false;

		m_cv.notify_all();
	}










	/***********************************************************************************************
	 class SoundInstance3D
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	SoundInstance3D::SoundInstance3D(const Sound& sound, float volume, const Audio3DPos& pos) :
		SoundInstance(SoundInstanceType::Surround, volume), m_o3DPos(pos)
	{
		auto wf = CreateWaveFormatEx(sound.getWaveFormat());

		if (!createVoices(wf) || !loadSound(sound))
			return; // couldn't create the necessary XAudio2 voices/load the sound data

		m_bPaused = false;
		m_bPlaying = true;

		auto pVoice = m_pSourceVoice->getPtr();

		pVoice->SetVolume(m_fVolume);
		pVoice->Start();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void SoundInstance3D::set3DPos(const Audio3DPos& pos)
	{
		if (!m_bVoiceExists)
			return;

		m_o3DPos = pos;
		applyPos();
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	bool SoundInstance3D::createVoices(const WAVEFORMATEX& format)
	{
		auto& oEngine = AudioEngine::GetInstance();

		HRESULT hr;

		// 1. create 7.1 surround voice
		hr = oEngine.createSubmixVoice(&m_pSubmixVoice_Surround, 8, format.nSamplesPerSec, 0, 1);
		if (FAILED(hr))
			return false;

		// 2. create mono voice
		AudioEngine::VoiceSendDescriptor oSendDescriptor = { 0, m_pSubmixVoice_Surround };
		AudioEngine::VoiceSends oSends = { 1, &oSendDescriptor };
		hr = oEngine.createSubmixVoice(&m_pSubmixVoice_Mono, 1, format.nSamplesPerSec, 0, 0, &oSends);
		if (FAILED(hr))
		{
			delete m_pSubmixVoice_Surround;
			return false;
		}

		// 3. create source voice
		oSendDescriptor.pOutputVoice = m_pSubmixVoice_Mono;
		hr = oEngine.createSourceVoice(&m_pSourceVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO,
			&oSends);
		if (FAILED(hr))
		{
			delete m_pSubmixVoice_Surround;
			return false;
		}

		applyPos();

		m_pSourceVoice->OnStreamEnd = [&]() { onEnd(); };
		m_bVoiceExists = true;
		return true;
	}

	void SoundInstance3D::destroyVoices()
	{
		delete m_pSubmixVoice_Surround;

		m_pSourceVoice = nullptr;
		m_pSubmixVoice_Mono = nullptr;
		m_pSubmixVoice_Surround = nullptr;

		m_bVoiceExists = false;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void SoundInstance3D::applyPos()
	{
		SurroundStructToFloatMatrix(m_o3DPos, m_fSurroundVolume);

		m_pSubmixVoice_Mono->getPtr()->SetOutputMatrix(m_pSubmixVoice_Surround->getPtr(), 1, 8,
			m_fSurroundVolume);
	}










	/***********************************************************************************************
	 class IAudioStream
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IAudioStream::~IAudioStream() { stop(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void IAudioStream::setVolume(float volume)
	{
		if (!m_pSourceVoice)
			return;

		if (volume < 0.0f)
			volume = 0.0f;
		m_pSourceVoice->getPtr()->SetVolume(volume);
		m_fVolume = volume;
	}

	void IAudioStream::pause()
	{
		if (!m_bRunning || m_bPaused)
			return;

		m_pSourceVoice->getPtr()->Stop();
		m_bPaused = true;
	}

	void IAudioStream::resume()
	{
		if (!m_bRunning || !m_bPaused)
			return;

		m_pSourceVoice->getPtr()->Start();
		m_bPaused = false;
	}

	void IAudioStream::stop()
	{
		if (!m_pSourceVoice)
			return;

		m_bRunning = false;

		if (m_trdSampleGeneration.joinable())
			m_trdSampleGeneration.join();

		delete m_pSourceVoice;
		m_pSourceVoice = nullptr;

		delete[] m_pBuffer;
		m_pBuffer = nullptr;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void IAudioStream::internalStart(const WaveFormat& format, float volume, size_t BufferBlockCount,
		size_t SamplesPerBufferBlock)
	{
		stop();
		if (volume < 0.0f)
			volume = 0.0f;

		if (BufferBlockCount < 2 || SamplesPerBufferBlock == 0)
			return;

		if (!ValidWaveFormat(format))
			return;

		m_oFormat = format;
		m_iByteDepth = static_cast<uint8_t>(m_oFormat.eBitDepth) / 8;
		m_fVolume = volume;
		m_iBlockCount = BufferBlockCount;
		m_iSamplesPerBlock = SamplesPerBufferBlock;
		m_iBlockSize = m_iSamplesPerBlock * m_iByteDepth;
		m_iFreeBlocks = m_iBlockCount;

		m_pBuffer = new uint8_t[m_iBlockCount * m_iBlockSize];

		const auto wfe = CreateWaveFormatEx(format);

		AudioEngine::GetInstance().createSourceVoice(&m_pSourceVoice, &wfe);
		m_pSourceVoice->getPtr()->SetVolume(m_fVolume);
		m_pSourceVoice->OnBufferEnd = [&](void* pBufferContext)
		{
			++m_iFreeBlocks;
			m_cv.notify_one();
		};

		m_trdSampleGeneration = std::thread(&rl::IAudioStream::threadFunc, this);
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void IAudioStream::threadFunc()
	{
		m_fTimePerSample = 1.0f / m_oFormat.iSampleRate;
		m_iSampleAlign = m_oFormat.iChannelCount * m_iByteDepth;

		m_bRunning = true;
		m_bPaused = false;

		// fill buffers at startup
		for (size_t i = 0; m_bRunning && i < m_iBlockCount; ++i)
		{
			fillBlock();
		}

		m_pSourceVoice->getPtr()->Start();

		while (m_bRunning)
		{
			if (m_iFreeBlocks == 0)
			{
				std::unique_lock lm(m_mux);
				m_cv.wait(lm);
			}
			fillBlock();
		}

		m_bPaused = false;
		auto ptr = m_pSourceVoice->getPtr();
		if (!m_bEndOfStream) // --> not stopped by XAUDIO2_END_OF_STREAM
		{
			ptr->Stop();
			ptr->FlushSourceBuffers();
		}
	}

	void IAudioStream::fillBlock()
	{
		--m_iFreeBlocks;

		XAUDIO2_BUFFER buf = {};
		buf.AudioBytes = (UINT32)(m_iSamplesPerBlock * m_oFormat.iChannelCount * m_iByteDepth);
		auto pData = &m_pBuffer[m_iCurrentBlock * m_iBlockSize];
		memset(pData, 0, buf.AudioBytes);
		buf.pAudioData = pData;
		for (size_t i = 0; i < m_iSamplesPerBlock; ++i)
		{
			rl::MultiChannelAudioSample sample = {};
			sample.iBitsPerSample = static_cast<uint8_t>(m_oFormat.eBitDepth);
			sample.iChannelCount = m_oFormat.iChannelCount;
			sample.val.p8 = pData + i * m_iSampleAlign;

			if (!m_bRunning)
				break;
			m_bEndOfStream = !nextSample(m_fTimePerSample, sample);

			m_bRunning = m_bRunning && !m_bEndOfStream;
			if (!m_bRunning)
				break;
		}
		if (!m_bRunning)
			buf.Flags = XAUDIO2_END_OF_STREAM;

		m_pSourceVoice->getPtr()->SubmitSourceBuffer(&buf);

		++m_iCurrentBlock;
		m_iCurrentBlock %= m_iBlockCount;
	}\

}