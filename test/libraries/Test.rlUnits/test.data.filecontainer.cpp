#include "tests.hpp"

#include <rl/data.filecontainer.hpp>

#include <Windows.h>



bool UnitTest_data_filecontainer()
{
	constexpr wchar_t szTestFolder[] =
		LR"(%GitHub_rl_libs%\test\libraries\Test.rlUnits\data.filecontainer)";

	wchar_t szTestFolder_Expanded[MAX_PATH + 1]{};
	ExpandEnvironmentStringsW(szTestFolder, szTestFolder_Expanded, MAX_PATH + 1);

	rl::FileContainer fc;
	fc.data().addDirectoryContents(szTestFolder_Expanded, true);

	return true;
}
