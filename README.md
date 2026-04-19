# libugomemo

C library for parsing and encoding Nintendo Flipnote Studio animation formats.

## Supported formats

| Format | Extension | Platform | Parse | Decode video | Decode audio | Encode video | Encode audio |
|--------|-----------|----------|-------|-------------|-------------|-------------|-------------|
| PPM | .ppm | DSi | Yes | Yes | Yes | Yes | Yes |
| KWZ | .kwz | 3DS | Yes | Yes | Yes | Yes | Yes |
| UGO | .ugo | DSi | Yes | - | - | - | - |
| NBF | .nbf | DSi | Yes | Yes | - | - | - |
| NPF | .npf | DSi | Yes | Yes | - | - | - |
| LST/PLS | .lst/.pls | DSi/3DS | Yes | - | - | - | - |
| KWZPCF | .kwzpcf | 3DS | Yes | - | - | - | - |

## Building

```sh
make release   # static library  -> build/release/libugomemo.a
make shared    # shared library  -> build/shared/libugomemo.dylib (or .so)
make debug     # debug static    -> build/debug/libugomemo.a
```

## Dependencies

- OpenSSL (libcrypto): SHA-256, SHA-1 hashing and DER key loading
- GNU Multiple Precision (libgmp): RSA operations
- zlib: gzip decompression and CRC32 hashing

## Usage

```c
#include <ugomemo.h>

// Open and parse a KWZ file
kwz_ctx *kwz = kwz_open("animation.kwz");
if (!kwz) return 1;

printf("Frames: %u\n", kwz_get_frame_count(kwz));
printf("Author: %s\n", kwz_get_current_username(kwz));

// Decode a frame to RGB pixels (caller must free)
rgb24_pixel *pixels = kwz_decode_frame_alloc(kwz, 0);
// pixels is 320*240 rgb24_pixel values
free(pixels);

// Decode an audio track (caller must free)
u32 sample_count;
i16 *audio = kwz_decode_track_alloc(kwz, 0, -1, &sample_count);
free(audio);

kwz_cleanup(kwz);
```

PPM files work the same way:

```c
ppm_ctx *ppm = ppm_open("animation.ppm");
printf("Frames: %u\n", ppm_get_frame_count(ppm));

rgb24_pixel *pixels = ppm_decode_frame_alloc(ppm, 0);
free(pixels);

ppm_close(ppm);
```

## Python integration

Build the shared library and load it with ctypes:

```python
import ctypes
lib = ctypes.CDLL("path/to/libugomemo.dylib")
```

The convenience API (`kwz_open`, `kwz_get_*`, `kwz_decode_frame_alloc`, etc.) is designed for FFI use -- all functions take and return simple types or opaque pointers.
