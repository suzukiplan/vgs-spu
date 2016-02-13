/* (C)2016, SUZUKI PLAN.
 *----------------------------------------------------------------------------
 * Description: VGS - Sound Processing Unit for Direct Sound
 *    Platform: Windows
 *      Author: Yoji Suzuki (SUZUKI PLAN)
 *----------------------------------------------------------------------------
 */
#include <Windows.h>
#include <process.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vgsspu.h"

struct DS {
    LPDIRECTSOUND8 dev;
    LPDIRECTSOUNDBUFFER8 buf;
    LPDIRECTSOUNDNOTIFY8 ntfy;
    DSBPOSITIONNOTIFY dspn;
};

struct SPU {
    volatile int alive;
    struct DS ds;
    void* buffer;
    void (*callback)(void* buffer, size_t size);
    uintptr_t tid;
    BYTE ctrl;
    size_t size;
    int sampling;
    int bit;
    int ch;
    int format;
};

static HWND get_current_hwnd(void);
static void sound_thread(void* arg);
static int ds_wait(BYTE wctrl);

void* __stdcall vgsspu_start(void (*callback)(void* buffer, size_t size))
{
    return vgsspu_start2(22050, 16, 1, 4410, callback);
}

void* __stdcall vgsspu_start2(int sampling, int bit, int ch, size_t size, void (*callback)(void* buffer, size_t size))
{
    struct SPU* result;

    result = (struct SPU*)malloc(sizeof(struct SPU));
    if (NULL == result) {
        return NULL;
    }
    memset(result, 0, sizeof(struct SPU));
    result->tid = -1;
    result->sampling = sampling;
    result->bit = bit;
    result->ch = ch;
    result->callback = callback;
    result->size = size;
    result->buffer = malloc(size);
    if (NULL == result->buffer) {
        vgsspu_end(result);
        return NULL;
    }
    if (init_ds(result)) {
        vgsspu_end(result);
        return NULL;
    }
    result->alive = 1;
    result->tid = _beginthread(sound_thread, 65536, result);
    if (-1L == c->tid) {
        vgsspu_end(result);
        return -1;
    }
    return result;
}

void __stdcall vgsspu_end(void* context)
{
    struct SPU* c = (struct SPU*)context;
    if (-1L != c->tid) {
        c->alive = 0;
        WaitForSingleObject((HANDLE)_uiSnd, INFINITE);
    }
    if (c->ds.ntfy) {
        c->ds.ntfy->Release();
        c->ds.ntfy = NULL;
    }
    if ((HANDLE)-1 == c->ds.dspn.hEventNotify || NULL == c->ds.dspn.hEventNotify) {
        CloseHandle(c->ds.dspn.hEventNotify);
        c->ds.dspn.hEventNotify = NULL;
    }
    if (c->ds.buf) {
        c->ds.buf->Release();
        c->ds.buf = NULL;
    }
    if (c->ds.dev) {
        c->ds.dev->Release();
        c->ds.dev = NULL;
    }
    if (c->buffer) {
        free(c->buffer);
    }
    free(c);
}

static int init_ds(struct SPU* c)
{
    HRESULT res;
    HWND hWnd;
    DSBUFFERDESC desc;
    LPDIRECTSOUNDBUFFER tmp = NULL;
    WAVEFORMATEX wFmt;

    res = DirectSoundCreate8(NULL, &c->ds.dev, NULL);
    if (FAILED(res)) {
        return -1;
    }

    hWnd = get_current_hwnd();
    res = c->ds.dev->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
    if (FAILED(res)) {
        return -1;
    }

    memset(&wFmt, 0, sizeof(wFmt));
    wFmt.wFormatTag = WAVE_FORMAT_PCM;
    wFmt.nChannels = c->ch;
    wFmt.nSamplesPerSec = c->bit;
    wFmt.wBitsPerSample = c->sampling;
    wFmt.nBlockAlign = wFmt.nChannels * wFmt.wBitsPerSample / 8;
    wFmt.nAvgBytesPerSec = wFmt.nSamplesPerSec * wFmt.nBlockAlign;
    wFmt.cbSize = 0;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = (DWORD)sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY;
    desc.dwFlags |= DSBCAPS_GLOBALFOCUS;
    desc.dwBufferBytes = c->size;
    desc.lpwfxFormat = &wFmt;
    desc.guid3DAlgorithm = GUID_NULL;
    res = c->ds.dev->CreateSoundBuffer(&desc, &tmp, NULL);
    if (FAILED(res)) {
        return -1;
    }

    res = tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&c->ds.buf);
    tmp->Release();
    if (FAILED(res)) {
        return -1;
    }

    res = c->ds.buf->QueryInterface(IID_IDirectSoundNotify, (void**)&c->ds.ntfy);
    if (FAILED(res)) {
        return -1;
    }

    c->ds.dspn.dwOffset = c->size - 1;
    c->ds.dspn.hEventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
    if ((HANDLE)-1 == c->ds.dspn.hEventNotify || NULL == c->ds.dspn.hEventNotify) {
        return -1;
    }
    res = c->ds.ntfy->SetNotificationPositions(1, &c->ds.dspn);
    if (FAILED(res)) {
        return -1;
    }

    return 0;
}

static HWND get_current_hwnd()
{
    HWND hwndFound;
    char pszNewWindowTitle[1024];
    char pszOldWindowTitle[1024];
    GetConsoleTitle(pszOldWindowTitle, 1024);
    wsprintf(pszNewWindowTitle, "%d/%d", GetTickCount(), GetCurrentProcessId());
    SetConsoleTitle(pszNewWindowTitle);
    Sleep(40);
    hwndFound = FindWindow(NULL, pszNewWindowTitle);
    SetConsoleTitle(pszOldWindowTitle);
    return hwndFound;
}

static void sound_thread(void* context)
{
    struct SPU* c = (struct SPU*)context;
    HRESULT res;
    LPVOID lpBuf;
    DWORD dwSize;

    while (c->alive) {
        dwSize = (DWORD)c->size;
        while (c->alive) {
            res = c->ds.buf->Lock(0, c->buffer, &lpBuf, &dwSize, NULL, NULL, DSBLOCK_FROMWRITECURSOR);
            if (!FAILED(res)) {
                break;
            }
            Sleep(1);
        }
        if (!c->alive) break;
        c->callback(lpBuf, c->size);
        if (FAILED(c->ds.buf->Unlock(lpBuf, dwSize, NULL, NULL))) break;
        ResetEvent(c->ds.dspn.hEventNotify);
        if (FAILED(c->ds.buf->SetCurrentPosition(0))) break;
        while (s->alive) {
            if (!FAILED(c->ds.buf->Play(0, 0, 0))) break;
            Sleep(1);
        }
        while (s->alive) {
            if (WAIT_OBJECT_0 == WaitForSingleObject(c->ds.dspn.hEventNotify, 1000)) {
                break;
            }
        }
    }
}
