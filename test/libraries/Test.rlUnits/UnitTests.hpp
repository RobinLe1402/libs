#pragma once
#ifndef ROBINLE_TEST_RLUNITS__UNITTEST
#define ROBINLE_TEST_RLUNITS__UNITTEST



#include <functional>
#include <string>
#include <vector>

struct UnitTest
{
	std::string name;
	std::function<bool()> func;
};

class UnitTestCollection
{
public: // static methods

	static const auto &UnitTests() { return s_oUnitTests; }


private: // static variables

	static std::vector<UnitTest> s_oUnitTests;


public: // methods
	UnitTestCollection() = delete; // --> singleton

};




#endif // ROBINLE_TEST_RLUNITS__UNITTEST