#include <rl/graphics.fonts.bitmap.reader.hpp>
#include <rl-dev/graphics.fonts.bitmap.creator.hpp>
#include <stdio.h>

int main(int argc, char* argv[])
{
	rl::FontFaceClass ffc;

	if (!ffc.loadFromFile(LR"(E:\[TempDel]\test.rlFNT)"))
		printf("Failure\n");
	else
		printf("Success\n");

	return 0;
}
