# kwz-cpp
A Flipnote Studio 3D .kwz file processing program made in C++.

Source code can be found at: https://www.github.com/meemo/kwz-cpp

# Installation

Edit `file_name`  in `main()` in `kwz.cpp` to specify your target file to process then compile with your favorite compiler (Visual Studio 2019, GCC, etc.) and run.

# Progress

- ✅ File metadata extraction
- ✅ Thumbnail extraction
- 🟩 Frame decoding - rough skeleton implemented
- 🟩 Audio track decoding - rough skeleton implemented
- ❌ Converting author names from UTF-16 to readable encoding\
- ❌ Conversion of timestamps (to unix epoch? to formatted date/time?)
- ❌ Frame output to file
- ❌ Audio track output to file
- ❌ Audio track mixing with sound effects from frame meta
- ❌ Conversion of entire flipnote to (mp4) file
- ❌ Handling all DSi Library conversion inconsistencies
- ❌ Running from CLI implementation

# Observed DSi Library Conversion Errors

- Parent file name is sometimes in the ppm format
- DSi Library folder structure uses the Flipnote Studio FSID format to sort by author IDs

# Credits

The Flipnote Collective's documentation https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/ that this entire program is based around.