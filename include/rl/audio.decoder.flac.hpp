/***************************************************************************************************
 FILE:	audio.decoder.flac.hpp
 CPP:	audio.decoder.flac.cpp
 DESCR:	A decoder for FLAC audio files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_TEMPLATE
#define ROBINLE_TEMPLATE





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned;
using uint64_t = unsigned long long;


#include <rl/audio.engine.hpp>
#include <istream>



//==================================================================================================
// DECLARATION
namespace rl
{

	namespace FLAC
	{

		using FileStream = std::basic_ifstream<uint8_t>;
		using BinStream = std::basic_istream<uint8_t>;


#define DEFINE_GETTER(var) inline auto var() const noexcept { return m_##var; } struct dummy
		class IDataSegment
		{
		public:

			inline operator bool() const noexcept { return m_bValid; }


		public: // methods

			inline bool read(BinStream& input) { return m_bValid = doRead(input); }


		protected: // static methods

			static bool Available(BinStream& stream, size_t iByteCount);


		protected: // methods

			virtual bool doRead(BinStream& input) = 0;


		private: // variables

			bool m_bValid = false;

		};



		class METADATA_BLOCK_HEADER : public IDataSegment
		{
		public: // methods

			DEFINE_GETTER(bLastMetadataBlock);
			DEFINE_GETTER(iBlockType);
			DEFINE_GETTER(iBlockLength);


		protected: // methods

			bool doRead(BinStream& input) override;


		private: // variables

			bool m_bLastMetadataBlock = false; // 1 bit
			uint8_t m_iBlockType = 0; // 7 bits
			uint32_t m_iBlockLength = 0; // 24 bits

		};



		class METADATA_BLOCK_STREAMINFO : public IDataSegment
		{
		public: // types

			using Signature = uint8_t[128 / 8];


		public: // methods

			DEFINE_GETTER(iMinBlockSize);
			DEFINE_GETTER(iMaxBlockSize);
			DEFINE_GETTER(iMinFrameSize);
			DEFINE_GETTER(iMaxFrameSize);
			DEFINE_GETTER(iSampleRate);
			DEFINE_GETTER(iChannelCount);
			DEFINE_GETTER(iBitsPerSample);
			DEFINE_GETTER(iSampleCount);
			inline const Signature& signature() const noexcept { return m_signature; }


		protected: // methods

			bool doRead(BinStream& input) override;


		private: // variables

			uint16_t m_iMinBlockSize = 0;
			uint16_t m_iMaxBlockSize = 0;
			uint32_t m_iMinFrameSize = 0; // 24 bits
			uint32_t m_iMaxFrameSize = 0; // 24 bits
			uint32_t m_iSampleRate = 0; // 20 bits
			uint8_t m_iChannelCount = 0; // 3 bits, -1
			uint8_t m_iBitsPerSample = 0; // 5 bits, -1
			uint64_t m_iSampleCount = 0; // 36 bits
			Signature m_signature{};
		};



		class FRAME_HEADER : public IDataSegment
		{
		public: // static variables

			static constexpr uint16_t iCRC8_Polynomial = 0b100000111; // x^8+x^2+x^1+x^0


		public: // methods

			DEFINE_GETTER(bVariableBlocksize);
			DEFINE_GETTER(iBlockSizeBits);
			DEFINE_GETTER(iSampleRateBits);
			DEFINE_GETTER(iChannelAssignment);
			DEFINE_GETTER(iSampleSizeBits);
			DEFINE_GETTER(iSampleSize);
			DEFINE_GETTER(iFrameOrSampleNumber);
			DEFINE_GETTER(iBlockSize);
			DEFINE_GETTER(iSampleRate);


		protected: // methods

			bool doRead(BinStream& input) override;


		private: // variables

			bool m_bVariableBlocksize = false; // = "blocking strategy' bit
			uint8_t m_iBlockSizeBits = 0;
			uint8_t m_iSampleRateBits = 0;
			uint8_t m_iChannelAssignment = 0;
			uint8_t m_iSampleSizeBits = 0;
			uint8_t m_iSampleSize = 0;
			uint64_t m_iFrameOrSampleNumber = 0; // sample no if variable block size, else frame no
			uint16_t m_iBlockSize = 0; // if blocksize bits = 011x
			uint32_t m_iSampleRate = 0; // if sample rate bits = 11xx
			
		};

		class SUBFRAME_HEADER : public IDataSegment
		{
		public: // methods

			DEFINE_GETTER(iSubframeType);
			DEFINE_GETTER(iWastedBitsPerSample);


		protected: // methods

			bool doRead(BinStream& input) override;


		private: // variables

			uint8_t m_iSubframeType = 0; // 6 bits
			uint8_t m_iWastedBitsPerSample = 0; // unknown size, 8 bits are probably enough

		};

		class FRAME_FOOTER : public IDataSegment
		{
		public: // static variables

			static constexpr uint32_t iCRC16_Polynomial = 0b11000000000000101; // x^16+x^15+x^2+x^0


		public: // methods

			DEFINE_GETTER(iCRC16);


		protected: // methods

			bool doRead(BinStream& input) override;


		private: // variables

			uint16_t m_iCRC16 = 0; // x^16+x^15+x^2+x^0, initalized with 0

		};

		



#undef DEFINE_GETTER

		struct MultiChannelAudioSamples
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
			/// <summary>
			/// The count of samples
			/// </summary>
			size_t iSampleCount;

			union
			{
				/// <summary>
				/// The destination for 8-bit PCM audio
				/// </summary>
				rl::audio8_t** p8;
				/// <summary>
				/// The destination for 16-bit PCM audio
				/// </summary>
				rl::audio16_t** p16;
				/// <summary>
				/// The destination for 24-bit PCM audio
				/// </summary>
				rl::audio24_t** p24;
				/// <summary>
				/// The destination for 32-bit PCM audio
				/// </summary>
				rl::audio32_t** p32;
			} val;

			/// <summary>
			/// Allocate data based on iChannelCount and iSampleCount
			/// </summary>
			void alloc();

			/// <summary>
			/// Delete allocated memory
			/// </summary>
			void free();

		};

	}

	class FLACStream : public IAudioStream
	{
	public: // methods

		bool start(const wchar_t* szFileName, float volume = 1.0f);

		virtual ~FLACStream();


	protected: // methods

		bool nextSample(float fElapsedTime, MultiChannelAudioSample& dest) noexcept override;


	private: // methods

		bool readFrame();


	private: // variables

		FLAC::METADATA_BLOCK_STREAMINFO m_oStreamInfo;
		FLAC::FRAME_HEADER m_oFrameHeader;

		size_t m_iCurrentSample_Global = 0;
		size_t m_iCurrentSample_Frame = 0;

		FLAC::FileStream* m_pInput = nullptr;
		bool m_bEndOfFrame = true;
		FLAC::MultiChannelAudioSamples m_oSamples;

	};



	class FLAC_test
	{
	public: // methods

		bool loadFromFile(const wchar_t* szFileName);

		inline bool loaded() const noexcept { return m_bLoaded; }


	private: // variables

		bool m_bLoaded = false;
		FLAC::METADATA_BLOCK_STREAMINFO m_oStreamInfo = {};
	};

}





// #undef foward declared definitions

#endif // ROBINLE_TEMPLATE