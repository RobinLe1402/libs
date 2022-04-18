#pragma once
#ifndef ROBINLE_LIB_ASSERT
#define ROBINLE_LIB_ASSERT


// never use assert() in release mode
#ifndef NDEBUG
#include <assert.h>
#define FONT__ASSERT(x) assert(x)
#else
#define FONT__ASSERT(x)
#endif // NDEBUG


#endif // ROBINLE_LIB_ASSERT