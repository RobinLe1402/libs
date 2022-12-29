#include "../../!Include/rlFNTPROJ.hpp"
namespace rlFont = rl::rlFNTPROJ;

int main(int argc, char *argv[])
{
	constexpr wchar_t szTESTFILE[] = LR"(E:\[TempDel]\Test.rlFNTPROJ)";
	constexpr char32_t chTest = 'A';



	// File save test
	{
		rlFont::Project oProject;
		auto &oFam = oProject.getFontFamily();

		rlFont::ProjectMeta oMeta;
		oMeta.sFoundry = L"RobinLe";
		oMeta.sDesigner = L"Robin Lemanska";
		oMeta.sComments = L"Test file";
		oProject.setMetadata(oMeta);

		rlFont::FontFamilyMeta oFFMeta;
		oFFMeta.sName = L"TestFont";
		oFFMeta.sCopyright = L"(c) 2022 RobinLe";
		oFam.setMetadata(oFFMeta);

		{
			rlFont::Font oFont;
			rlFont::FontMeta oFontMeta;
			oFontMeta.sName = L"Regular";
			oFont.setMetadata(oFontMeta);
			oFam.getFonts().push_back(std::move(oFont));
		}
		auto &oFont = oFam.getFonts()[0];

		{
			rlFont::FontFace oFontFace;
			oFont.getFaces().push_back(std::move(oFontFace));
		}
		auto &oFontFace = oFont.getFaces()[0];

		auto &oChar = oFontFace.getCharacters()[chTest];
		oChar.create(8, 8);

		const char szChar[] =
			"   XX   "
			"  XXXX  "
			" XX  XX "
			"XX    XX"
			"XXXXXXXX"
			"XX    XX"
			"XX    XX"
			"XX    XX";

		for (uint8_t iX = 0; iX < 8; ++iX)
		{
			for (uint8_t iY = 0; iY < 8; ++iY)
			{
				if (szChar[iY * 8 + iX] == 'X')
					oChar.setPixel(iX, iY, 1);
			}
		}

		if (!oProject.saveToFile(szTESTFILE))
		{
			printf("Error saving test file to \"%ls\".\n", szTESTFILE);
			return 1;
		}
		else
			printf("Successfully created test file \"%ls\".\n", szTESTFILE);
	}


	// File load test
	{
		rlFont::Project oProject;
		if (!oProject.loadFromFile(szTESTFILE))
		{
			printf("Couldn't load file!\n");
			return 1;
		}
		else
		{
			printf("Successfully loaded file.\n");
			printf("Character '%c':\n", (char)chTest);
			auto &oChar =
				oProject.getFontFamily().getFonts()[0].getFaces()[0].getCharacters().at(chTest);



			printf("  +");
			for (unsigned iX = 0; iX < oChar.getWidth(); ++iX)
				printf("--");
			printf("+\n");

			for (unsigned iY = 0; iY < oChar.getHeight(); ++iY)
			{
				printf("  |");
				for (unsigned iX = 0; iX < oChar.getWidth(); ++iX)
				{
					const char c1 = (oChar.getPixel(iX, iY) ? '[' : '.');
					const char c2 = (c1 == '[' ? ']' : ' ');
					printf("%c%c", c1, c2);
				}
				printf("|\n");
			}

			printf("  +");
			for (unsigned iX = 0; iX < oChar.getWidth(); ++iX)
				printf("--");
			printf("+\n");
		}
	}



	return 0;
}
