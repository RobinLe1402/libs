/***************************************************************************************************
 FILE:	FileReader.hpp
 CPP:	FileReader.cpp
 DESCR:	Implementation of IDataReader for files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_ROBINLEFLAC_FILEREADER
#define ROBINLE_LIB_ROBINLEFLAC_FILEREADER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <headername>
// forward declarations


#define LIBRARY_EXPORTS
#include "rl/dll/RobinLeFLAC.hpp"

#include <fstream>



//==================================================================================================
// DECLARATION
namespace rl
{
	using namespace RobinLeFLACDLL;
	


	class FileReader : public IDataReader
	{
	public: // methods

		FileReader(const wchar_t* szFileName);

		bool getData(size_t& cb, uint8_t* dest) override;
		inline bool eof() override { return m_oInput.eof(); }
		inline size_t size() override { return m_iFilesize; }
		inline size_t tellPos() override { return m_oInput.tellg(); }
		bool seekPos(size_t offset) override;

	private: // variables

		std::basic_ifstream<uint8_t> m_oInput;
		size_t m_iFilesize = 0;

	};
	
}





// #undef foward declared definitions

#endif // ROBINLE_LIB_ROBINLEFLAC_FILEREADER