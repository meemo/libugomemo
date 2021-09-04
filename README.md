# kwz-cpp

A work in progress Flipnote Studio 3D .kwz file processing program made in C++. Progress is actively updated below as portions are completed.

# Installation

Program is compatible with Linux only until completed. Porting to other platforms will then be attempted.

`make`

Ensure `build-essential` or equivalent is installed.

# Usage



# Progress

- ✅ File metadata extraction

- ✅ Thumbnail extraction - fully implemented

- ✅ Audio track decoding - fully implemented wav file output

- 🟩 Frame decoding - rough skeleton implemented

- 🟩 Handling all DSi Library conversion inconsistencies (all known issues handled)

- ❌ Outputting all metadata as JSON

- ❌ Converting author names from UTF-16 to readable encoding

- ❌ Frame output to file

- ❌ Audio track mixing with sound effects from frame meta

- ❌ Conversion of entire Flipnote to .MP4 or .MKV file


# Observed issues with the .kwz format

- Parent file name is sometimes in the ppm format for DSi Library notes

- The DSi Library S3 bucket folder structure uses the Flipnote Studio FSID format to sort by author IDs

- Thumbnails are officially extracted with 3 unknown bytes at the end of the file, after the `0xD9` byte that terminates the JPG file normally.

- In some Flipnote Gallery World notes, the thumbnail JPG data appears to be corrupted. This has not been observed in DSi Library flipnotes.


# Credits

 - Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)
