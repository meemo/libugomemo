# kwz-cpp
A (very rough) Flipnote Studio 3D .kwz file processing program made in C++.

Source code can be found at: https://www.github.com/meemo/kwz-cpp

# Installation

Edit `file_name`  in `main()` in `kwz.cpp` to specify your target file to process then compile with your favorite compiler (Visual Studio 2019, GCC, etc.) and run.

# Progress

- ✅ File metadata extraction
- ✅ Thumbnail extraction
- 🟩 Frame decoding - rough skeleton implemented
- 🟩 Audio track decoding - rough skeleton implemented
- 🟩 Conversion of timestamps (to unix epoch? to formatted date/time?) currently outputs the number of seconds from 00:00:00 1/1/2000 epoch.
- ❌ Converting author names from UTF-16 to readable encoding
- ❌ Frame output to file
- ❌ Audio track output to file
- ❌ Audio track mixing with sound effects from frame meta
- ❌ Conversion of entire flipnote to (mp4) file
- ❌ Handling all DSi Library conversion inconsistencies
- ❌ Running from CLI implementation
- ❌ Replace expensive for loops with individually setting values

# Observed DSi Library Conversion Errors

- Parent file name is sometimes in the ppm format
- DSi Library folder structure uses the Flipnote Studio FSID format to sort by author IDs
- Thumbnails are officially extracted with 3 bytes of unkown use at the end, after the `0xD9` byte that terminates all JPG files.

# Credits

The Flipnote Collective's documentation https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/ that this entire program is based around.