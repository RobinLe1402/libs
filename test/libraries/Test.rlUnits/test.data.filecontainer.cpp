#include "tests.hpp"

#include <rl/data.filecontainer.hpp>

#include <Windows.h>



bool UnitTest_data_filecontainer()
{
	constexpr wchar_t szTestFolder[] =
		LR"(%GitHub_rl_libs%\test\libraries\Test.rlUnits\data.filecontainer)";
	constexpr wchar_t szTestFile[] =
		LR"(E:\[TempDel]\test.rlPAK)";

	wchar_t szTestFolder_Expanded[MAX_PATH + 1]{};
	ExpandEnvironmentStringsW(szTestFolder, szTestFolder_Expanded, MAX_PATH + 1);

	rl::FileContainer fc;
	fc.rootDir().addDirectoryContents(szTestFolder_Expanded, true);

	// ASCII version ===============================================================================
	if (!fc.save(szTestFile, false))
	{
		std::printf("ERROR: Failed to save ASCII rlPAK file.\n");
		return false;
	}
	else
		std::printf("SUCCESS: Saved ASCII rlPAK file.\n");
	fc.rootDir().clear();
	if (!fc.load(szTestFile))
	{
		std::printf("ERROR: Failed to load ASCII rlPAK file.\n");
		return false;
	}
	else
		std::printf("SUCCESS: Loaded ASCII rlPAK file.\n");


	// UTF-16 version ==============================================================================
	fc.clear();
	fc.rootDir().addDirectoryContents(szTestFolder_Expanded, true);

	if (!fc.save(szTestFile, true))
	{
		std::printf("ERROR: Failed to save Unicode rlPAK file.\n");
		return false;
	}
	else
		std::printf("SUCCESS: Saved Unicode rlPAK file.\n");
	fc.clear();
	if (!fc.load(szTestFile))
	{
		std::printf("ERROR: Failed to load Unicode rlPAK file.\n");
		return false;
	}
	else
		std::printf("SUCCESS: Loaded Unicode rlPAK file.\n");



	if (fc.rootDir().extractToDirectory(LR"(E:\[TempDel]\rlPAK extracted)"))
		std::printf("SUCCESS: Extracted files.\n");
	else
		std::printf("ERROR: Couldn't extract all files.\n");


	return true;
}
