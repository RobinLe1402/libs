#pragma once
#ifndef ROBINLE_APPLAUNCHER_EXECUTEAPP
#define ROBINLE_APPLAUNCHER_EXECUTEAPP





#include <string>



void UpdateLauncher();

void UpdateLibraries();

void UpdateApp(const std::wstring &sAppName);

void ExecuteApp(const std::wstring &sAppName);

void ShowAppList();





#endif // ROBINLE_APPLAUNCHER_EXECUTEAPP