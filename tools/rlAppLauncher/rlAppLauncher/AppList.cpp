#include "MainActions.hpp"
#include "OnlineInterface.hpp"
#include "XMLParser.hpp"
#include "Constants.IndexXML.hpp"

#include <cstdio>
#include <array>
#include <regex>

namespace
{

	class App
	{
	public: // methods

		auto &name() { return m_sName; }
		const auto &name() const { return m_sName; }

		auto &version() { return m_oVersion; }
		const auto &version() const { return m_oVersion; }

		auto &summary() { return m_sSummary; }
		const auto &summary() const { return m_sSummary; }

		auto &description() { return m_sDescription; }
		const auto &description() const { return m_sDescription; }


	private: // variables

		std::string m_sName;
		std::array<unsigned, 4> m_oVersion{};
		std::string m_sSummary;
		std::string m_sDescription;
	};
	
	class AppList
	{
	public: // methods

		bool loadFromInternet()
		{
			auto oOnlineList = OnlineInterface::DownloadFileToMemory(
				L"https://download.robinle.de/_hidden/software/index.xml");

			if (!oOnlineList)
				return false;



			// "convert" downloaded data into zero-terminated string
			const size_t iBufLen = oOnlineList.size();
			auto up_szXML        = std::make_unique<char[]>(iBufLen + 1);
			memcpy_s(up_szXML.get(), iBufLen, oOnlineList.data(), oOnlineList.size());
			up_szXML[iBufLen]    = 0; // add terminating zero

			oOnlineList.clear();



			// parse XML
			XMLDoc oDoc;
			if (!oDoc.loadFromString(up_szXML.get()))
				return false;
			oDoc.resolveTexts();

			up_szXML = nullptr;



			if (oDoc.rootNode().name() != szROOTNODE_NAME)
				return false; // invalid root node name

			if (oDoc.rootNode().attributes().at(szROOTNODE_ATTRIB_VERSION) != "1.0.0.0")
				return false; // unknown format version

			for (auto &oProgNode : oDoc.rootNode().children())
			{
				if (oProgNode.name() != szNODE_PROGRAM)
					continue; // unknown node

				App oApp;
				
				oApp.name() = oProgNode.attributes().at(szNODE_PROGRAM_ATTRIB_NAME);

				const auto &sVersion = oProgNode.attributes().at(szNODE_PROGRAM_ATTRIB_VERSION);
				std::regex oRegEx(R"REGEX(^(\d+)\.(\d+)\.(\d+)\.(\d+)$)REGEX");
				std::smatch oMatch;
				if (!std::regex_match(sVersion, oMatch, oRegEx))
					return false;
				oApp.version()[0] = std::stoi(oMatch[1]);
				oApp.version()[1] = std::stoi(oMatch[2]);
				oApp.version()[2] = std::stoi(oMatch[3]);
				oApp.version()[3] = std::stoi(oMatch[4]);

				for (auto &oSubNode : oProgNode.children())
				{
					if (oSubNode.name() == szNODE_SUMMARY)
						oApp.summary() = oSubNode.textValue();
					else if (oSubNode.name() == szNODE_DESCRIPTION)
						oApp.description() = oSubNode.textValue();
				}

				m_oApps.push_back(oApp);

			}

			return true;
		}

		const auto &apps() const { return m_oApps; }


	private: // variables

		std::vector<App> m_oApps;

	};

}



void ShowAppList()
{
	std::printf("=== RobinLe App List ===\n");
	// TODO

	AppList oAppList;
	if (!oAppList.loadFromInternet())
	{
		std::printf("Couldn't load app list from the internet.\n");
		system("PAUSE");
		return;
	}

	for (const auto &oApp : oAppList.apps())
	{
		std::printf("Title:   %s\n", oApp.name().c_str());
		std::printf("Summary: %s\n", oApp.summary().c_str());
		std::printf("Description:\n");
		std::printf("%s\n\n", oApp.description().c_str());
	}
	
	system("PAUSE");

	return;
}