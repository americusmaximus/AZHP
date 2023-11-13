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

#pragma once

#include "Basic.hxx"
#include "Native.Basic.hxx"

namespace Sound
{
    struct SoundContainer
    {
        u32* _Options; // 0x009ef608

        LPCRITICAL_SECTION* _Mutex; // 0x00a09b04
    };

    extern SoundContainer SoundState;

    void** DAT_00a09b08; // 0x00a09b08 // TODO

    typedef const void(*FUN_00500C80)(void); // TODO
    FUN_00500C80 FUN_00500c80 = (FUN_00500C80)0x00500c80; // TODO

    void SoundServeHandler(void);
    void SoundWindowHandler(const BOOL mode);
    BOOL Is3DSoundAvailable(void);
    u32 AcquireSoundCapabilities(void);
}