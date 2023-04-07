/***************************************************************************************************
 FILE:	elevate.hpp
 CPP:	elevate.cpp
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
	public: // methods

		RunAsAdmin();
		~RunAsAdmin() = default;

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
		/// <returns>
		/// If a new process with administrator privileges got successfully created, the function
		/// returns <c>true</c>. Otherwise, <c>false</c> is returned.<para />
		/// Note: If the application is already running with administrator privileges, the function
		/// will also return <c>false</c> as no new process got created. Please check the
		/// <c>isAdmin()</c> method to be sure.
		/// </returns>
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
		/// <returns></returns>
		ElevationResult execute(const wchar_t *szExe, const wchar_t *szArgs = L"",
			const wchar_t *szCurrentDir =  nullptr);


	private: // variables

		bool m_bAdmin = false;

	};

	/// <summary>
	/// Elevate this application.
	/// </summary>
	/// <param name="bExitIfFailed">Should the current instance exit if the function fails?</param>
	ElevationResult ElevateSelf(bool bExitIfFailed, bool bErrorDialog = true);

}





#endif // ROBINLE_RUNASADMIN