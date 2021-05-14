/***************************************************************************************************
 FILE:	rlAudioEngine.hpp
 CPP:	rlAudioEngine.cpp
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
#ifndef ROBINLE_AUDIOSTREAM
#define ROBINLE_AUDIOSTREAM





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;
typedef short int int16_t;
typedef unsigned int uint32_t;


#include <atomic>
#include <mutex>
#include <vector>
#include <xaudio2.h>



//==================================================================================================
// DECLARATION
namespace rl
{

	typedef uint8_t audio8_t;
	typedef int16_t audio16_t;
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
	typedef float audio32_t;





	/// <summary>
	/// Helper struct for 3-dimensional audio mapping<para/>
	/// Compatible with every channel count from 1 (mono) to 8 (7.1 surround)
	/// </summary>
	struct SurroundVolume
	{
		float FrontLeft = 1.0f, FrontRight = 1.0f;
		float FrontCenter = 1.0;
		float SideLeft = 1.0f, SideRight = 1.0f;
		float BackLeft = 1.0f, BackRight = 1.0f;
		float LFE = 1.0f; // subwoofer
	};





	/// <summary>
	/// Create a <c>WAVEFORMATEX</c> struct<para/>
	/// Throws an <c>std::exception</c> when the parameters are incorrect
	/// </summary>
	/// <param name="BitsPerSample">= multiple of 8, up to 32 (8/16/24/32)</param>
	/// <param name="ChannelCount">= 1 to <c>XAUDIO2_MAX_AUDIO_CHANNELS</c></param>
	/// <param name="SampleRate">
	/// = multiple of <c>XAUDIO2_QUANTUM_DENOMINATOR</c>,
	/// at least <c>XAUDIO2_MIN_SAMPLE_RATE</c>,
	/// at most <c>XAUDIO2_MAX_SAMPLE_RATE</c>
	/// </param>
	WAVEFORMATEX CreateWaveFmt(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate);

	bool ValidWaveFmt(WAVEFORMATEX& fmt);



	/// <summary>
	/// Helper struct for short WAV sound clips that are fully loaded into memory
	/// </summary>
	struct WaveformData
	{
		void* pData;
		size_t bytes; // size of buffer pointed to by pData, in bytes 
		uint8_t iBitsPerSample;
		uint8_t iChannelCount;
		uint32_t iSampleRate; // samples per seconds (Hz)
	};



	class AudioEngine;



	/// <summary>
	/// For playing short sound clips
	/// </summary>
	class Sound final
	{
	public: // methods

		/// <summary>
		/// Create a <c>Sound</c> object, assign data
		/// </summary>
		/// <param name="ManageData">
		/// = should the data pointed to by <c>data.pData</c> automatically be deleted on
		/// destruction?
		/// </param>
		Sound(AudioEngine& engine, WaveformData data, bool ManageData = true);

		~Sound();



		/// <summary>
		/// Play a new instance of this sound
		/// </summary>
		void play(float volume = 1.0);

		/// <summary>
		/// Pause all instances of this sound
		/// </summary>
		void pauseAll();

		/// <summary>
		/// Resume all paused instances of this sound
		/// </summary>
		void resumeAll();

		/// <summary>
		/// Stop all instances of this sound
		/// </summary>
		void stopAll();


	private: // methods

		// function to run in threads
		void threadPlay(float volume);


	private: // types

		/// <summary>
		/// Representation of a XAudio2 source voice
		/// </summary>
		class Voice : public IXAudio2VoiceCallback
		{
		public: // methods

			Voice(AudioEngine& engine, std::mutex& mux, std::condition_variable& cv,
				WaveformData& data, float volume = 1.0);
			~Voice();

			void OnStreamEnd() override;

			virtual inline void OnBufferEnd(void* pBufferContext) override {}
			inline void OnBufferStart(void* pBufferContext) override {}
			inline void OnLoopEnd(void* pBufferContext) override {}
			inline void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
			inline void OnVoiceProcessingPassEnd() override {}
			inline void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}

			/// <summary>
			/// Pause playback until <c>resume()</c> is called
			/// </summary>
			void pause();

			/// <summary>
			/// Resume playback after <c>pause()</c> has been called
			/// </summary>
			void resume();

			/// <summary>
			/// Stop playback, flush audio buffer
			/// </summary>
			void stop();


		private: // variables

			std::mutex& m_mux; // for notifying owner thread
			std::condition_variable& m_cv; // for notifying owner thread
			IXAudio2SourceVoice* m_pVoice; // associated XAudio2 source voice interface
			std::atomic<bool> m_bPaused; // is this voice currently paused?

		};


	private: // variables

		AudioEngine& m_oEngine; // destination engine
		WaveformData m_oData; // WAV audio
		bool m_bManage; // delete WAV data on destruction?
		std::vector<Voice*> m_oVoices; // all voices currently playing this sound
		std::mutex m_muxVector; // for thread-safe access of m_oVoices
		std::atomic<uint64_t> m_iThreadCount; // count of currently running threads

	};




	/// <summary>
	/// For streaming audio data of unknown or very big length
	/// </summary>
	class IAudioStream
	{
	protected: // interface methods

		/// <summary>
		/// Optional. Called before first sample set gets requested.
		/// </summary>
		virtual void OnStartup() {}

		/// <summary>
		/// Must be overridden. Called when a new sample is needed.
		/// </summary>
		/// <param name="fElapsedTime">= time, in seconds, since last call</param>
		/// <param name="pDest">
		/// = destination buffer for the audio samples<para/>
		/// The buffer's size is <c>getBitsPerSample() / 8 * getChannelCount()</c>
		/// </param>
		/// <returns>Were samples written (= is the stream still running?)</returns>
		virtual bool OnUpdate(float fElapsedTime, void* pDest)
		{
			return false;
		}

		/// <summary>
		/// Optional. Called after the stream ended.
		/// </summary>
		virtual void OnShutdown() {}


	public: // methods

		IAudioStream(AudioEngine& engine);
		~IAudioStream();



		/// <summary>
		/// Start the audio stream
		/// </summary>
		void run(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate = 44100,
			float volume = 1.0f, size_t BufferBlocks = 8, size_t BufferBlockSamples = 512);

		/// <summary>
		/// Pause this stream
		/// </summary>
		void pause();

		/// <summary>
		/// Resume this stream if it is currently paused
		/// </summary>
		void resume();

		/// <summary>
		/// Stop this stream
		/// </summary>
		void stop();

		/// <summary>
		/// Instantly set the volume to this value
		/// </summary>
		/// <param name="volume">= volume multiplier (>= 0, 1.0 = original volume)</param>
		void setVolume(float volume);

		/// <summary>
		/// Is this stream currently paused?
		/// </summary>
		inline bool isPaused() { return m_bPaused; }


	protected: // methods

		inline uint8_t getBitsPerSample() { return m_iBitsPerSample; }
		inline uint8_t getChannelCount() { return m_iChannelCount; }
		inline uint32_t getSampleRate() { return m_iSampleRate; }


	private: // methods

		void threadFunc(uint8_t iBitsPerSample, uint8_t iChannelCount, uint32_t iSampleRate,
			float fVolume);


	private: // types

		/// <summary>
		/// Representation of a XAudio2 source voice
		/// </summary>
		class Callback : public IXAudio2VoiceCallback
		{
		public: // methods

			Callback(IAudioStream* stream, std::mutex& mux, std::condition_variable& cv);
			~Callback() {}

			void OnBufferEnd(void* pBufferContext) override;
			void OnStreamEnd() override;

			inline void OnBufferStart(void* pBufferContext) override {}
			inline void OnLoopEnd(void* pBufferContext) override {}
			inline void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
			inline void OnVoiceProcessingPassEnd() override {}
			inline void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}


		private: // variables

			std::mutex& m_mux;
			std::condition_variable& m_cv;
			IAudioStream* m_pStream;

		};
		friend class Callback;


	private: // variables

		AudioEngine& m_oEngine; // destination engine

		uint8_t m_iBitsPerSample = 0;
		uint8_t m_iChannelCount = 0;
		uint32_t m_iSampleRate = 0;

		std::atomic<bool> m_bRunning = false;
		std::atomic<bool> m_bPaused = false;

		IXAudio2SourceVoice* m_pVoice = nullptr;

		std::mutex m_mux;
		std::condition_variable m_cv;
		std::thread m_trd;
		Callback m_oCallback;

		size_t m_iBufferCurrentBlock = 0;
		std::atomic<size_t> m_iBufferBlocks = 0;
		std::atomic<size_t> m_iBufferBlockSamples = 0;
		std::atomic<size_t> m_iBlocksFree = 0;
		std::atomic<size_t> m_iBlockSize = 0;

		uint8_t* m_pBufferData = nullptr;

	};





	/// <summary>
	/// XAudio2-based audio engine, with 3D audio support
	/// </summary>
	class AudioEngine final
	{
	public: // methods

		static AudioEngine& getInstance();

		/// <summary>
		/// Create the mastering voice<para/>
		/// Destroys the current one, if present
		/// </summary>
		/// <param name="samplerate">
		/// must be a multiple of <c>XAUDIO2_QUANTUM_DENOMINATOR</c> (100) and betweeen
		/// <c>XAUDIO2_MIN_SAMPLE_RATE</c> (1000) and <c>XAUDIO2_MAX_SAMPLE_RATE</c> (200000)
		/// </param>
		/// <returns>Was the mastering voice successfully created?</returns>
		bool create(const wchar_t* szDeviceID = NULL, uint8_t ChannelCount = 2,
			uint32_t samplerate = 44100, size_t StreamBufBlockCount = 8,
			size_t StreamBufSamples = 512);

		/// <summary>
		/// Destroy the mastering voice
		/// </summary>
		/// <param name="StopVoices">= should all current voices be stopped?</param>
		/// <param name="UnregisterSources">
		/// = should all sources (<c>Sound</c> and <c>IAudioStream</c> objects) be unregistered?
		/// (StopVoices will get ignored if this is <c>true</c>)
		/// </param>
		void destroy(bool StopSounds = true);


		/// <summary>
		/// Get a pointer to the current <c>IXAudio2</c> interface<para/>
		/// Returns <c>nullptr</c> if there is currently no <c>IXAudio2</c> interface
		/// </summary>
		inline IXAudio2* getEngine() { return m_pXAudio2; }

		/// <summary>
		/// Get a pointer to the current <c>IXAudio2MasteringVoice</c> interface<para/>
		/// Returns <c>nullptr</c> if there currently is no <c>IXAudio2MasteringVoice</c> interface
		/// </summary>
		inline IXAudio2MasteringVoice* getMasterVoice() { return m_pMaster; }

		/// <summary>
		/// Is there a mastering voice currently running?
		/// </summary>
		/// <returns></returns>
		inline bool isRunning() { return m_bRunning; }


		/// <summary>
		/// Add a sound to the list of played audio
		/// </summary>
		void registerSound(Sound* snd);


		/// <summary>
		/// Remove a sound from the list of played audio
		/// </summary>
		void unregisterSound(Sound* snd);



		/// <summary>
		/// Add an audio stream to the list of played audio
		/// </summary>
		void registerStream(IAudioStream* stream);

		/// <summary>
		/// Remove an audio stream from the list of played audio
		/// </summary>
		void unregisterStream(IAudioStream* stream);



		/// <summary>
		/// Get the current version of <c>rl::AudioEngine</c>
		/// </summary>
		void getVersion(uint8_t(&dest)[4]);


	private: // methods

		AudioEngine(); // --> singleton
		~AudioEngine();


	private: // variables

		static IXAudio2* m_pXAudio2; // XAudio2 object
		static IXAudio2MasteringVoice* m_pMaster; // mastering voice

		static std::vector<Sound*> m_pSounds;
		static std::mutex m_muxSounds;

		static std::vector<IAudioStream*> m_pStreams;
		static std::mutex m_muxStreams;

		static std::atomic<bool> m_bRunning;

	};

}





// #undefs

#endif // ROBINLE_AUDIOSTREAM