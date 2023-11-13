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

#include "SoundModule.Import.hxx"

#define SOUND_MODULE_NAME "eacsnd.dll"

namespace SoundModule
{
    struct SoundModuleContainer
    {
        struct
        {
            HMODULE* _Handle; // 0x009ef600
        } Module;

        SOUNDMODULEACQUIRECAPABILITIES* _AcquireCapabilities; // 0x009ef61c
        SOUNDMODULESTART* _Start; // 0x009ef620
        SOUNDMODULESTOP* _Stop; // 0x009ef624
        SOUNDMODULESTARTRECORDING* _StartRecording; // 0x009ef628
        SOUNDMODULEPACKETRECORDING* _PacketRecording; // 0x009ef62c
        SOUNDMODULESTOPRECORDING* _StopRecording; // 0x009ef630
        SOUNDMODULESERVE* _Serve; // 0x009ef634
        SOUNDMODULECREATESOUNDBUFFER* _CreateBuffer; // 0x009ef638
        SOUNDMODULEREMOVESOUNDBUFFER* _RemoveBuffer; // 0x009ef63c
        SOUNDMODULEPLAYSOUNDBUFFER* _PlayBuffer; // 0x009ef640
        SOUNDMODULEPOSITIONSOUNDBUFFER* _PositionBuffer; // 0x009ef644
        SOUNDMODULEVOLUMESOUNDBUFFER* _VolumeBuffer; // 0x009ef648
        SOUNDMODULERATESOUNDBUFFER* _RateBuffer; // 0x009ef64c
        SOUNDMODULESTOPSOUNDBUFFER* _StopBuffer; // 0x009ef650
    };

    extern SoundModuleContainer SoundModuleState;

    s32 InitializeSoundModule(void);

    void STDCALLAPI AcquireSoundDataLambda(void* buffer, const u32 size);
    void STDCALLAPI StopSoundBufferLambda(const u32 indx);
    void STDCALLAPI LogSoundLambda(const char* message);
    void STDCALLAPI AcquireSoundBufferPositionLambda(const u32 p1, const u32 p2, f32x3* position);

    typedef const void(*FUN_00500864)(void* buffer, const u32 size); // TODO
    static const FUN_00500864 FUN_00500864a = (FUN_00500864)0x00500864; // TODO

    typedef const void(*FUN_004FFB64)(const u32 indx); // TODO
    static const FUN_004FFB64 FUN_004ffb64 = (FUN_004FFB64)0x004ffb64; // TODO

    typedef const void(*FUN_004FFCE0)(const u32 p1, const u32 p2, f32x3* position);
    static const FUN_004FFCE0 FUN_004ffce0 = (FUN_004FFCE0)0x004ffce0; // TODO

    // 0x004e1ae4
    // a.k.a. iSNDconvertsample
    const static SOUNDMODULECONVERTSOUNDSAMPLELAMBDA ConvertSoundSampleLambda = (SOUNDMODULECONVERTSOUNDSAMPLELAMBDA)0x004e1ae4; // TODO

    const static SOUNDMODULEUNKNOWN4LAMBDA FUN_004e1a64 = (SOUNDMODULEUNKNOWN4LAMBDA)0x004e1a64; // TODO
    const static SOUNDMODULEUNKNOWN5LAMBDA FUN_004e1adc = (SOUNDMODULEUNKNOWN5LAMBDA)0x004e1adc; // TODO
    const static SOUNDMODULEUNKNOWN6LAMBDA FUN_004e1a34 = (SOUNDMODULEUNKNOWN6LAMBDA)0x004e1a34; // TODO
}