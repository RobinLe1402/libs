# RobinLe Libraries
This repository used to be my main code base. It contains several useful modules I might either extract or remake as their own static/dynamic library at one point.

> [!IMPORTANT]  
> Note that I stopped working with this codebase years ago. It only exists as a reference for new versions of the contained modules I might create in the future.


## Directory structure
In this repo, I mainly use the UNIX-style directory structure typical for libraries etc.

Here's a definition of what each directory contains:

| Dir | Contents |
|-----|----------|
| `bin` | *(only locally)* non-library binary files (compiled from `test` and `tools`) |
| `doc` | Misc. non-wiki documentation files |
| `etc` | Misc. non-sourcecode files |
| `include` | `.hpp` and `.h` files for including |
| `lib` | *(only locally)* `.dll` and `.lib` files (compiled from `src`) |
| `res` | Misc. resource files |
| `src` | Source files (both for libraries and single units) |
| `template` | Misc. template files |
| `test` | Source code for test applications (mainly for testing libraries) |
| `tools` | Source code for misc. applications |
| `vs` | Visual Studio-specific files |

Further info about the directory structure and contents are available in `readme.md`s in the subdirectories.


## Executable downloads
Since this repository is the home of a multiple of unrelated applications and the GitHub release feed is not too good for this purpose, I outsourced downloads of executables to my own website.

~~Download executables here~~ (the executables are no longer available for download)


## Documentation
I try to document my code as well as possible, while trying not to overdo it.

Unfortunately, I currently document my code in several places, inconsistently. I'm planning on creating a unified documentation system, probably HTML-based, but that will take a while.


These are the ways I document my code in the moment:

### In the source code
Mostly XML documentation (`summary`, `param`, `returns` etc.), but also regular comments (mostly with "internal" code)

### The GitHub wiki
I try to maintain the [wiki of this repo](https://github.com/RobinLe1402/libs/wiki), though most of the time I'm not invested enough to update it as it seems to be the most tedious type of documentation to me.
 
### In `readme.md` files
I put `readme.md` files in some directories containing more general documentation about their content.
