/***************************************************************************************************
 FILE:	BitReader.hpp
 CPP:	BitReader.cpp
 DESCR:	Implementation of class rl::RobinLeFLACDLL::FLACDecoder::BitReader
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_ROBINLEFLAC_BITREADER
#define ROBINLE_LIB_ROBINLEFLAC_BITREADER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint32_t = unsigned int;
using int32_t = int;
using uint64_t = unsigned long long;
#ifdef _WIN64 // 64-bit
using size_t = unsigned long long;
#else // 32-bit
using size_t = unsigned int;
#endif



//==================================================================================================
// INCLUDES

#include "rl/dll/RobinLeFLAC.hpp"

#include <memory>



//==================================================================================================
// DECLARATION
namespace rl
{
	
	/// <summary>
	/// A class for reading data bit by bit
	/// </summary>
	class RobinLeFLACDLL::private_::BitReader
	{
	private: // types

#ifdef _WIN64 // 64-bit
		using word_t = uint64_t;
#else // 32-bit
		using word_t = uint32_t;
#endif


	private: // static variables

		constexpr static size_t s_iBytesPerWord = sizeof(word_t);
		constexpr static size_t s_iBitsPerWord = s_iBytesPerWord * 8;
		constexpr static size_t s_iWordAllSet = (word_t)-1; // -1 is "all bits set"
		constexpr static size_t s_iCapacity = 65536u / s_iBitsPerWord;


	public: // methods

		BitReader(IDataReader* pReader, bool bOwnsReader);
		~BitReader();

		/// <summary>
		/// Is this <c>BitReader</c> currently byte-aligned?
		/// </summary>
		inline bool byteAligned() { return (m_iConsumedBits & 7) == 0; }
		/// <summary>
		/// How many bits have to be read for this <c>BitReader</c> to become byte-aligned?
		/// </summary>
		inline uint8_t bitsUntilByteAlignment() { return 8 - (m_iConsumedBits & 7); }
		/// <summary>
		/// How many bits available for reading are left in the current buffer?
		/// </summary>
		inline size_t bitsLeft()
		{
			return (s_iCapacity * s_iBitsPerWord - (m_iConsumedWords * s_iBitsPerWord) -
				m_iConsumedBits);
		}

		bool readRawUInt32(uint32_t& iDest, uint8_t iBitCount);
		bool readRawInt32(int32_t& iDest, uint8_t iBitCount);
		bool readRawUInt64(uint64_t& iDest, uint8_t iBitCount);
		bool skipBits(size_t iBitCount);
		bool skipByteBlockAligned(size_t iBitCount);
		bool readByteBlockAligned(uint8_t* pDest, size_t iBitCount);
		bool readUnaryUnsigned(uint32_t& iDest);
		bool readRiceSigned(int& iDest, uint32_t iParameter);

		void clear();


	private: // methods

		bool readFromClient();


	private: // variables

		IDataReader* const m_pReader;
		const bool m_bOwnsReader;
		std::unique_ptr<word_t[]> m_upBuffer;
		word_t* const m_pBuffer = nullptr;

		size_t m_iBufferWords = 0; // count of complete words in buffer
		size_t m_iBufferBytes = 0; // count of bytes in last word of buffer
		size_t m_iConsumedWords = 0; // the number of words consumed
		size_t m_iConsumedBits = 0; // the number of bits consumed in addition to the consumed words
	};
	
}





// #undef foward declared definitions

#endif // ROBINLE_LIB_ROBINLEFLAC_BITREADER