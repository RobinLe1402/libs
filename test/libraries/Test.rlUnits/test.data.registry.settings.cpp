#include <rl/data.registry.settings.hpp>


bool UnitTest_data_registry_settings()
{
	rl::AppSettings oApp;
	rl::GlobalSettings oGlobal;


	std::printf("TRYING TO LOAD SETTINGS\n");

	if (!oApp.load())
		std::printf("FAIL:    App settings couldn't be loaded.\n");
	else
		std::printf("SUCCESS: App settings loaded.\n");

	if (!oGlobal.load())
		std::printf("FAIL:    Global settings couldn't be loaded.\n");
	else
		std::printf("SUCCESS: Global settings loaded.\n");


	std::printf("\n\nTRYING TO SAVE SETTINGS\n");

	oApp.rootKey().values()[L"!TestValue"] = L"Hello app registry! xD";
	if (!oApp.rootKey().values().contains(L"RunCount"))
		oApp.rootKey().values()[L"RunCount"] = (QWORD)1;
	else
		oApp.rootKey().values()[L"RunCount"] = oApp.rootKey().values()[L"RunCount"].asQWORD() + 1;
	if (!oApp.save())
		std::printf("FAIL:    App settings couldn't be saved.\n");
	else
		std::printf("SUCCESS: App settings saved.\n");

	oGlobal.rootKey().values()[L"!TestValue"] = L"Hello global registry! :)";
	if (!oGlobal.save())
		std::printf("FAIL:    Global settings couldn't be saved.\n");
	else
		std::printf("SUCCESS: Global settings saved.\n");

	std::printf("\n");
	return true;
}
