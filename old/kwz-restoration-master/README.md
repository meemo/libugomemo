# kwz-restoration

In the process of Nintendo's Flipnote Hatena to Flipnote Gallery World conversion to the Flipnote Studio 3D .kwz format, the audio was improperly encoded in to the new format. Nintendo appears to not have reset the decoder variables between conversions of files, so some files have [extremely distorted audio](https://twitter.com/AustinSudomemo/status/1220367326085832704?s=20) using the default initial decoder state. This program finds proper initial step index value for all audio tracks in order to decode the best possible audio given the data in the converted .kwz files.

Note: decoder variable naming is from the [IMA ADPCM standard](http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf)


# Compilation

`./make.sh`

Requires GCC.


# Usage

`./kwz-restoration [input .kwz file path] [optional: output .wav file path] [optional: output track index]`

By specifying an output .wav file path, the BGM track will be decoded with the found correct initial step index then will be written to the file path specified.

Refer to the chart below for valid track indexes. If no index is specified, the BGM track (0) will be the output.

| Track Index | Description           |
|-------------|-----------------------|
| 0 (Default) | BGM, background music |
| 1           | Sound Effect 1 (A)    |
| 2           | Sound Effect 2 (X)    |
| 3           | Sound Effect 3 (Y)    |

# Restoration Process

To decode an audio track properly, follow this procedure first. After that, decode the track with the resulting initial step index.

 - Decode the track with the step index from 0 to 40 and with the predictor as 0
   - The track must be converted fully for all 40 step index values because the impact of an improper step index is extremely subtle in a short period, however when the entire track is decoded the increased values are easily caught by the next step:
   - The reasoning for the 0-40 range is that >40 trips the 4 bit detection flag too early, screwing up the ordering of bit decoding for the rest of the track, followed by the usual incorrect step index causing gradually higher and higher amplitude tracks.

 - Calculate the RMS of the decoded track:
   - Square each sample then add together
   - Divide by the total number of samples
   - Take the square root of that value to get your RMS.

 - The step index with the lowest RMS is the result


# Credits

 - Everyone mentioned in [flipnote.js acknowledgments](https://flipnote.js.org/pages/docs/acknowledgements.html)

 - [Sudodad](https://github.com/tychoaussie) - for early ideas to approach restoration mathematically
