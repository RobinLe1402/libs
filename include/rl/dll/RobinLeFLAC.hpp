/*
 ROBINLE'S MINIMALISTIC FLAC DECODER LIBRARY
 (c) 2022 RobinLe
 
 This C++ FLAC decoder library is basically Xiph's libFLAC, but rewritten using modern C++, while
 including only the functions necessary for reading raw audio data from FLAC data.
 
 Removed/left-out functionality compared to libFLAC:
 - CRC16 check --> files are assumed to contain no errors
 - metadata reading
 
 Please note that I never claimed to provide the most optimized execution.
 
 
 
 == WHAT THIS LIBRARY IS **NOT** INTENDED FOR ==
 (though I can't stop you if you still want to use it this way)
 - use in professional software
 
 == WHAT THIS LIBRARY **IS** INTENDED FOR ==
 - use in my very own software
 - (hopefully) simpler understanding of how FLAC decoding works
   -->  I try to achieve this by changing the original libFLAC code to be object-oriented and
        by adding some more documentation comments (mostly XML)
 

 The highly optimized original code by the Xiph.Org Foundation can be found here:
 https://github.com/xiph/flac
*/

/***************************************************************************************************
 FILE:  RobinLeFLAC.hpp
 DLL:   RobinLeFLAC.dll
 LIB:   RobinLeFLAC.lib
 DESCR: A FLAC Decoder
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DLL_FLAC
#define ROBINLE_DLL_FLAC

#ifdef LIBRARY_EXPORTS
	#define ROBINLE_FLAC_API __declspec(dllexport)
#else
	#define ROBINLE_FLAC_API __declspec(dllimport)
#endif





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint32_t = unsigned int;
using int64_t = long long;
#ifdef _WIN64 // 64-bit
using size_t = unsigned long long;
#else // 32-bit
using size_t = unsigned long;
#endif


#include <functional>





//==================================================================================================
// DECLARATION
namespace rl
{
	namespace RobinLeFLACDLL
	{

		/// <summary>
		/// Where should a jump be based?
		/// </summary>
		enum class DataPosSeekMethod
		{
			/// <summary>
			/// Jump from the beginning of the data<para />
			/// Jump fails if the jump value is negative or the destination would be out of bounds
			/// </summary>
			Begin,
			/// <summary>
			/// Jump from the current position<para />
			/// Jump fails if the destination would be out of bounds
			/// </summary>
			Current,
			/// <summary>
			/// Jump from the end of the data<para />
			/// Jump fails if the jump value is positive or the destination would be out of bounds
			/// </summary>
			End
		};





		/// <summary>
		/// Interface for a data reader.<para />
		/// Provides data for the FLAC decoder on demand.
		/// </summary>
		class ROBINLE_FLAC_API IDataReader
		{
		public: // methods

			/// <summary>
			/// Called by <c>rl::FLACDecoder</c> when new data is needed
			/// </summary>
			/// <param name="cb">
			/// The count of bytes expected.<para />
			/// Will be set to the count of bytes actually read.
			/// </param>
			/// <param name="dest">the buffer the read data should be written to</param>
			/// <returns>The count of bytes actually read</returns>
			virtual bool getData(size_t& cb, uint8_t* dest) = 0;

			/// <summary>
			/// Was the end of the data stream reached?<para />
			/// Returns <c>true</c> if the reader isn't able to read any data
			/// </summary>
			virtual bool eof() = 0;

			/// <summary>
			/// Total size (in bytes) of the data being read.
			/// </summary>
			virtual size_t size() = 0;

			/// <summary>
			/// Tell the current position (in bytes).
			/// </summary>
			virtual size_t tellPos() = 0;

			/// <summary>
			/// Jump to an absolute position in the data.
			/// </summary>
			/// <returns>Was the jump successful?</returns>
			virtual bool seekPos(size_t pos) = 0;

			/// <summary>
			/// Jump to a relative position in the data.<para />
			/// The seek fails if the destination would be out of bounds.
			/// </summary>
			/// <param name="offset">
			/// The count of bytes to move from the given base.
			/// </param>
			/// <returns>Was the jump successful?</returns>
			bool seekPos(int64_t offset, DataPosSeekMethod method);


		};





		// forward declaration
		namespace private_ { class BitReader; }





		/// <summary>
		/// The main class for decoding FLAC-encoded audio data
		/// </summary>
		class ROBINLE_FLAC_API FLACDecoder
		{
		public: // methods

			FLACDecoder();
			FLACDecoder(const wchar_t* szFileName);
			FLACDecoder(IDataReader* pReader);
			~FLACDecoder();

			bool open(const wchar_t* szFileName);
			bool open(IDataReader* pReader);
			void close();

			/// <summary>
			/// Has the last sample been read?
			/// </summary>
			bool eof();
			/// <summary>
			/// Is a stream being read currently?
			/// </summary>
			inline bool isOpened() { return m_pBitReader; }

			bool nextSample();
			bool getSample(uint32_t& iDest, uint8_t iChannel);


		private: // methods

			/// <summary>
			/// Initialize the decoder by starting to read from from <c>m_pBitReader</c>.
			/// </summary>
			/// <returns>Was the initialization successful?</returns>
			bool initialize();


		private: // variables

			private_::BitReader* m_pBitReader;
			IDataReader* m_pReader;

		};

	}
}





#endif // ROBINLE_DLL_FLAC