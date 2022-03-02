# libugomemo Documentation

Welcome to the libugomemo documentation! All relevant information to this library will be located here in a format that (hopefully) doesn't suck.


# Goals

To have functions for:

- extracting standardized data from files
- converting standardized data to the file's native format
- forming console-accurate file buffers where applicable

for every file type produced by Flipnote Studio and Flipnote Studio 3D.

Examples of standardized formats include:

- converting audio to/from a s16le PCM data stream,
- converting frames to/from raw RGB data

To make as few assumptions about the target system and target compiler as possible to facilitate embedded system development including:

- ANSI C compliant code
- Not relying on any compiler/implementation defined behavior, and accounting for it where viable
- Reconfigurable malloc, free, etc.
- Not relying on fixed width datatypes where possible
- Not assuming target endianness
- Not using external libraries
- (Low priority) Option to not use stdlib


The assumptions that will be made are:

- The target toolchain accepts ANSI C compliant code
- The target is capable of floating point math (for some functions)
- The target has an 8 bit unisgned type
  - This may be changed in the future, however it is low priority because of how rare it is to not exist.

# Layout

Files located in the top level directory (`src/`) contain generic functions that are used by the other source files or helper functions that may be useful when using the library.

Each directory within `src/` has functions relating to the corresponding folder name (e.g. `src/kwz/` contains functions for `.kwz` files).
