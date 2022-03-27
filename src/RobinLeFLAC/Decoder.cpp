#define LIBRARY_EXPORTS
#include "rl/dll/RobinLeFLAC.hpp"

#include "include/Assert.hpp"
#include "include/BitReader.hpp"
#include "include/FileReader.hpp"

namespace rl
{
	using namespace RobinLeFLACDLL;
	using private_::BitReader;

	
	
	/***********************************************************************************************
	class IDataReader
	***********************************************************************************************/

	bool IDataReader::seekPos(int64_t offset, DataPosSeekMethod method)
	{
		using DataPosSeekMethod::Begin;
		using DataPosSeekMethod::End;
		using DataPosSeekMethod::Current;

		FLAC__ASSERT(method == Begin || method == End || method == Current);

		const auto len = size();
		const auto pos = tellPos();
		switch (method)
		{
		case Begin:
			if (offset < 0)
				return false;
			return seekPos(offset);
		case End:
			if (offset > 0 || (size_t(-offset) > len))
				return false;
			return seekPos(len - size_t(-offset));
		case Current:
			if (offset == 0)
				return true;

			if (offset < 0) // jump back
			{
				if (size_t(-offset) > pos)
					return false;
				return seekPos(pos - size_t(-offset));
			}
			else // jump forward
			{
				if (len - pos < size_t(offset))
					return false;

				return seekPos(pos + offset);
			}
		}
	}





	/***********************************************************************************************
	class FLACDecoder
	***********************************************************************************************/

	FLACDecoder::FLACDecoder() : m_pBitReader(nullptr), m_pReader(nullptr) { }

	FLACDecoder::FLACDecoder(const wchar_t* szFileName) :
		m_pReader(new FileReader(szFileName)),
		m_pBitReader(new BitReader(m_pReader, true))
	{
		if (!initialize())
			close();
	}

	FLACDecoder::FLACDecoder(IDataReader* pReader) :
		m_pReader(pReader),
		m_pBitReader(new BitReader(m_pReader, false))
	{
		if (!initialize())
			close();
	}

	FLACDecoder::~FLACDecoder() { delete m_pBitReader; }

	bool FLACDecoder::open(const wchar_t* szFileName)
	{
		close();

		m_pReader = new FileReader(szFileName);
		if (m_pReader->eof())
		{
			delete m_pReader;
			m_pReader = nullptr;
			return false;
		}

		m_pBitReader = new BitReader(m_pReader, true);

		if (!initialize())
		{
			close();
			return false;
		}

		return true;
	}

	bool FLACDecoder::open(IDataReader* pReader)
	{
		FLAC__ASSERT(pReader != nullptr);

		if (pReader->eof())
			return false;

		m_pReader = pReader;
		m_pBitReader = new BitReader(m_pReader, false);

		if (!initialize())
		{
			close();
			return false;
		}

		return true;
	}

	void FLACDecoder::close()
	{
		delete m_pBitReader; // also deletes m_pReader if it's owned by the BitReader
		m_pBitReader = nullptr;
		m_pReader = nullptr;
	}

}
