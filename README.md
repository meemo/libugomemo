# libugomemo
libugomemo is a WIP Flipnote Studio and Flipnote Studio 3D animation file parsing library made in C.

The goals of this library are as follows:

- To have an implementation of every major function for processing .kwz and .ppm files, including metadata, audio, and video processing

- To be as universally compatible with C compilers/SDKs by having the following:

    - ANSI C compliant code
        - this is low priority until completion, but all code will be at minimum C99 compliant

    - No external libraries required
        - possibly no relying on stdlib (very low priority)

    - No mandatory asm inlines
        - asm inlines will be added for specific platforms (ARM, x86 primarily), but all functions will work without them

    - Reconfigurable malloc, realloc, free, etc.

 - For each function to have as small of a memory footprint as possible

# Credits
- Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)

- The [Flipnote Collective](https://github.com/Flipnote-Collective) for their [Flipnote Studio](https://github.com/Flipnote-Collective/flipnote-studio-docs) and [Flipnote Studio 3D](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs) documentation
