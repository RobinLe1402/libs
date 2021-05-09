/***************************************************************************************************
 FILE:	rlAudioStream.hpp
 CPP:	rlAudioStream.cpp
 DESCR:	Audio stream engine based on XAudio2, created for games
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
	/// Helper struct for short sound clips that are loaded into memory
	/// </summary>
	struct WaveformData
	{
		void* pData;
		size_t bytes; // size of buffer pointed to by <c>pData</c>, in bytes 
		uint8_t iBitsPerSample;
		uint8_t iChannelCount;
		uint32_t iSampleRate; // samples per seconds (Hz)
	};



	class SourceVoice : public IXAudio2VoiceCallback
	{
	private: // variables

		IXAudio2* const m_pEngine;

		IXAudio2SourceVoice* m_pVoice = nullptr;
		std::atomic<bool> m_bRunning = false;
		std::atomic<bool> m_bPaused = false;


	public: // methods

		SourceVoice(IXAudio2* pEngine) : m_pEngine(pEngine) {}
		~SourceVoice() { destroy(); }


		inline bool isAvailable() { return !m_bRunning; }
		inline bool isPaused() { return m_bPaused; }

		void play(WaveformData& data, float volume = 1.0);
		void pause();
		void resume();
		void stop();
		void destroy();

		virtual inline void OnBufferEnd(void* pBufferContext) override {}
		inline void OnBufferStart(void* pBufferContext) override {}
		inline void OnLoopEnd(void* pBufferContext) override {}
		inline void OnStreamEnd() override { stop(); }
		inline void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
		inline void OnVoiceProcessingPassEnd() override {}
		inline void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
	};



	class SoundVoice : public IXAudio2VoiceCallback
	{
	public: // methods

		SoundVoice(std::mutex& mux, std::condition_variable& cv,
			WaveformData& data, float volume = 1.0);
		~SoundVoice();

		void OnStreamEnd() override; // TODO:

		virtual inline void OnBufferEnd(void* pBufferContext) override {}
		inline void OnBufferStart(void* pBufferContext) override {}
		inline void OnLoopEnd(void* pBufferContext) override {}
		inline void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
		inline void OnVoiceProcessingPassEnd() override {}
		inline void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}

		void pause();
		void resume();
		void stop();


	private: // variables

		std::mutex& m_mux;
		std::condition_variable& m_cv;
		IXAudio2SourceVoice* m_pVoice;
		std::atomic<bool> m_bPaused;

	};

	class Sound
	{
	public:

		/// <summary>
		/// Create a <c>Sound</c> object, assign data
		/// </summary>
		/// <param name="ManageData">
		/// = should the data pointed to by <c>data.pData</c> automatically be deleted on
		/// destruction?
		/// </param>
		Sound(WaveformData data, bool ManageData = true);

		~Sound();

		void play(float volume = 1.0);
		void pauseAll();
		void resumeAll();
		void stopAll();


	private: // methods

		// function to run in threads
		void threadPlay(float volume);


	private: // variables

		WaveformData m_oData;
		bool m_bManage = false;
		std::vector<SoundVoice*> m_oVoices;
		std::mutex m_muxVector;
		std::atomic<uint64_t> m_iThreadCount; // count of currently running threads

	};





	/// <summary>
	/// An XAudio2-based audio engine
	/// </summary>
	class AudioEngine
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
		/// <returns>Could the mastering voice be created?</returns>
		bool create(const wchar_t* szDeviceID = NULL, uint8_t ChannelCount = 2,
			uint32_t samplerate = 44100, size_t BufBlockCount = 8, size_t BufSamples = 512);

		void destroy();


		/// <summary>
		/// Get a pointer to the current <c>IXAudio2</c> object<para/>
		/// Returns <c>nullptr</c> if there is no current <c>IXAudio2</c> object
		/// </summary>
		inline static IXAudio2* getEnginePtr() { return m_pXAudio2; }


	private: // methods

		AudioEngine(); // --> singleton
		~AudioEngine();


		void threadFunc(IXAudio2SourceVoice* m_pVoice);


	private: // variables

		static IXAudio2* m_pXAudio2; // XAudio2 object
		static IXAudio2MasteringVoice* m_pMaster; // mastering voice

		static std::atomic<bool> m_bRunning;

	};

}





// #undefs

#endif // ROBINLE_AUDIOSTREAM