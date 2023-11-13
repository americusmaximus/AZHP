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

#include "Logger.hxx"
#include "SoundModule.hxx"

using namespace Logger;

namespace SoundModule
{
    SoundModuleContainer SoundModuleState;

    // 0x004e1a24
    void STDCALLAPI AcquireSoundDataLambda(void* buffer, const u32 size)
    {
        FUN_00500864a(buffer, size);
    }

    // 0x004e1a00
    void STDCALLAPI StopSoundBufferLambda(const u32 indx)
    {
        FUN_004ffb64(indx);
    }

    // 0x004e1a88
    // a.k.a. abortmessagewrapper
    void STDCALLAPI LogSoundLambda(const char* message)
    {
        // TODO NOT IMPLEMENTED FUN_004f7d70(&DAT_00566f72);

        Message(message);
    }

    // 0x004e1a0c
    void STDCALLAPI AcquireSoundBufferPositionLambda(const u32 p1, const u32 p2, f32x3* position)
    {
        FUN_004ffce0(p1, p2, position);
    }

    // 0x004e1dd4
    // a.k.a. iSNDloaddll
    s32 InitializeSoundModule(void)
    {
        if (*SoundModuleState.Module._Handle != NULL) { return SOUND_MODULE_SUCCESS; }

        *SoundModuleState.Module._Handle = LoadLibraryA(SOUND_MODULE_NAME);

        if (*SoundModuleState.Module._Handle == NULL)
        {
            Message("iSNDloaddll - COULDN'T LOAD/FIND EACSND.DLL. THIS SHOULD BE IN THE SAME DIRECTORY AS THE GAME.\n");

            return SOUND_MODULE_FAILURE;
        }

        {
            const SOUNDMODULEACQUIREVERSION version = (SOUNDMODULEACQUIREVERSION)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_ACQUIRE_VERSION_NAME);

            if (version == NULL)
            {
                Message("iSNDloaddll - COULDN'T FIND iSNDdllversion.\n");

                return SOUND_MODULE_FAILURE;
            }

            {
                const s32 ver = version();

                if (ver != SOUND_MODULE_VERSION)
                {
                    Message("iSNDloaddll - EACSND.DLL VERSION MISMATCH. REQUIRED %i.%i, FOUND %i.%i.\n",
                        SOUND_MODULE_VERSION_MAJOR, SOUND_MODULE_VERSION_MINOR,
                        ACQUIRE_SOUND_MODULE_VERSION_MAJOR(ver), ACQUIRE_SOUND_MODULE_VERSION_MINOR(ver));

                    return SOUND_MODULE_INVALID_VERSION_FAILURE;
                }
            }
        }

        {
            const SOUNDMODULESELECTLAMBDAS lambdas = (SOUNDMODULESELECTLAMBDAS)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_SELECT_LAMBDAS_NAME);

            if (lambdas == NULL)
            {
                Message("iSNDloaddll - COULDN'T FIND setfunctions.\n");

                return SOUND_MODULE_FAILURE;
            }

            lambdas((SOUNDMODULEACQUIREDATALAMBDA)AcquireSoundDataLambda,
                (SOUNDMODULESTOPSOUNDBUFFERLAMBDA)StopSoundBufferLambda,
                (SOUNDMODULELOGMESSAGELAMBDA)LogSoundLambda,
                (SOUNDMODULEUNKNOWN4LAMBDA)FUN_004e1a64,
                (SOUNDMODULEUNKNOWN5LAMBDA)FUN_004e1adc,
                (SOUNDMODULEUNKNOWN6LAMBDA)FUN_004e1a34,
                (SOUNDMODULEACQUIRESOUNDBUFFERPOSITIONLAMBDA)AcquireSoundBufferPositionLambda,
                (SOUNDMODULECONVERTSOUNDSAMPLELAMBDA)ConvertSoundSampleLambda);
        }

        *SoundModuleState._AcquireCapabilities = (SOUNDMODULEACQUIRECAPABILITIES)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_ACQUIRE_CAPABILITIES_NAME);
        *SoundModuleState._Start = (SOUNDMODULESTART)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_START_NAME);
        *SoundModuleState._Stop = (SOUNDMODULESTOP)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_STOP_NAME);
        *SoundModuleState._StartRecording = (SOUNDMODULESTARTRECORDING)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_START_RECORDING_NAME);
        *SoundModuleState._PacketRecording = (SOUNDMODULEPACKETRECORDING)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_PACKET_RECORDING_NAME);
        *SoundModuleState._StopRecording = (SOUNDMODULESTOPRECORDING)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_STOP_RECORDING_NAME);
        *SoundModuleState._Serve = (SOUNDMODULESERVE)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_SERVE_NAME);
        *SoundModuleState._CreateBuffer = (SOUNDMODULECREATESOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_CREATE_SOUND_BUFFER_NAME);
        *SoundModuleState._RemoveBuffer = (SOUNDMODULEREMOVESOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_REMOVE_SOUND_BUFFER_NAME);
        *SoundModuleState._PlayBuffer = (SOUNDMODULEPLAYSOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_PLAY_SOUND_BUFFER_NAME);
        *SoundModuleState._PositionBuffer = (SOUNDMODULEPOSITIONSOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_POSITION_SOUND_BUFFER_NAME);
        *SoundModuleState._VolumeBuffer = (SOUNDMODULEVOLUMESOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_VOLUME_SOUND_BUFFER_NAME);
        *SoundModuleState._RateBuffer = (SOUNDMODULERATESOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_RATE_SOUND_BUFFER_NAME);
        *SoundModuleState._StopBuffer = (SOUNDMODULESTOPSOUNDBUFFER)GetProcAddress(*SoundModuleState.Module._Handle, SOUND_MODULE_STOP_SOUND_BUFFER_NAME);

        if (*SoundModuleState._AcquireCapabilities == NULL || *SoundModuleState._Start == NULL
            || *SoundModuleState._Stop == NULL || *SoundModuleState._StartRecording == NULL
            || *SoundModuleState._PacketRecording == NULL || *SoundModuleState._StopRecording == NULL
            || *SoundModuleState._Serve == NULL || *SoundModuleState._CreateBuffer == NULL
            || *SoundModuleState._RemoveBuffer == NULL || *SoundModuleState._PlayBuffer == NULL
            || *SoundModuleState._PlayBuffer == NULL || *SoundModuleState._PositionBuffer == NULL
            || *SoundModuleState._VolumeBuffer == NULL || *SoundModuleState._RateBuffer == NULL
            || *SoundModuleState._StopBuffer == NULL)
        {
            Message("iSNDloaddll - COULDN'T FIND NEEDED FUNCTION.\n");

            return SOUND_MODULE_FAILURE;
        }


        return SOUND_MODULE_SUCCESS;
    }
}