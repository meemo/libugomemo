# libugomemo
libugomemo is a WIP Flipnote Studio and Flipnote Studio 3D animation file parsing library made in C.

The goals of this library are as follows:

- To have an implementation of every major function for processing .kwz and .ppm files, including metadata, audio, and video processing

- To be as universally compatible with C compilers/SDKs by having the following:

    - ANSI C compliant code
        - full compliance is low priority until completion

    - No libraries required except for stdlib
        - reconfigurable malloc, realloc, free

    - No mandatory asm inlines
        - they will be added for specific platforms (ARM, x86 primarily), but all functions will work without them

 - For each function to have as small of a memory footprint as possible

# Credits
- Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)

- The [Flipnote Collective](https://github.com/Flipnote-Collective) for their documentation on the formats used by:
    - [Flipnote Studio](https://github.com/Flipnote-Collective/flipnote-studio-docs)
    - [Flipnote Studio 3D](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs)
