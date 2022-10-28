# libugomemo

libugomemo is a work-in-progress ANSI C library for processing the files created by and for Nintendo's 
[Flipnote Studio](https://en.wikipedia.org/wiki/Flipnote_Studio) and [Flipnote Studio 3D](https://en.wikipedia.org/wiki/Flipnote_Studio_3D)
animation apps on the [Nintendo DSi](https://en.wikipedia.org/wiki/Nintendo_DSi) and
[Nintendo 3DS](https://en.wikipedia.org/wiki/Nintendo_3DS), respectively. This will include the animations produced by the app
along with certain files that are used in their respective online services.

## Goals

To have functions for:

- extracting standardized data from files
- converting standardized data to the file's native format
- forming console-accurate file buffers where applicable

for every file type produced by Flipnote Studio and Flipnote Studio 3D. Extraction is the highest priority, however
conversion is intended to be implemented at some point.

To be as compatible as possible with toolchains (including those for embedded devices) by using ANSI (C89/90) compliant code.

The following are additional low priority goals that may or may not be implemented in the future:

- Not assuming target endianness (i.e. accounting for big endian targets)
- Option to not use stdlib
- Accounting for compiler/implementation defined behavior where viable
- Optional assembly inlines for select targets
- To have the overally fastest implementation of flipnote-related parsing code of any library.

## Documentation

For more extensive documentation of the formats and apps in this library, see the Flipnote Collective's:

- [Flipnote Studio docs](https://github.com/Flipnote-Collective/flipnote-studio-docs)
- [Flipnote Studio 3D docs](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs)

Documentation for usage of the library will be eventually located in [docs/](docs/). 
For now, documentation of is located in the comments above each function andfile.

## Compilation and Usage

Compilation is currently set up to be done with the [`make.sh`](make.sh) script located at the root of the repository. Change the
variables near the top of the file to point to the toolchain that you wish to use for compilation and a static library (.a) will be
produced. With this you can add the following to the compilation command of the rest of your source files:

- `-Ipath/to/libugomemo/include`
- `-Lpath/to/libugomemo`
- `-lugomemo`

Where `path/to/libugomemo` is replaced with the absolute path to the root of the repository. 

Alteratively, you could integrate the source files in [src/](src/) and the .h files in [include/](include/) into the source tree
of your project, where it can be compiled just like every other file. Since libugomemo is written with portability in mind, there
shouldn't be any need for any changes to compilation arguments. If you find a situation where this is not the case then please open
an issue.

In either case, all you need to use the library in your project is to include the following line in the relevant source files:

    #include <libugomemo.h>

If you have not set up include paths with the -I option but instead of stuck everything ont he same level as the source files then use:

    #include "libugomo.h"

After this, all functions are available for you to use.

## Credits

- Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)

## License

libugomemo is licensed under the Apache License, version 2.0. A copy of this license can be found at [LICENSE](LICENSE).
