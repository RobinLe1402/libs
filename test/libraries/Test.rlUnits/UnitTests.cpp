#include "UnitTests.hpp"

#include "tests.hpp"



std::vector<UnitTest> UnitTestCollection::s_oUnitTests(
	{
		{ "[global]",               UnitTest_global                 },
		{ "audio.engine",           UnitTest_audio_engine           },
		{ "data.online",            UnitTest_data_online            },
		{ "graphics.opengl.window", UnitTest_graphics_opengl_window },
		{ "input.keyboard",         UnitTest_input_keyboard         },
		{ "runasadmin",             UnitTest_runasadmin             },
		{ "splashscreen",           UnitTest_splashscreen           },
		{ "text.fileio",            UnitTest_text_fileio            }
	});
