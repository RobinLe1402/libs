#include "audio.decoder.flac.hpp"

#include <Windows.h>
#include <fstream>





namespace rl
{

	namespace FLAC
	{
		constexpr char szE_EOS[] = "Unexpected end of stream";


		// CRC calculation code based on C# code provided at
		// http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html

		template <typename T>
		struct CRCLookupTable
		{
			template <typename T>
			constexpr CRCLookupTable(T iPolynomial)
			{
				for (uint16_t iDivident = 0; iDivident < 256; ++iDivident)
				{
					T iCurrent = T(iDivident) << ((sizeof(T) - 1) * 8);
					for (uint8_t iBit = 0; iBit < 8; ++iBit)
					{
						if (iCurrent & (T(1) << (sizeof(T) * 8 - 1)))
						{
							iCurrent <<= 1;
							iCurrent ^= iPolynomial;
						}
						else
							iCurrent <<= 1;
					}

					iLookupTable[iDivident] = iCurrent;
				}
			}

			T iLookupTable[256];
		};

		constexpr CRCLookupTable<uint8_t> CRCLookup_FrameHeader((uint8_t)FRAME_HEADER::iCRC8_Polynomial);
		constexpr CRCLookupTable<uint16_t> CRC16Lookup(FRAME_FOOTER::iCRC16_Polynomial);

		uint8_t CRC_FrameHeader(FLAC::BinStream& input, size_t len)
		{
			uint8_t crc = 0;

			uint8_t byte = 0;
			for (size_t i = 0; i < len; ++i)
			{
				input.read(&byte, 1);

				crc = CRCLookup_FrameHeader.iLookupTable[byte ^ crc];
			}

			return crc;
		}


		void MultiChannelAudioSamples::alloc()
		{
			val.p8 = new uint8_t * [iChannelCount];

			for (size_t i = 0; i < iChannelCount; ++i)
			{
				val.p8[i] = new uint8_t[iSampleCount * (iBitsPerSample / 8)];
			}
		}

		void MultiChannelAudioSamples::free()
		{
			for (size_t i = 0; i < iChannelCount; ++i)
			{
				delete[] val.p8[i];
			}

			delete[] val.p8;
		}










		/*******************************************************************************************
		 class IDataSegment
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// STATIC METHODS

		bool IDataSegment::Available(BinStream& stream, size_t iByteCount)
		{
			auto pos = stream.tellg();
			bool bResult = (bool)stream.seekg(iByteCount, std::ios_base::cur);
			stream.seekg(pos);

			return bResult;
		}










		/*******************************************************************************************
		 class METADATA_BLOCK_HEADER
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// PROTECTED METHODS

		bool METADATA_BLOCK_HEADER::doRead(BinStream& input)
		{
			if (!Available(input, 4))
				return false;

			uint8_t iTmp1 = 0;

			input.read(&iTmp1, 1);
			m_bLastMetadataBlock = (iTmp1 >> 7) & 1;
			m_iBlockType = iTmp1 & 0x7F;

			uint8_t iTmp3[3];
			input.read(iTmp3, 3);
			m_iBlockLength = 0;
			for (uint8_t i = 0; i < 3; ++i)
			{
				m_iBlockLength |= uint32_t(iTmp3[i]) << ((2 - i) * 8);
			}

			return true;
		}










		/*******************************************************************************************
		 class FRAME_HEADER
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// PROTECTED METHODS

		bool FRAME_HEADER::doRead(BinStream& input)
		{
			// check for minimum required size --> check for exact size later
			if (!Available(input, 6))
				return false;

			const auto start = input.tellg(); // for CRC comparison at the end

			uint8_t buf[7]{};
			uint16_t buf16 = 0;
			uint8_t& buf8 = buf[0];



			input.read(buf, 2);
			buf16 = (uint16_t(buf[0]) << 8) | buf[1];

			const uint16_t iSyncCode = buf16 >> 2;
			if (iSyncCode != 0b11'1111'1111'1110)
				return false;

			m_bVariableBlocksize = buf16 & 1;



			input.read(buf, 1);
			m_iBlockSizeBits = buf8 >> 4;
			m_iSampleRateBits = buf8 & 0x0F;

			// evaluate block size bits
			switch (m_iBlockSizeBits)
			{
			case 0b0001:
				m_iBlockSize = 192;
				break;

			case 0b0010:
			case 0b0011:
			case 0b0101:
				m_iBlockSize = 576 * (1ui16 << (m_iBlockSizeBits - 2)); // 576 * (2^(n-2))
				break;

			case 0b1000:
			case 0b1001:
			case 0b1010:
			case 0b1011:
			case 0b1100:
			case 0b1101:
			case 0b1110:
			case 0b1111:
				m_iBlockSize = 256 * (1ui32 << (m_iBlockSizeBits - 8)); // 256 * (2^(n-8))
				break;

			default:
				m_iBlockSize = 0;
			}

			// evaluate sample rate bits
			switch (m_iSampleRateBits)
			{
			case 0b0001:
				m_iSampleRate = 88'200;
				break;
			case 0b0010:
				m_iSampleRate = 176'400;
				break;
			case 0b0011:
				m_iSampleRate = 192'000;
				break;
			case 0b0100:
				m_iSampleRate = 8'000;
				break;
			case 0b0101:
				m_iSampleRate = 16'000;
				break;
			case 0b0110:
				m_iSampleRate = 22'050;
				break;
			case 0b0111:
				m_iSampleRate = 24'000;
				break;
			case 0b1000:
				m_iSampleRate = 32'000;
				break;
			case 0b1001:
				m_iSampleRate = 44'100;
				break;
			case 0b1010:
				m_iSampleRate = 48'000;
				break;
			case 0b1011:
				m_iSampleRate = 96'000;
				break;
			default:
				m_iSampleRateBits = 0;
			}




			input.read(buf, 1);
			m_iChannelAssignment = buf8 >> 4;
			m_iSampleSizeBits = (buf8 >> 1) & 0x07;

			// evaluate sample size bits
			switch (m_iSampleSizeBits)
			{
			case 0b001:
				m_iSampleSize = 8;
				break;
			case 0b010:
				m_iSampleSize = 12;
				break;
			case 0b100:
				m_iSampleSize = 16;
				break;
			case 0b101:
				m_iSampleSize = 20;
				break;
			case 0b110:
				m_iSampleSize = 24;
				break;

			default:
				m_iSampleSize = 0;
			}



			// read "UTF-8" coded value (frame number or sample number)
			{
				memset(buf, 0, 7); // clear buffer

				input.read(buf, 1);
				if ((buf[0] & 0x80) == 0)
					m_iFrameOrSampleNumber = buf[0];
				else
				{
					uint8_t iCodeUnitCount = 2;
					const uint8_t iCodeUnitCountMax = m_bVariableBlocksize ? 56 / 8 : 48 / 8;

					while (iCodeUnitCount <= iCodeUnitCountMax &&
						((buf[0] >> (7 - iCodeUnitCount)) & 1))
						++iCodeUnitCount;

					if (iCodeUnitCount > iCodeUnitCountMax)
						return false; // coding error!

					const uint8_t iFirstBytePayloadBits = 8 - 1 - iCodeUnitCount;

					const uint8_t iBitCount = (iCodeUnitCount - 1) * 6 + iFirstBytePayloadBits;

					uint8_t iCurrentShift = iBitCount - 1;

					m_iFrameOrSampleNumber =
						uint64_t(buf[0] & (0xFF >> (iCodeUnitCount + 1))) << iCurrentShift;
					iCurrentShift += iFirstBytePayloadBits;

					input.read(buf + 1, (std::streamsize)iCodeUnitCount - 1);

					for (uint8_t i = 1; i < iCodeUnitCount; ++i)
					{
						m_iFrameOrSampleNumber |= uint64_t(buf[i]) << iCurrentShift;
						iCurrentShift += 6;
					}
				}
			}



			// (maybe) block size
			if (m_iBlockSize == 0)
			{
				if (m_iBlockSizeBits == 0b0110) // 8 bit
				{
					input.read(buf, 1);
					m_iBlockSize = buf8;

					++m_iBlockSize; // saved value is value-1
				}
				else if (m_iBlockSizeBits == 0b0111) // 16 bit
				{
					input.read(buf, 2);
					m_iBlockSize = uint16_t(buf[0]) << 8 | buf[1];

					++m_iBlockSize; // saved value is value-1
				}
			}



			// (maybe) sample rate
			if (m_iSampleRate == 0)
			{
				if (m_iSampleRateBits == 0b1100) // 8 bit kHz
				{
					input.read(buf, 1);
					m_iSampleRate = 10 * buf[0];
				}
				else if (m_iSampleRateBits == 0b1101) // 16 bit Hz
				{
					input.read(buf, 2);
					m_iSampleRate = uint16_t(buf[0]) << 8 | buf[1];
				}
				else if (m_iSampleRate == 0b1110) // 16 bit 10Hz
				{
					input.read(buf, 2);
					m_iSampleRate = uint16_t(buf[0]) << 8 | buf[1];
					m_iSampleRate *= 10;
				}
			}



			// for CRC8 calculation
			const auto end = input.tellg();


			// check CRC8
			input.seekg(start);
			uint8_t iCRC8 = CRC_FrameHeader(input, end - start);
			input.read(buf, 1);
			if (iCRC8 != buf8)
				return false;


			return true;
		}










		/*******************************************************************************************
		 class SUBFRAME_HEADER
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// PROTECTED METHODS

		bool SUBFRAME_HEADER::doRead(BinStream& input)
		{
			uint8_t iBuf = 0;
			if (!input.read(&iBuf, 1))
				return false;

			m_iSubframeType = (iBuf >> 1) & 0x3F;

			if ((iBuf & 1) == 0)
				m_iWastedBitsPerSample = 0;
			else
			{
				m_iWastedBitsPerSample = 0;
				bool bFirstSetBitFount = false;
				do
				{
					input.read(&iBuf, 1);
					for (uint8_t i = 0; i < 8; ++i)
					{
						++m_iWastedBitsPerSample;

						if (iBuf & (1 << (7 - i)))
						{
							bFirstSetBitFount = true;
							break; // bit for loop
						}
					}
				} while (!bFirstSetBitFount);
				// ! potential error - assuming subframe header ends on full byte !
			}

			return true;
		}










		/*******************************************************************************************
		 class FRAME_FOOTER
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// PROTECTED METHODS

		bool FRAME_FOOTER::doRead(BinStream& input)
		{
			if (!Available(input, 2))
				return false;

			uint8_t iBuffer[2]{};
			input.read(iBuffer, 2);

			m_iCRC16 = (uint16_t(iBuffer[0]) << 8) | iBuffer[1];
			return true;
		}










		/*******************************************************************************************
		 class METADATA_BLOCK_STREAMINFO
		*******************************************************************************************/

		//==========================================================================================
		// METHODS


		//------------------------------------------------------------------------------------------
		// PROTECTED METHODS

		bool METADATA_BLOCK_STREAMINFO::doRead(BinStream& input)
		{
			if (!Available(input, 34))
				return false;

			uint8_t iBuffer[34]{};
			input.read(iBuffer, 34);

			size_t iOffset = 0;


			// 16 bits
			m_iMinBlockSize = (uint16_t(iBuffer[iOffset]) << 8) | iBuffer[iOffset + 1];
			iOffset += 2;

			// 16 bits
			m_iMaxBlockSize = (uint16_t(iBuffer[iOffset]) << 8) | iBuffer[iOffset + 1];
			iOffset += 2;

			// 24 bits
			m_iMinFrameSize = (uint32_t(iBuffer[iOffset]) << 16) |
				(uint16_t(iBuffer[iOffset + 1]) << 8) | iBuffer[iOffset + 2];
			iOffset += 3;

			// 24 bits
			m_iMaxFrameSize = (uint32_t(iBuffer[iOffset]) << 16) |
				(uint16_t(iBuffer[iOffset + 1]) << 8) | iBuffer[iOffset + 2];
			iOffset += 3;



			// temporary sub-buffer for values of unusual bit count
			uint64_t iSubBuf64 = 0;
			for (size_t i = 0; i < sizeof(iSubBuf64); ++i) // fill the sub-buffer
			{
				iSubBuf64 |= uint64_t(iBuffer[iOffset + i]) << ((sizeof(iSubBuf64) - 1 - i) * 8);
			}
			iOffset += sizeof(iSubBuf64);

			// !! following fields are filled in reverse order for optimization purposes !!

			// 36 bits
			m_iSampleCount = iSubBuf64 & 0x0FFFFFFFFF;
			iSubBuf64 >>= 36;

			// 5 bits
			m_iBitsPerSample = iSubBuf64 & 0x1F;
			iSubBuf64 >>= 5;
			++m_iBitsPerSample; // saved value is value-1

			// 3 bits
			m_iChannelCount = iSubBuf64 & 0x07;
			iSubBuf64 >>= 3;
			++m_iChannelCount; // saved value is value-1

			// 20 bits
			m_iSampleRate = (uint32_t)iSubBuf64;



			// 128 bits
			for (size_t i = 0; i < 128 / 8; ++i)
			{
				m_signature[i] = iBuffer[iOffset + i];
			}


			return true;
		}


	}



	/***********************************************************************************************
	 class FLAC_test
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool FLAC_test::loadFromFile(const wchar_t* szFileName)
	{
		using namespace FLAC;

		FileStream file(szFileName);

		if (!file)
			return false;

		char cMagicNo[4]{};
		file.read(reinterpret_cast<uint8_t*>(cMagicNo), 4);
		if (memcmp(cMagicNo, "fLaC", 4) != 0)
			return false;

		METADATA_BLOCK_HEADER oHeader;

		if (!oHeader.read(file))
			return false;

		if (!m_oStreamInfo.read(file))
			return false;

		// skip other metadata blocks
		if (!oHeader.bLastMetadataBlock())
		{
			do
			{
				oHeader.read(file);
				file.seekg(oHeader.iBlockLength(), std::ios_base::cur);
			} while (!oHeader.bLastMetadataBlock());
		}

		FRAME_HEADER oFrameHeader;
		oFrameHeader.read(file);

		SUBFRAME_HEADER oSubframeHeader;
		oSubframeHeader.read(file);

		const auto iSubframeType = oSubframeHeader.iSubframeType();

		if (iSubframeType == 0b000000) // SUBFRAME_CONSTANT
		{
			// ToDo: SUBFRAME_CONSTANT
		}
		else if (iSubframeType == 0b000001) // SUBFRAME_VERBATIM
		{
			// ToDo: SUBFRAME_VERBATIM
		}
		else if ((iSubframeType & 0b111000) == 0b001000) // SUBFRAME_FIXED
		{
			const uint8_t iOrder = iSubframeType & 0x07;
			if (iOrder > 4)
				return false; // reserved value

			// ToDo: SUBFRAME_FIXED
		}
		else if (iSubframeType & 0b100000) // SUBFRAME_LPC
		{
			const uint8_t iOrder = iSubframeType & 0x1F;

			// ToDo: SUBFRAME_LPC
		}

		FRAME_FOOTER oFrameFooter;
		oFrameFooter.read(file);

		// todo: read next frame, ...

		return true;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class FLACStream
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	FLACStream::~FLACStream()
	{
		delete m_pInput;
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	// operators





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool FLACStream::start(const wchar_t* szFileName, float volume)
	{
		using namespace FLAC;

		stop();
		delete m_pInput;
		m_pInput = nullptr;
		m_bEndOfFrame = true;

		m_pInput = new FileStream(szFileName);
		auto& file = *m_pInput;

		file.open(szFileName);

		if (!file)
			return false;

		char cMagicNo[4]{};
		file.read(reinterpret_cast<uint8_t*>(cMagicNo), 4);
		if (memcmp(cMagicNo, "fLaC", 4) != 0)
			return false;

		METADATA_BLOCK_HEADER oHeader;

		if (!oHeader.read(file))
			return false;

		if (!m_oStreamInfo.read(file))
			return false;



		// skip other metadata blocks
		if (!oHeader.bLastMetadataBlock())
		{
			do
			{
				oHeader.read(file);
				file.seekg(oHeader.iBlockLength(), std::ios_base::cur);
			} while (!oHeader.bLastMetadataBlock());
		}



		WaveFormat fmt;

		if (m_oStreamInfo.iBitsPerSample() <= 8)
			fmt.eBitDepth = AudioBitDepth::Audio8;
		else if (m_oStreamInfo.iBitsPerSample() <= 16)
			fmt.eBitDepth = AudioBitDepth::Audio16;
		else if (m_oStreamInfo.iBitsPerSample() <= 24)
			fmt.eBitDepth = AudioBitDepth::Audio24;
		else
			fmt.eBitDepth = AudioBitDepth::Audio32;

		fmt.iChannelCount = m_oStreamInfo.iChannelCount();
		fmt.iSampleRate = m_oStreamInfo.iSampleRate();



		m_oSamples.free();
		m_oSamples.val.p8 = nullptr;
		m_oSamples.iBitsPerSample = (uint8_t)fmt.eBitDepth;
		m_oSamples.iChannelCount = m_oStreamInfo.iChannelCount();


		internalStart(fmt, volume);
		return true;
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	bool FLACStream::readFrame()
	{
		if (!m_oFrameHeader.read(*m_pInput))
			return false;

		m_iCurrentSample_Frame = 0;

		m_oSamples.free();
		m_oSamples.iSampleCount = m_oFrameHeader.iBlockSize();
		m_oSamples.alloc();

		FLAC::SUBFRAME_HEADER oSubframeHeader;

		for (uint8_t iChannel = 0; iChannel < m_oSamples.iChannelCount; ++iChannel)
		{
			if (!oSubframeHeader.read(*m_pInput))
				return false;

			const auto iSubframeType = oSubframeHeader.iSubframeType();

			if (iSubframeType == 0b000000) // SUBFRAME_CONSTANT
			{
				uint8_t iData[4]{};
				const uint8_t iBytesPerSample = m_oStreamInfo.iBitsPerSample() / 8;
				m_pInput->read(iData, iBytesPerSample);

				uint32_t iVal = 0;
				for (uint8_t j = 0; j < iBytesPerSample; ++j)
				{
					iVal |= uint32_t(iData[j]) << (j * 8);
				}

				if (oSubframeHeader.iWastedBitsPerSample() > 0)
					iVal >>= oSubframeHeader.iWastedBitsPerSample();


				// fill the sample buffer with the constant value
				for (size_t iSampleID = 0; iSampleID < m_oSamples.iSampleCount; ++iSampleID)
				{
					for (uint8_t iByteID = 0; iByteID < iBytesPerSample; ++iByteID)
					{
						m_oSamples.val.p8[iChannel][iSampleID * iBytesPerSample + iByteID] =
							iVal >> ((sizeof(iVal) - 1 - iByteID) * 8);
					}
				}

				// todo: check if correct
			}
			else if (iSubframeType == 0b000001) // SUBFRAME_VERBATIM
			{
				uint8_t iData[4]{};
				for (size_t iSampleID = 0; iSampleID < m_oSamples.iSampleCount; ++iSampleID)
				{

				}
			}
			else if ((iSubframeType & 0b111000) == 0b001000) // SUBFRAME_FIXED
			{
				const uint8_t iOrder = iSubframeType & 0x07;
				if (iOrder > 4)
					return false; // reserved value

				// ToDo: SUBFRAME_FIXED
			}
			else if (iSubframeType & 0b100000) // SUBFRAME_LPC
			{
				const uint8_t iOrder = iSubframeType & 0x1F;

				// ToDo: SUBFRAME_LPC
			}

			else // unknown subframe type --> zero memory
				memset(m_oSamples.val.p8, 0,
					(size_t)m_oSamples.iChannelCount * (m_oSamples.iBitsPerSample / 8));
		}

		FLAC::FRAME_FOOTER oFrameFooter;
		if (!oFrameFooter.read(*m_pInput))
			return false;

		return true;
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