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

- Not relying on any compiler defined behavior
- Reconfigurable malloc, free, etc.
- Not relying on any fixed width datatypes
- Not assuming target endianness


# Layout

Files located in the top level directory (`src/`) contain generic functions that are used by the other source files or helper functions that may be useful when using the library.

Each directory within `src/` has functions relating to the corresponding folder name (e.g. `src/kwz/` contains functions for `.kwz` files).
