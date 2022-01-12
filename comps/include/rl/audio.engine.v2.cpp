#include "audio.engine.v2.hpp"
#include "tools.hresult.hpp"

#include <memory> // memcpy
#include <stdint.h>
#define NOMINMAX
#include <Windows.h>

#undef min
#undef max





namespace rl
{

	/***********************************************************************************************
	 struct int24_t
	***********************************************************************************************/

	constexpr bool BigEndian()
	{
		constexpr uint16_t i = 0x00FF;
		return (const uint8_t&)i == 0x00;
	}

	constexpr bool bBigEndian = BigEndian();


	int32_t int24_t::asInt32()
	{
		int32_t i = 0;

#if _MSVC_LANG < 201703L // < C++17
		memcpy((uint8_t*)&i + (bBigEndian ? 1 : 0), this, 3);
#else // >= C++17
		if constexpr (bBigEndian)
			memcpy((uint8_t*)&i + 1, this, 3);
		else
			memcpy(&i, this, 3);
#endif

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


		// quit message loop
		PostMsg(MessageVal::Quit, nullptr);
		if (trdMsgLoop.joinable())
			trdMsgLoop.join();

		// uninitialize COM
		CoUninitialize();
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	AudioEngine& AudioEngine::getInstance()
	{
		static AudioEngine oInstance;
		return oInstance;
	}

	void AudioEngine::MessageLoop()
	{
		std::unique_lock lm(muxMsgLoop);

		while (true)
		{
			while (oMessageQueue.size() > 0)
			{
				auto& msg = oMessageQueue.front();

				static const char* szEnumStrings[] =
				{
					"DestroyVoice", "DestroyEngine", "Recover", "Quit"
				};

				printf("Message received: %-12s (%u) for pointer 0x%p\n",
					szEnumStrings[(int)msg.eMsg], msg.eMsg, msg.ptr);

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

	void AudioEngine::PostMsg(MessageVal eMsg, void* ptr)
	{
		std::unique_lock lm(muxMsgLoop);
		oMessageQueue.push({ eMsg, ptr });
		lm.unlock();
		cvMsgLoop.notify_one();
	}

	void AudioEngine::ProcessMessages() { cvMsgLoop.notify_one(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool AudioEngine::create(const wchar_t* DeviceID, uint8_t ChannelCount, uint32_t SampleRate)
	{
		destroy();

		IXAudio2MasteringVoice* pMasteringVoice = nullptr;
		hr = m_pEngine->CreateMasteringVoice(&pMasteringVoice, ChannelCount, SampleRate,
			0, DeviceID);

		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 mastering voice:\n");

		std::set<SubmixVoice*> oParents;
		m_pMasteringVoice = new MasteringVoice(pMasteringVoice);

		// todo: save channelcount.

		m_bCreated = true;

		return true;
	}

	void AudioEngine::destroy()
	{
		if (!m_bCreated)
			return;

		// todo
		std::unique_lock lm(m_muxVoices);
		m_pMasteringVoice->destroyNoMutex();
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

		HRESULT hr = m_pEngine->CreateSubmixVoice(&pSubmixVoice, InputChannels, InputSampleRate,
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
		HRESULT hr;
		hr = XAudio2Create(&m_pEngine);
		if (FAILED(hr))
			ThrowHResultException(hr, "Couldn't create XAudio2 object:\n");

		// ToDo: m_pEngine->RegisterForCallbacks(&m_oCallback);
		m_bEngineExists = true;
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
		std::unique_lock lm(AudioEngine::getInstance().m_muxVoices);

		for (auto p : parents)
		{
			m_oParents.insert(p);
			p->m_oSubVoices.insert(this);
		}
	}

	AudioEngine::Voice::~Voice()
	{
		for (auto p : m_oParents)
		{
			p->m_oSubVoices.erase(this);
		}
		m_oParents.clear();

		destroyNoMutex();
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioEngine::Voice::destroyNoMutex()
	{
		if (!m_pVoice)
			return;

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

	AudioEngine::SourceVoice::~SourceVoice()
	{
		delete[] m_pCallback;
	}










	/***********************************************************************************************
	 class AudioEngine::SubmixVoice
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioEngine::SubmixVoice::destroyNoMutex()
	{
		// destroy subvoices
		while (m_oSubVoices.size() > 0)
		{
			auto pVoice = *m_oSubVoices.begin();

			pVoice->destroyNoMutex();
			delete pVoice;
		}
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
	 class XXX
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}