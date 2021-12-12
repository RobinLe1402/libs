//#include <rl/graphics.fonts.bitmap.reader.hpp>
#include <rl-dev/graphics.fonts.bitmap.creator.hpp>

#include <stdio.h>


void ConvertChar(rl::BitmapFontChar& chr, const uint8_t* pGraphics)
{
	// function is written for 8x8 characters, see below

	for (uint8_t iY = 0; iY < 8; ++iY)
	{
		const uint8_t iRowData = pGraphics[iY];
		for (uint8_t iX = 0; iX < 8; ++iX)
		{
			if ((iRowData >> (7 - iX)) & 1)
				chr.setPixel(iX, iY, 1);
		}
	}
}

int main(int argc, char* argv[])
{
	const uint8_t dataEQUALS[] =
	{
		0b00000000,
		0b00000000,
		0b11111110,
		0b00000000,
		0b11111110,
		0b00000000,
		0b00000000,
		0b00000000
	};

	const uint8_t dataEXCLAMATION[] =
	{
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00000000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataQUESTIONMARK[] =
	{
		0b01111100,
		0b10000010,
		0b00000010,
		0b00011100,
		0b00010000,
		0b00000000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataSPACE[] =
	{
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	};

	const uint8_t dataUNDEFINED[] =
	{
		0b00010000,
		0b00101000,
		0b01110100,
		0b11101110,
		0b01111100,
		0b00101000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataA[] =
	{
		0b01111100,
		0b10000010,
		0b10000010,
		0b11111110,
		0b10000010,
		0b10000010,
		0b10000010,
		0b00000000
	};

	const uint8_t dataB[] =
	{
		0b11111100,
		0b10000010,
		0b10000010,
		0b11111100,
		0b10000010,
		0b10000010,
		0b11111100,
		0b00000000
	};

	const uint8_t dataC[] =
	{
		0b01111110,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b01111110,
		0b00000000
	};

	const uint8_t dataD[] =
	{
		0b11111100,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b11111100,
		0b00000000
	};

	const uint8_t dataE[] =
	{
		0b11111110,
		0b10000000,
		0b10000000,
		0b11111110,
		0b10000000,
		0b10000000,
		0b11111110,
		0b00000000
	};

	const uint8_t dataF[] =
	{
		0b11111110,
		0b10000000,
		0b10000000,
		0b11111110,
		0b10000000,
		0b10000000,
		0b10000000,
		0b00000000
	};

	const uint8_t dataG[] =
	{
		0b01111110,
		0b10000000,
		0b10000000,
		0b10001110,
		0b10000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t dataH[] =
	{
		0b10000010,
		0b10000010,
		0b10000010,
		0b11111110,
		0b10000010,
		0b10000010,
		0b10000010,
		0b00000000
	};

	const uint8_t dataI[] =
	{
		0b00111000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00111000,
		0b00000000
	};

	const uint8_t dataJ[] =
	{
		0b00111110,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b00001000,
		0b01110000,
		0b00000000
	};

	const uint8_t dataK[] =
	{
		0b10000110,
		0b10011000,
		0b10100000,
		0b11111000,
		0b10000100,
		0b10000010,
		0b10000010,
		0b00000000
	};

	const uint8_t dataL[] =
	{
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b11111110,
		0b00000000
	};

	const uint8_t dataM[] =
	{
		0b10000010,
		0b11000110,
		0b11000110,
		0b10101010,
		0b10101010,
		0b10010010,
		0b10010010,
		0b00000000
	};

	const uint8_t dataN[] =
	{
		0b10000010,
		0b11000010,
		0b10100010,
		0b10010010,
		0b10001010,
		0b10000110,
		0b10000010,
		0b00000000
	};

	const uint8_t dataO[] =
	{
		0b00111000,
		0b01000100,
		0b10000010,
		0b10000010,
		0b10000010,
		0b01000100,
		0b00111000,
		0b00000000
	};

	const uint8_t dataP[] =
	{
		0b11111100,
		0b10000010,
		0b10000010,
		0b11111100,
		0b10000000,
		0b10000000,
		0b10000000,
		0b00000000
	};

	const uint8_t dataQ[] =
	{
		0b00111000,
		0b01000100,
		0b10000010,
		0b10000010,
		0b10011010,
		0b01001100,
		0b00111110,
		0b00000000
	};

	const uint8_t dataR[] =
	{
		0b11111100,
		0b10000010,
		0b10000010,
		0b11111100,
		0b10000010,
		0b10000010,
		0b10000010,
		0b00000000
	};

	const uint8_t dataS[] =
	{
		0b01111100,
		0b10000010,
		0b10000000,
		0b01111100,
		0b00000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t dataT[] =
	{
		0b11111110,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataU[] =
	{
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t dataV[] =
	{
		0b10000010,
		0b10000010,
		0b01000100,
		0b01000100,
		0b00101000,
		0b00111000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataW[] =
	{
		0b10010010,
		0b10010010,
		0b10101010,
		0b10101010,
		0b11000110,
		0b11000110,
		0b10000010,
		0b00000000
	};

	const uint8_t dataX[] =
	{
		0b10000010,
		0b01000100,
		0b00101000,
		0b00010000,
		0b00101000,
		0b01000100,
		0b10000010,
		0b00000000
	};

	const uint8_t dataY[] =
	{
		0b10000010,
		0b01000100,
		0b00101000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00000000
	};

	const uint8_t dataZ[] =
	{
		0b11111110,
		0b00000100,
		0b00001000,
		0b00010000,
		0b00100000,
		0b01000000,
		0b11111110,
		0b00000000
	};

	const uint8_t data0[] =
	{
		0b01111100,
		0b10000010,
		0b10001110,
		0b10010010,
		0b11100010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t data1[] =
	{
		0b00010000,
		0b00110000,
		0b01010000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00111000,
		0b00000000
	};

	const uint8_t data2[] =
	{
		0b01111100,
		0b10000010,
		0b00000100,
		0b00001000,
		0b00010000,
		0b01100000,
		0b11111110,
		0b00000000
	};

	const uint8_t data3[] =
	{
		0b01111100,
		0b10000010,
		0b00000010,
		0b00000100,
		0b00000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t data4[] =
	{
		0b10000010,
		0b10000010,
		0b10000010,
		0b11111110,
		0b00000010,
		0b00000010,
		0b00000010,
		0b00000000
	};

	const uint8_t data5[] =
	{
		0b11111110,
		0b10000000,
		0b10000000,
		0b11111100,
		0b00000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t data6[] =
	{
		0b01111100,
		0b10000010,
		0b10000000,
		0b11111100,
		0b10000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t data7[] =
	{
		0b11111110,
		0b00000100,
		0b00001000,
		0b11111110,
		0b00100000,
		0b01000000,
		0b10000000,
		0b00000000
	};

	const uint8_t data8[] =
	{
		0b01111100,
		0b10000010,
		0b10000010,
		0b01111100,
		0b10000010,
		0b10000010,
		0b01111100,
		0b00000000
	};

	const uint8_t data9[] =
	{
			0b01111100,
			0b10000010,
			0b10000010,
			0b01111110,
			0b00000010,
			0b10000010,
			0b01111100,
			0b00000000
	};




	rl::FontFaceCreatorConfig config = {};
	config.bGenericUse = true;
	config.eClassification = rl::FontFaceClassification::SansSerif;
	config.iFaceVersion[0] = 1;
	config.iFaceVersion[1] = 0;
	config.iFaceVersion[2] = 0;
	config.iFaceVersion[3] = 0;
	config.iFallbackChar = 0xFFFD;
	config.iPaddingFlags = RL_FNT_PADFLAG_RIGHT | RL_FNT_PADFLAG_BOTTOM;
	config.iWeight = RL_FNT_WEIGHT_REGULAR;

	rl::BitmapFontFaceCreator creator;
	creator.create(config, rl::FontFaceBinaryFormat::BitPlanes, 1, 8, 8, "DevFont", "Regular",
		"(c) Robin Lemanska 2021");


#define ADDCHAR2(ch, name) ConvertChar(creator.createChar(ch, 0), data##name)

	ADDCHAR2('=', EQUALS);
	ADDCHAR2('!', EXCLAMATION);
	ADDCHAR2('?', QUESTIONMARK);
	ADDCHAR2(' ', SPACE);
	ADDCHAR2(0xFFFD, UNDEFINED);

#define ADDCHAR(letter) ADDCHAR2(#@letter, letter)

	ADDCHAR(A);
	ADDCHAR(B);
	ADDCHAR(C);
	ADDCHAR(D);
	ADDCHAR(E);
	ADDCHAR(F);
	ADDCHAR(G);
	ADDCHAR(H);
	ADDCHAR(I);
	ADDCHAR(J);
	ADDCHAR(K);
	ADDCHAR(L);
	ADDCHAR(M);
	ADDCHAR(N);
	ADDCHAR(O);
	ADDCHAR(P);
	ADDCHAR(Q);
	ADDCHAR(R);
	ADDCHAR(S);
	ADDCHAR(T);
	ADDCHAR(U);
	ADDCHAR(V);
	ADDCHAR(W);
	ADDCHAR(X);
	ADDCHAR(Y);
	ADDCHAR(Z);
	ADDCHAR(0);
	ADDCHAR(1);
	ADDCHAR(2);
	ADDCHAR(3);
	ADDCHAR(4);
	ADDCHAR(5);
	ADDCHAR(6);
	ADDCHAR(7);
	ADDCHAR(8);
	ADDCHAR(9);

#undef ADDCHAR
#undef ADDCHAR2

	creator.saveToFile(LR"(E:\[Temp]\!DevFont.rlFNT)");

	return 0;




	/*rl::FontFaceClass ffc;

	if (!ffc.loadFromFile(LR"(E:\[TempDel]\test.rlFNT)"))
		printf("Failure\n");
	else
		printf("Success\n");*/

	try
	{
		rl::FontFaceCreatorConfig config = {};
		config.eClassification = rl::FontFaceClassification::SansSerif;
		config.iFaceVersion[0] = 0;
		config.iFaceVersion[1] = 0;
		config.iFaceVersion[2] = 0;
		config.iFaceVersion[3] = 1;
		config.iWeight = RL_FNT_WEIGHT_REGULAR;
		config.iFallbackChar = '?';

		rl::BitmapFontFaceCreator face;
		/*face.create(config, rl::FontFaceBinaryFormat::BitPlanes, 1, 8, 4, "Testfont", "Regular",
			"(c) 2021 Robin Lemanska. All rights reserved.");

		rl::BitmapFontChar chr = face.createChar('?', 0);

		if (face.saveToFile(LR"(E:\[TempDel]\Test.rlFNT)"))
			printf("Saving: Success\n");
		else
		{
			printf("Saving: Failure\n");
			return -1;
		}*/

		face.clear();
		if (face.loadFromFile(LR"(E:\[TempDel]\Test.rlFNT)"))
			printf("Loading: Success\n");
		else
		{
			printf("Loading: Failure\n");
			return -1;
		}

		auto& ch = face.getChar('?');
	}
	catch (std::exception e)
	{
		printf("Exception: %s\n", e.what());
	}


	return 0;
}
