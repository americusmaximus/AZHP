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

#include "Graphics.Basic.hxx"
#include "Renderer.hxx"
#include "RendererValues.hxx"

#include <stdarg.h>
#include <stdio.h>

#define MAX_MESSAGE_BUFFER_LENGTH 512

using namespace Renderer;
using namespace RendererModuleValues;

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
        if (RendererDeviceIndex < DEFAULT_RENDERER_DEVICE_INDEX
            && (State.Lambdas.AcquireWindow != NULL || State.Window.HWND != NULL))
        {
            const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONEMNT_PROPERTY_NAME);

            SelectDevice(value == NULL ? DEFAULT_RENDERER_DEVICE_INDEX : atoi(value));
        }
    }

    // 0x600015e8
    u32 AcquireRendererDeviceCount(void)
    {
        State.Devices.Count = 0;
        State.Device.Identifier = NULL;

        u32 indx = DEFAULT_RENDERER_DEVICE_INDEX;
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
        if (MAX_RENDERER_DEVICE_COUNT <= (State.Devices.Count + 1)) { return FALSE; }

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
                            MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));

                        ModuleDescriptor.Capabilities.Capabilities[1].Width = GRAPHICS_RESOLUTION_640;
                        ModuleDescriptor.Capabilities.Capabilities[1].Height = GRAPHICS_RESOLUTION_480;
                        ModuleDescriptor.Capabilities.Capabilities[1].Bits = GRAPHICS_BITS_PER_PIXEL_16;
                        ModuleDescriptor.Capabilities.Capabilities[1].Format = RENDERER_PIXEL_FORMAT_16_BIT_565;
                        ModuleDescriptor.Capabilities.Capabilities[1].Unk03 = 1;
                        ModuleDescriptor.Capabilities.Capabilities[1].Unk04 = 1;
                        ModuleDescriptor.Capabilities.Capabilities[1].Unk02 = 1;

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

                if ((MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT - 1) < indx) { return DDENUMRET_CANCEL; }

                *(u32*)context = indx + 1;
            }

            const u32 count = State.Settings.MaxAvailableMemory / (height * width * bytes);

            ModuleDescriptor.Capabilities.Capabilities[indx].Width = width;
            ModuleDescriptor.Capabilities.Capabilities[indx].Height = height;
            ModuleDescriptor.Capabilities.Capabilities[indx].Bits =
                format == RENDERER_PIXEL_FORMAT_16_BIT_555 ? (GRAPHICS_BITS_PER_PIXEL_16 - 1) : bits;
            ModuleDescriptor.Capabilities.Capabilities[indx].Unk02 = 1;

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
            if (red == 0x7c00 && green == 0x3e0 && blue == 0x1f) { return RENDERER_PIXEL_FORMAT_16_BIT_555; }
            else if (red == 0xf800 && green == 0x7e0 && blue == 0x1f) { return RENDERER_PIXEL_FORMAT_16_BIT_565; }
            else if (red == 0xf00 && green == 0xf0 && blue == 0xf && format->dwRGBAlphaBitMask == 0xf000) { return RENDERER_PIXEL_FORMAT_16_BIT_444; }
        }
        else if (red == 0xff0000 && green == 0xff00 && blue == 0xff)
        {
            if (bits == GRAPHICS_BITS_PER_PIXEL_24) { return RENDERER_PIXEL_FORMAT_24_BIT; }
            else if (bits == GRAPHICS_BITS_PER_PIXEL_32) { return RENDERER_PIXEL_FORMAT_32_BIT; }
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
                            : MIN_RENDERER_DEVICE_AVAIABLE_VIDEO_MEMORY;
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
                    MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));

                ModuleDescriptor.Capabilities.Capabilities[1].Width = GRAPHICS_RESOLUTION_640;
                ModuleDescriptor.Capabilities.Capabilities[1].Height = GRAPHICS_RESOLUTION_480;
                ModuleDescriptor.Capabilities.Capabilities[1].Bits = GRAPHICS_BITS_PER_PIXEL_16;
                ModuleDescriptor.Capabilities.Capabilities[1].Format = RENDERER_PIXEL_FORMAT_16_BIT_565;
                ModuleDescriptor.Capabilities.Capabilities[1].Unk03 = 3;
                ModuleDescriptor.Capabilities.Capabilities[1].Unk04 = 2;
                ModuleDescriptor.Capabilities.Capabilities[1].Unk02 = 1;

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
        for (u32 x = (MAX_ACTIVE_SURFACE_COUNT - 1); x != 0; x--)
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
                desc.ddsCaps.dwCaps = RendererDeviceType == RENDERER_MODULE_DEVICE_TYPE_ACCELERATED
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
            SyncGameWindow(0);
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

        if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_32_BIT) == 0)
        {
            if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_24_BIT) == 0)
            {
                if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_16_BIT) == 0)
                {
                    if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_8_BIT) == 0)
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

        switch (RendererDeviceType)
        {
        case RENDERER_MODULE_DEVICE_TYPE_RAMP:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRampDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_RGB:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRGBDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_MMX:
        {
            result = State.DX.DirectX->CreateDevice(IID_IDirect3DMMXDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_INVALID:
        case RENDERER_MODULE_DEVICE_TYPE_ACCELERATED:
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
            const f32 value = 1.0f; // ORIGINAL: 1.4013e-45f;
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
}