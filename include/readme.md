# `#include` files
When using any of the `.props` files at `vs/`, this directory gets added to the include directories.

This is necessary for using most of the source files in this repo, as I'm always including my headers via `#include "rl/[...].hpp"` in source files (though within this directory's headers, I might use relative paths for minimalism).
