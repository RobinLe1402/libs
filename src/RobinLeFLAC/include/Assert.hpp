/***************************************************************************************************
 FILE:	Assert.hpp
 CPP:	<n/a>
 DESCR:	Custom assert definition
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_ROBINLEFLAC_ASSERT
#define ROBINLE_LIB_ROBINLEFLAC_ASSERT





#ifndef NDEBUG
#include <assert.h>
#define FLAC__ASSERT(x) assert(x)
#else
#define FLAC__ASSERT(x)
#endif





#endif // ROBINLE_LIB_ROBINLEFLAC_ASSERT