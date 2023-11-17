/*
Copyright (c) 2023 Americus Maximus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "AppWindow.hxx"
#include "Logger.hxx"
#include "Sound.hxx"
#include "SoundModule.hxx"

using namespace AppWindow;
using namespace Logger;
using namespace SoundModule;

namespace Sound
{
    SoundContainer SoundState;

    // 0x004e2234
    void SoundServeHandler(void)
    {
        EnterCriticalSection(*SoundState._Mutex);

        (*SoundModuleState._Serve)();

        LeaveCriticalSection(*SoundState._Mutex);
    }

    // 0x004e1d48
    // a.k.a. audiofocushandler
    void SoundWindowHandler(const BOOL mode)
    {
        EnterCriticalSection(*SoundState._Mutex);

        if (mode)
        {
            const HWND hwnd = AcquireWindow();

            if (hwnd == NULL) { Error("audiofocushandler - MAIN WINDOW HANDLE IS INVALID.\n"); }

            (*SoundModuleState._Start)(*SoundState._Options, hwnd);
        }
        else
        {
            (*SoundModuleState._Stop)();
        }

        LeaveCriticalSection(*SoundState._Mutex);
    }

    // 0x0040faf0
    BOOL Is3DSoundAvailable(void)
    {
        return (AcquireSoundCapabilities() & SOUND_MODULE_CAPABILITIES_3D_BUFFER_SUPPORTED) ? TRUE : FALSE;
    }

    // 0x004e21b0
    // a.k.a. SNDcaps
    u32 AcquireSoundCapabilities(void)
    {
        if (*DAT_00a09b08 == NULL) { *DAT_00a09b08 = FUN_00500c80; }

        InitializeSoundModule();

        const HWND hwnd = AcquireWindow();

        if (hwnd == NULL) { Error("SNDcaps - MAIN WINDOW HANDLE IS INVALID.\n"); }

        u32 caps = (*SoundModuleState._AcquireCapabilities)(hwnd);

        if (0 < (s32)caps) { caps = caps | SOUND_MODULE_CAPABILITIES_UNKNOWN; }

        return caps;
    }
}