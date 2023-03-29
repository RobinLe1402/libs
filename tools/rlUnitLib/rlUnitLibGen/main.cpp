#define WIN32_LEAN_AND_MEAN

#include "CompileTools.hpp"
#include "SourceGenerator.hpp"

#include <rl/commandline.hpp>
#include <rl/text.fileio.hpp>

#include <stdio.h>
#include <Windows.h>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

bool GenerateCode();
bool Compile();

int main(int argc, char *argv[])
{
	constexpr wchar_t szArg_GenSource[] = L"CODE";
	constexpr wchar_t szArg_Compile[]   = L"COMPILE";

	auto &cmd = rl::Commandline::Instance();

	const auto &itCode = cmd.findFlag(szArg_GenSource, false);
	if (itCode != cmd.end() && !GenerateCode())
		return 1;

	const auto &itCompile = cmd.findFlag(szArg_Compile, false);
	if (itCompile != cmd.end() && !Compile())
		return 1;

	return 0;
}

namespace
{
	constexpr wchar_t szEnvVar_Git[] = L"GitHub_rl_Libs";
	constexpr wchar_t szProjectFileRel[] = LR"(tools\rlUnitLib\rlUnitLib\rlUnitLib.vcxproj)";
	
	std::wstring sGitRootDir;
	std::wstring sRLUnitLib_VCXPROJ;

	bool GetPaths()
	{
		if (sGitRootDir.empty())
		{
			DWORD dwLength = GetEnvironmentVariableW(szEnvVar_Git, NULL, 0);
			if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
			{
				std::printf("Error: Environment variable \"%%%ls%%\" not found.\n", szEnvVar_Git);
				return false;
			}

			sGitRootDir.resize((size_t)dwLength - 1);
			GetEnvironmentVariableW(szEnvVar_Git, sGitRootDir.data(), dwLength);
			sRLUnitLib_VCXPROJ = sGitRootDir + szProjectFileRel;
		}

		return true;
	}
}


bool GenerateCode()
{
	if (!GetPaths())
		return false;

	std::printf("Generating source code ...\n\n");
	// get all unit CPP files (in "src" root dir)
	std::vector<std::wstring> oCPPFiles;
	{
		WIN32_FIND_DATAW fd;
		HANDLE hFind = FindFirstFileW((sGitRootDir + LR"(src\*.cpp)").c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			std::wstring sFilePrefix = std::wstring(L"$(") + szEnvVar_Git + L")src\\";

			do
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue; // skip directories

				oCPPFiles.push_back(sFilePrefix + fd.cFileName);
				std::printf("Including \"%ls\".\n", fd.cFileName);
			} while (FindNextFileW(hFind, &fd));
			FindClose(hFind);
		}
	}

	const std::wstring sFiltersFile = sRLUnitLib_VCXPROJ + L".filters";

	const std::wstring *oFiles[] = { &sRLUnitLib_VCXPROJ, &sFiltersFile };



	SourceFile sf;
	for (const auto pString : oFiles)
	{
		const auto &s = *pString;

		if (!sf.open(s.c_str()))
		{
			std::printf("Error: Couldn't read file \"%ls\"\n", s.c_str());
			return false;
		}
		sf.units().append_range(oCPPFiles);
		if (!sf.save())
		{
			std::printf("Error: Couldn't write file \"%ls\"\n", s.c_str());
			return false;
		}
	}


	std::printf("Successfully generated source code.\n\n\n");
	return true;
}

bool Compile()
{
	if (!GetPaths())
		return false;


	const wchar_t *szConfig[] =
	{
		rl::CompilerConfiguration::Debug,
		rl::CompilerConfiguration::Release
	};
	const wchar_t *szPlatform[] =
	{
		rl::CompilerPlatform::Windows32Bit,
		rl::CompilerPlatform::Windows64Bit
	};

	for (auto szC : szConfig)
	{
		for (auto szP : szPlatform)
		{
			std::printf("Building %ls version for %ls... ", szC, szP);

			const auto result = rl::Compiler::Compile(sRLUnitLib_VCXPROJ.c_str(), szP, szC);

			if (!result)
			{
				std::printf("Error: %s\n", result.errorMessage().c_str());
				continue;
			}
			else if (result.projects().size() > 0)
			{
				auto &resultProject = result.projects().begin()->second;

				std::printf("%zu warnings, %zu errors.\n",
					resultProject.warningCount(), resultProject.errorCount());
			}
			else
				printf("Succeeded.\n");
		}
	}

	std::printf("Finished compiling library.\n\n\n");
	return true;
}