# [WIP] VGS SPU 
SUZUKI PLAN - Video Game System - Sound Processing Unit

## License
[The BSD 2-Clause License](https://github.com/suzukiplan/vgs-spu/blob/master/LICENSE.txt)

## Description
Abstracts the following sound APIs:
- DirectSound (Windows)
- OpenSL/ES (Android)
- OpenAL (MacOSX and iOS)
- ALSA; Advanced Linux Sound Architecture (Linux)

## How to use
- start
- callback
- end

## Example
#### sounds about 440Hz square wave 1sec

```c
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include "vgsspu.h"

#ifdef _WIN32
static void usleep(int usec)
{
    Sleep(usec / 1000);
}
#endif

static unsigned int hz;
static unsigned int pw = 22050;

static void buffering(void* buffer, size_t size)
{
    short* sbuf = (short*)buffer;
    size_t s2 = size >> 1;
    size_t s1;
    for (s1 = 0; s1 < s2; s1++, sbuf++, hz++) {
        if (hz % 50 < 25) {
            *sbuf = (short)((16384 * pw) / 22050);
        } else {
            *sbuf = (short)((-16384 * pw) / 22050);
        }
        if (pw) pw--;
    }
}

int main(int argc, char* argv[])
{
    void* context = vgsspu_start(buffering);
    if (NULL == context) {
        puts("failed");
        return -1;
    }
    usleep(1000000);
    vgsspu_end(context);
    return 0;
}
```

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

