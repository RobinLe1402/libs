#pragma once
#ifndef ROBINLE_UNITLIB_COMPILETOOLS
#define ROBINLE_UNITLIB_COMPILETOOLS





#include <cstdint>
#include <map>
#include <string>
#include <vector>



namespace rl
{

	namespace CompilerPlatform
	{
		constexpr wchar_t Windows32Bit[] = L"Win32";
		constexpr wchar_t Windows64Bit[] = L"x64";
	}

	namespace CompilerConfiguration
	{
		constexpr wchar_t Debug[]   = L"Debug";
		constexpr wchar_t Release[] = L"Release";
	}



	enum class CompileMessageType
	{
		Warning,
		Error
	};

	struct CompileMessage
	{
		CompileMessageType eType{};

		size_t iLine   = 0;
		size_t iColumn = 0;

		std::wstring sFile;
		std::wstring sCode;
		std::wstring sDescription;
	};

	class ProjectCompileResult final
	{
	public: // methods

		auto &items() { return m_oMessages; }
		auto &items() const { return m_oMessages; }

		size_t warningCount() const;
		size_t errorCount() const;

		bool warnings() const;
		bool errors() const;


	private: // variables

		std::vector<CompileMessage> m_oMessages;

	};

	class CompileResult final
	{
	public: // methods

		CompileResult() = default;
		CompileResult(const char *szErrorMessage);
		CompileResult(const CompileResult &) = default;
		CompileResult(CompileResult &&) = default;
		~CompileResult() = default;

		CompileResult &operator=(const CompileResult &) = default;
		CompileResult &operator=(CompileResult &&) = default;

		operator bool() const { return m_bValid; }
		const auto &errorMessage() const { return m_sErrorMessage; }

		auto &projects() { return m_oProjects; }
		auto &projects() const { return m_oProjects; }


	private: // variables

		bool m_bValid = true;
		std::string m_sErrorMessage;
		std::map<std::wstring, ProjectCompileResult> m_oProjects;

	};





	class Compiler
	{
	public: // static methods

		static const std::wstring &VSPath();
		static const std::wstring &Path();

		static CompileResult Compile(const wchar_t *szProject,
			const wchar_t *szPlatform = CompilerPlatform::Windows64Bit,
			const wchar_t *szConfiguration = CompilerConfiguration::Release);


	private: // static variables

		static const std::wstring s_LogPath;

	};



}





#endif // ROBINLE_UNITLIB_COMPILETOOLS