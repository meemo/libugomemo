# kwz-cpp
A work in progress Flipnote Studio 3D .kwz file processing program made in C++. Progress is actively updated below as work is done.

# Installation

Edit the `input_file_path` string in `main()` in `kwz.cpp` to specify your input file (remember to escape back slashes if on windows!) to process then compile with Visual Studio 2019, GCC, etc. and run.

# Progress

- ✅ File metadata extraction
- ✅ Thumbnail extraction - fully implemented
- ✅ Audio track decoding - fully implemented wav file output
- 🟩 Frame decoding - rough skeleton implemented
- ❌ Conversion of timestamps (to unix epoch? to formatted date/time?) currently outputs the raw timestamp, the number of seconds from 00:00:00 1/1/2000.
- ❌ Converting author names from UTF-16 to readable encoding
- ❌ Frame output to file
- ❌ Audio track output to file
- ❌ Audio track mixing with sound effects from frame meta
- ❌ Conversion of entire flipnote to mp4 file
- ❌ Handling all DSi Library conversion inconsistencies
- ❌ Running from CLI implementation
- ❌ Replace expensive for loops with individually setting values

# Observed issues with the .kwz format

- Parent file name is sometimes in the ppm format for DSi Library notes
- The DSi Library S3 bucket folder structure uses the Flipnote Studio FSID format to sort by author IDs
- Thumbnails are officially extracted with 3 unknown bytes at the end of the file, after the `0xD9` byte that terminates the JPG file normally.
- Audio decoding issues resulting in corrupt-sounding audio in DSi Library flipnotes.
- In some Flipnote Gallery World notes, the thumbnail JPG data appears to be corrupted. This has not been observed in DSi Library flipnotes.

# Credits

The Flipnote Collective's [documentation](https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format) and individual assistance that made this program possible.

# Source Code

Source code can be found at: https://www.github.com/meemo/kwz-cpp