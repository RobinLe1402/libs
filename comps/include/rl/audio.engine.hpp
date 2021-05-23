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
	/// 3D audio position data
	/// </summary>
	struct Audio3DPos
	{
		float x; // -1.0f = left, 0.0f = center, 1.0f = right
		float z; // 0.0f = front, 1.0f = side, 2.0f = back
		float radius = 2.5f; // emission radius, scale is same as x and z
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

	/// <summary>
	/// Check <c>WAVEFORMATEX</c> for consistency and valid data
	/// </summary>
	bool ValidWaveFmt(WAVEFORMATEX& fmt);






	// forward declaration
	class IAudio;





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
		bool create(const wchar_t* DeviceID = 0, uint8_t ChannelCount = 2,
			uint32_t samplerate = 44100);

		/// <summary>
		/// Destroy the mastering voice
		/// </summary>
		void destroy();


		/// <summary>
		/// Stop all audio linked to this engine
		/// </summary>
		void stopAllAudio();

		/// <summary>
		/// Pauses all audio linked to this engine
		/// </summary>
		void pauseAllAudio();

		/// <summary>
		/// Unpauses all audio linked to this engine
		/// </summary>
		void resumeAllAudio();


		/// <summary>
		/// Get a pointer to the current <c>IXAudio2</c> interface<para/>
		/// Returns <c>nullptr</c> if there is currently no <c>IXAudio2</c> interface
		/// </summary>
		IXAudio2* getEngine();

		/// <summary>
		/// Get a pointer to the current <c>IXAudio2MasteringVoice</c> interface<para/>
		/// Returns <c>nullptr</c> if there currently is no <c>IXAudio2MasteringVoice</c> interface
		/// </summary>
		inline IXAudio2MasteringVoice* getMasterVoice() { return m_pMaster; }

		/// <summary>
		/// Get the current channel count
		/// </summary>
		/// <returns>
		/// When running --> Channel count of the mastering voice<para/>
		/// Else --> 0
		/// </returns>
		inline uint8_t getChannelCount() { return m_iChannelCount; }

		/// <summary>
		/// Is there a mastering voice currently running?
		/// </summary>
		/// <returns></returns>
		inline bool isRunning() { return m_bRunning; }



		/// <summary>
		/// Create a 3D volume matrix based on 2D coordinates
		/// </summary>
		void get3DOutputVolume(Audio3DPos pos, float(&OutputMatrix)[8]);



		/// <summary>
		/// Has an error occured that is forcing a shutdown right now?
		/// </summary>
		inline bool error() { return m_bError; }



		/// <summary>
		/// Get the current version of <c>rl::AudioEngine</c>
		/// </summary>
		void getVersion(uint8_t(&dest)[4]);


	protected:

		/// <summary>
		/// Add item to the list of played audio
		/// </summary>
		void registerAudio(IAudio* audio);


		/// <summary>
		/// Remove item from the list of played audio
		/// </summary>
		void unregisterAudio(IAudio* audio);


	private: // methods

		AudioEngine(); // --> singleton
		~AudioEngine();

		void createEngine(); // create m_pEngine
		void destroyEngine(); // destroy m_pEngine

		void threadFunc();


	private: // types

		class Callback : public IXAudio2EngineCallback
		{
		public: // methods

			Callback(AudioEngine& engine) : m_oEngine(engine) {}

			void __stdcall OnCriticalError(HRESULT Error) override;

			inline void __stdcall OnProcessingPassEnd() override {}
			inline void __stdcall OnProcessingPassStart() override {}


		private: // variables

			AudioEngine& m_oEngine;

		};


	private: // variables

		IXAudio2* m_pEngine = nullptr; // XAudio2 object
		std::atomic<bool> m_bEngineExists = false;

		IXAudio2MasteringVoice* m_pMaster = nullptr; // mastering voice
		Callback m_oCallback;

		std::vector<IAudio*> m_pAudio;
		std::mutex m_muxAudio;

		std::atomic<bool> m_bRunning = false;
		uint8_t m_iChannelCount = 0;

		std::thread m_trdEngine;
		std::mutex m_mux;
		std::condition_variable m_cv;
		std::atomic<bool> m_bError = false; // true if a critical error has occured

		std::vector<std::string> m_oExceptions;

		friend class IAudio;

	};










	/// <summary>
	/// Interface for any audio played via the audio engine
	/// </summary>
	class IAudio
	{
	public: // methods

		IAudio(AudioEngine& engine) : m_oEngine(engine) {}
		virtual ~IAudio();

		virtual void pause() = 0;
		virtual void resume() = 0;
		virtual void stop() = 0;



		/// <summary>
		/// Is this audio instance running in 3D mode?
		/// </summary>
		inline bool is3D() { return m_b3D; }

		/// <summary>
		/// Recalculate the 3D output volume matrix<para/>
		/// Has to be called when playing back in 3D mode, after recreating the mastering voice
		/// with a different channel count
		/// </summary>
		void refresh3DOutput();



		/// <summary>
		/// If this audio instance is running in 3D mode, change the playback position
		/// </summary>
		/// <param name="x">= left/right position (0.0 = center, -1.0 = left, 1.0 = right)</param>
		/// <param name="z">= front/back position (0.0 = front, 0.5 = center, 1.0 = back)</param>
		/// <param name="volume">= pseudo y position</param>
		void setPos(Audio3DPos pos, float volume = 1.0f);

		void setVolume(float volume);


	protected: // methods

		void registerAudio();
		void unregisterAudio();

		// creates the necessary XAudio2 voices. Return value: Succeeded?
		bool createVoices(WAVEFORMATEX wavfmt, const char* szClassName = nullptr);

		void destroyVoices();


	protected: // variables

		AudioEngine& m_oEngine;

		IXAudio2SourceVoice* m_pVoice = nullptr;
		IXAudio2SubmixVoice* m_pVoiceMono = nullptr;
		IXAudio2SubmixVoice* m_pVoiceSurround = nullptr;

		IXAudio2VoiceCallback* m_pCallback = nullptr;

		std::atomic<bool> m_bRunning;
		bool m_b3D = false;
		Audio3DPos m_oPos = {};

		float m_f3DVolume[8] = {};

	};










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





	/// <summary>
	/// For playing waveform data from RAM
	/// </summary>
	class SoundInstance final : public IAudio
	{
	public: // methods

		SoundInstance(AudioEngine& engine) : IAudio(engine), m_oCallback(*this, m_mux, m_cv) {}
		virtual ~SoundInstance();

		void play(WaveformData& data, float volume = 1.0f);
		void play3D(WaveformData& data, Audio3DPos& pos, float volume = 1.0f);

		void pause() override;
		void resume() override;
		void stop() override;


	private: // methods

		void playInternal(WaveformData& data, float fVolume);
		void threadFunc();


	private: // types

		class Callback : public IXAudio2VoiceCallback
		{
		public: // methods

			Callback(SoundInstance& sound, std::mutex& mux, std::condition_variable& cv) :
				m_oSound(sound), m_mux(mux), m_cv(cv) {}
			~Callback() {}

			void __stdcall OnStreamEnd() override;

			inline void __stdcall OnBufferEnd(void* pBufferContext) override {}
			inline void __stdcall OnBufferStart(void* pBufferContext) override {}
			inline void __stdcall OnLoopEnd(void* pBufferContext) override {}
			inline void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override {}
			inline void __stdcall OnVoiceProcessingPassEnd() override {}
			inline void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}


		private: // variables

			std::mutex& m_mux;
			std::condition_variable& m_cv;
			SoundInstance& m_oSound;

		};


	private: // variables

		std::atomic<bool> m_bPaused = false;

		std::mutex m_mux;
		std::condition_variable m_cv;
		std::thread m_trd;
		Callback m_oCallback;

	};




	/// <summary>
	/// For streaming audio data of unknown or very big length
	/// </summary>
	class IAudioStream : public IAudio
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

		IAudioStream(AudioEngine& engine) :
			IAudio(engine), m_oCallback(this, m_mux, m_cv) {}
		~IAudioStream();



		/// <summary>
		/// Start the audio stream
		/// </summary>
		void play(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate = 44100,
			float volume = 1.0f, size_t BufferBlocks = 8, size_t BufferBlockSamples = 512);

		/// <summary>
		/// Start the audio stream mixed down to mono at a certain 3D position
		/// </summary>
		/// <param name="x">= left/right position (0.0 = center, -1.0 = left, 1.0 = right)</param>
		/// <param name="z">= front/back position (0.0 = front, 0.5 = center, 1.0 = back)</param>
		void play3D(Audio3DPos pos, uint8_t BitsPerSample, uint8_t ChannelCount,
			uint32_t SampleRate = 44100, float volume = 1.0f, size_t BufferBlocks = 8,
			size_t BufferBlockSamples = 512);

		/// <summary>
		/// Pause this stream
		/// </summary>
		void pause() override;

		/// <summary>
		/// Resume this stream if it is currently paused
		/// </summary>
		void resume() override;

		/// <summary>
		/// Stop this stream
		/// </summary>
		void stop() override;

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


		void playInternal(uint8_t BitsPerSample, uint8_t ChannelCount, uint32_t SampleRate,
			float volume, size_t BufferBlocks, size_t BufferBlockSamples);


	private: // types

		class Callback : public IXAudio2VoiceCallback
		{
		public: // methods

			Callback(IAudioStream* stream, std::mutex& mux, std::condition_variable& cv) :
				m_pStream(stream), m_mux(mux), m_cv(cv) {}
			~Callback() {}

			void __stdcall OnBufferEnd(void* pBufferContext) override;
			void __stdcall OnStreamEnd() override;

			inline void __stdcall OnBufferStart(void* pBufferContext) override {}
			inline void __stdcall OnLoopEnd(void* pBufferContext) override {}
			inline void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override {}
			inline void __stdcall OnVoiceProcessingPassEnd() override {}
			inline void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}


		private: // variables

			std::mutex& m_mux;
			std::condition_variable& m_cv;
			IAudioStream* m_pStream;

		};
		friend class Callback;


	private: // variables

		uint8_t m_iBitsPerSample = 0;
		uint8_t m_iChannelCount = 0;
		uint32_t m_iSampleRate = 0;

		std::atomic<bool> m_bPaused = false;

		std::mutex m_mux;
		std::condition_variable m_cv;
		std::thread m_trdStream;
		Callback m_oCallback;

		size_t m_iBufferCurrentBlock = 0;
		std::atomic<size_t> m_iBufferBlocks = 0;
		std::atomic<size_t> m_iBufferBlockSamples = 0;
		std::atomic<size_t> m_iBlocksFree = 0;
		std::atomic<size_t> m_iBlockSize = 0;

		uint8_t* m_pBufferData = nullptr;

	};

}





#endif // ROBINLE_AUDIO_ENGINE