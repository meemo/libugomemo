# libugomemo

libugomemo is a WIP ANSI C library for processing the files created by Nintendo's Flipnote Studio and Flipnote Studio
3D animation apps.


# Goals

To have functions for:

- extracting standardized data from files
- converting standardized data to the file's native format
- forming console-accurate file buffers where applicable

for every file type produced by Flipnote Studio and Flipnote Studio 3D. Extraction is the highest priority, however
conversion will eventually be implemented.

To be as compatible as possible with almost all toolchains (including embedded devices) by using ANSI C89 compliant
code.

The assumptions that will be made are:

- The target toolchain has a compiler compatible with ANSI C code
- The target is capable of floating point math (for some functions)
- The target has fixed width datatypes (i.e. is byte addressed)
  - This is unlikely to change due to how this affects structs and due to how uncommon it is.

The following are additional low priority goals that may or may not be implemented in the future:

- Not assuming target endianness (i.e. accounting for big endian targets)
  - Nearly all targets are natively little endian or operating in little endian mode and the formats are natively little endian, so this is low priority to account for.
- Option to not use stdlib
- Accounting for compiler/implementation defined behavior where viable
- Optional assembly inlines for select targets
- Overall speed of code


# Documentation

For more extensive documentation of the formats and apps in this library, see [the Flipnote Collective's](https://github.com/Flipnote-Collective):

- [Flipnote Studio docs](https://github.com/Flipnote-Collective/flipnote-studio-docs)
- [Flipnote Studio 3D docs](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs)

Documentation for usage of the library will be located in [docs/](docs/) once the layout of the functions has been finalized. For now, documentation is located in the comments above each function and  file.


# Credits

- Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)
