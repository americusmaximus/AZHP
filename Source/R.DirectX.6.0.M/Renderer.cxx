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

#include <stdio.h>

#define MAX_MESSAGE_BUFFER_LENGTH 512

using namespace Renderer;
using namespace RendererModuleValues;

namespace RendererModule
{
    RendererModuleState State;

    // 0x600052d0
    // 0x60005310
    // 0x60005350
    // NOTE: Originally there are 3 different methods for error, warning, and info (which is never being called).
    void Message(const u32 severity, const char* format, ...)
    {
        char buffer[MAX_MESSAGE_BUFFER_LENGTH];

        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, MAX_MESSAGE_BUFFER_LENGTH, format, args);
        va_end(args);

        if (State.Lambdas.Log != NULL) { State.Lambdas.Log(severity, buffer); }
    }

    // 0x6000e514
    void InitializeVertexes(Renderer::RTLVX* vertexes, const u32 count)
    {
        for (u32 x = 0; x < count; x++)
        {
            vertexes[x].XYZ.X = vertexes[x].XYZ.X - 0.5f;
            vertexes[x].XYZ.Y = vertexes[x].XYZ.Y - 0.5f;
        }
    }

    // 0x60001830
    void SelectRendererDevice(void)
    {
        if (RendererDeviceIndex < DEFAULT_RENDERER_DEVICE_INDEX
            && (State.Lambdas.Lambdas.AcquireWindow != NULL || State.Window.HWND != NULL))
        {
            const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONEMNT_PROPERTY_NAME);

            SelectDevice(value == NULL ? DEFAULT_RENDERER_DEVICE_INDEX : atoi(value));
        }
    }

    // 0x60006bc0
    u32 ClearRendererViewPort(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        D3DRECT rect;

        DWORD options = State.Device.Capabilities.IsDepthAvailable ? (D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET) : D3DCLEAR_TARGET;

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

    // 0x60006830
    u32 EndRendererScene(void)
    {
        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }

        if (State.Scene.IsActive)
        {
            State.Scene.IsActive = FALSE;

            RendererDepthBias = 0.0f;

            State.DX.Device->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, TRUE);

            return State.DX.Device->EndScene() == DD_OK;
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600056d0
    u32 RendererRenderScene(void)
    {
        if (!State.Scene.IsActive)
        {
            BeginRendererScene();

            State.Scene.IsActive = TRUE;
        }

        InitializeVertexes(State.Data.Vertexes.Vertexes, State.Data.Vertexes.Count);

        const HRESULT result = State.DX.Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX,
            State.Data.Vertexes.Vertexes, State.Data.Vertexes.Count,
            State.Data.Indexes.Medium, State.Data.Indexes.Count, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);

        State.Data.Vertexes.Count = 0;
        State.Data.Indexes.Count = 0;

        return result;
    }

    // 0x600067d0
    // a.k.a. startrender
    BOOL BeginRendererScene(void)
    {
        if (State.Lock.IsActive)
        {
            LOGERROR("D3D startrender called in while locked\n");

            if (State.Lock.IsActive) { UnlockGameWindow(NULL); }
        }

        return State.DX.Device->BeginScene() == DD_OK;
    }

    // 0x60001800
    u32 AcquireRendererDeviceCount(void)
    {
        State.Devices.Count = 0;
        State.Device.Identifier = NULL;

        u32 indx = DEFAULT_RENDERER_DEVICE_INDEX;
        DirectDrawEnumerateA(EnumerateRendererDevices, &indx);

        return State.Devices.Count;
    }

    // 0x60001970
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

    // 0x60004490
    BOOL AcquireRendererDeviceState(void)
    {
        return State.DX.Active.IsInit;
    }

    // 0x600043a0
    void ReleaseRendererDevice(void)
    {
        if (AcquireRendererDeviceState() && State.Scene.IsActive)
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

        if (State.DX.Clipper != NULL)
        {
            State.DX.Clipper->Release();
            State.DX.Clipper = NULL;
        }
    }

    // 0x600026e0
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

    // 0x60001ad0
    u32 InitializeRendererDevice(void)
    {
        if (State.Window.HWND != NULL)
        {
            State.Settings.CooperativeLevel = State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_NORMAL | DDSCL_FULLSCREEN);

            AcquireWindowModeCapabilities();

            State.Settings.CooperativeLevel = State.Settings.IsWindowMode
                ? State.Settings.CooperativeLevel | DDSCL_SETFOCUSWINDOW | DDSCL_ALLOWMODEX | DDSCL_NORMAL
                : State.Settings.CooperativeLevel | DDSCL_SETFOCUSWINDOW | DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

            IDirectDraw* instance;
            State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

            if (State.DX.Code == DD_OK)
            {
                State.DX.Code = instance->QueryInterface(IID_IDirectDraw4, (void**)&State.DX.Instance);

                instance->Release();

                if (State.DX.Code == DD_OK)
                {
                    State.DX.Code = State.DX.Instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

                    if (State.DX.Code == DD_OK)
                    {
                        {
                            DWORD total = 0, free = 0;

                            {
                                DDSCAPS2 caps;
                                ZeroMemory(&caps, sizeof(DDSCAPS2));

                                caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                                const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                                ModuleDescriptor.VideoMemorySize = result == DD_OK ? free : 0;
                            }

                            {

                                DDSCAPS2 caps;
                                ZeroMemory(&caps, sizeof(DDSCAPS2));

                                caps.dwCaps = DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                                const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                                ModuleDescriptor.TotalMemorySize = result == DD_OK ? free : 0; // TODO
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

                        InitializeRendererDeviceCapabilities();

                        return RENDERER_MODULE_SUCCESS;
                    }
                }
            }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001c58
    void InitializeRendererDeviceCapabilities(void)
    {
        ZeroMemory(ModuleDescriptor.Capabilities.Capabilities,
            MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));


        ModuleDescriptor.Capabilities.Count = 0;

        State.DX.Instance->EnumDisplayModes(DDEDM_NONE, NULL, 0, EnumerateRendererDeviceModes);

        ModuleDescriptor.Capabilities.Count = ModuleDescriptor.Capabilities.Count + 1;
    }

    // 0x60001a00
    void AcquireWindowModeCapabilities(void)
    {
        IDirectDraw* instance = NULL;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);
        State.DX.Code = instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

        IDirectDraw4* dd = NULL;
        State.DX.Code = instance->QueryInterface(IID_IDirectDraw4, (void**)&dd);

        DDCAPS hal;
        ZeroMemory(&hal, sizeof(DDCAPS));

        hal.dwSize = sizeof(DDCAPS);

        DDCAPS hel;
        ZeroMemory(&hel, sizeof(DDCAPS));

        hel.dwSize = sizeof(DDCAPS);

        dd->GetCaps(&hal, &hel);

        if ((hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) == 0) { State.Settings.IsWindowMode = FALSE; }

        dd->Release();
    }

    // 0x60001c90
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC2 desc, LPVOID context)
    {
        RendererModuleDescriptorDeviceCapabilities caps;
        ZeroMemory(&caps, sizeof(RendererModuleDescriptorDeviceCapabilities));

        caps.Format = AcquirePixelFormat(&desc->ddpfPixelFormat);

        if (caps.Format != RENDERER_PIXEL_FORMAT_NONE)
        {
            caps.Width = desc->dwWidth;
            caps.Height = desc->dwHeight;

            if ((GRAPHICS_RESOLUTION_640 - 1) < caps.Width && (GRAPHICS_RESOLUTION_480 - 1) < caps.Height)
            {
                caps.Bits = caps.Format == RENDERER_PIXEL_FORMAT_16_BIT_555
                    ? (GRAPHICS_BITS_PER_PIXEL_16 - 1)
                    : desc->ddpfPixelFormat.dwRGBBitCount;

                caps.Unk02 = 1; // TODO

                const u32 bytes = caps.Bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1) ? 2 : (caps.Bits >> 3);

                caps.Unk03 = State.Settings.MaxAvailableMemory / (bytes * caps.Width * caps.Height);

                if (caps.Unk03 < 4) // TODO
                {
                    caps.Unk04 = caps.Unk03 - 1;
                }
                else
                {
                    caps.Unk03 = 3;
                    caps.Unk04 = 3;
                }

                InitializeRendererDeviceCapabilities(&caps);
            }
        }

        return DDENUMRET_OK;
    }

    // 0x600028e0
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

    // 0x6000e54c
    BOOL InitializeRendererDeviceCapabilities(RendererModuleDescriptorDeviceCapabilities* caps)
    {
        if (ModuleDescriptor.Capabilities.Count < MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT)
        {
            CopyMemory(&ModuleDescriptorDeviceCapabilities[ModuleDescriptor.Capabilities.Count], caps,
                sizeof(RendererModuleDescriptorDeviceCapabilities));

            ModuleDescriptor.Capabilities.Count = ModuleDescriptor.Capabilities.Count + 1;

            return TRUE;
        }

        return FALSE;
    }

    // 0x60001df0
    u32 InitializeRendererDeviceLambdas(void)
    {
        if (State.Mutex == NULL) { State.Mutex = CreateEventA(NULL, FALSE, FALSE, NULL); }

        State.Window.HWND = State.Lambdas.Lambdas.AcquireWindow();

        if (State.Window.HWND != NULL)
        {
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceExecute);
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_RELEASE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)ReleaseRendererDeviceExecute);
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceSurfacesExecute);

            State.Settings.CooperativeLevel = State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_NORMAL | DDSCL_FULLSCREEN);

            AcquireWindowModeCapabilities();

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

    // 0x60001ec0
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        IDirectDraw* instance;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

        if (State.DX.Code == DD_OK)
        {
            State.DX.Code = instance->QueryInterface(IID_IDirectDraw4, (void**)&State.DX.Instance);

            State.Lambdas.Lambdas.SelectInstance(instance);

            instance->Release();

            State.DX.Code = State.DX.Instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

            if (State.DX.Code == DD_OK)
            {
                u32 pitch = 0;
                u32 height = 0;

                {
                    DDSURFACEDESC2 desc;
                    ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                    desc.dwSize = sizeof(DDSURFACEDESC2);
                    desc.dwFlags = DDSD_CAPS;
                    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

                    IDirectDrawSurface4* surface = NULL;
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
                        LOGWARNING("*** FAILURE in creating primary surface (error code: %8x)***\n", State.DX.Code);
                    }

                    if (surface != NULL) { surface->Release(); }
                }

                {
                    DWORD free = 0;
                    DWORD total = 0;

                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;

                        const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                        State.Settings.MaxAvailableMemory = result == DD_OK
                            ? height * pitch + total
                            : MIN_RENDERER_DEVICE_AVAIABLE_VIDEO_MEMORY;
                    }

                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                        if (State.DX.Instance->GetAvailableVidMem(&caps, &total, &free) == DD_OK)
                        {
                            ModuleDescriptor.TotalMemorySize = 3; // TODO
                            ModuleDescriptor.VideoMemorySize = State.Settings.MaxAvailableMemory;
                        }
                        else
                        {
                            ModuleDescriptor.TotalMemorySize = 0; // TODO
                            ModuleDescriptor.VideoMemorySize = 0;
                        }
                    }

                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

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

                InitializeRendererDeviceCapabilities();
            }
        }

        SetEvent(State.Mutex);

        *result = State.DX.Code;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002610
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDeviceSurfaces();

        State.Lambdas.Lambdas.SelectInstance(NULL);

        State.DX.Instance->Release();
        State.DX.Instance = NULL;

        SetEvent(State.Mutex);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600021f0
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDevice();
        ReleaseRendererDeviceSurfaces();

        if (!State.Settings.IsWindowModeActive || !State.Settings.IsWindowMode)
        {
            State.Window.Width = ModuleDescriptor.Capabilities.Capabilities[wp].Width;
            State.Window.Height = ModuleDescriptor.Capabilities.Capabilities[wp].Height;
        }
        else
        {
            const HDC hdc = GetDC(hwnd);

            State.Window.Width = GetDeviceCaps(hdc, HORZRES);
            State.Window.Height = GetDeviceCaps(hdc, VERTRES);

            ReleaseDC(hwnd, hdc);
        }

        const u32 bits = ModuleDescriptor.Capabilities.Capabilities[wp].Bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1)
            ? GRAPHICS_BITS_PER_PIXEL_16 : ModuleDescriptor.Capabilities.Capabilities[wp].Bits;

        State.DX.Code = DD_OK;

        if (!State.Settings.IsWindowMode)
        {
            State.DX.Code = State.DX.Instance->SetDisplayMode(ModuleDescriptor.Capabilities.Capabilities[wp].Width,
                ModuleDescriptor.Capabilities.Capabilities[wp].Height, bits, 0, DDSDM_NONE);

            if (State.DX.Code == DD_OK)
            {
                DDSURFACEDESC2 desc;
                ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                desc.dwSize = sizeof(DDSURFACEDESC2);
                desc.dwFlags = DDSD_BACKBUFFERCOUNT | DDSD_CAPS;
                desc.dwBackBufferCount = lp - 1;
                desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
                    ? DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX
                    : DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);

                while (State.DX.Code == DDERR_OUTOFVIDEOMEMORY && 1 < desc.dwBackBufferCount)
                {
                    desc.dwBackBufferCount = desc.dwBackBufferCount - 1;

                    State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    if (State.DX.Surfaces.Back == NULL)
                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

                        caps.dwCaps = DDSCAPS_BACKBUFFER;

                        State.DX.Code = State.DX.Surfaces.Main->GetAttachedSurface(&caps, &State.DX.Surfaces.Back);
                    }
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface4, (void**)&State.DX.Surfaces.Active[1]); // TODO
                }

                if (State.DX.Surfaces.Back != NULL)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface4, (void**)&State.DX.Surfaces.Active[2]); // TODO
                }
            }
        }
        else
        {
            DDSURFACEDESC2 desc;
            ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

            desc.dwSize = sizeof(DDSURFACEDESC2);
            desc.dwFlags = DDSD_CAPS;
            desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

            State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);

            if (State.DX.Code == DD_OK)
            {
                if (State.Settings.IsWindowMode)
                {
                    State.DX.Code = State.DX.Instance->CreateClipper(0, &State.DX.Clipper, NULL);
                    State.DX.Code = State.DX.Surfaces.Main->SetClipper(State.DX.Clipper);
                    State.DX.Code = State.DX.Clipper->SetHWnd(0, State.Window.HWND);
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface4, (void**)&State.DX.Surfaces.Active[1]); // TODO
                }

                desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
                desc.dwHeight = State.Window.Height;
                desc.dwWidth = State.Window.Width;
                desc.ddsCaps.dwCaps = RendererDeviceType == RENDERER_MODULE_DEVICE_TYPE_ACCELERATED
                    ? DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN
                    : DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Back, NULL);

                if (State.DX.Code == DD_OK)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface4, (void**)&State.DX.Surfaces.Active[2]); // TODO
                }
                else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { LOGWARNING("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
                else { LOGWARNING("CreateSurface for window back buffer failed %8x.\n", State.DX.Code); }
            }
            else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { LOGWARNING("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
            else { LOGWARNING("CreateSurface for window front buffer failed %8x.\n", State.DX.Code); }
        }

        InitializeRendererDeviceAcceleration();

        if (State.Lambdas.Lambdas.AcquireWindow != NULL)
        {
            SetEvent(State.Mutex);

            *result = State.DX.Code;
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600044a0
    // a.k.a. createD3D
    u32 InitializeRendererDeviceAcceleration(void)
    {
        State.DX.Active.Instance = State.DX.Instance;

        State.DX.Active.Surfaces.Main = State.DX.Surfaces.Main;
        State.DX.Active.Surfaces.Back = State.DX.Surfaces.Back;

        State.DX.Active.Surfaces.Active.Main = State.DX.Surfaces.Active[1]; // TODO
        State.DX.Active.Surfaces.Active.Back = State.DX.Surfaces.Active[2]; // TODO

        if (State.DX.Instance->QueryInterface(IID_IDirect3D3, (void**)&State.DX.DirectX) != DD_OK)
        {
            LOGERROR("Creation of IDirect3D3 failed.\nCheck DX6 installed.\n"); // ORIGINAL: IDirect3D2
        }

        InitializeConcreteRendererDevice();

        D3DDEVICEDESC hal;
        ZeroMemory(&hal, sizeof(D3DDEVICEDESC));

        hal.dwSize = sizeof(D3DDEVICEDESC);

        D3DDEVICEDESC hel;
        ZeroMemory(&hel, sizeof(D3DDEVICEDESC));

        hel.dwSize = sizeof(D3DDEVICEDESC);

        if (State.DX.Device->GetCaps(&hal, &hel) != DD_OK) { LOGERROR("GetCaps of IDirect3D3 Failed\n"); }

        if (hal.dcmColorModel == D3DCOLOR_NONE)
        {
            State.Device.Capabilities.IsAccelerated = FALSE;

            State.Device.Capabilities.RendererBits = hel.dwDeviceRenderBitDepth;
            State.Device.Capabilities.IsPerspectiveTextures = hel.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE;
            State.Device.Capabilities.IsAlphaTextures = FALSE;
            State.Device.Capabilities.IsAlphaFlatBlending = FALSE;
            State.Device.Capabilities.IsModulateBlending = FALSE;
            State.Device.Capabilities.IsSourceAlphaBlending = FALSE;
            State.Device.Capabilities.IsColorBlending = FALSE;
            State.Device.Capabilities.IsSpecularBlending = FALSE;
            State.Device.Capabilities.DepthBits = hel.dwDeviceZBufferBitDepth;
        }
        else
        {
            State.Device.Capabilities.IsAccelerated = TRUE;

            State.Device.Capabilities.RendererBits = hal.dwDeviceRenderBitDepth;
            State.Device.Capabilities.DepthBits = hal.dwDeviceZBufferBitDepth;

            State.Device.Capabilities.AntiAliasing = (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT) != 0;

            if (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
            {
                State.Device.Capabilities.AntiAliasing = State.Device.Capabilities.AntiAliasing | RENDERER_MODULE_ANTIALIASING_SORT_INDEPENDENT;
            }

            State.Device.Capabilities.IsWBuffer = (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WBUFFER) != 0;
            State.Device.Capabilities.IsDither = (hal.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER) != 0;
            State.Device.Capabilities.IsPerspectiveTextures = hal.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE;
            State.Device.Capabilities.IsAlphaTextures = (hal.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA) != 0;

            {
                const u32 phong = hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND;
                const u32 gouraud = hal.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_ALPHAFLATBLEND);

                State.Device.Capabilities.IsAlphaFlatBlending = (phong || gouraud) ? TRUE : FALSE;
            }

            {
                const u32 phong = hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND;
                const u32 gouraud = hal.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND;

                State.Device.Capabilities.IsAlphaProperBlending = (phong || gouraud) ? TRUE : FALSE;
            }

            State.Device.Capabilities.IsModulateBlending = (hal.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA) != 0;

            if ((hal.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) == 0)
            {
                State.Device.Capabilities.IsSourceAlphaBlending = TRUE;

                if ((hal.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE) == 0)
                {
                    State.Device.Capabilities.IsSourceAlphaBlending = FALSE;
                }
            }

            State.Device.Capabilities.IsColorBlending = (hal.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_COLORPHONGRGB | D3DPSHADECAPS_COLORGOURAUDRGB)) != 0;
            State.Device.Capabilities.IsSpecularBlending = (hal.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_SPECULARPHONGRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB)) != 0;

            if (!State.Device.Capabilities.IsColorBlending) { State.Device.Capabilities.IsSourceAlphaBlending = FALSE; }

            {
                DDCAPS hal;
                ZeroMemory(&hal, sizeof(DDCAPS));

                hal.dwSize = sizeof(DDCAPS);

                DDCAPS hel;
                ZeroMemory(&hel, sizeof(DDCAPS));

                hel.dwSize = sizeof(DDCAPS);

                if (State.DX.Active.Instance->GetCaps(&hal, &hel) != DD_OK) { LOGERROR("GetCaps of IDirectDraw4 Failed.\n"); }

                State.Device.Capabilities.IsWindowMode = (hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) != 0;
            }

            State.Device.Capabilities.IsDepthComparisonAvailable = AcquireRendererDeviceDepthBufferNotEqualComparisonCapabilities();

            State.Device.Capabilities.IsStripplingAvailable = AcquireRendererDeviceStripplingCapabilities();
        }

        if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_32_BIT) == 0)
        {
            if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_24_BIT) == 0)
            {
                if ((State.Device.Capabilities.DepthBits & DEPTH_BIT_MASK_16_BIT) == 0)
                {
                    State.Device.Capabilities.DepthBits = (State.Device.Capabilities.DepthBits >> 8) & 8;
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
        vp.dvClipHeight = (State.Window.Height + State.Window.Height) / (f32)State.Window.Width;
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

    // 0x60004980
    void InitializeConcreteRendererDevice(void)
    {
        HRESULT result = DD_OK;

        switch (RendererDeviceType)
        {
        case RENDERER_MODULE_DEVICE_TYPE_RAMP:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRampDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device, NULL);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_RGB:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRGBDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device, NULL);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_MMX:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DMMXDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device, NULL);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_INVALID:
        case RENDERER_MODULE_DEVICE_TYPE_ACCELERATED:
        {
            State.DX.Active.IsSoft = FALSE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DHALDevice,
                State.DX.Active.Surfaces.Back, &State.DX.Device, NULL);

            break;
        }
        default: { LOGERROR("D3D device type not recognised\n"); return; }
        }

        if (result != DD_OK) { LOGERROR("IDirect3D3_CreateDevice failed.\n"); } // ORIGINAL: IDirect3D2_CreateDevice
    }

    // 0x60004fc0
    BOOL AcquireRendererDeviceDepthBufferNotEqualComparisonCapabilities(void)
    {
        D3DDEVICEDESC hal;
        ZeroMemory(&hal, sizeof(D3DDEVICEDESC));

        hal.dwSize = sizeof(D3DDEVICEDESC);

        D3DDEVICEDESC hel;
        ZeroMemory(&hel, sizeof(D3DDEVICEDESC));

        hel.dwSize = sizeof(D3DDEVICEDESC);

        State.DX.Device->GetCaps(&hal, &hel);

        return (hal.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL) != 0;
    }
    
    // 0x60005020
    BOOL AcquireRendererDeviceStripplingCapabilities(void)
    {
        D3DDEVICEDESC hal;
        ZeroMemory(&hal, sizeof(D3DDEVICEDESC));

        hal.dwSize = sizeof(D3DDEVICEDESC);

        D3DDEVICEDESC hel;
        ZeroMemory(&hel, sizeof(D3DDEVICEDESC));

        hel.dwSize = sizeof(D3DDEVICEDESC);

        State.DX.Device->GetCaps(&hal, &hel);

        return (hal.dpcLineCaps.dwStippleHeight & 0x8000) != 0;
    }

    // 0x60004a90
    // a.k.a. createzbuffer
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height)
    {
        if (State.Device.Capabilities.IsStripplingAvailable) { return TRUE; }

        DDPIXELFORMAT format;
        ZeroMemory(&format, sizeof(DDPIXELFORMAT));

        if (!State.Device.Capabilities.IsAccelerated) { LOGERROR("I currently don't allow Zbuffers for software.\n"); }
        else
        {
            const HRESULT result = State.DX.DirectX->EnumZBufferFormats(IID_IDirect3DHALDevice, EnumerateRendererDevicePixelFormats, &format);

            if (result != DD_OK) { LOGERROR("EnumZBufferFormats failure! %8x\n", result); }
        }

        if (format.dwSize != sizeof(DDPIXELFORMAT)) { return FALSE; }

        DDSURFACEDESC2 desc;
        ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

        desc.dwSize = sizeof(DDSURFACEDESC2);
        desc.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        desc.dwHeight = height;
        desc.dwWidth = width;

        CopyMemory(&desc.ddpfPixelFormat, &format, sizeof(DDPIXELFORMAT));

        desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
            ? DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY
            : DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;

        {
            const HRESULT result = State.DX.Active.Instance->CreateSurface(&desc, &State.DX.Active.Surfaces.Depth, NULL);

            if (result != DD_OK) { LOGERROR("CreateSurface failure! %8x\n", result); }
        }

        {
            const HRESULT result = State.DX.Active.Surfaces.Depth->QueryInterface(IID_IDirectDrawSurface4, (void**)&State.DX.Active.Surfaces.Active.Depth);

            if (result != DD_OK)
            {
                if (result != DDERR_OUTOFMEMORY && result != DDERR_OUTOFVIDEOMEMORY)
                {
                    LOGWARNING("CreateSurface for Z-buffer failed %d.\n", result);

                    return FALSE;
                }

                LOGWARNING("There was not enough video memory to create the Z-buffer surface.\nPlease restart the program and try another fullscreen mode with less resolution or lower bit depth.\n");

                return FALSE;
            }
        }

        {
            HRESULT result = State.DX.Active.Surfaces.Back->AddAttachedSurface(State.DX.Active.Surfaces.Depth);

            if (result == DD_OK)
            {
                DDSURFACEDESC2 sdesc;
                ZeroMemory(&sdesc, sizeof(DDSURFACEDESC2));

                sdesc.dwSize = sizeof(DDSURFACEDESC2);

                result = State.DX.Active.Surfaces.Active.Depth->GetSurfaceDesc(&sdesc);

                if (result == DD_OK)
                {
                    if (!State.Device.Capabilities.IsAccelerated) { return TRUE; }

                    if (sdesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) { return TRUE; }

                    LOGWARNING("Could not fit the Z-buffer in video memory for this hardware device.\n");
                }
                else { LOGWARNING("Failed to get surface description of Z buffer %d.\n", result); }
            }
            else { LOGWARNING("AddAttachedBuffer failed for Z-Buffer %d.\n", result); }
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

    // 0x60004d10
    HRESULT CALLBACK EnumerateRendererDevicePixelFormats(LPDDPIXELFORMAT format, LPVOID context)
    {
        if (format->dwFlags & DDPF_ZBUFFER)
        {
            CopyMemory(context, format, sizeof(DDPIXELFORMAT));

            return D3DENUMRET_CANCEL;
        }

        return D3DENUMRET_OK;
    }

    // 0x60006e50
    void AcquireRendererDeviceTextureFormats(void)
    {
        State.Textures.Formats.Count = 0;

        s32 count = INVALID_TEXTURE_FORMAT_COUNT;
        State.DX.Device->EnumTextureFormats(EnumerateRendererDeviceTextureFormats, &count);

        State.Textures.Formats.Indexes[0] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[1] = AcquireRendererDeviceTextureFormatIndex(4, 0, 0, 0, 0, FALSE);
        State.Textures.Formats.Indexes[2] = AcquireRendererDeviceTextureFormatIndex(8, 0, 0, 0, 0, FALSE);
        State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 1, 5, 5, 5, FALSE);
        State.Textures.Formats.Indexes[4] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 6, 5, FALSE);
        State.Textures.Formats.Indexes[5] = AcquireRendererDeviceTextureFormatIndex(0, 0, 8, 8, 8, FALSE);
        State.Textures.Formats.Indexes[6] = AcquireRendererDeviceTextureFormatIndex(0, 8, 8, 8, 8, FALSE);
        State.Textures.Formats.Indexes[7] = AcquireRendererDeviceTextureFormatIndex(0, 4, 4, 4, 4, FALSE);
        State.Textures.Formats.Indexes[12] = AcquireRendererDeviceTextureFormatIndex(0, 0, 0, 0, 0, TRUE);

        RendererTextureFormatStates[0] = 0; // TODO
        RendererTextureFormatStates[1] = State.Textures.Formats.Indexes[1] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[2] = State.Textures.Formats.Indexes[2] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[4] = State.Textures.Formats.Indexes[4] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[5] = State.Textures.Formats.Indexes[5] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[6] = State.Textures.Formats.Indexes[6] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[7] = State.Textures.Formats.Indexes[7] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[12] = State.Textures.Formats.Indexes[12] != INVALID_TEXTURE_FORMAT_INDEX;

        if (State.Textures.Formats.Indexes[3] == INVALID_TEXTURE_FORMAT_INDEX)
        {
            State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 5, 5, FALSE);
            RendererTextureFormatStates[3] = (State.Textures.Formats.Indexes[3] != INVALID_TEXTURE_FORMAT_INDEX) ? 5 : 0; // TODO
        }
        else
        {
            RendererTextureFormatStates[3] = 1; // TODO
        }
    }

    // 0x60007090
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDPIXELFORMAT format, LPVOID context)
    {
        ZeroMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count], sizeof(TextureFormat));

        CopyMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat, format, sizeof(DDPIXELFORMAT));

        State.Textures.Formats.Formats[State.Textures.Formats.Count].IsDXT = FALSE;

        if (format->dwFlags & DDPF_FOURCC)
        {
            if (format->dwFourCC == FOURCC_DXT1)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsDXT = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXT = FOURCC_DXT1;
            }
            else if (format->dwFourCC == FOURCC_DXT2)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsDXT = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXT = FOURCC_DXT2;
            }
            else if (format->dwFourCC == FOURCC_DXT3)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsDXT = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXT = FOURCC_DXT3;
            }
        }

        if (format->dwFlags & DDPF_PALETTEINDEXED8)
        {
            State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
            State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 8;
        }
        else
        {
            if (format->dwFlags & DDPF_PALETTEINDEXED4)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 4;
            }
            else
            {
                if (format->dwFlags & DDPF_RGB)
                {
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = FALSE;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 0;

                    u32 red = 0;

                    {
                        u32 value = format->dwRBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { red = red + 1; value = value >> 1; }
                    }

                    u32 green = 0;

                    {
                        u32 value = format->dwGBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { green = green + 1; value = value >> 1; }
                    }

                    u32 blue = 0;

                    {
                        u32 value = format->dwBBitMask;

                        while ((value & 1) == 0) { value = value >> 1; }

                        while ((value & 1) != 0) { blue = blue + 1; value = value >> 1; }
                    }

                    State.Textures.Formats.Formats[State.Textures.Formats.Count].RedBitCount = red;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].GreenBitCount = green;
                    State.Textures.Formats.Formats[State.Textures.Formats.Count].BlueBitCount = blue;

                    if (format->dwFlags & DDPF_ALPHAPIXELS)
                    {
                        u32 alpha = 0;

                        {
                            u32 value = format->dwRGBAlphaBitMask;

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

    // 0x60007000
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue, const BOOL dxt)
    {
        for (u32 x = 0; x < State.Textures.Formats.Count; x++)
        {
            const TextureFormat* format = &State.Textures.Formats.Formats[x];

            if (format->RedBitCount == red && format->GreenBitCount == green && format->BlueBitCount == blue)
            {
                if (format->PaletteColorBits == palette)
                {
                    if (format->AlphaBitCount == alpha && format->IsDXT == dxt) { return x; }
                    else if (format->PaletteColorBits == palette && palette != 0) { return x; }
                }
            }
            else if (format->PaletteColorBits == palette && palette != 0) { return x; }
        }

        return INVALID_TEXTURE_FORMAT_INDEX;
    }

    // 0x60004d40
    void InitializeRendererState(void)
    {
        State.DX.Device->BeginScene();

        InitializeRendererTransforms();

        State.DX.Device->SetCurrentViewport(State.DX.ViewPort);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);

        State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_MAGFILTER, D3DTFN_LINEAR);
        State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_MINFILTER, D3DTFN_LINEAR);

        if (!State.Device.Capabilities.IsModulateBlending)
        {
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
        else
        {
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        }

        State.DX.Device->SetTextureStageState(RENDERER_TEXTURE_STAGE_0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 0x10101);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_EXP2);

        {
            f32 value = 0.0f;
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD*)(&value));
        }

        {
            f32 value = 1.0f; // ORIGINAL: 1.4013e-45f;
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGEND, *(DWORD*)(&value));
        }

        State.DX.Device->EndScene();
    }

    // 0x6000e500
    void InitializeRendererTransforms(void)
    {
        SelectRendererTransforms(1.0f, 65535.0f);
    }

    // 0x600030b0
    u32 SelectRendererTransforms(const f32 zNear, const f32 zFar)
    {
        if (zFar <= zNear) { return TRUE; }

        D3DMATRIX world;

        world._11 = 1.0f;
        world._12 = 0.0f;
        world._13 = 0.0f;
        world._14 = 0.0f;

        world._21 = 0.0f;
        world._22 = 1.0f;
        world._23 = 0.0f;
        world._24 = 0.0f;

        world._31 = 0.0f;
        world._32 = 0.0f;
        world._33 = 1.0f;
        world._34 = 0.0f;

        world._41 = 0.0f;
        world._42 = 0.0f;
        world._43 = 0.0f;
        world._44 = 1.0f;

        D3DMATRIX view;

        view._11 = 1.0f;
        view._12 = 0.0f;
        view._13 = 0.0f;
        view._14 = 0.0f;

        view._21 = 0.0f;
        view._22 = 1.0f;
        view._23 = 0.0f;
        view._24 = 0.0f;

        view._31 = 0.0f;
        view._32 = 0.0f;
        view._33 = 1.0f;
        view._34 = 0.0f;

        view._41 = 0.0f;
        view._42 = 0.0f;
        view._43 = 0.0f;
        view._44 = 1.0f;

        D3DMATRIX projection;

        projection._11 = 1.0f;
        projection._12 = 0.0f;
        projection._13 = 0.0f;
        projection._14 = 0.0f;

        projection._21 = 0.0f;
        projection._22 = 1.0f;
        projection._23 = 0.0f;
        projection._24 = 0.0f;

        projection._31 = 0.0f;
        projection._32 = 0.0f;
        projection._33 = 1.0f;
        projection._34 = 0.0f;

        projection._41 = 0.0f;
        projection._42 = 0.0f;
        projection._43 = 0.0f;
        projection._44 = 1.0f;

        HRESULT result = State.DX.Device->SetTransform(D3DTRANSFORMSTATE_WORLD, &world);

        if (result == DD_OK)
        {
            result = State.DX.Device->SetTransform(D3DTRANSFORMSTATE_VIEW, &view);

            if (result == DD_OK)
            {
                projection._44 = zNear;
                projection._34 = 1.0f;
                projection._33 = zNear / (zFar - zNear) + 1.0f;
                projection._43 = 0.0f;

                result = State.DX.Device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &projection);
            }
        }

        return result;
    }

    // 0x60001220
    void InitializeViewPort(void)
    {
        State.ViewPort.X0 = 0;
        State.ViewPort.Y0 = 0;
        State.ViewPort.X1 = 0;
        State.ViewPort.Y1 = 0;
    }
}