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

#ifdef __WATCOMC__
#include <RendererModule.Export.hxx>
#else
#include "RendererModule.Export.hxx"
#endif

#define DIRECTDRAW_VERSION 0x700
#include <ddraw.h>

#ifdef __WATCOMC__
#undef PURE
#define PURE
#endif

#define DIRECT3D_VERSION 0x700
#include <d3d.h>

#ifdef __WATCOMC__
#include <mmsystem.h>
#endif

#define DDGDI_NONE 0
#define DDEDM_NONE 0
#define D3DDP_NONE 0
#define D3DVBCAPS_NONE 0
#define D3DVBOPTIMIZE_NONE 0
#define DDSDM_NONE 0
#define D3DCOLOR_NONE 0

#define RENDERER_STATE_INACTIVE 0
#define RENDERER_STATE_ACTIVE 1

#define RENDERER_MODULE_ENVIRONMENT_SECTION_NAME "DX7"

#define MAX_ENUMERATE_RENDERER_DEVICE_COUNT 60 /* ORIGINAL: 16 */
#define MAX_ENUMERATE_RENDERER_DEVICE_NAME_COUNT 60 /* ORIGINAL: 10 */
#define MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH 80

#define MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT 8
#define MAX_RENDERER_MODULE_TEXTURE_STATE_STATE_COUNT 120

namespace Renderer
{

}

namespace RendererModule
{
    struct TextureStageState
    {
        s32 Values[MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT];
    };

    struct RendererModuleState
    {
        struct
        {
            GUID* Identifier; // 0x600186d8
        } Device;

        struct
        {
            u32 Count; // 0x600186e0

            GUID* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60018250

            struct
            {
                u32 Count; // 0x60059000
                BOOL IsAvailable; // 0x60059004

                char Names[MAX_ENUMERATE_RENDERER_DEVICE_NAME_COUNT][MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH]; // 0x60018340

                struct
                {
                    GUID* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058e40
                    GUID Identifiers[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058e80
                } Identifiers;

                struct
                {
                    HMONITOR* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058f80
                    HMONITOR Monitors[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058fc0
                } Monitors;

                DDDEVICEIDENTIFIER Identifier; // 0x60059010
            } Enumeration;
        } Devices;

        struct
        {
            TextureStageState StageStates[MAX_RENDERER_MODULE_TEXTURE_STATE_STATE_COUNT]; // 0x6007b8a0
        } Textures;
    };

    extern RendererModuleState State;

    BOOL AcquireRendererDeviceAccelerationState(const u32 indx);
    BOOL CALLBACK EnumerateDirectDrawDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context, HMONITOR monitor);
    s32 AcquireSettingsValue(const s32 value, const char* section, const char* name);
    u32 AcquireDirectDrawDeviceCount(GUID** uids, HMONITOR** monitors, const char* section);
    u32 AcquireRendererDeviceCount(void);
    void InitializeTextureStateStates(void);
}