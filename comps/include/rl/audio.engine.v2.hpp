/***************************************************************************************************
 FILE:	audio.engine.v2.hpp
 CPP:	audio.engine.v2.cpp
 DESCR:	Audio engine based on XAudio2, created for games


 XAudio2 Default Channel Mapping (from Microsoft Docs):

1	Always maps to FrontLeft and FrontRight at full scale in both speakers
	(special case for mono sounds)
2	FrontLeft, FrontRight
	(basic stereo configuration)
3	FrontLeft, FrontRight, LowFrequency
	(2.1 configuration)
4	FrontLeft, FrontRight, BackLeft, BackRight
	(quadraphonic)
5	FrontLeft, FrontRight, FrontCenter, SideLeft, SideRight
	(5.0 configuration)
6	FrontLeft, FrontRight, FrontCenter, LowFrequency, SideLeft, SideRight
	(5.1 configuration)
7	FrontLeft, FrontRight, FrontCenter, LowFrequency, SideLeft, SideRight, BackCenter
	(6.1 configuration)
8	FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight
	(7.1 configuration)
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_AUDIO_ENGINE_V2
#define ROBINLE_AUDIO_ENGINE_V2





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using int16_t = short;
using int32_t = int;
using uint32_t = unsigned int;


#include <functional>
#include <mutex>
#include <queue>
#include <set>
#include <vector>
#include <xaudio2.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	
	using audio8_t = uint8_t;
	using audio16_t = int16_t;
	typedef struct int24_t
	{
		uint8_t iData[3];

		int24_t() : iData{ 0, 0, 0 } {}
		int24_t(int32_t i) { assign(i); }

		inline void operator=(int32_t other) { assign(other); }
		inline operator int32_t() { return asInt32(); }

		int32_t asInt32();
		void assign(int32_t i);
	} audio24_t;
	using audio32_t = float;





	/// <summary>
	/// XAudio2-based audio engine
	/// </summary>
	class AudioEngine final
	{
	public: // static methods

		static AudioEngine& getInstance();


	private: // types
		
		enum class MessageVal
		{
			DestroyVoice,
			DestroyEngine,
			Recover, // TODO: try to recreate the audio engine (ptr = CreateParams*)
			Quit
		};

		struct Message
		{
			MessageVal eMsg;
			void* ptr;
		};

		struct CreateParams
		{
			std::wstring sDeviceID;
			uint8_t iChannelCount;
			uint32_t iSampleRate;
		};


	private: // static methods

		static void MessageLoop();
		static void PostMsg(MessageVal eMsg, void* ptr);
		static void ProcessMessages();


	private: // static variables

		static std::thread trdMsgLoop;
		static std::mutex muxMsgLoop;
		static std::condition_variable cvMsgLoop;
		static std::queue<Message> oMessageQueue;
		static HRESULT hr;


	public: // types

		enum class VoiceType
		{
			MasteringVoice,
			SubmixVoice,
			SourceVoice
		};

		class SubmixVoice; // forward declaration

		/// <summary>
		/// An own version of the <c>XAUDIO2_SEND_DESCRIPTOR</c> struct
		/// </summary>
		struct VoiceSendDescriptor
		{
			UINT32 iFlags;
			SubmixVoice* pOutputVoice;
		};

		/// <summary>
		/// An own version of the <c>XAUDIO2_VOICE_SENDS</c> struct
		/// </summary>
		struct VoiceSends
		{
			uint32_t iSendCount;
			VoiceSendDescriptor* pSends;
		};

		class Voice
		{
		public: // methods

			Voice(IXAudio2Voice* ptr, const std::set<SubmixVoice*>& parents, VoiceType type);
			virtual ~Voice();

			// destroy this voice without locking the AudioEngine::m_muxVoicese mutex
			virtual void destroyNoMutex();

			inline auto getPtr() { return m_pVoice; }
			inline auto& getParents() { return m_oParents; }
			inline auto getType() { return m_eVoiceType; }


		private: // variables

			IXAudio2Voice* m_pVoice;
			std::set<SubmixVoice*> m_oParents;
			const VoiceType m_eVoiceType;

		};

		class SourceVoice : public Voice
		{
		public: // types

			class Callback : public IXAudio2VoiceCallback
			{
			public: // methods

				inline void __stdcall OnBufferEnd(void* pBufferContext) override
				{
					invoke(pOwner->OnBufferEnd, pBufferContext);
				}
				inline void __stdcall OnBufferStart(void* pBufferContext) override
				{
					invoke(pOwner->OnBufferStart, pBufferContext);
				}
				inline void __stdcall OnLoopEnd(void* pBufferContext) override
				{
					invoke(pOwner->OnLoopEnd, pBufferContext);
				}
				inline void __stdcall OnStreamEnd() override
				{
					invoke(pOwner->OnStreamEnd);
				}
				inline void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override
				{
					invoke(pOwner->OnVoiceError, pBufferContext, Error);
				}
				inline void __stdcall OnVoiceProcessingPassEnd() override
				{
					invoke(pOwner->OnVoiceProcessingPassEnd);
				}
				inline void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override
				{
					invoke(pOwner->OnVoiceProcessingPassStart, BytesRequired);
				}


			public: // variables

				SourceVoice* pOwner = nullptr;


			private: // methods

				template <typename ...Args>
				void invoke(std::function<void(Args...)> fn, Args... args);
			};


		public: // methods

			// pCallback will be deleted on destruction
			SourceVoice(IXAudio2SourceVoice* ptr, const std::set<SubmixVoice*>& parents,
				Callback* pCallback) :
				Voice(ptr, parents, VoiceType::SourceVoice), m_pCallback(pCallback) {}

			virtual ~SourceVoice();

			inline auto getPtr() { return reinterpret_cast<IXAudio2SourceVoice*>(Voice::getPtr()); }


		public: // event handlers

			std::function<void(void* pBufferContext)> OnBufferEnd = nullptr;
			std::function<void(void* pBufferContext)> OnBufferStart = nullptr;
			std::function<void(void* pBufferContext)> OnLoopEnd = nullptr;
			std::function<void()> OnStreamEnd = nullptr;
			std::function<void(void* pBufferContext, HRESULT Error)> OnVoiceError = nullptr;
			std::function<void()> OnVoiceProcessingPassEnd = nullptr;
			std::function<void(UINT32 BytesRequired)> OnVoiceProcessingPassStart = nullptr;


		private: // variables

			Callback* m_pCallback;

		};

		class SubmixVoice : public Voice
		{
			friend class Voice;
		public: // methods

			SubmixVoice(IXAudio2SubmixVoice* ptr, const std::set<SubmixVoice*>& parents) :
				Voice(ptr, parents, VoiceType::SubmixVoice) {}

			inline auto getPtr() { return reinterpret_cast<IXAudio2SubmixVoice*>(Voice::getPtr()); }

			// destroy this voice without locking the AudioEngine::m_muxVoicese mutex
			void destroyNoMutex() override;

			inline auto& getSubVoices() { return m_oSubVoices; }


		protected: // methods

			SubmixVoice(IXAudio2MasteringVoice* ptr) : Voice(ptr, {}, VoiceType::MasteringVoice) {}


		private: // variables

			std::set<Voice*> m_oSubVoices;

		};

		class MasteringVoice : public SubmixVoice
		{
		public: // methods

			MasteringVoice(IXAudio2MasteringVoice* ptr) : SubmixVoice(ptr) {}

			inline auto getPtr()
			{
				return reinterpret_cast<IXAudio2MasteringVoice*>(Voice::getPtr());
			}

		};


	public: // operators

		inline operator bool() const { return m_bCreated; }


	public: // methods

		/// <summary>
		/// Create the mastering voice<para/>
		/// Destroys the current one, if present
		/// </summary>
		/// <param name="samplerate">
		/// must be a multiple of <c>XAUDIO2_QUANTUM_DENOMINATOR</c> (100) and betweeen
		/// <c>XAUDIO2_MIN_SAMPLE_RATE</c> (1000) and <c>XAUDIO2_MAX_SAMPLE_RATE</c> (200000)
		/// </param>
		/// <returns>Was the mastering voice successfully created?</returns>
		[[nodiscard]]
		bool create(const wchar_t* DeviceID = 0, uint8_t ChannelCount = 2,
			uint32_t SampleRate = 44100);

		void destroy();

		inline auto getMasteringVoice() { return m_pMasteringVoice; }

		HRESULT createSubmixVoice(SubmixVoice** dest, UINT32 InputChannels, UINT32 InputSampleRate,
			UINT32 Flags, UINT32 ProcessingStage, const VoiceSends* sends = nullptr,
			const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);

		HRESULT createSourceVoice(SourceVoice** dest, const WAVEFORMATEX* pSourceFormat,
			UINT32 Flags = 0, float MaxFrequencyRatio = 2.0f, const VoiceSends* sends = nullptr,
			const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);

		inline HRESULT getHRESULT() { return hr; }


	private: // methods

		AudioEngine(); // --> singleton
		~AudioEngine();

		/// <summary>
		/// Create the XAudio2 instance
		/// </summary>
		void createEngine();


	private: // variables

		std::atomic<bool> m_bEngineExists = false;
		std::atomic<bool> m_bCreated = false;
		MasteringVoice* m_pMasteringVoice = nullptr;
		IXAudio2* m_pEngine = nullptr;
		std::mutex m_muxVoices;

	};
	
}





// #undef foward declared definitions

#endif // ROBINLE_AUDIO_ENGINE_V2