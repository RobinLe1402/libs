/***************************************************************************************************
 FILE:	template.hpp
 CPP:	template.cpp
 DESCR:	Template for new source files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_ROBINLEFLAC_EXCEPTION
#define ROBINLE_LIB_ROBINLEFLAC_EXCEPTION





//==================================================================================================
// INCLUDES

#include <exception>
#include <string>



//==================================================================================================
// DECLARATION
namespace rl
{
	
	class FLACException : public std::exception
	{
	public: // methods

		explicit FLACException(const char* szFunctionSignature, const char* szWhat);
		virtual ~FLACException() throw() {}

		virtual const char* what() const override { return m_sText.c_str(); }
		const char* signature() const { return m_sFunctionSignature.c_str(); }


	private: // variables

		std::string m_sFunctionSignature;
		std::string m_sText;

	};

	/// <summary>
	/// Show a message dialog based on a thrown <c>rl::FLACException</c>
	/// </summary>
	void FLACExceptionMessage(const FLACException& exception);
	
}





// #undef foward declared definitions

#endif // ROBINLE_LIB_ROBINLEFLAC_EXCEPTION