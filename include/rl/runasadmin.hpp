/***************************************************************************************************
 FILE:	runasadmin.hpp
 CPP:	runasadmin.cpp
 DESCR:	Elevate an application.
***************************************************************************************************/

#pragma once
#ifndef ROBINLE_RUNASADMIN
#define ROBINLE_RUNASADMIN





//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// The result of an attempted elevation.
	/// </summary>
	enum class ElevationResult
	{
		Success,
		AlreadyElevated,
		Denied,
		Failed
	};



	/// <summary>
	/// A class for running an application with administrator privileges.
	/// </summary>
	class RunAsAdmin final
	{
	public: // static methods

		static RunAsAdmin &Instance() { return s_oInstance; }


	private: // static variables

		static RunAsAdmin s_oInstance;


	public: // methods

		bool isAdmin() const { return m_bAdmin; }

		/// <summary>
		/// Create a new process for this application, but with administrator privileges.
		/// </summary>
		/// <param name="szArgs">
		/// The parameters to be used.<para />
		/// Note these special cases:<para />
		/// * Empty string: No parameters are used.<para />
		/// * <c>nullptr</c>: The parameters of the current process will be used.
		/// </param>
		/// <param name="szCurrentDir">
		/// The directory in which to launch the application.<para />
		/// If this parameter is <c>nullptr</c> or an empty string, the current directory of the
		/// current process will be used.
		/// </param>
		ElevationResult elevateSelf(const wchar_t *szArgs = nullptr,
			const wchar_t *szCurrentDir = nullptr);

		/// <summary>
		/// Execute an application with administrator privileges.
		/// </summary>
		/// <param name="szExe">The EXE file to execute.</param>
		/// <param name="szArgs">
		/// The parameters to be used.<para />
		/// Note these special cases:<para />
		/// * Empty string: No parameters are used.<para />
		/// * <c>nullptr</c>: The parameters of the current process will be used.
		/// </param>
		/// <param name="szCurrentDir">
		/// The directory in which to launch the application.<para />
		/// If this parameter is <c>nullptr</c> or an empty string, the current directory of the
		/// current process will be used.
		/// </param>
		ElevationResult execute(const wchar_t *szExe, const wchar_t *szArgs = L"",
			const wchar_t *szCurrentDir =  nullptr);


	private: // methods
		
		RunAsAdmin();
		~RunAsAdmin() = default;


	private: // variables

		bool m_bAdmin = false;

	};

	/// <summary>
	/// Elevate this application.
	/// </summary>
	/// <param name="bExitIfFailed">Should the current instance exit if the function fails?</param>
	ElevationResult ElevateSelf(bool bExitIfFailed, bool bErrorDialog = true);

	/// <summary>
	/// Check if this application is already elevated.
	/// </summary>
	/// <returns></returns>
	bool IsElevated();

}





#endif // ROBINLE_RUNASADMIN