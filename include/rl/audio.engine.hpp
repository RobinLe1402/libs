/***************************************************************************************************
 FILE:	audio.engine.hpp
 CPP:	audio.engine.cpp
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
#ifndef ROBINLE_AUDIO_ENGINE
#define ROBINLE_AUDIO_ENGINE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using int16_t = short;
using int32_t = int;
using uint32_t = unsigned int;


#include <condition_variable>
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

	constexpr bool BigEndian = (const uint8_t&)0x00FF == 0x00;

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
	/// All bit depths supported by the RobinLe Audio Engine
	/// </summary>
	enum class AudioBitDepth
	{
		Audio8 = 8,
		Audio16 = 16,
		Audio24 = 24,
		Audio32 = 32
	};



	/// <summary>
	/// 3D audio position data
	/// </summary>
	struct Audio3DPos
	{
		float x; // -1.0f = left, 0.0f = center, 1.0f = right
		float z; // 0.0f = front, 1.0f = side, 2.0f = back
		float radius = 2.5f; // emission radius, scale is same as x and z

		static const Audio3DPos Center;
		static const Audio3DPos Left;
		static const Audio3DPos Right;
	};




	/// <summary>
	/// Own, reduced version of the <c>WAVEFORMATEX</c> struct for PCM waveforms
	/// </summary>
	struct WaveFormat
	{
		AudioBitDepth eBitDepth = AudioBitDepth::Audio16;
		uint8_t iChannelCount = 2;
		uint32_t iSampleRate = 44100;
	};

	constexpr bool ValidWaveFormat(const WaveFormat& format) noexcept;

	constexpr WAVEFORMATEX CreateWaveFormatEx(const WaveFormat& format) noexcept;





	/// <summary>
	/// Audio data for one or multiple channels
	/// </summary>
	struct MultiChannelAudioSample
	{
		/// <summary>
		/// The bitdepth<para/>
		/// Use to find out what pointer to use
		/// </summary>
		uint8_t iBitsPerSample;
		/// <summary>
		/// The count of channels
		/// </summary>
		uint8_t iChannelCount;

		union
		{
			/// <summary>
			/// The destination for 8-bit PCM audio
			/// </summary>
			rl::audio8_t* p8;
			/// <summary>
			/// The destination for 16-bit PCM audio
			/// </summary>
			rl::audio16_t* p16;
			/// <summary>
			/// The destination for 24-bit PCM audio
			/// </summary>
			rl::audio24_t* p24;
			/// <summary>
			/// The destination for 32-bit PCM audio
			/// </summary>
			rl::audio32_t* p32;
		} val;


		/// <summary>
		/// Delete allocated memory
		/// </summary>
		void free();
	};





	/// <summary>
	/// XAudio2-based audio engine
	/// </summary>
	class AudioEngine final
	{
	public: // types

		enum class MessageVal
		{
			DestroyVoice,
			DestroyEngine,
			DeleteSoundInstance,
			Quit
		};

		struct Message
		{
			MessageVal eMsg;
			void* ptr;
		};


	public: // static methods

		static AudioEngine& GetInstance();
		static void PostMsg(MessageVal eMsg, void* ptr);


	private: // static methods

		static void MessageLoop();
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
			virtual void destroy();

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
				Callback* pCallback);

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

			~SubmixVoice();

			inline auto getPtr() { return reinterpret_cast<IXAudio2SubmixVoice*>(Voice::getPtr()); }

			// destroy this voice without locking the AudioEngine::m_muxVoicese mutex
			void destroy() override;

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
			~MasteringVoice();

			using SubmixVoice::destroy;

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
		bool create(uint8_t ChannelCount = 2,
			uint32_t SampleRate = 44100);

		void destroy();

		inline auto getMasteringVoice() { return m_pMasteringVoice; }

		HRESULT createSubmixVoice(SubmixVoice** dest, UINT32 InputChannels, UINT32 InputSampleRate,
			UINT32 Flags, UINT32 ProcessingStage, const VoiceSends* sends = nullptr,
			const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);

		HRESULT createSourceVoice(SourceVoice** dest, const WAVEFORMATEX* pSourceFormat,
			UINT32 Flags = 0, float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
			const VoiceSends* sends = nullptr, const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);

		inline HRESULT getHRESULT() { return hr; }


	private: // methods

		AudioEngine(); // --> singleton
		~AudioEngine();

		/// <summary>
		/// Create the XAudio2 instance
		/// </summary>
		void createEngine();

		void destroyEngine();


	private: // variables

		std::atomic<bool> m_bEngineExists = false;
		std::atomic<bool> m_bCreated = false;
		MasteringVoice* m_pMasteringVoice = nullptr;
		IXAudio2* m_pEngine = nullptr;
		std::mutex m_muxVoices;
		uint8_t m_iChannelCount = 0;

	};





	// forward declaration
	class SoundInstance;
	class SoundInstance3D;

	/// <summary>
	/// An audio track that's loaded into memory at once<br />
	/// Currently only supports WAV PCM data (8/16/24/32 bits per sample)
	/// </summary>
	class Sound
	{
	public: // operators

		Sound& operator=(const Sound& other);
		Sound& operator=(Sound&& rval) noexcept;


	public: // static variables

		/// <summary>
		/// Load a WAV file
		/// </summary>
		/// <returns>
		/// * On success: Pointer to a <c>Sound</c> instance<para />
		/// * On failure: <c>nullptr</c>
		/// </returns>
		static Sound* FromFile(const wchar_t* szFileName);
		/// <summary>
		/// Load a <c>"WAVE"</c> resource
		/// </summary>
		/// <returns>
		/// * On success: Pointer to a <c>Sound</c> instance<para />
		/// * On failure: <c>nullptr</c>
		/// </returns>
		static Sound* FromResource(HMODULE hModule, LPCWSTR lpName);
		/// <summary>
		/// Load audio from RIFF waveform data in memory
		/// </summary>
		/// <returns>
		/// * On success: Pointer to a <c>Sound</c> instance<para />
		/// * On failure: <c>nullptr</c>
		/// </returns>
		static Sound* FromMemory(const void* data, size_t size);


	public: // methods

		Sound();
		Sound(const Sound& other);
		Sound(Sound&& rval) noexcept;
		Sound(const WaveFormat& Format, size_t SampleCount);
		~Sound();

		void clear();

		bool getSample(MultiChannelAudioSample& dest, size_t iSampleID) const;

		inline const void* getDataPtr() const { return m_pData; }
		inline auto getDataSize() const { return m_iDataSize; }

		inline auto getSampleCount() const { return m_iSampleCount; }
		inline const auto& getWaveFormat() const { return m_oWavFmt; }

		SoundInstance* play(float volume = 1.0f);
		SoundInstance3D* play3D(const Audio3DPos& pos, float volume = 1.0f);


	private: // variables

		WaveFormat m_oWavFmt;
		size_t m_iSampleAlign;
		size_t m_iSampleCount;
		size_t m_iDataSize;

		uint8_t* m_pData;

	};



	enum class SoundInstanceType
	{
		Default, // sound is played unchanged
		Surround // sound is played at a certain 3D position
	};

	/// <summary>
	/// An instance of a sound effect
	/// </summary>
	class SoundInstance
	{
	public: // methods

		SoundInstance(const Sound& sound, float volume);
		virtual ~SoundInstance();

		void pause();
		void resume();
		void stop();

		void waitForEnd();

		inline auto getType() const { return m_eType; }
		inline bool getPlaying() const { return m_bPlaying; }
		inline bool getPaused() const { return m_bPaused; }


	protected: // methods

		SoundInstance(SoundInstanceType type, float volume) : m_eType(type), m_fVolume(volume) {}

		virtual bool createVoices(const WAVEFORMATEX& format);
		virtual void destroyVoices();

		bool loadSound(const Sound& sound);
		void onEnd();


	protected: // variables

		AudioEngine::SourceVoice* m_pSourceVoice = nullptr;
		std::atomic<bool> m_bVoiceExists = false;
		std::atomic<bool> m_bPlaying = false;
		std::atomic<bool> m_bPaused = false;
		float m_fVolume;


	private: // variables

		const SoundInstanceType m_eType;
		std::mutex m_mux;
		std::condition_variable m_cv;
	};

	/// <summary>
	/// An instance of a sound effect, played at a certain position in 3-dimensional space
	/// </summary>
	class SoundInstance3D : public SoundInstance
	{
	public: // methods

		SoundInstance3D(const Sound& sound, float volume, const Audio3DPos& pos);

		void set3DPos(const Audio3DPos& pos);
		inline auto get3DPos() const { return m_o3DPos; }


	protected: // methods

		bool createVoices(const WAVEFORMATEX& format) override;
		void destroyVoices() override;


	private: // methods

		void applyPos();


	private: // variables

		Audio3DPos m_o3DPos;
		float m_fSurroundVolume[8] = {};
		AudioEngine::SubmixVoice* m_pSubmixVoice_Surround = nullptr;
		AudioEngine::SubmixVoice* m_pSubmixVoice_Mono = nullptr;

	};





	/// <summary>
	/// An interface for buffered real-time audio generation
	/// </summary>
	class IAudioStream
	{
	public: // methods

		virtual ~IAudioStream();

		void setVolume(float volume);
		inline float getVolume() const noexcept { return m_fVolume; }

		void pause();
		void resume();

		void stop();

		inline bool running() const noexcept { return m_bRunning; }
		inline bool paused() const noexcept { return m_bPaused; }


	protected: // methods

		/// <summary>
		/// Start playback of the audio stream<para/>
		/// Does nothing when the stream is already running
		/// </summary>
		/// <param name="format"></param>
		void internalStart(const WaveFormat& format, float volume = 1.0f, size_t BufferBlockCount = 8,
			size_t SamplesPerBufferBlock = 512);

		/// <summary>
		/// Get the next audio sample
		/// </summary>
		/// <param name="fElapsedTime">
		/// = The elapsed time, in seconds, since the last call
		/// </param>
		/// <param name="dest">= The destination for the sample data to generate</param>
		/// <returns>
		/// Was sample data written? (if not, the audio stream will terminate)
		/// </returns>
		virtual bool nextSample(float fElapsedTime, MultiChannelAudioSample& dest) noexcept = 0;


	private: // methods

		void threadFunc(); // audio generation thread
		void fillBlock(); // fill the current buffer block with audio data


	private: // variables

		AudioEngine::SourceVoice* m_pSourceVoice = nullptr;
		uint8_t* m_pBuffer = nullptr;

		std::mutex m_mux;
		std::condition_variable m_cv;

		// audio metadata
		WaveFormat m_oFormat = {};
		float m_fVolume = 0.0f;

		// block info
		size_t m_iBlockCount = 0;
		size_t m_iBlockSize = 0;
		size_t m_iSamplesPerBlock = 0;
		std::atomic<size_t> m_iFreeBlocks = 0;
		size_t m_iCurrentBlock = 0;

		// precalculated values (constant between start() and stop())
		float m_fTimePerSample = 0.0f;
		uint8_t m_iByteDepth = 0; // m_oFormat.eBitDepth in bytes
		uint8_t m_iSampleAlign = 0;

		// thread stuff
		std::thread m_trdSampleGeneration;
		std::atomic_bool m_bPaused = false;
		std::atomic_bool m_bRunning = false;
		bool m_bEndOfStream = false; // nextSample() returned FALSE

	}; \

}





#endif // ROBINLE_AUDIO_ENGINE