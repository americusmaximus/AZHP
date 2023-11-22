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

#define DIRECTDRAW_VERSION 0x500
#include <ddraw.h>

#ifdef __WATCOMC__
#undef PURE
#define PURE
#endif

#define DIRECT3D_VERSION 0x500
#include <d3d.h>

#define DDGDI_NONE 0
#define DDEDM_NONE 0
#define D3DDP_NONE 0
#define D3DVBCAPS_NONE 0
#define D3DVBOPTIMIZE_NONE 0
#define DDSDM_NONE 0
#define D3DCOLOR_NONE 0

#define MAX_RENDERER_DEVICE_COUNT 16 /* ORIGINAL: 10 */

#define MAX_DEVICE_NAME_LENGTH 32

#define MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT 128 /* ORIGINAL: 100 */

#define MAX_USABLE_TEXTURE_FORMAT_COUNT 14

namespace Renderer
{
}

namespace RendererModule
{
    struct RendererModuleState
    {
        struct
        {
            struct
            {
                BOOL IsSoft; // 0x60015074
            } Active;
        } DX;

        struct
        {
            GUID* Identifier; // 0x600149bc
        } Device;

        struct
        {
            u32 Count; // 0x600149c0

            GUID* Indexes[MAX_RENDERER_DEVICE_COUNT]; // 0x60014768
            GUID Identifiers[MAX_RENDERER_DEVICE_COUNT]; // 0x60014790
            char Names[MAX_RENDERER_DEVICE_COUNT][MAX_DEVICE_NAME_LENGTH]; // 0x60014830
        } Devices;

        struct
        {
            RendererModuleLambdaContainer Lambdas; // 0x6003bcc0
        } Lambdas;

        struct
        {
            HWND HWND; // 0x6003bce0
        } Window;
    };

    extern RendererModuleState State;

    void SelectRendererDevice(void);
    u32 AcquireRendererDeviceCount(void);
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context);
}