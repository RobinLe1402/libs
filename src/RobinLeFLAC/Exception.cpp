#include "include/Exception.hpp"

#include <Windows.h>





namespace rl
{

	void FLACExceptionMessage(const FLACException& exception)
	{
		std::string sText;
		if (exception.signature()[0] != 0)
		{
			sText = std::string("[") + exception.signature() + "] ";
		}
		if (exception.what()[0] == 0)
			sText += "An exception was thrown";
		else
			sText += exception.what();

		MessageBoxA(NULL, sText.c_str(), "Exception", MB_ICONERROR | MB_APPLMODAL);
	}


	
	/***********************************************************************************************
	 class FLACException
	***********************************************************************************************/
	
	//==============================================================================================
	// STATIC VARIABLES
	
	// static variables
	
	
	
	
	
	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS
	
	FLACException::FLACException(const char* szFunctionSignature, const char* szWhat) :
		m_sFunctionSignature(szFunctionSignature), m_sText(szWhat) { }
	
	
	
	
	
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