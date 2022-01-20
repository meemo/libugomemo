# libugomemo

libugomemo is a WIP Flipnote Studio and Flipnote Studio 3D animation file parsing library made in C.

Documentation is located in [docs/](docs/).


# Goals

To have functions for:

- extracting standardized data
- converting standardized data to the file's native format

for every file type produced by Flipnote Studio and Flipnote Studio 3D.

Examples of standardized formats include:

- converting audio to/from a s16le PCM data stream,
- converting frames to/from raw RGB data

File formats supported currently:

- .kwz
- .ppm

To make as few assumptions about the target system and target compiler as possible to facilitate embeded system development including:

- Not relying on any compiler defined behavior
- Reconfigurable malloc, free, etc.
- Not relying on any fixed width datatypes
- Not assuming target endianness


# Credits

- Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)

- The [Flipnote Collective](https://github.com/Flipnote-Collective) for their documentation on the formats used by:
    - [Flipnote Studio](https://github.com/Flipnote-Collective/flipnote-studio-docs)
    - [Flipnote Studio 3D](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs)
