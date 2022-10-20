# Todo

## High-priority

### PPM

- [ ] Audio decoding
  - [x] Decode entire track
  - [ ] Decode sample by sample
    - Complicated by the fact that bytes have 2 samples each
- [ ] Frame decoding
- [ ] Thumbnail decoding
- [ ] File validation

### KWZ

- [ ] Audio decoding
  - [x] Decode entire track
  - [ ] Decode sample by sample
    - Complicated by the fact that bytes have anywhere from 2-4 samples each
- [ ] Frame decoding
- [ ] Port correct step index calculation for DSi Library flipnotes
- [ ] File validation

### audio.c

- [ ] Track mixing
  - [ ] KWZ BGM + 4 sound effects option
  - [ ] PPM BGM + 3 sound effects option
  - [ ] 2 arbitrary tracks
- [ ] Console-accurate resampling
- Maybe other resampling methods

### Misc

- [ ] BMP output for RGB8 stuff (frame decoding)
- [ ] Set up a proper build system
- [ ] Find a documentation generator
- [ ] Create a .clang-format for code style
- [ ] Set up real test cases
- [ ] Set up CI for style/tests/builds

## Low-priority

### PPM

- [ ] Frame encoding
- [ ] Audio encoding

### KWZ

- [ ] Frame encoding (this is going to be rough)
- [ ] Audio encoding

### Crypto

- [ ] Signature validation/creation
