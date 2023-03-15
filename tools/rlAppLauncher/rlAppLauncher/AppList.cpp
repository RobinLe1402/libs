#include "MainActions.hpp"
#include "OnlineInterface.hpp"
#include "XMLParser.hpp"
#include "Constants.IndexXML.hpp"

#include <cstdio>
#include <array>
#include <regex>

namespace
{

	/// <summary>A single RobinLe App.</summary>
	struct App
	{
		std::string             sName;
		std::array<unsigned, 4> oVersion{};
		std::string             sSummary;
		std::string             sDescription;
	};
	
	/// <summary>
	/// A list of RobinLe Apps loaded from the internet.
	/// </summary>
	class AppList
	{
	public: // methods

		bool loadFromInternet()
		{
			m_bTriedLoading = true;
			m_bLoaded = false;

			m_oApps.clear();

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
				
				oApp.sName = oProgNode.attributes().at(szNODE_PROGRAM_ATTRIB_NAME);

				const auto &sVersion = oProgNode.attributes().at(szNODE_PROGRAM_ATTRIB_VERSION);
				std::regex oRegEx(R"REGEX(^(\d+)\.(\d+)\.(\d+)\.(\d+)$)REGEX");
				std::smatch oMatch;
				if (!std::regex_match(sVersion, oMatch, oRegEx))
					return false;
				oApp.oVersion[0] = std::stoi(oMatch[1]);
				oApp.oVersion[1] = std::stoi(oMatch[2]);
				oApp.oVersion[2] = std::stoi(oMatch[3]);
				oApp.oVersion[3] = std::stoi(oMatch[4]);

				for (auto &oSubNode : oProgNode.children())
				{
					if (oSubNode.name() == szNODE_SUMMARY)
						oApp.sSummary = oSubNode.textValue();
					else if (oSubNode.name() == szNODE_DESCRIPTION)
						oApp.sDescription = oSubNode.textValue();
				}

				m_oApps.push_back(oApp);

			}

			m_bLoaded = true;
			return true;
		}

		const auto &apps() const { return m_oApps; }

		auto loadingAttempted() const { return m_bTriedLoading; }
		auto loadingSucceeded() const { return m_bLoaded; }


	private: // variables

		std::vector<App> m_oApps;
		bool m_bTriedLoading = false;
		bool m_bLoaded = false;

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
		std::printf("Title:   %s\n", oApp.sName.c_str());
		std::printf("Summary: %s\n", oApp.sSummary.c_str());
		std::printf("Description:\n");
		std::printf("%s\n\n", oApp.sDescription.c_str());
	}
	
	system("PAUSE");

	return;
}