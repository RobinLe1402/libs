#include "include/BitReader.hpp"

// Project
#include "include/Assert.hpp"

// RobinLe
#include "rl/data.endian.hpp"

// STL
#include <stdint.h>





namespace rl
{
	using namespace RobinLeFLACDLL;

	using private_::BitReader;



	/***********************************************************************************************
	 class BitReader
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	BitReader::BitReader(IDataReader* pReader, bool bOwnsReader) :
		m_bOwnsReader(bOwnsReader),
		m_pReader(pReader),
		m_upBuffer(std::make_unique<word_t[]>(s_iCapacity)),
		m_pBuffer(m_upBuffer.get())
	{
		FLAC__ASSERT(pReader != nullptr);
	}

	BitReader::~BitReader()
	{
		if (m_bOwnsReader)
			delete m_pReader;
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// FLAC__bitreader_read_raw_uint32
	bool BitReader::readRawUInt32(uint32_t& iDest, uint8_t iBitCount)
	{
		FLAC__ASSERT(m_pBuffer != nullptr);

		FLAC__ASSERT(iBitCount <= 32); // only up to 32 bits can be read
		FLAC__ASSERT((s_iCapacity * s_iBitsPerWord) * 2 >= iBitCount);
		FLAC__ASSERT(m_iConsumedWords <= m_iBufferWords);

		// method only works with a word_t of at least 32 bits.
		FLAC__ASSERT(s_iBitsPerWord >= 32);

		// should something be read?
		if (iBitCount == 0)
		{
			iDest = 0;
			return true;
		}

		// read enough data to be able to fulfill the request for data
		while (bitsLeft() < iBitCount)
		{
			if (!readFromClient())
				return false; // couldn't read enough bits
		}

		// branch for optimization, see "else"
		if (m_iConsumedWords < m_iBufferWords)
		{
			// not within a last, partially undefined word

			if (m_iConsumedBits)
			{
				// would be slower if m_iConsumedBits == 0

				const uint32_t iBitsLeftInWord = s_iBitsPerWord - m_iConsumedBits;
				const word_t iWord = m_pBuffer[m_iConsumedWords];
				const word_t iMask = (m_iConsumedBits < s_iBitsPerWord) ?
					(s_iWordAllSet >> m_iConsumedBits) : 0;

				if (iBitCount < iBitsLeftInWord)
				{
					// more bits available than needed
					const uint32_t iShift = iBitsLeftInWord - iBitCount;
					iDest = (iShift < s_iBitsPerWord) ? uint32_t((iWord & iMask) >> iShift) : 0;
					m_iConsumedBits += iBitCount;
					return true;
				}

				// exactly enough or too few bits left
				iDest = uint32_t(iWord & iMask);
				iBitCount -= iBitsLeftInWord;
				++m_iConsumedWords;
				m_iConsumedBits = 0;
				if (iBitCount)
				{
					// there are still bits left to read
					// --> cannot be more than 32, must fit into next word

					const uint32_t iShift = s_iBitsPerWord - iBitCount;
					iDest = (iBitCount < 32) ? (iDest << iBitCount) : 0;
					iDest |= (iShift < s_iBitsPerWord) ?
						uint32_t(m_pBuffer[m_iConsumedWords] >> iShift) : 0;
				}

				return true;

			}
			else // m_iConsumedBits == 0
			{
				const word_t word = m_pBuffer[m_iConsumedWords];
				if (iBitCount < s_iBitsPerWord)
				{
					iDest = uint32_t(word >> (s_iBitsPerWord - iBitCount));
					m_iConsumedBits = iBitCount;
					return true;
				}

				// at this point: reading full word of 32 bits (--> previous assertion: max 32 bits)
				iDest = word;
				++m_iConsumedWords;
				return true;
			}




			// ToDo: Line 360 @ bitreader.c
		}
		else
		{
			// special case: inside last, partially undefined word of buffer
			// --> single word definitely contains all bits needed

			// branch for optimization
			if (m_iConsumedBits)
			{
				// would be slower if there were no bits consumed

				FLAC__ASSERT(m_iConsumedBits + iBitCount <= m_iBufferBytes * 8);
				iDest = uint32_t((m_pBuffer[m_iConsumedWords] & (s_iWordAllSet >> m_iConsumedBits))
					>> (m_iConsumedBits - iBitCount));
				m_iConsumedBits += iBitCount;
				return true;
			}
			else
			{
				iDest = uint32_t(m_pBuffer[m_iConsumedWords] >> (s_iBitsPerWord - iBitCount));
				m_iConsumedBits += iBitCount;
				return true;
			}
		}
	}

	bool BitReader::readRawInt32(int32_t& iDest, uint8_t iBitCount)
	{
		// ToDo: readRawInt32()
		return false;
	}

	bool BitReader::readRawUInt64(uint64_t& iDest, uint8_t iBitCount)
	{
		// ToDo: readRawUInt64()
		return false;
	}

	bool BitReader::skipBits(size_t iBitCount)
	{
		// ToDo: skipBits()
		return false;
	}

	bool BitReader::skipByteBlockAligned(size_t iBitCount)
	{
		// ToDo: skipByteBlockAligned()
		return false;
	}

	bool BitReader::readByteBlockAligned(uint8_t* pDest, size_t iBitCount)
	{
		// ToDo: readByteBlockAligned()
		return false;
	}

	bool BitReader::readUnaryUnsigned(uint32_t& iDest)
	{
		// ToDo: readUnaryUnsigned()
		return false;
	}

	bool BitReader::readRiceSigned(int& iDest, uint32_t iParameter)
	{
		// ToDo: readRiceSigned()
		return false;
	}

	// FLAC__bitreader_clear
	void BitReader::clear()
	{
		m_iBufferWords = 0;
		m_iBufferBytes = 0;

		m_iConsumedWords = 0;
		m_iConsumedBits = 0;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// bitreader_read_from_client_
	bool BitReader::readFromClient()
	{
		FLAC__ASSERT(m_pBuffer != nullptr);
		FLAC__ASSERT(m_pReader != nullptr);

		size_t iStart;
		size_t iEnd;
		size_t iBytes;

		// Full bytes have been read --> move unread data to start of buffer
		if (m_iConsumedWords > 0)
		{
			iStart = m_iConsumedWords;
			iEnd = m_iBufferWords + (m_iBufferBytes ? 1 : 0);
			memmove(m_pBuffer, m_pBuffer + iStart, (iEnd - iStart) * s_iBytesPerWord);

			m_iBufferWords -= iStart;
			m_iConsumedWords = 0;
		}

		if (m_pReader->eof())
			return false; // there is no more data

		iBytes = (s_iCapacity - m_iBufferWords) * s_iBytesPerWord - m_iBufferBytes;
		if (iBytes == 0)
			return false; // no free bytes --> cannot read from client

		const auto pDest = reinterpret_cast<uint8_t*>(m_pBuffer + m_iBufferWords) + m_iBufferBytes;

		// if there's a partial trailing word, the last word might have to be flipped
		// before writing new data to the buffer due to endianness and the above cast to a
		// byte pointer.
		//
		// The following figure shows example buffer data for a 32-bit buffer.
		// "??" = undefined values
		// 
		// Bitstream:  11 22 33 44 55
		// Buffer BE:  11 22 33 44 55 ?? ?? ??
		// Buffer LE:  44 33 22 11 ?? ?? ?? 55
		//                            ^^--------pBuffer points here
		//
		// As you can see, on lE machines, a byteswap is necessary to avoid overwriting current
		// data in a partial trailing word.

		if constexpr (!BigEndian)
		{
			if (m_iBufferBytes)
				m_pBuffer[m_iConsumedWords] = Endian::Swap(m_pBuffer[m_iConsumedWords]);
		}

		// Now memory looks like this:
		// 
		// Bitstream:  11 22 33 44 55
		// Buffer BE:  11 22 33 44 55 ?? ?? ??
		// Buffer LE:  44 33 22 11 55 ?? ?? ??
		//                            ^^--------pBuffer points here

		if (!m_pReader->getData(iBytes, pDest))
			return false; // data couldn't be read

		// After reading bytes 66 77 88 99 AA BB CC DD EE FF from client, memory looks like this:
		// Bitstream:  11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
		// Buffer BE:  11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF ??
		// Buffer LE:  44 33 22 11 55 66 77 88 99 AA BB CC DD EE FF ??
		//
		// Since the buffer is processed as word_t, the endian of the newly filled words must be
		// swapped on LE machines.

		if constexpr (!BigEndian)
		{
			iEnd = m_iBufferWords +
				((m_iBufferBytes + iBytes + (s_iBytesPerWord - 1)) / s_iBytesPerWord);

			for (iStart = m_iBufferWords; iStart < iEnd; ++iStart)
			{
				m_pBuffer[iStart] = Endian::Swap(m_pBuffer[iStart]);
			}
		}

		// Now memory looks like this:
		// Bitstream:  11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
		// Buffer BE:  11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF ??
		// Buffer LE:  44 33 22 11 88 77 66 55 CC BB AA 99 ?? FF EE DD

		iEnd = m_iBufferWords * s_iBytesPerWord + m_iBufferBytes + iBytes;
		m_iBufferWords = iEnd / s_iBytesPerWord;
		m_iBufferBytes = iEnd % s_iBytesPerWord;

		return true;
	}

}