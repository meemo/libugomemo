# kwz-audio

A Flipnote Studio 3D .kwz file audio track extraction tool focusing primarily on speed and quality. All tested extractions take well under 100ms to convert on a modern CPU.

This program converts the variable sample size IMA ADPCM audio in the flipnote to signed 16 bit little endian PCM audio in a WAV file.

# Usage

`./kwz-audio [Track Index] [Input File] [Output File]` 

Where:

`[Track Index]` is the index of the track to decode (numbered in the table below)

`[Input File]` is the absolute path to your input .kwz file

`[Output File]` is the absolute path to the .wav file you wish for the program to output.

| Track Index | Description           |
|-------------|-----------------------|
| 0           | BGM, background music |
| 1           | Sound Effect 1 (A)    |
| 2           | Sound Effect 2 (X)    |
| 3           | Sound Effect 3 (Y)    |
| 4           | Sound Effect 4 (Up)   |

# Compilation

### Linux/Unix/MacOS (GCC)

For MacOS, install GCC via [homebrew](https://docs.brew.sh/Installation).

Requires gcc-cpp. Compile using the following command:

`g++ -o kwz-audio -O3 kwz-audio.cpp`

Run using usage explained above.

# ADPCM Decoding Settings

ADPCM decoder initital states are avaliable to modify in `kwz-audio.hpp`.

# Credits

The Flipnote Collective's documentation https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/ that this entire program is based around.

# Source code

Source code can be found at https://github.com/meemo/kwz-audio
