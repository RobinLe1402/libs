#include "include/FileReader.hpp"





namespace rl
{
	
	/***********************************************************************************************
	 class FileReader
	***********************************************************************************************/
	
	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS
	
	FileReader::FileReader(const wchar_t* szFileName) : m_oInput(szFileName, std::ios::binary)
	{
		if (!m_oInput)
			return;

		m_oInput.seekg(0, std::ios::end);
		m_iFilesize = m_oInput.tellg();
		m_oInput.seekg(0);
	}
	
	
	
	
	
	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS
	
	bool FileReader::getData(size_t& cb, uint8_t* dest)
	{
		if (m_oInput.eof())
			return false;

		cb = m_oInput.readsome(dest, cb);
		return true;
	}

	bool FileReader::seekPos(size_t offset)
	{
		const auto pos = m_oInput.tellg();
		m_oInput.clear(); // might be eof()

		if (!m_oInput.seekg(offset))
		{
			m_oInput.clear();
			m_oInput.seekg(pos);
			return false;
		}

		return true;
	}
	
}