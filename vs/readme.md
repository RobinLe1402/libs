# Visual Studio-specific files

## `solutions/`
Currently contains all `.sln` files that include any of the projects in this repo.

## `.editorconfig`
My current IDE editor configuration

## `robinle.props`
Property sheet used in all of my application projects.
* Adds `%GitHub_rl_libs%` to the include directories.
* Sets the output directory to `bin/[...]`

## `robinle_dll.props`
Property sheet used in all of my dynamic library projects.
* Sets the output type to `DLL`
* Adds `%GitHub_rl_libs%` to the include directories.
* Sets the output directory to `lib/[...]`

## `robinle_lib.props`
Property sheet used in all of my static library projects.
* Sets the output type to `LIB`
* Adds `%GitHub_rl_libs%` to the include directories.
* Sets the output directory to `lib/[...]`
