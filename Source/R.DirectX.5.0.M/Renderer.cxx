/*
Copyright (c) 2023 - 2025 Americus Maximus

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

#include "Graphics.Basic.hxx"
#include "Renderer.hxx"
#include "RendererValues.hxx"
#include "Settings.hxx"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_MESSAGE_BUFFER_LENGTH 512

using namespace Renderer;
using namespace RendererModuleValues;
using namespace Settings;

namespace RendererModule
{
    RendererModuleState State;

    // 0x60005300
    // NOTE: In the original renderer the method does not perform any meaningful actions.
    void Message(const char* format, ...) { }

    // 0x600026b4
    BOOL Error(const char* format, ...)
    {
        if (!State.IsError)
        {
            State.IsError = TRUE;

            char buffer[MAX_MESSAGE_BUFFER_LENGTH];

            if (format == NULL) { buffer[0] = NULL; }
            else
            {
                va_list args;
                va_start(args, format);
                vsnprintf_s(buffer, MAX_MESSAGE_BUFFER_LENGTH, format, args);
                va_end(args);
            }

            OutputDebugStringA(buffer);
            Message("%s\n", buffer);

            RestoreGameWindow();

            if (State.Lambdas.Log != NULL) { State.Lambdas.Log(RENDERER_MODULE_MESSAGE_SEVERITY_ERROR, buffer); }

            exit(EXIT_SUCCESS);

        }

        return TRUE;
    }

    // 0x60001618
    void SelectRendererDevice(void)
    {
        if (RendererDeviceIndex < DEFAULT_DEVICE_INDEX
            && (State.Lambdas.AcquireWindow != NULL || State.Window.HWND != NULL))
        {
            const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONMENT_PROPERTY_NAME);

            SelectDevice(value == NULL ? DEFAULT_DEVICE_INDEX : atoi(value));
        }
    }

    // 0x600015e8
    u32 AcquireRendererDeviceCount(void)
    {
        State.Devices.Count = 0;
        State.Device.Identifier = NULL;

        u32 indx = DEFAULT_DEVICE_INDEX;
        DirectDrawEnumerateA(EnumerateRendererDevices, &indx);

        return State.Devices.Count;
    }

    // 0x60001734
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context)
    {
        if (uid == NULL)
        {
            State.Devices.Indexes[State.Devices.Count] = NULL;
        }
        else
        {
            State.Devices.Indexes[State.Devices.Count] = &State.Devices.Identifiers[State.Devices.Count];
            State.Devices.Identifiers[State.Devices.Count] = *uid;
        }

        if (State.Devices.Count == *(u32*)context)
        {
            State.Device.Identifier = State.Devices.Indexes[State.Devices.Count];
        }

        strncpy(State.Devices.Names[State.Devices.Count], name, MAX_DEVICE_NAME_LENGTH);

        // NOTE: Additional extra check to prevent writes outside of the array bounds.
        if (MAX_DEVICE_COUNT <= (State.Devices.Count + 1)) { return FALSE; }

        State.Devices.Count = State.Devices.Count + 1;

        return TRUE;
    }

    // 0x60002048
    u32 InitializeRendererDevice(void)
    {
        if (State.Window.HWND != NULL)
        {
            State.Settings.CooperativeLevel = State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_NORMAL | DDSCL_FULLSCREEN);

            State.Settings.CooperativeLevel = State.Settings.IsWindowMode
                ? State.Settings.CooperativeLevel | DDSCL_SETFOCUSWINDOW | DDSCL_ALLOWMODEX | DDSCL_NORMAL
                : State.Settings.CooperativeLevel | DDSCL_SETFOCUSWINDOW | DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

            IDirectDraw* instance;
            State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

            if (State.DX.Code == DD_OK)
            {
                State.DX.Code = instance->QueryInterface(IID_IDirectDraw2, (void**)&State.DX.Instance);

                instance->Release();

                if (State.DX.Code == DD_OK)
                {
                    State.DX.Code = State.DX.Instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

                    if (State.DX.Code == DD_OK)
                    {
                        {
                            DWORD total = 0, free = 0;

                            {
                                DDSCAPS caps;
                                ZeroMemory(&caps, sizeof(DDSCAPS));

                                caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                                const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                                ModuleDescriptor.VideoMemorySize = result == DD_OK ? free : 0;
                            }

                            {

                                DDSCAPS caps;
                                ZeroMemory(&caps, sizeof(DDSCAPS));

                                caps.dwCaps = DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                                const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                                ModuleDescriptor.TotalMemorySize = result == DD_OK ? total : 0; // TODO
                            }
                        }

                        {
                            DDCAPS hal;
                            ZeroMemory(&hal, sizeof(DDCAPS));

                            hal.dwSize = sizeof(DDCAPS);

                            DDCAPS hel;
                            ZeroMemory(&hel, sizeof(DDCAPS));

                            hel.dwSize = sizeof(DDCAPS);

                            if (State.DX.Instance->GetCaps(&hal, &hel) == DD_OK)
                            {
                                State.Device.Capabilities.IsAccelerated = hal.dwCaps & DDCAPS_3D;
                            }
                        }

                        ZeroMemory(ModuleDescriptor.Capabilities.Capabilities,
                            MAX_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));

                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Width = GRAPHICS_RESOLUTION_640;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Height = GRAPHICS_RESOLUTION_480;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Bits = GRAPHICS_BITS_PER_PIXEL_16;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Format = RENDERER_PIXEL_FORMAT_R5G6B5;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Unk03 = 1;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Unk04 = 1;
                        ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].IsActive = TRUE;

                        ModuleDescriptor.Capabilities.Count = 2;

                        State.DX.Instance->EnumDisplayModes(DDEDM_NONE, NULL,
                            &ModuleDescriptor.Capabilities.Count, EnumerateRendererDeviceModes);

                        return RENDERER_MODULE_SUCCESS;
                    }
                }
            }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600017bc
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC desc, LPVOID context)
    {
        const u32 format = AcquirePixelFormat(&desc->ddpfPixelFormat);

        if (format != RENDERER_PIXEL_FORMAT_NONE)
        {
            if (desc->dwWidth < GRAPHICS_RESOLUTION_640 || desc->dwHeight < GRAPHICS_RESOLUTION_480) { return DDENUMRET_OK; }

            const u32 bits = desc->ddpfPixelFormat.dwRGBBitCount;
            const u32 bytes = bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1) ? 2 : (bits >> 3);

            const u32 width = desc->dwWidth;
            const u32 height = desc->dwHeight;

            u32 indx = 0;

            {
                indx = *(u32*)context;

                if ((MAX_DEVICE_CAPABILITIES_COUNT - 1) < indx) { return DDENUMRET_CANCEL; }

                *(u32*)context = indx + 1;
            }

            const u32 count = State.Settings.MaxAvailableMemory / (height * width * bytes);

            ModuleDescriptor.Capabilities.Capabilities[indx].Width = width;
            ModuleDescriptor.Capabilities.Capabilities[indx].Height = height;
            ModuleDescriptor.Capabilities.Capabilities[indx].Bits =
                format == RENDERER_PIXEL_FORMAT_R5G5B5 ? (GRAPHICS_BITS_PER_PIXEL_16 - 1) : bits;
            ModuleDescriptor.Capabilities.Capabilities[indx].IsActive = TRUE;

            if (count < 4) // TODO
            {
                ModuleDescriptor.Capabilities.Capabilities[indx].Unk03 = count;
                ModuleDescriptor.Capabilities.Capabilities[indx].Unk04 = count - 1;
            }
            else
            {
                ModuleDescriptor.Capabilities.Capabilities[indx].Unk03 = 3;
                ModuleDescriptor.Capabilities.Capabilities[indx].Unk04 = 3;
            }
        }

        return DDENUMRET_OK;
    }

    // 0x600025f0
    u32 AcquirePixelFormat(const DDPIXELFORMAT* format)
    {
        const u32 bits = format->dwRGBBitCount;

        const u32 red = format->dwRBitMask;
        const u32 green = format->dwGBitMask;
        const u32 blue = format->dwBBitMask;

        if (bits == GRAPHICS_BITS_PER_PIXEL_16)
        {
            if (red == 0x7c00 && green == 0x3e0 && blue == 0x1f) { return RENDERER_PIXEL_FORMAT_R5G5B5; }
            else if (red == 0xf800 && green == 0x7e0 && blue == 0x1f) { return RENDERER_PIXEL_FORMAT_R5G6B5; }
            else if (red == 0xf00 && green == 0xf0 && blue == 0xf && format->dwRGBAlphaBitMask == 0xf000) { return RENDERER_PIXEL_FORMAT_R4G4B4; }
        }
        else if (red == 0xff0000 && green == 0xff00 && blue == 0xff)
        {
            if (bits == GRAPHICS_BITS_PER_PIXEL_24) { return RENDERER_PIXEL_FORMAT_R8G8B8; }
            else if (bits == GRAPHICS_BITS_PER_PIXEL_32) { return RENDERER_PIXEL_FORMAT_A8R8G8B8; }
        }

        return RENDERER_PIXEL_FORMAT_NONE;
    }

    // 0x600022b0
    u32 InitializeRendererDeviceLambdas(void)
    {
        if (State.Mutex == NULL) { State.Mutex = CreateEventA(NULL, FALSE, FALSE, NULL); }

        State.Window.HWND = State.Lambdas.AcquireWindow();

        if (State.Window.HWND != NULL)
        {
            State.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceExecute);
            State.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_RELEASE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)ReleaseRendererDeviceExecute);
            State.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceSurfacesExecute);

            State.Settings.CooperativeLevel = State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_NORMAL | DDSCL_FULLSCREEN);

            State.Settings.CooperativeLevel = State.Settings.IsWindowMode
                ? State.Settings.CooperativeLevel | DDSCL_NORMAL
                : State.Settings.CooperativeLevel | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

            SetForegroundWindow(State.Window.HWND);
            PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, 0, 0);
            WaitForSingleObject(State.Mutex, INFINITE);

            return State.DX.Code;
        }

        State.DX.Code = RENDERER_MODULE_INITIALIZE_DEVICE_SUCCESS;

        return RENDERER_MODULE_INITIALIZE_DEVICE_SUCCESS;
    }

    // 0x600018e4
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        IDirectDraw* instance;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

        if (State.DX.Code == DD_OK)
        {
            State.DX.Code = instance->QueryInterface(IID_IDirectDraw2, (void**)&State.DX.Instance);

            State.Lambdas.SelectInstance(instance);

            instance->Release();

            State.DX.Code = State.DX.Instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

            if (State.DX.Code == DD_OK)
            {
                u32 pitch = 0;
                u32 height = 0;

                {
                    DDSURFACEDESC desc;
                    ZeroMemory(&desc, sizeof(DDSURFACEDESC));

                    desc.dwSize = sizeof(DDSURFACEDESC);
                    desc.dwFlags = DDSD_CAPS;
                    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

                    IDirectDrawSurface* surface = NULL;
                    State.DX.Code = State.DX.Instance->CreateSurface(&desc, &surface, NULL);

                    if (State.DX.Code == DD_OK)
                    {
                        surface->GetSurfaceDesc(&desc);

                        DDCAPS hal;
                        ZeroMemory(&hal, sizeof(DDCAPS));

                        hal.dwSize = sizeof(DDCAPS);
                        hal.dwCaps = DDCAPS_BLTDEPTHFILL | DDCAPS_OVERLAYSTRETCH;

                        DDCAPS hel;
                        ZeroMemory(&hel, sizeof(DDCAPS));

                        hel.dwSize = sizeof(DDCAPS);

                        State.DX.Instance->GetCaps(&hal, &hel);

                        pitch = desc.lPitch;
                        height = desc.dwHeight;
                    }
                    else
                    {
                        Message("*** FAILURE in creating primary surface***\n");
                    }

                    if (surface != NULL) { surface->Release(); }
                }

                {
                    DWORD free = 0;
                    DWORD total = 0;

                    {
                        DDSCAPS caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS));

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;

                        const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                        State.Settings.MaxAvailableMemory = result == DD_OK
                            ? height * pitch + total
                            : MIN_DEVICE_AVAIABLE_VIDEO_MEMORY;
                    }

                    {
                        DDSCAPS caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS));

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                        if (State.DX.Instance->GetAvailableVidMem(&caps, &total, &free) == DD_OK)
                        {
                            ModuleDescriptor.TotalMemorySize = 3; // TODO
                            ModuleDescriptor.VideoMemorySize = total;
                        }
                        else
                        {
                            ModuleDescriptor.TotalMemorySize = 0; // TODO
                            ModuleDescriptor.VideoMemorySize = 0;
                        }
                    }

                    {
                        DDSCAPS caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS));

                        caps.dwCaps = DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                        if (State.DX.Instance->GetAvailableVidMem(&caps, &total, &free) == DD_OK)
                        {
                            if (total != 0) { ModuleDescriptor.TotalMemorySize = ModuleDescriptor.TotalMemorySize | 0x8000; } // TODO
                        }
                    }
                }

                {
                    DDCAPS hal;
                    ZeroMemory(&hal, sizeof(DDCAPS));

                    hal.dwSize = sizeof(DDCAPS);

                    DDCAPS hel;
                    ZeroMemory(&hel, sizeof(DDCAPS));

                    hel.dwSize = sizeof(hel);

                    if (State.DX.Instance->GetCaps(&hal, &hel) == DD_OK)
                    {
                        State.Device.Capabilities.IsAccelerated = hal.dwCaps & DDCAPS_3D;
                    }
                }

                ZeroMemory(ModuleDescriptor.Capabilities.Capabilities,
                    MAX_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));

                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Width = GRAPHICS_RESOLUTION_640;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Height = GRAPHICS_RESOLUTION_480;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Bits = GRAPHICS_BITS_PER_PIXEL_16;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Format = RENDERER_PIXEL_FORMAT_R5G6B5;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Unk03 = 3;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].Unk04 = 2;
                ModuleDescriptor.Capabilities.Capabilities[RENDERER_RESOLUTION_MODE_640_480_16].IsActive = TRUE;

                ModuleDescriptor.Capabilities.Count = 2;

                State.DX.Instance->EnumDisplayModes(DDEDM_NONE, NULL,
                    &ModuleDescriptor.Capabilities.Count, EnumerateRendererDeviceModes);
            }
        }

        SetEvent(State.Mutex);

        *result = State.DX.Code;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002010
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDeviceSurfaces();

        State.Lambdas.SelectInstance(NULL);

        State.DX.Instance->Release();
        State.DX.Instance = NULL;

        SetEvent(State.Mutex);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x6000242c
    void ReleaseRendererDeviceSurfaces(void)
    {
        for (s32 x = (MAX_ACTIVE_SURFACE_COUNT - 1); x >= 0; x--)
        {
            if (State.DX.Surfaces.Active[x] != NULL)
            {
                State.DX.Surfaces.Active[x]->Release();
                State.DX.Surfaces.Active[x] = NULL;
            }
        }

        if (State.DX.Surfaces.Back != NULL)
        {
            State.DX.Surfaces.Back->Release();
            State.DX.Surfaces.Back = NULL;
        }

        if (State.DX.Surfaces.Main != NULL)
        {
            State.DX.Surfaces.Main->Release();
            State.DX.Surfaces.Main = NULL;
        }
    }

    // 0x60001c74
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDevice();
        ReleaseRendererDeviceSurfaces();

        State.Window.Width = ModuleDescriptor.Capabilities.Capabilities[wp].Width;
        State.Window.Height = ModuleDescriptor.Capabilities.Capabilities[wp].Height;

        const u32 bits = ModuleDescriptor.Capabilities.Capabilities[wp].Bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1)
            ? GRAPHICS_BITS_PER_PIXEL_16 : ModuleDescriptor.Capabilities.Capabilities[wp].Bits;

        State.DX.Code = DD_OK;

        if (!State.Settings.IsWindowMode)
        {
            State.DX.Code = State.DX.Instance->SetDisplayMode(ModuleDescriptor.Capabilities.Capabilities[wp].Width,
                ModuleDescriptor.Capabilities.Capabilities[wp].Height, bits, 0, DDSDM_NONE);

            if (State.DX.Code == DD_OK)
            {
                DDSURFACEDESC desc;
                ZeroMemory(&desc, sizeof(DDSURFACEDESC));

                desc.dwSize = sizeof(DDSURFACEDESC);
                desc.dwFlags = DDSD_BACKBUFFERCOUNT | DDSD_CAPS;

                desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
                    ? DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
                    : DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

                desc.dwBackBufferCount = lp - 1;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);

                while (State.DX.Code == DDERR_OUTOFVIDEOMEMORY && 1 < desc.dwBackBufferCount)
                {
                    desc.dwBackBufferCount = desc.dwBackBufferCount - 1;

                    State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);
                }

                if (State.DX.Surfaces.Main != NULL && State.DX.Surfaces.Back == NULL)
                {
                    DDSCAPS caps;
                    ZeroMemory(&caps, sizeof(DDSCAPS));

                    caps.dwCaps = DDSCAPS_BACKBUFFER;

                    State.DX.Code = State.DX.Surfaces.Main->GetAttachedSurface(&caps, &State.DX.Surfaces.Back);
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface2, (void**)&State.DX.Surfaces.Active[1]); // TODO
                }

                if (State.DX.Surfaces.Back != NULL)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface2, (void**)&State.DX.Surfaces.Active[2]); // TODO
                }
            }
        }
        else
        {
            DDSURFACEDESC desc;
            ZeroMemory(&desc, sizeof(DDSURFACEDESC));

            desc.dwSize = sizeof(DDSURFACEDESC);
            desc.dwFlags = DDSD_CAPS;
            desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

            State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);

            if (State.DX.Code == DD_OK)
            {
                State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface2, (void**)&State.DX.Surfaces.Active[1]); // TODO

                desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
                desc.dwHeight = State.Window.Height;
                desc.dwWidth = State.Window.Width;
                desc.ddsCaps.dwCaps = RendererDeviceType == RENDERER_MODULE_DEVICE_TYPE_0_ACCELERATED
                    ? DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN
                    : DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Back, NULL);

                if (State.DX.Code == DD_OK)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface2, (void**)&State.DX.Surfaces.Active[2]); // TODO
                }
                else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { Message("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
                else { Message("CreateSurface for window back buffer failed %8x.\n", State.DX.Code); }
            }
            else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { Message("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
            else { Message("CreateSurface for window back buffer failed %8x.\n", State.DX.Code); }
        }

        InitializeRendererDeviceAcceleration();

        if (State.Lambdas.AcquireWindow != NULL)
        {
            SetEvent(State.Mutex);

            *result = State.DX.Code;
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003838
    void ReleaseRendererDevice(void)
    {
        if (State.DX.Active.IsInit && State.Scene.IsActive)
        {
            FlushGameWindow();
            SyncGameWindow(RENDERER_MODULE_SYNC_NORMAL);
            Idle();

            State.Scene.IsActive = FALSE;
        }

        ResetTextures();

        State.DX.Active.Instance = NULL;

        State.DX.Active.Surfaces.Main = NULL;
        State.DX.Active.Surfaces.Back = NULL;
        State.DX.Active.Surfaces.Active.Main = NULL;
        State.DX.Active.Surfaces.Active.Back = NULL;

        if (State.DX.Active.Surfaces.Active.Depth != NULL)
        {
            State.DX.Active.Surfaces.Active.Depth->Release();
            State.DX.Active.Surfaces.Active.Depth = NULL;
        }

        if (State.DX.Active.Surfaces.Depth != NULL)
        {
            State.DX.Active.Surfaces.Depth->Release();
            State.DX.Active.Surfaces.Depth = NULL;
        }

        if (State.DX.Material != NULL)
        {
            State.DX.Material->Release();
            State.DX.Material = NULL;
        }

        if (State.DX.ViewPort != NULL)
        {
            State.DX.ViewPort->Release();
            State.DX.ViewPort = NULL;
        }

        if (State.DX.Device != NULL)
        {
            State.DX.Device->Release();
            State.DX.Device = NULL;
        }

        if (State.DX.DirectX != NULL)
        {
            State.DX.DirectX->Release();
            State.DX.DirectX = NULL;
        }

        State.DX.Active.IsInit = FALSE;
    }

    // 0x60003a3c
    // a.k.a. createD3D
    u32 InitializeRendererDeviceAcceleration(void)
    {
        State.DX.Active.Instance = State.DX.Instance;

        State.DX.Active.Surfaces.Main = State.DX.Surfaces.Main;
        State.DX.Active.Surfaces.Back = State.DX.Surfaces.Back;

        State.DX.Active.Surfaces.Active.Main = State.DX.Surfaces.Active[1]; // TODO
        State.DX.Active.Surfaces.Active.Back = State.DX.Surfaces.Active[2]; // TODO

        if (State.DX.Instance->QueryInterface(IID_IDirect3D2, (void**)&State.DX.DirectX) != DD_OK)
        {
            Error("Creation of IDirect3D2 failed.\nCheck DX5 installed.\n");
        }

        InitializeConcreteRendererDevice();

        D3DDEVICEDESC hal;
        ZeroMemory(&hal, sizeof(D3DDEVICEDESC));

        hal.dwSize = sizeof(D3DDEVICEDESC);

        D3DDEVICEDESC hel;
        ZeroMemory(&hel, sizeof(D3DDEVICEDESC));

        hel.dwSize = sizeof(D3DDEVICEDESC);

        State.DX.Device->GetCaps(&hal, &hel);

        if (hal.dcmColorModel == D3DCOLOR_NONE)
        {
            State.Device.Capabilities.RendererBits = hel.dwDeviceRenderBitDepth;
            State.Device.Capabilities.DepthBits = hel.dwDeviceZBufferBitDepth;
            State.Device.Capabilities.IsPerspectiveTextures = ((hel.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) != 0);
            State.Device.Capabilities.IsAlphaTextures = FALSE;
            State.Device.Capabilities.IsAlphaBlend = FALSE;
            State.Device.Capabilities.IsModulateBlending = FALSE;
            State.Device.Capabilities.IsSourceAlphaBlending = FALSE;
            State.Device.Capabilities.IsColorBlending = FALSE;
            State.Device.Capabilities.IsSpecularBlending = FALSE;
        }
        else
        {
            State.Device.Capabilities.RendererBits = hal.dwDeviceRenderBitDepth;
            State.Device.Capabilities.DepthBits = hal.dwDeviceZBufferBitDepth;

            State.Device.Capabilities.ResterOperationCaps = (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_XOR) != 0;

            if (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_PAT)
            {
                State.Device.Capabilities.ResterOperationCaps = State.Device.Capabilities.ResterOperationCaps | RENDERER_MODULE_RASTER_OPERATION_CAPABILITIES_PATTERN;
            }

            State.Device.Capabilities.IsPerspectiveTextures = ((hal.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) != 0);

            State.Device.Capabilities.IsAlphaTextures = ((hal.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA) != 0);

            State.Device.Capabilities.IsAlphaBlend = ((hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND) != 0
                || (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND) != 0
                || (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATBLEND) != 0);

            State.Device.Capabilities.IsNonFlatAlphaBlend = ((hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND) != 0
                || (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND) != 0);

            State.Device.Capabilities.IsModulateBlending = (hal.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA) != 0;

            State.Device.Capabilities.IsSourceAlphaBlending = ((hal.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) != 0 && (hal.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE) != 0);

            State.Device.Capabilities.IsColorBlending = (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) != 0 && (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORPHONGRGB) != 0;

            State.Device.Capabilities.IsSpecularBlending = (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB) != 0 || (hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGRGB) != 0;

            if (!State.Device.Capabilities.IsColorBlending) { State.Device.Capabilities.IsSourceAlphaBlending = FALSE; }
        }

        if ((State.Device.Capabilities.DepthBits & RENDERER_MODULE_DEPTH_BIT_MASK_32_BIT) == 0)
        {
            if ((State.Device.Capabilities.DepthBits & RENDERER_MODULE_DEPTH_BIT_MASK_24_BIT) == 0)
            {
                if ((State.Device.Capabilities.DepthBits & RENDERER_MODULE_DEPTH_BIT_MASK_16_BIT) == 0)
                {
                    if ((State.Device.Capabilities.DepthBits & RENDERER_MODULE_DEPTH_BIT_MASK_8_BIT) == 0)
                    {
                        State.Device.Capabilities.DepthBits = 0;
                    }
                    else
                    {
                        State.Device.Capabilities.DepthBits = GRAPHICS_BITS_PER_PIXEL_8;
                    }
                }
                else
                {
                    State.Device.Capabilities.DepthBits = GRAPHICS_BITS_PER_PIXEL_16;
                }
            }
            else
            {
                State.Device.Capabilities.DepthBits = GRAPHICS_BITS_PER_PIXEL_24;
            }
        }
        else
        {
            State.Device.Capabilities.DepthBits = GRAPHICS_BITS_PER_PIXEL_32;
        }

        if (InitializeRendererDeviceDepthSurfaces(State.Window.Width, State.Window.Height))
        {
            State.Device.Capabilities.IsDepthAvailable = TRUE;
        }

        State.DX.Device->Release();

        InitializeConcreteRendererDevice();

        AcquireRendererDeviceTextureFormats();

        State.DX.DirectX->CreateViewport(&State.DX.ViewPort, NULL);
        State.DX.Device->AddViewport(State.DX.ViewPort);

        D3DVIEWPORT2 vp;
        ZeroMemory(&vp, sizeof(D3DVIEWPORT2));

        vp.dwSize = sizeof(D3DVIEWPORT2);
        vp.dwX = 0;
        vp.dwY = 0;
        vp.dwWidth = State.Window.Width;
        vp.dwHeight = State.Window.Height;
        vp.dvClipX = -1.0f;
        vp.dvClipY = 0.5f * State.Window.Height;
        vp.dvClipWidth = 2.0f;
        vp.dvClipHeight = (2.0f * State.Window.Height) / (f32)State.Window.Width;
        vp.dvMinZ = 0.0f;
        vp.dvMaxZ = 1.0f;

        State.DX.ViewPort->SetViewport2(&vp);

        State.DX.DirectX->CreateMaterial(&State.DX.Material, NULL);

        D3DMATERIAL material;
        ZeroMemory(&material, sizeof(D3DMATERIAL));

        material.dwSize = sizeof(D3DMATERIAL);
        material.dwRampSize = 1;

        State.DX.Material->SetMaterial(&material);

        D3DMATERIALHANDLE handle;
        State.DX.Material->GetHandle(State.DX.Device, &handle);

        State.DX.ViewPort->SetBackground(handle);

        InitializeRendererState();
        InitializeViewPort();

        State.DX.Active.IsInit = TRUE;

        return TRUE;
    }

    // 0x6000395c
    void InitializeConcreteRendererDevice(void)
    {
        HRESULT result = DD_OK;

        if (SettingsState.Accelerate)
        {
            const HRESULT res = State.DX.DirectX->CreateDevice(IID_IDirect3DHALDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            if (res == DD_OK) { return; }
        }

        switch (RendererDeviceType)
        {
        case RENDERER_MODULE_DEVICE_TYPE_0_RAMP:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRampDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_0_RGB:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRGBDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_0_MMX:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DMMXDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_0_INVALID:
        case RENDERER_MODULE_DEVICE_TYPE_0_ACCELERATED:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DHALDevice,
                State.DX.Active.Surfaces.Back != NULL ? State.DX.Active.Surfaces.Back : State.DX.Active.Surfaces.Main, &State.DX.Device);

            break;
        }
        default: { Error("D3D device type not recognised\n"); return; }
        }

        if (result != DD_OK) { Error("IDirect3D2_CreateDevice failed.\n"); }
    }

    // 0x60003ee0
    // a.k.a. createzbuffer
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height)
    {
        if (State.Device.Capabilities.DepthBits == 0)
        {
            Message("zbuffer depth = 0\n");

            return FALSE;
        }

        DDSURFACEDESC desc;
        ZeroMemory(&desc, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);
        desc.dwFlags = DDSD_ZBUFFERBITDEPTH | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;

        desc.dwHeight = height;
        desc.dwWidth = width;
        desc.dwZBufferBitDepth = State.Device.Capabilities.DepthBits;

        desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
            ? DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY
            : DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;

        State.DX.Active.Instance->CreateSurface(&desc, &State.DX.Active.Surfaces.Depth, NULL);

        {
            const HRESULT result = State.DX.Active.Surfaces.Depth->QueryInterface(IID_IDirectDrawSurface2, (void**)&State.DX.Active.Surfaces.Active.Depth);

            if (result != DD_OK)
            {
                if (result != DDERR_OUTOFMEMORY && result != DDERR_OUTOFVIDEOMEMORY)
                {
                    Message("CreateSurface for Z-buffer failed %d.\n", result);

                    return FALSE;
                }

                Message("There was not enough video memory to create the Z-buffer surface.\nPlease restart the program and try another fullscreen mode with less resolution or lower bit depth.\n");

                return FALSE;
            }
        }

        {
            const HRESULT result = State.DX.Active.Surfaces.Back->AddAttachedSurface(State.DX.Active.Surfaces.Depth);

            if (result != DD_OK)
            {
                Message("AddAttachedBuffer failed for Z-Buffer %d.\n", result);

                if (State.DX.Active.Surfaces.Active.Depth != NULL)
                {
                    State.DX.Active.Surfaces.Active.Depth->Release();
                    State.DX.Active.Surfaces.Active.Depth = NULL;
                }

                if (State.DX.Active.Surfaces.Depth != NULL)
                {
                    State.DX.Active.Surfaces.Depth->Release();
                    State.DX.Active.Surfaces.Depth = NULL;
                }

                return FALSE;
            }
        }

        {
            DDSURFACEDESC desc;
            ZeroMemory(&desc, sizeof(DDSURFACEDESC));

            desc.dwSize = sizeof(DDSURFACEDESC);

            const HRESULT result = State.DX.Active.Surfaces.Active.Depth->GetSurfaceDesc(&desc);

            if (result != DD_OK)
            {
                Message("Failed to get surface description of Z buffer %d.\n", result);

                if (State.DX.Active.Surfaces.Active.Depth != NULL)
                {
                    State.DX.Active.Surfaces.Active.Depth->Release();
                    State.DX.Active.Surfaces.Active.Depth = NULL;
                }

                if (State.DX.Active.Surfaces.Depth != NULL)
                {
                    State.DX.Active.Surfaces.Depth->Release();
                    State.DX.Active.Surfaces.Depth = NULL;
                }

                return FALSE;
            }

            if (!State.Device.Capabilities.IsAccelerated || (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)) { return TRUE; }

            Message("Could not fit the Z-buffer in video memory for this hardware device.\n");
        }

        if (State.DX.Active.Surfaces.Active.Depth != NULL)
        {
            State.DX.Active.Surfaces.Active.Depth->Release();
            State.DX.Active.Surfaces.Active.Depth = NULL;
        }

        if (State.DX.Active.Surfaces.Depth != NULL)
        {
            State.DX.Active.Surfaces.Depth->Release();
            State.DX.Active.Surfaces.Depth = NULL;
        }

        return FALSE;
    }

    // 0x600059bc
    void AcquireRendererDeviceTextureFormats(void)
    {
        State.Textures.Formats.Count = 0;

        s32 count = INVALID_TEXTURE_FORMAT_COUNT;
        State.DX.Device->EnumTextureFormats(EnumerateRendererDeviceTextureFormats, &count);

        State.Textures.Formats.Indexes[0] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[1] = AcquireRendererDeviceTextureFormatIndex(4, 0, 0, 0, 0);
        State.Textures.Formats.Indexes[2] = AcquireRendererDeviceTextureFormatIndex(8, 0, 0, 0, 0);
        State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 1, 5, 5, 5);
        State.Textures.Formats.Indexes[4] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 6, 5);
        State.Textures.Formats.Indexes[5] = AcquireRendererDeviceTextureFormatIndex(0, 0, 8, 8, 8);
        State.Textures.Formats.Indexes[6] = AcquireRendererDeviceTextureFormatIndex(0, 8, 8, 8, 8);
        State.Textures.Formats.Indexes[7] = AcquireRendererDeviceTextureFormatIndex(0, 4, 4, 4, 4);

        RendererTextureFormatStates[0] = 0; // TODO
        RendererTextureFormatStates[1] = State.Textures.Formats.Indexes[1] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[2] = State.Textures.Formats.Indexes[2] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[4] = State.Textures.Formats.Indexes[4] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[5] = State.Textures.Formats.Indexes[5] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[6] = State.Textures.Formats.Indexes[6] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[7] = State.Textures.Formats.Indexes[7] != INVALID_TEXTURE_FORMAT_INDEX;

        if (State.Textures.Formats.Indexes[3] == INVALID_TEXTURE_FORMAT_INDEX)
        {
            State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 5, 5);
            RendererTextureFormatStates[3] = (State.Textures.Formats.Indexes[3] != INVALID_TEXTURE_FORMAT_INDEX) ? 5 : 0; // TODO
        }
        else
        {
            RendererTextureFormatStates[3] = 1; // TODO
        }
    }

    // 0x600057e8
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDSURFACEDESC desc, LPVOID context)
    {
        ZeroMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count], sizeof(TextureFormat));

        CopyMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor, desc, sizeof(DDSURFACEDESC));

        if (desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
        {
            State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
            State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 8;
        }
        else
        {
            if (desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 4;
            }
            else
            {
                if (desc->ddpfPixelFormat.dwFlags & DDPF_RGB)
                {
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = FALSE;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 0;

                    u32 red = 0;

                    {
                        u32 value = desc->ddpfPixelFormat.dwRBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { red = red + 1; value = value >> 1; }
                    }

                    u32 green = 0;

                    {
                        u32 value = desc->ddpfPixelFormat.dwGBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { green = green + 1; value = value >> 1; }
                    }

                    u32 blue = 0;

                    {
                        u32 value = desc->ddpfPixelFormat.dwBBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { blue = blue + 1; value = value >> 1; }
                    }

                    State.Textures.Formats.Formats[State.Textures.Formats.Count].RedBitCount = red;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].GreenBitCount = green;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].BlueBitCount = blue;

                    if (desc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
                    {
                        u32 alpha = 0;

                        {
                            u32 value = desc->ddpfPixelFormat.dwRGBAlphaBitMask;

                            while ((value & 1) == 0) { value = value >> 1; }

                            while ((value & 1) != 0) { alpha = alpha + 1; value = value >> 1; }
                        }

                        State.Textures.Formats.Formats[State.Textures.Formats.Count].AlphaBitCount = alpha;
                    }
                }
            }
        }

        if (*(s32*)context == INVALID_TEXTURE_FORMAT_COUNT)
        {
            *(s32*)context = State.Textures.Formats.Count;
        }

        // NOTE: The original does not have this check,
        // thus it is prone the array overflow that can cause crash in some cases.
        if (MAX_TEXTURE_FORMAT_COUNT <= State.Textures.Formats.Count + 1) { return DDENUMRET_CANCEL; }

        State.Textures.Formats.Count = State.Textures.Formats.Count + 1;

        return DDENUMRET_OK;
    }

    // 0x60005744
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue)
    {
        for (u32 x = 0; x < State.Textures.Formats.Count; x++)
        {
            const TextureFormat* format = &State.Textures.Formats.Formats[x];

            if (format->RedBitCount == red && format->GreenBitCount == green && format->BlueBitCount == blue
                && format->PaletteColorBits == palette && format->AlphaBitCount == alpha)
            {
                return x;
            }
            else if (format->PaletteColorBits == palette && palette != 0) { return x; }
        }

        return INVALID_TEXTURE_FORMAT_INDEX;
    }

    // 0x6000410c
    void InitializeRendererState(void)
    {
        State.DX.Device->BeginScene();

        State.DX.Device->SetCurrentViewport(State.DX.ViewPort);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,
            State.Device.Capabilities.IsModulateBlending ? D3DTBLEND_MODULATEALPHA : D3DTBLEND_MODULATE);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 0x10101);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);

        {
            const f32 value = 0.0f;
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD*)(&value));
        }

        {
            const f32 value = 1.0f; // ORIGINAL: 1.4013e-45f
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGEND, *(DWORD*)(&value));
        }

        State.DX.Device->EndScene();
    }

    // 0x600011b4
    void InitializeViewPort(void)
    {
        State.ViewPort.X0 = 0;
        State.ViewPort.Y0 = 0;
        State.ViewPort.X1 = 0;
        State.ViewPort.Y1 = 0;
    }

    // 0x6000519c
    u32 ClearRendererViewPort(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        D3DRECT rect;

        DWORD options = State.Device.Capabilities.IsDepthAvailable
            ? (D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET) : D3DCLEAR_TARGET;

        if (x1 == 0)
        {
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = State.Window.Width;
            rect.y2 = State.Window.Height;
        }
        else
        {
            rect.x1 = x0;
            rect.x2 = x1;
            rect.y1 = y0;
            rect.y2 = y1;
        }

        return State.DX.ViewPort->Clear(1, &rect, options) == DD_OK;
    }

    // 0x60004f64
    BOOL SelectRendererState(const D3DRENDERSTATETYPE type, const DWORD value)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }

        return State.DX.Device->SetRenderState(type, value) == DD_OK;
    }

    // 0x60004e30
    // a.k.a. startrender
    BOOL BeginRendererScene(void)
    {
        if (State.Lock.IsActive)
        {
            Error("D3D startrender called in while locked\n");

            if (State.Lock.IsActive) { UnlockGameWindow(NULL); }
        }

        return State.DX.Device->BeginScene() == DD_OK;
    }

    // 0x600042f4
    u32 RendererRenderScene(void)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        const HRESULT result = State.DX.Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX,
            State.Data.Vertexes.Vertexes, State.Data.Vertexes.Count,
            State.Data.Indexes.Indexes, State.Data.Indexes.Count, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);

        State.Data.Vertexes.Count = 0;
        State.Data.Indexes.Count = 0;

        return result;
    }

    // 0x60005128
    void SelectRendererMaterial(const f32 r, const f32 g, const f32 b)
    {
        D3DMATERIAL material;
        ZeroMemory(&material, sizeof(D3DMATERIAL));

        material.dwSize = sizeof(D3DMATERIAL);

        material.diffuse.r = r;
        material.diffuse.g = g;
        material.diffuse.b = b;

        material.ambient.r = r;
        material.ambient.g = g;
        material.ambient.b = b;

        material.dwRampSize = 1;

        State.DX.Material->SetMaterial(&material);
    }

    // 0x600042e8
    void AttemptRenderScene(void)
    {
        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }
    }

    // 0x60005100
    void RestoreRendererSurfaces(void)
    {
        State.DX.Active.Surfaces.Active.Main->Restore();
        State.DX.Active.Surfaces.Active.Depth->Restore();

        RestoreRendererTextures();
    }

    // 0x60003784
    void RestoreRendererTextures(void)
    {
        RendererTexture* tex = State.Textures.Current;

        while (tex != NULL)
        {
            if (tex->Surface2 != NULL) { tex->Surface2->Restore(); }

            tex = tex->Previous;
        }
    }

    // 0x60002a50
    void SelectRendererFogAlphas(const u8* input, u8* output)
    {
        if (input == NULL) { return; }

        for (u32 x = 0; x < MAX_OUTPUT_FOG_ALPHA_COUNT; x++)
        {
            const f32 value = roundf(x / 255.0f) * 63.0f;
            const u32 indx = (u32)value;

            const f32 diff = value - indx;

            if (0.0f < diff)
            {
                const u8 result = (u8)roundf(input[indx] + (input[indx + 1] - input[indx]) * diff);
                output[x] = (u8)(MAX_OUTPUT_FOG_ALPHA_VALUE - result);
            }
            else
            {

                output[x] = (u8)(MAX_OUTPUT_FOG_ALPHA_VALUE - input[indx]);
            }
        }
    }

    // 0x60003830
    void SelectRendererDeviceType(const u32 type)
    {
        RendererDeviceType = type;
    }

    // 0x60004e90
    u32 EndRendererScene(void)
    {
        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }

        if (State.Scene.IsActive)
        {
            RendererDepthBias = 0.0f;

            State.DX.Device->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, TRUE);
            State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);

            const HRESULT result = State.DX.Device->EndScene();

            State.Scene.IsActive = FALSE;

            if (result != DD_OK) { return RENDERER_MODULE_FAILURE; }
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60001880
    void RendererLockWindow(const BOOL mode)
    {
        if (State.Lambdas.LockWindow != NULL) { State.Lambdas.LockWindow(mode); }
    }

    // 0x60004fb4
    // a.k.a. showbackbuffer
    u32 ToggleRenderer(void)
    {
        if (!State.Settings.IsWindowMode)
        {
            const HRESULT result = State.DX.Active.Surfaces.Main->Flip(NULL, DDFLIP_WAIT);

            if (result != DDERR_SURFACELOST)
            {
                if (result == DD_OK) { return RENDERER_MODULE_SUCCESS; }

                Error("Flipping complex display surface failed. %8x\n ", result);

                return RENDERER_MODULE_FAILURE;
            }
        }
        else
        {
            RECT r1;
            GetClientRect(State.Window.HWND, &r1);

            POINT point;
            ZeroMemory(&point, sizeof(POINT));

            ClientToScreen(State.Window.HWND, &point);
            OffsetRect(&r1, point.x, point.y);

            RECT r2;
            SetRect(&r2, 0, 0, State.Window.Width, State.Window.Height);

            const HRESULT result = State.DX.Active.Surfaces.Active.Main->Blt(&r1,
                State.DX.Active.Surfaces.Active.Back, &r2, DDBLT_WAIT, NULL);

            if (result != DDERR_SURFACELOST)
            {
                if (result == DD_OK) { return RENDERER_MODULE_SUCCESS; }

                Error("showbackbuffer - error %d %8x\n", result, result);

                return RENDERER_MODULE_FAILURE;
            }
        }

        State.DX.Active.Surfaces.Active.Main->Restore();
        State.DX.Active.Surfaces.Active.Back->Restore();

        ClearRendererViewPort(0, 0, 0, 0);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002398
    u32 ReleaseRendererWindow(void)
    {
        if (State.DX.Instance == NULL) { return RENDERER_MODULE_FAILURE; }

        SetForegroundWindow(State.Window.HWND);
        PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_RELEASE_DEVICE, 0, 0);
        WaitForSingleObject(State.Mutex, INFINITE);
        CloseHandle(State.Mutex);

        State.Mutex = NULL;
        State.Window.HWND = NULL;

        return State.DX.Code;
    }

    // 0x60002408
    u32 ReleaseRendererDeviceInstance(void)
    {
        ReleaseRendererDeviceSurfaces();

        State.DX.Instance->Release();
        State.DX.Instance = NULL;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003500
    RendererTexture* InitializeRendererTexture(void)
    {
        RendererTexture* result = (RendererTexture*)malloc(sizeof(RendererTexture));

        if (result == NULL) { Error("D3D texture allocation ran out of memory\n"); }

        return result;
    }

    // 0x60005ba8
    BOOL InitializeRendererTextureDetails(RendererTexture* tex)
    {
        DDSURFACEDESC desc;

        CopyMemory(&desc, &State.Textures.Formats.Formats[tex->FormatIndex].Descriptor, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);
        desc.dwFlags = DDSD_MIPMAPCOUNT | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        desc.dwHeight = tex->Height;
        desc.dwWidth = tex->Width;
        desc.dwMipMapCount = 1;
        desc.ddsCaps.dwCaps = DDSCAPS_MIPMAP | DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_COMPLEX;

        IDirectDrawSurface* surf = NULL;
        if (State.DX.Active.Instance->CreateSurface(&desc, &surf, NULL) != DD_OK) { return FALSE; }

        if (surf == NULL) { return FALSE; }

        IDirectDrawSurface3* surface1 = NULL;
        if (surf->QueryInterface(IID_IDirectDrawSurface3, (void**)&surface1) != DD_OK)
        {
            surf->Release();

            return FALSE;
        }

        surf->Release();

        IDirect3DTexture2* texture1 = NULL;
        if (surface1->QueryInterface(IID_IDirect3DTexture2, (void**)&texture1) != DD_OK)
        {
            if (surface1 != NULL) { surface1->Release(); }

            return FALSE;
        }

        tex->Surface1 = surface1;
        tex->Texture1 = texture1;

        ZeroMemory(&desc, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);

        if (surface1->GetSurfaceDesc(&desc) != DD_OK)
        {
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }

            return FALSE;
        }

        CopyMemory(&tex->Descriptor, &desc, sizeof(DDSURFACEDESC));

        desc.dwFlags = DDSD_MIPMAPCOUNT | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
            ? DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_COMPLEX
            : DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE | DDSCAPS_COMPLEX;

        if (State.DX.Active.Instance->CreateSurface(&desc, &surf, NULL) != DD_OK)
        {
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }

            return FALSE;
        }

        IDirectDrawSurface3* surface2 = NULL;
        if (surf->QueryInterface(IID_IDirectDrawSurface3, (void**)&surface2) != DD_OK)
        {
            if (surf != NULL) { surf->Release(); }
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }

            return FALSE;
        }

        surf->Release();

        if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
        {
            tex->Colors = 256;
        }
        else if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
        {
            tex->Colors = 16;
        }
        else
        {
            tex->Palette = NULL;
            tex->Colors = 0;
        }

        if ((desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) || (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4))
        {
            DWORD options = desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8
                ? DDPCAPS_ALLOW256 | DDPCAPS_8BIT
                : DDPCAPS_4BIT;

            PALETTEENTRY entries[MAX_TEXTURE_PALETTE_COLOR_COUNT];
            ZeroMemory(&entries, MAX_TEXTURE_PALETTE_COLOR_COUNT * sizeof(PALETTEENTRY));

            IDirectDrawPalette* palette = NULL;
            if (State.DX.Active.Instance->CreatePalette(options, entries, &palette, NULL) != DD_OK)
            {
                if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
                if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
                if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }

                return FALSE;
            }

            if (surface2->SetPalette(palette) != DD_OK)
            {
                if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
                if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
                if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }
                if (palette != NULL) { palette->Release(); }

                return FALSE;
            }

            tex->Palette = palette;
        }

        IDirect3DTexture2* texture2 = NULL;
        if (surface2->QueryInterface(IID_IDirect3DTexture2, (void**)&texture2) != DD_OK)
        {
            if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }
            if (tex->Palette != NULL) { tex->Palette->Release(); tex->Palette = NULL; }

            return FALSE;
        }

        if (texture2->Load(texture1) != DD_OK)
        {
            if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }
            if (texture2 != NULL) { texture2->Release(); texture2 = NULL; }
            if (tex->Palette != NULL) { tex->Palette->Release(); tex->Palette = NULL; }

            return FALSE;
        }

        ZeroMemory(&desc, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);

        if (surface2->GetSurfaceDesc(&desc) != DD_OK)
        {
            if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }
            if (texture2 != NULL) { texture2->Release(); texture2 = NULL; }
            if (tex->Palette != NULL) { tex->Palette->Release(); tex->Palette = NULL; }

            return FALSE;
        }

        tex->MemoryType = desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY
            ? (desc.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM ? RENDERER_MODULE_TEXTURE_LOCATION_NON_LOCAL_VIDEO_MEMORY : RENDERER_MODULE_TEXTURE_LOCATION_LOCAL_VIDEO_MEMORY)
            : (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ? RENDERER_MODULE_TEXTURE_LOCATION_NON_LOCAL_VIDEO_MEMORY : RENDERER_MODULE_TEXTURE_LOCATION_SYSTEM_MEMORY);

        tex->Surface2 = surface2;
        tex->Texture2 = texture2;

        if (texture2->GetHandle(State.DX.Device, &tex->Handle) != DD_OK)
        {
            if (surface2 != NULL) { surface2->Release(); surface2 = NULL; }
            if (surface1 != NULL) { surface1->Release(); surface1 = NULL; }
            if (texture1 != NULL) { texture1->Release(); texture1 = NULL; }
            if (texture2 != NULL) { texture2->Release(); texture2 = NULL; }
            if (tex->Palette != NULL) { tex->Palette->Release(); tex->Palette = NULL; }

            return FALSE;
        }

        return TRUE;
    }

    // 0x6000560f
    void ReleaseRendererTexture(RendererTexture* tex)
    {
        if (tex != NULL) { free(tex); }
    }

    // 0x60004ef8
    BOOL SelectRendererTexture(RendererTexture* tex)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }

        return State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, tex == NULL ? 0 : tex->Handle) == DD_OK;
    }

    // 0x60006468
    BOOL UpdateRendererTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        if (pixels != NULL)
        {
            tex->Descriptor.dwFlags = DDSD_LPSURFACE;
            tex->Descriptor.lpSurface = (void*)pixels;

            if (tex->Surface1->SetSurfaceDesc(&tex->Descriptor, 0) != DD_OK) { return FALSE; }

            if (State.Scene.IsActive)
            {
                AttemptRenderScene();

                State.DX.Device->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, TRUE);
            }

            tex->Surface1->PageLock(0);
            tex->Texture2->Load(tex->Texture1);
            tex->Surface1->PageUnlock(0);

            tex->Texture2->GetHandle(State.DX.Device, &tex->Handle);
        }

        if (palette == NULL || !tex->IsPalette) { return TRUE; }

        PALETTEENTRY entries[MAX_TEXTURE_PALETTE_COLOR_COUNT];

        for (u32 x = 0; x < MAX_TEXTURE_PALETTE_COLOR_COUNT; x++)
        {
            entries[x].peRed = (u8)RGBA_GETRED(palette[x]);
            entries[x].peGreen = (u8)RGBA_GETGREEN(palette[x]);
            entries[x].peBlue = (u8)RGBA_GETBLUE(palette[x]);
            entries[x].peFlags = 0;
        }

        if (tex->Palette->SetEntries(0, 0, tex->Colors, entries) != DD_OK) { return FALSE; }

        if (tex->Texture2->PaletteChanged(0, tex->Colors) != DD_OK) { return FALSE; }

        return TRUE;
    }

    // 0x60004c80
    BOOL RenderLines(RTLVX* vertexes, const u32 count)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        for (u32 x = 0; x < count; x++)
        {
            vertexes[x].Specular = ((u32)RendererFogAlphas[(u32)(vertexes[x].XYZ.Z * 255.0f)]) << 24;

            vertexes[x].XYZ.Z = RendererDepthBias + vertexes[x].XYZ.Z;
        }

        return State.DX.Device->DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX,
            vertexes, count, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP) == DD_OK;
    }

    // 0x60004de4
    BOOL RenderPoints(RTLVX* vertexes, const u32 count)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        return State.DX.Device->DrawPrimitive(D3DPT_POINTLIST, D3DVT_TLVERTEX,
            vertexes, count, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP) == DD_OK;
    }

    // 0x60004980
    void RenderQuad(RTLVX* a, RTLVX* b, RTLVX* c, RTLVX* d)
    {
        if (MaximumRendererVertexCount - 4 < State.Data.Vertexes.Count) { RendererRenderScene(); }

        RTLVX* verts = (RTLVX*)State.Data.Vertexes.Vertexes;

        // A
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 0];

            vertex->XYZ.X = a->XYZ.X;
            vertex->XYZ.Y = a->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + a->XYZ.Z;

            vertex->RHW = a->RHW;

            vertex->Color = a->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(a->XYZ.Z * 255.0)]) << 24;

            vertex->UV.X = a->UV.X;
            vertex->UV.Y = a->UV.Y;
        }

        // B
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 1];

            vertex->XYZ.X = b->XYZ.X;
            vertex->XYZ.Y = b->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + b->XYZ.Z;

            vertex->RHW = b->RHW;

            vertex->Color = b->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(b->XYZ.Z * 255.0)]) << 24;

            vertex->UV.X = b->UV.X;
            vertex->UV.Y = b->UV.Y;
        }

        // C
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 2];

            vertex->XYZ.X = c->XYZ.X;
            vertex->XYZ.Y = c->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + c->XYZ.Z;

            vertex->RHW = c->RHW;

            vertex->Color = c->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(c->XYZ.Z * 255.0)]) << 24;

            vertex->UV.X = c->UV.X;
            vertex->UV.Y = c->UV.Y;
        }

        // D
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 3];

            vertex->XYZ.X = d->XYZ.X;
            vertex->XYZ.Y = d->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + d->XYZ.Z;

            vertex->RHW = d->RHW;

            vertex->Color = d->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(d->XYZ.Z * 255.0)]) << 24;

            vertex->UV.X = d->UV.X;
            vertex->UV.Y = d->UV.Y;
        }

        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 0] = State.Data.Vertexes.Count + 0;
        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 1] = State.Data.Vertexes.Count + 1;
        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 2] = State.Data.Vertexes.Count + 2;
        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 3] = State.Data.Vertexes.Count + 0;
        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 4] = State.Data.Vertexes.Count + 2;
        State.Data.Indexes.Indexes[State.Data.Indexes.Count + 5] = State.Data.Vertexes.Count + 3;

        State.Data.Indexes.Count = State.Data.Indexes.Count + 6;
        State.Data.Vertexes.Count = State.Data.Vertexes.Count + 4;
    }

    // 0x6000470c
    void RenderQuadMesh(RTLVX* vertexes, const u32* indexes, const u32 count)
    {
        RTLVX* verts = (RTLVX*)State.Data.Vertexes.Vertexes;

        for (u32 x = 0; x < count; x++)
        {
            if (MaximumRendererVertexCount - 4 < State.Data.Vertexes.Count) { RendererRenderScene(); }

            RTLVX* a = &vertexes[indexes[x * 4 + 0]];
            RTLVX* b = &vertexes[indexes[x * 4 + 1]];
            RTLVX* c = &vertexes[indexes[x * 4 + 2]];
            RTLVX* d = &vertexes[indexes[x * 4 + 3]];

            RenderQuad(a, b, c, d);
        }
    }

    // 0x6000435c
    void RenderTriangle(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        if (MaximumRendererVertexCount - 3 < State.Data.Vertexes.Count) { RendererRenderScene(); }

        RTLVX* verts = (RTLVX*)State.Data.Vertexes.Vertexes;

        // A
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 0];

            vertex->XYZ.X = a->XYZ.X;
            vertex->XYZ.Y = a->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + a->XYZ.Z;

            vertex->RHW = a->RHW;

            vertex->Color = a->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(a->XYZ.X * 255.0)]) << 24;

            vertex->UV.X = a->UV.X;
            vertex->UV.Y = a->UV.Y;

            State.Data.Indexes.Indexes[State.Data.Indexes.Count + 0] = State.Data.Vertexes.Count + 0;
        }

        // B
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 1];

            vertex->XYZ.X = b->XYZ.X;
            vertex->XYZ.Y = b->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + b->XYZ.Z;

            vertex->RHW = b->RHW;

            vertex->Color = b->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(b->XYZ.X * 255.0)]) << 24;

            vertex->UV.X = b->UV.X;
            vertex->UV.Y = b->UV.Y;

            State.Data.Indexes.Indexes[State.Data.Indexes.Count + 1] = State.Data.Vertexes.Count + 1;
        }

        // C
        {
            RTLVX* vertex = &verts[State.Data.Vertexes.Count + 2];

            vertex->XYZ.X = c->XYZ.X;
            vertex->XYZ.Y = c->XYZ.Y;
            vertex->XYZ.Z = RendererDepthBias + c->XYZ.Z;

            vertex->RHW = c->RHW;

            vertex->Color = c->Color;
            vertex->Specular = ((u32)RendererFogAlphas[(u32)(c->XYZ.X * 255.0)]) << 24;

            vertex->UV.X = c->UV.X;
            vertex->UV.Y = c->UV.Y;

            State.Data.Indexes.Indexes[State.Data.Indexes.Count + 2] = State.Data.Vertexes.Count + 2;
        }

        State.Data.Indexes.Count = State.Data.Indexes.Count + 3;
        State.Data.Vertexes.Count = State.Data.Vertexes.Count + 3;
    }

    // 0x60004bdc
    BOOL RenderTriangleFan(RTLVX* vertexes, const u32 vertexCount, const u32 indexCount, const u16* indexes)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        return State.DX.Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX,
            vertexes, vertexCount, (WORD*)indexes, indexCount, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP) == DD_OK;
    }

    // 0x60004504
    void RenderTriangleMesh(RTLVX* vertexes, const u32* indexes, const u32 count)
    {
        for (u32 x = 0; x < count; x++)
        {
            if (MaximumRendererVertexCount - 3 < State.Data.Vertexes.Count) { RendererRenderScene(); }

            RTLVX* a = &vertexes[indexes[x * 3 + 0]];
            RTLVX* b = &vertexes[indexes[x * 3 + 1]];
            RTLVX* c = &vertexes[indexes[x * 3 + 2]];

            RenderTriangle(a, b, c);
        }
    }

    // 0x60004b90
    BOOL RenderTriangleStrip(RTLVX* vertexes, const u32 count)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        for (u32 x = 0; x < count; x++)
        {
            vertexes[x].Specular = ((u32)RendererFogAlphas[(u32)(vertexes[x].XYZ.Z * 255.0f)]) << 24;

            vertexes[x].XYZ.Z = RendererDepthBias + vertexes[x].XYZ.Z;
        }

        return State.DX.Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX,
            vertexes, count, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP) == DD_OK;
    }
}