# Todo

## High-priority

### PPM

- [ ] Audio decoding
  - [x] Decode entire track
  - [ ] Decode sample by sample
    - [x] Create decoder state struct to be passed around
    - [ ] Create init
    - [ ] Create get next sample
    - [ ] Create finish (clearing the state)
- [ ] Thumbnail decoding
- [ ] Frame decoding
- [ ] File validation

### KWZ

- [ ] Audio decoding
  - [x] Decode entire track
  - [ ] Decode sample by sample
    - [ ] Create decoder state struct to be passed around
    - [ ] Create init
    - [ ] Create get next sample
    - [ ] Create finish (clearing the state)
- [ ] Thumbnail extraction
- [ ] Frame decoding
  - This is going to be *fun*
- [ ] Re-implement correct step index calculation for DSi Library audio
- [ ] File validation

### audio.c

- [ ] Track mixing
  - [ ] KWZ BGM + 4 sound effects option
  - [ ] PPM BGM + 3 sound effects option
  - [ ] 2 arbitrary tracks
- [ ] Console-accurate resampling
- [ ] Maybe other resampling methods

### Misc

- [ ] BMP output for RGB8 data (from KWZ/PPM frame decoding and PPM thumbnail decoding)
- [ ] Set up a proper build system
- [ ] Find a documentation generator
- [ ] Create a .clang-format for code style
- [ ] Set up real test cases
  - [ ] In the meantime create basic test cases programs that return 1 on failure
- [ ] Set up CI for style/tests/builds

## Low-priority

### PPM

- [ ] Frame encoding
- [ ] Audio encoding

### KWZ

- [ ] Frame encoding (this is going to be rough)
- [ ] Audio encoding

### Crypto and related

- [x] CRC32 calculation
- [ ] Signature validation/creation
