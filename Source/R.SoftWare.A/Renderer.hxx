/*
Copyright (c) 2024 Americus Maximus

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

#include "DirectDraw.hxx"

#define DEFAULT_DEVICE_AVAIABLE_VIDEO_MEMORY (16 * 1024 * 1024) /* ORIGINAL: 0x200000 (2 MB) */
#define DEFAULT_RENDERER_MODE (-1)
#define DEFAULT_RENDERER_SURFACE_STRIDE (GRAPHICS_RESOLUTION_640 * sizeof(u16))
#define MAX_ACTIVE_SURFACE_COUNT 8
#define MAX_ACTIVE_UNKNOWN_COUNT 4
#define MAX_ACTIVE_USABLE_TEXTURE_FORMAT_COUNT 9
#define MAX_UNKNOWN_COLOR_ARAY_COUNT 16
#define MAX_UNKNOWN_COUNT (MAX_ACTIVE_UNKNOWN_COUNT + 2)
#define MAX_USABLE_TEXTURE_FORMAT_COUNT (MAX_ACTIVE_USABLE_TEXTURE_FORMAT_COUNT + 2)
#define MIN_DEVICE_AVAIABLE_VIDEO_MEMORY (16 * 1024 * 1024) /* ORIGINAL: 0x8000 (32 KB) */
#define RENDERER_SURFACE_ALIGNMENT_MASK 0xffffff00
#define RENDERER_SURFACE_SIZE_MOFIFIER 256

#define RENDERER_CULL_MODE_CLOCK_WISE           0x00000000
#define RENDERER_CULL_MODE_NONE                 0x00000001
#define RENDERER_CULL_MODE_COUNTER_CLOCK_WISE   0x80000000

namespace Renderer
{

}

namespace RendererModule
{
    struct RendererModuleState
    {
        struct
        {
            GUID* ID; // 0x6003e098
            GUID Value; // 0x60040090
        } Device;

        struct
        {
            u32 Bits; // 0x600400c0

            HRESULT Code; // 0x6003e04c

            IDirectDraw2* Instance; // 0x6003e060

            struct
            {
                u32 Bits; // 0x600400cc

                IDirectDrawSurface* Main; // 0x6003e064
                IDirectDrawSurface* Back; // 0x6003e068

                IDirectDrawSurface2* Active[MAX_ACTIVE_SURFACE_COUNT]; // 0x6003e06c

                IDirectDrawSurface2* Window; // 0x6003e08c
            } Surfaces;
        } DX;

        struct
        {
            RendererModuleLambdaContainer Lambdas; // 0x600400a0
        } Lambdas;

        struct
        {
            BOOL IsActive; // 0x6003e0b0

            IDirectDrawSurface2* Surface; // 0x6003e0b4

            RendererModuleWindowLock State; // 0x600400e4
        } Lock;

        HANDLE Mutex; // 0x6003e090

        struct
        {
            struct
            {
                u32 Stride; // 0x6003e104
                u32 Length; // 0x6003e108
                u32 Width; // 0x6003e10c
                u32 Height; // 0x6003e110
                void* Surface; // 0x6003e114
            } Active;

            struct
            {
                u32* Unknown4; // 0x60041348
                u32* Unknown3; // 0x6004134c
                u32* Unknown2; // 0x60041350
                u32* Unknown1; // 0x60041354
            } Colors;

            struct
            {
                void* Surface; // 0x6003e0fc
                void* Allocated; // 0x6003e100
            } Surface;

            struct
            {
                u32 Length; // 0x6003e0f0
                u32 Width; // 0x6003e0f4
                u32 Height; // 0x6003e0f8
            } Settings;
        } Renderer;

        struct
        {
            u32 CooperativeLevel; // 0x6003e050
            BOOL IsWindowMode; // 0x6003e054

            u32 MaxAvailableMemory; // 0x6003e05c
        } Settings;

        struct
        {
            u32 X; // 0x6003e0b8
            u32 Y; // 0x6003e0bc

            u32 Left; // 0x6003e0c0
            u32 Top; // 0x6003e0c8
            u32 Right; // 0x6003e0c4
            u32 Bottom; // 0x6003e0cc
        } ViewPort;

        struct
        {
            u32 Width; // 0x600400c4
            u32 Height; // 0x600400c8
            u32 Bits; // 0x600400d0

            u32 Stride; // 0x600400e0

            HWND HWND; // 0x6003e048
        } Window;
    };

    extern RendererModuleState State;

    void Message(const char* format, ...);

    u32 RendererClearGameWindow(void);
    void* AcquireRendererSurface(void);
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context);
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC desc, LPVOID context);
    u32 AcquirePixelFormat(const DDPIXELFORMAT* format);
    u32 InitializeRendererDeviceLambdas(void);
    u32 ReleaseRendererWindow(void);
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    void ReleaseRendererDeviceSurfaces(void);
    void SelectRendererColorMasks(const u32 bits);
    void SelectRendererSettings(const u32 width, const u32 height, const u32 bits);
}