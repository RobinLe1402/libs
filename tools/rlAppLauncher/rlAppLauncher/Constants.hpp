#pragma once
#ifndef ROBINLE_APPLAUNCHER_CONSTANTS
#define ROBINLE_APPLAUNCHER_CONSTANTS



constexpr wchar_t szMUTEXNAME[] = L"MUTEX_ROBINLE_LAUNCHER";

constexpr wchar_t szARG_APP[]                = L"App";                     // name of app to launch
constexpr wchar_t szARG_SKIPLAUNCHERUPDATE[] = L"SkipLauncherUpdateCheck"; // skip launcher updates
constexpr wchar_t szARG_SKIPLIBUPDATE[]      = L"SkipLibUpdateCheck";      // skip DLL updates
constexpr wchar_t szARG_SKIPAPPUPDATE[]      = L"SkipAppUpdateCheck";      // skip app updates
constexpr wchar_t szARG_OFFLINEMODE[]        = L"OfflineMode";             // skip all updates
constexpr wchar_t szARG_SILENTMODE[]         = L"NoAppList";               // don't show anything



#endif // ROBINLE_APPLAUNCHER_CONSTANTS