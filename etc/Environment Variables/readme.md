# Environment variable BATCH scripts

BATCH scripts for installing and uninstalling the environment variables needed for this repo's source files.

## The environment variables
### `%GitHub_rl_libs%`
The path of the repo, including a trailing backslash.  
Example: `C:\Users\RobinLe\Source\Repos\Libs\`


## The script files
### `_INSTALL.BAT`
Add the environment variables (calls `SCRIPT.BAT` with parameters)

### `_UNINST.BAT`
Removes the environment variables (calls `SCRIPT.BAT` with parameters)

### `SCRIPT.BAT`
The main script file.  
Don't call directly, but rather by calling `_INSTALL.BAT` or `_UNINST.BAT`
