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
#include "Direct.Sound.Basic.hxx"
#include "Sound.Basic.hxx"
#include "SoundModule.Basic.hxx"
#include "SoundModule.Export.hxx"

#define SOUND_BITS_0 0
#define SOUND_BITS_8 8
#define SOUND_BITS_16 16

#define SOUND_CHANNELS_0 0
#define SOUND_CHANNELS_1 1
#define SOUND_CHANNELS_2 2

#define SOUND_FREQUENCY_0 0

#define SOUND_FREQUENCY_11025 11025
#define SOUND_FREQUENCY_16000 16000
#define SOUND_FREQUENCY_22050 22050
#define SOUND_FREQUENCY_32000 32000
#define SOUND_FREQUENCY_44100 44100

#define INVALID_SOUND_SLOT_INDEX 0
#define INVALID_SOUND_SLOT_ID 0

#define MAX_SOUND_BUFFER_COUNT 16

#define MIN_VALID_SOUND_SLOT_COUNT 1
#define MAX_SOUND_SLOT_COUNT 64

#define SOUNDVOLUMEVALUELOW(value) ((value << 16) >> 16)

#define SOUNDVOLUMEVALUEHIGH(value) (value >> 16)

#define MAX_SOUND_VOLUME_COUNT 64

#define SOUND_VOLUME_INDEX_MULTIPLIER 2

#define DEFAULT_SOUND_BUFFER_BLOCK_ALIGN 2

#define SOUND_FREQUENCY_MULTIPLIER 1000

#define SOUND_PLAY_ITERATION_STEP 16

#define DEFAULT_SOFTWARE_SOUND_BUFFER_SIZE 40960

#define SOUND_BUFFER_LENGTH_MASK 0xf

#define SOUND_BUFFER_POSITION_MULTIPLIER (1.0 / 65536.0)

#define SOUND_BUFFER_POSITION_ALIGNMENT_MASK 0xffff0
#define SOUND_BUFFER_WRITE_ALIGNMENT_MASK 0xfffff0

namespace SoundModule
{
    typedef enum SOUND_MODE
    {
        SOUND_MODE_SOFTWARE = 0,
        SOUND_MODE_HARDWARE = 1
    } SOUND_MODE;

    struct SoundSlot
    {
        u32 ID;

        IDirectSoundBuffer* Buffer;

        s32 Unk03; // TODO
        s32 Unk04; // TODO
        s32 Unk05; // TODO

        u16 Frequency;
        u8 Unk07;
        u8 Volume;
        u8 Unk10;
        u8 Channels;
        u8 Unk12;
        u8 Unk13;
    };

    struct SoundBuffer
    {
        struct
        {
            IDirectSoundBuffer* Active;
            IDirectSoundBuffer* Main;
            IDirectSound3DBuffer* Volume;
        } Buffers;

        u16 Unk05; // TODO
        u16 Unk06; // TODO

        struct
        {
            u16 Unk1;
            bool Unk2;
            u8 Unk3;
            u8 Unk4;
            u8 Unk5;
            u8 Unk6; // TODO
            u8 Unk7; // TODO
        } Settings;
    };

    struct ModuleContainer
    {
        struct
        {
            IDirectSound* Instance; // 0x0040a89c

            struct
            {
                IDirectSoundBuffer* Main; // 0x0040a8a0
                IDirectSoundBuffer* Software; // 0x0040a8a4
                IDirectSoundBuffer* Active; // 0x0040a8a8
            } Buffers;
        } DX;

        struct
        {
            bool IsInit; // 0x0040a8b0
            bool IsActive; // 0x0040a8b1
            bool IsLive; // 0x0040a8b2
        } State;

        struct
        {
            u8 BlockAlignBits; // 0x0040a8b3
            u8 BlockAlignBytes; // 0x0040a8b4
            bool IsEmulated; // 0x0040a8b5

            bool Is3D; // 0x0040a8b7
            u32 Unk01; // 0x0040a8b8
            u32 Unk02; // 0x0040a8bc
            DWORD CurrentWrite; // 0x0040a8c0
            DWORD MinimumWrite; // 0x0040a8c4
            u32 Unk05; // 0x0040a8c8
            s32 MaximumPlay; // 0x0040a8cc
            s32 MinimumPlay; // 0x0040a8d0

            u32 BufferSize; // 0x0040a8d8
            u32 Unk09; // 0x0040a8dc
            u32 Unk10; // 0x0040a8e0
            u32 Unk11; // 0x0040a8e4
            u32 Unk12; // 0x0040a8e8
            u32 Unk13; // 0x0040a8ec

            struct
            {
                void* Data; // 0x0040a8f0
                DWORD Size; // 0x0040a8f4
            } Lock;

            s32 Unk16; //0x0040a8f8
            u32 Capabilities; // 0x0040a8fc
            u32 Frequency; // 0x0040a900
        } Settings;

        struct
        {
            SOUNDMODULEACQUIREDATALAMBDA AcquireData; // 0x0040a904
            SOUNDMODULESTOPSOUNDBUFFERLAMBDA StopBuffer; // 0x0040a908
            SOUNDMODULELOGMESSAGELAMBDA Log; // 0x0040a90c
            SOUNDMODULEUNKNOWN4LAMBDA Lambda4; // 0x0040a910
            SOUNDMODULEUNKNOWN5LAMBDA Lambda5; // 0x0040a914
            SOUNDMODULEUNKNOWN6LAMBDA Lambda6; // 0x0040a918
            SOUNDMODULEACQUIRESOUNDBUFFERPOSITIONLAMBDA AcquireSoundBufferPosition; // 0x0040a91c
            SOUNDMODULECONVERTSOUNDSAMPLELAMBDA ConvertSoundSample; // 0x0040a920
        } Lambdas;

        SoundBuffer Buffers[MAX_SOUND_BUFFER_COUNT]; // 0x0040a924
        SoundSlot Slots[MAX_SOUND_SLOT_COUNT]; // 0x0040aaa4

        struct
        {
            IDirectSound3DListener* Listener; // 0x0040a8ac
        } EAX;

        struct
        {
            u32 Unknown1; // 0x0040b2c4
            u32 Unknown2; // 0x0040b2c8
        } Counters;
    };

    extern ModuleContainer State;
    extern const u32 Volume[MAX_SOUND_VOLUME_COUNT];

    void Message(const char* format, ...);

    s32 InitializeSoundBuffers(void);
    s32 InitializeSounds(const SOUND_MODE mode, const u32 options, const HWND hwnd);
    s32 InitializeSoundSlot(const u32 indx);
    s32 InitializeSoundSlots(void);
    u32 AcquireAvailableSoundSlotIndex(void);
    u32 AcquireSoundPosition(void);
    void AcquireSoundData(void* buffer, const s32 size);
    void StopSoundBuffers(void);
}