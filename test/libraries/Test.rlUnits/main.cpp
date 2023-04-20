// project
#include "UnitTests.hpp"
#include "ConsoleIO.hpp"

// Win32
#include <Windows.h>



int main(int argc, char *argv[])
{
	auto &con = Console::Instance();

	while (true)
	{
		con.clear();
		SetConsoleTitleA("Unit tests");

		con.write("=====| UNIT TESTS |=====\n\n");
		for (size_t i = 0; i < UnitTestCollection::UnitTests().size(); ++i)
		{
			auto &oUnitTest = UnitTestCollection::UnitTests()[i];
			con.write("[%zu] %s\n", i + 1, oUnitTest.name.c_str());
		}
		con.write("\n[0] Exit\n");

		con.write("\nPlease select a test to run: ");
		uint64_t iTest;
		if (!con.readUInt(iTest, UnitTestCollection::UnitTests().size()))
		{
			con.bell();
			continue;
		}

		if (iTest == 0)
			return 0;



		auto &oUnitTest = UnitTestCollection::UnitTests()[iTest - 1];

		SetConsoleTitleA(((std::string)"Unit test: " + oUnitTest.name).c_str());
		con.clear();
		if (oUnitTest.func())
			con.write("Test passed!\n");
		else
			con.write("Test failed!\n");

		con.pause();
	}

	return 0;
}
