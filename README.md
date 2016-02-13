# VGS SPU 
SUZUKI PLAN - Video Game System - Sound Processing Unit

## Description
Abstracts the following sound APIs:
- DirectX (Windows)
- OpenSL/ES (Android)
- OpenAL (MacOSX and iOS)
- ALSA; Advanced Linux Sound Architecture (Linux)

## How to use
- start
- callback
- end

## Start
#### prototyping
```
void* vgsspu_start(void (*callback)(void* buffer, size_t size));
void* vgsspu_start2(int sampling, int bit, int ch, size_t size, void (*callback)(void* buffer, size_t size));
```

_NOTE: `vgsspu_start` specifies 22050Hz, 16bit, 1ch and 4410 bytes as size._

#### arguments
- `sampling` : sampling rate _(48000, 44100, 22050, 11025, 8000 etc)_
- `bit` : bit rate _(24, 16, 8 etc)_
- `ch` : number of channel _(1:monoral, 2:stereo)_
- `size` : size of buffer
- `callback` : buffering callback

#### return value
- `!NULL` : context (succeed)
- `NULL` : failed

## End
#### prototyping
```
void vgsspu_end(void* context)
```

#### arguments
- `context` : context

