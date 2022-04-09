#pragma once
#ifndef ROBINLE_DOCGEN_MAIN
#define ROBINLE_DOCGEN_MAIN





int wmain(int argc, wchar_t* argv[]);

void PrintUsage();

/// <summary>
/// Outputs " [Y/N] ", waits until user inputs "Y"/"y"/"N"/"n",
/// then prints "Y" or "N" and a linebreak
/// </summary>
/// <returns>Did the user input "y"/"Y"?</returns>
bool GetUserConfirmation();





#endif // ROBINLE_DOCGEN_MAIN