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
#include "Settings.hxx"

#include <malloc.h>
#include <math.h>
#include <stdio.h>

#define MAX_MESSAGE_BUFFER_LENGTH 512

#define MAX_RENDERER_MESSAGE_BUFFER_LENGTH 80
#define MAX_RENDERER_MESSAGE_DESCRIPTION_BUFFER_LENGTH 64

#define MAX_SETTINGS_BUFFER_LENGTH 80

using namespace Renderer;
using namespace RendererModuleValues;
using namespace Settings;

namespace RendererModule
{
    RendererModuleState State;

    u32 DAT_6005ab50; // TODO
    u32 DAT_6005ab5c; // TODO

    // 0x60009250
    s32 AcquireSettingsValue(const s32 value, const char* section, const char* name)
    {
        char buffer[MAX_SETTINGS_BUFFER_LENGTH];

        sprintf(buffer, "%s_%s", section, name);

        const char* tv = getenv(buffer);

        if (tv == NULL)
        {
            sprintf(buffer, "THRASH_%s", name);

            const char* ttv = getenv(buffer);

            return ttv == NULL ? value : atoi(ttv);
        }

        return atoi(tv);
    }

    // 0x6000b9e0
    // 0x6000ba10
    // NOTE: Originally there are 3 different methods for error, warning, and info (which is never being called).
    void Message(const u32 severity, const char* format, ...)
    {
        char buffer[MAX_MESSAGE_BUFFER_LENGTH];

        va_list args;
        va_start(args, format);
        vsnprintf_s(buffer, MAX_MESSAGE_BUFFER_LENGTH, format, args);
        va_end(args);

        if (severity == RENDERER_MODULE_MESSAGE_SEVERITY_ERROR)
        {
            if (State.Lambdas.Log != NULL) { State.Lambdas.Log(severity, buffer); }
            else { MessageBoxA(NULL, buffer, "Abort Message", MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONERROR | MB_OKCANCEL); }
        }
    }

    // 0x6000d580
    void InitializeVertexes(void* vertexes, const u32 count)
    {
        for (u32 x = 0; x < count; x++)
        {
            f32x3* xyz = (f32x3*)((addr)vertexes + (addr)(x * RendererVertexSize));

            xyz->X = xyz->X - 0.5f;
            xyz->Y = xyz->Y - 0.5f;
        }
    }

    // 0x600036b0
    void ReleaseRendererModule(void)
    {
        RestoreGameWindow();
    }

    // 0x6000b970
    const char* AcquireRendererMessageDescription(const HRESULT code)
    {
        static char buffer[MAX_RENDERER_MESSAGE_DESCRIPTION_BUFFER_LENGTH]; // 0x60058db0

        sprintf(buffer, "Direct draw error code (0x%lx, %d)", code, code & 0xffff);

        return buffer;
    }

    // 0x60003a20
    const char* AcquireRendererMessage(const HRESULT code)
    {
        static char buffer[MAX_RENDERER_MESSAGE_BUFFER_LENGTH]; // 0x600186f0

        sprintf(buffer, "DX7 Error Code: %s (%8x)", AcquireRendererMessageDescription(code), code);

        return buffer;
    }

    // 0x60001f40
    u32 AcquireRendererDeviceCount(void)
    {
        State.Devices.Count = 0;
        State.Device.Identifier = NULL;

        GUID* uids[MAX_ENUMERATE_RENDERER_DEVICE_COUNT];

        State.Devices.Count = AcquireDirectDrawDeviceCount(uids, NULL, RENDERER_MODULE_ENVIRONMENT_SECTION_NAME);

        u32 indx = 0;

        for (u32 x = 0; x < State.Devices.Count; x++)
        {
            if (AcquireRendererDeviceAccelerationState(x))
            {
                State.Devices.Indexes[indx] = uids[x];

                IDirectDraw* instance = NULL;
                DirectDrawCreate(uids[x], &instance, NULL);

                IDirectDraw7* dd = NULL;
                if (instance->QueryInterface(IID_IDirectDraw7, (void**)&dd) != DD_OK) { instance->Release(); continue; }

                DDDEVICEIDENTIFIER2 identifier;
                ZeroMemory(&identifier, sizeof(DDDEVICEIDENTIFIER2));

                dd->GetDeviceIdentifier(&identifier, DDGDI_GETHOSTIDENTIFIER);

                strncpy(State.Devices.Enumeration.Names[indx], identifier.szDescription, MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH);

                if (dd != NULL) { dd->Release(); dd = NULL; }
                if (instance != NULL) { instance->Release(); instance = NULL; }

                indx = indx + 1;
            }
        }

        State.Device.Identifier = State.Devices.Indexes[0];
        State.Devices.Count = indx;

        return State.Devices.Count;
    }

    // 0x6000bb10
    u32 AcquireDirectDrawDeviceCount(GUID** uids, HMONITOR** monitors, const char* section)
    {
        State.Devices.Enumeration.Count = 0;
        State.Devices.Enumeration.IsAvailable = FALSE;

        {
            const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONMENT_PROPERTY_NAME);

            if (value == NULL || atoi(value) != 0)
            {
                if (!AcquireState(RENDERER_MODULE_STATE_SELECT_WINDOW_MODE_STATE))
                {
                    DirectDrawEnumerateExA(EnumerateDirectDrawDevices, NULL, DDENUM_ATTACHEDSECONDARYDEVICES);
                    DirectDrawEnumerateExA(EnumerateDirectDrawDevices, NULL, DDENUM_NONDISPLAYDEVICES);
                }
                else
                {
                    State.Devices.Enumeration.Count = 1;
                    State.Devices.Enumeration.IsAvailable = TRUE;
                }
            }
            else
            {
                State.Devices.Enumeration.Count = 1;
                State.Devices.Enumeration.IsAvailable = TRUE;
            }
        }

        if (uids != NULL)
        {
            for (u32 x = 0; x < State.Devices.Enumeration.Count; x++)
            {
                uids[x] = State.Devices.Enumeration.Identifiers.Indexes[x];
            }
        }

        if (monitors != NULL)
        {
            for (u32 x = 0; x < State.Devices.Enumeration.Count; x++)
            {
                monitors[x] = State.Devices.Enumeration.Monitors.Indexes[x];
            }
        }

        State.Devices.Enumeration.Count = AcquireSettingsValue(State.Devices.Enumeration.Count, section, "displays");

        return State.Devices.Enumeration.Count;
    }

    // 0x6000bbd0
    BOOL CALLBACK EnumerateDirectDrawDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context, HMONITOR monitor)
    {
        IDirectDraw* instance = NULL;
        if (DirectDrawCreate(uid, &instance, NULL) != DD_OK) { return FALSE; }

        IDirectDraw4* dd = NULL;
        if (instance->QueryInterface(IID_IDirectDraw4, (void**)&dd) != DD_OK) { return FALSE; }

        instance->Release();

        DDDEVICEIDENTIFIER idn;
        ZeroMemory(&idn, sizeof(DDDEVICEIDENTIFIER));

        dd->GetDeviceIdentifier(&idn, DDGDI_NONE);

        DDDEVICEIDENTIFIER idh;
        ZeroMemory(&idh, sizeof(DDDEVICEIDENTIFIER));

        dd->GetDeviceIdentifier(&idh, DDGDI_GETHOSTIDENTIFIER);

        BOOL skip = FALSE;

        if (State.Devices.Enumeration.Count == 0)
        {
            CopyMemory(&State.Devices.Enumeration.Identifier, &idh, sizeof(DDDEVICEIDENTIFIER));
        }
        else
        {
            const BOOL same = strcmp(State.Devices.Enumeration.Identifier.szDescription, idh.szDescription) == 0;

            if (!same && (!State.Devices.Enumeration.IsAvailable || uid == NULL))
            {
                skip = TRUE;

                if (uid != NULL) { State.Devices.Enumeration.IsAvailable = TRUE; }
            }
        }

        if (dd != NULL) { dd->Release(); }

        if (!skip && State.Devices.Enumeration.Count < MAX_ENUMERATE_RENDERER_DEVICE_COUNT)
        {
            if (uid != NULL)
            {
                State.Devices.Enumeration.Identifiers.Identifiers[State.Devices.Enumeration.Count] = *uid;
                State.Devices.Enumeration.Monitors.Monitors[State.Devices.Enumeration.Count] = monitor;
            }

            State.Devices.Enumeration.Identifiers.Indexes[State.Devices.Enumeration.Count] = &State.Devices.Enumeration.Identifiers.Identifiers[State.Devices.Enumeration.Count];
            State.Devices.Enumeration.Monitors.Indexes[State.Devices.Enumeration.Count] = &State.Devices.Enumeration.Monitors.Monitors[State.Devices.Enumeration.Count];

            State.Devices.Enumeration.Count = State.Devices.Enumeration.Count + 1;
        }

        return TRUE;
    }

    // 0x6000bd60
    BOOL AcquireRendererDeviceAccelerationState(const u32 indx)
    {
        IDirectDraw* instance = NULL;
        if (DirectDrawCreate(State.Devices.Enumeration.Identifiers.Indexes[indx], &instance, NULL) != DD_OK) { return FALSE; }

        IDirectDraw4* dd = NULL;
        if (instance->QueryInterface(IID_IDirectDraw4, (void**)&dd) != DD_OK) { return FALSE; }

        instance->Release();

        DDCAPS hal;
        ZeroMemory(&hal, sizeof(DDCAPS));

        hal.dwSize = sizeof(DDCAPS);

        dd->GetCaps(&hal, NULL);

        dd->Release();

        return hal.dwCaps & DDCAPS_3D;
    }

    // 0x600034c0
    u32 InitializeRendererDeviceLambdas(void)
    {
        if (State.Mutex == NULL) { State.Mutex = CreateEventA(NULL, FALSE, FALSE, NULL); }

        State.Window.HWND = State.Lambdas.Lambdas.AcquireWindow();

        if (State.Window.HWND != NULL)
        {
            State.Settings.CooperativeLevel = (State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))
                | (DDSCL_MULTITHREADED | DDSCL_NORMAL);

            AcquireWindowModeCapabilities();

            State.Settings.CooperativeLevel = State.Settings.IsWindowMode
                ? State.Settings.CooperativeLevel | DDSCL_MULTITHREADED | DDSCL_NORMAL
                : State.Settings.CooperativeLevel | DDSCL_MULTITHREADED | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

            SetForegroundWindow(State.Window.HWND);
            PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, 0, 0);
            WaitForSingleObject(State.Mutex, INFINITE);

            return State.DX.Code;
        }

        State.DX.Code = RENDERER_MODULE_INITIALIZE_DEVICE_SUCCESS;

        return RENDERER_MODULE_INITIALIZE_DEVICE_SUCCESS;
    }

    // 0x600030b0
    u32 InitializeRendererDevice(void)
    {
        if (State.Window.HWND != NULL) { return RENDERER_MODULE_FAILURE; }

        State.Settings.CooperativeLevel = (State.Settings.CooperativeLevel & ~(DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))
            | (DDSCL_MULTITHREADED | DDSCL_NORMAL);

        AcquireWindowModeCapabilities();

        State.Settings.CooperativeLevel = State.Settings.IsWindowMode
            ? State.Settings.CooperativeLevel | (DDSCL_MULTITHREADED | DDSCL_NORMAL)
            : State.Settings.CooperativeLevel | (DDSCL_MULTITHREADED | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);

        IDirectDraw* instance = NULL;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

        if (State.DX.Code != DD_OK) { return RENDERER_MODULE_FAILURE; }

        State.DX.Code = DirectDrawCreateEx(State.Device.Identifier, (void**)&State.DX.Instance, IID_IDirectDraw7, NULL);

        instance->Release();

        if (State.DX.Code != DD_OK) { return RENDERER_MODULE_FAILURE; }

        State.DX.Code = State.DX.Instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

        if (State.DX.Code == DD_OK) { return RENDERER_MODULE_FAILURE; }

        u32 pitch = 0;
        u32 height = 0;

        {
            DDSURFACEDESC2 desc;
            ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

            desc.dwSize = sizeof(DDSURFACEDESC2);
            desc.dwFlags = DDSD_CAPS;
            desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

            IDirectDrawSurface7* surface = NULL;
            State.DX.Code = State.DX.Instance->CreateSurface(&desc, &surface, NULL);

            if (State.DX.Code == DD_OK)
            {
                surface->GetSurfaceDesc(&desc);

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

                caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                if (State.DX.Instance->GetAvailableVidMem(&caps, &total, &free) == DD_OK)
                {
                    State.Settings.MaxAvailableMemory = total;

                    ModuleDescriptor.VideoMemorySize = total;
                    ModuleDescriptor.TotalMemorySize = 3; // TODO
                }
                else
                {
                    State.Settings.MaxAvailableMemory = 0;

                    ModuleDescriptor.VideoMemorySize = 0;
                    ModuleDescriptor.TotalMemorySize = 0; // TODO
                }
            }

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

        AcquireRendererDeviceCapabilities();

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002eb0
    void AcquireWindowModeCapabilities(void)
    {
        IDirectDraw* instance = NULL;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);

        State.DX.Code = instance->SetCooperativeLevel(State.Window.HWND, State.Settings.CooperativeLevel);

        IDirectDraw7* dd = NULL;
        State.DX.Code = instance->QueryInterface(IID_IDirectDraw7, (void**)&dd);

        DDCAPS hal;
        ZeroMemory(&hal, sizeof(DDCAPS));

        hal.dwSize = sizeof(DDCAPS);

        DDCAPS hel;
        ZeroMemory(&hel, sizeof(DDCAPS));

        hel.dwSize = sizeof(DDCAPS);

        dd->GetCaps(&hal, &hel);

        if ((hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) == 0) { State.Settings.IsWindowMode = FALSE; }

        const HWND hwnd = GetDesktopWindow();
        const HDC hdc = GetWindowDC(hwnd);

        const u32 bits = GetDeviceCaps(hdc, BITSPIXEL);

        if (bits < (GRAPHICS_BITS_PER_PIXEL_8 + 1) || bits == GRAPHICS_BITS_PER_PIXEL_24) { State.Settings.IsWindowMode = FALSE; }
        else if (bits == GRAPHICS_BITS_PER_PIXEL_32)
        {
            IDirect3D3* dx = NULL;

            {
                const HRESULT result = instance->QueryInterface(IID_IDirect3D3, (void**)&dx);

                if (result != DD_OK) { LOGERROR("SetDesktopMode Test failed! %s\n", AcquireRendererMessage(result)); }
            }

            {
                const HRESULT result = dx->EnumZBufferFormats(IID_IDirect3DHALDevice, EnumerateRendererDeviceDepthPixelFormats, NULL);

                if (result != DD_OK) { LOGERROR("DX7_SetDesktopModeIfNotWin: %s\n", AcquireRendererMessage(result)); }
            }

            BOOL found = FALSE;

            for (u32 x = 0; x < MAX_TEXTURE_DEPTH_FORMAT_COUNT; x++)
            {
                if (State.Textures.Formats.Depth.Formats[x] == bits) { found = TRUE; }
            }

            if (!found) { State.Settings.IsWindowMode = FALSE; }

            if (dx != NULL) { dx->Release(); }
        }

        dd->Release();
    }

    // 0x60003080
    HRESULT CALLBACK EnumerateRendererDeviceDepthPixelFormats(LPDDPIXELFORMAT format, LPVOID ctx)
    {
        State.Textures.Formats.Depth.Formats[State.Textures.Formats.Depth.Count] = format->dwRGBBitCount;
        State.Textures.Formats.Depth.Count = State.Textures.Formats.Depth.Count + 1;

        // NOTE: The original does not have this check,
        // thus it is prone the array overflow that can cause crash in some cases.
        if (MAX_TEXTURE_DEPTH_FORMAT_COUNT <= (State.Textures.Formats.Depth.Count + 1)) { return D3DENUMRET_CANCEL; }

        return D3DENUMRET_OK;
    }

    // 0x60002578
    void AcquireRendererDeviceCapabilities(void)
    {
        IDirect3D7* dx = NULL;
        if (State.DX.Instance->QueryInterface(IID_IDirect3D7, (void**)&dx) == DD_OK)
        {
            dx->EnumDevices(EnumerateDirectDrawAcceleratedDevices, NULL);

            ZeroMemory(ModuleDescriptor.Capabilities.Capabilities,
                MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT * sizeof(RendererModuleDescriptorDeviceCapabilities));

            ModuleDescriptor.Capabilities.Count = 0;

            State.DX.Instance->EnumDisplayModes(DDEDM_NONE, NULL, 0, EnumerateRendererDeviceModes);

            ModuleDescriptor.Capabilities.Count = ModuleDescriptor.Capabilities.Count + 1;
        }

        if (dx != NULL) { dx->Release(); }
    }

    // 0x600025ec
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC2 desc, LPVOID context)
    {
        if ((MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT - 1) < ModuleDescriptor.Capabilities.Count) { return DDENUMRET_CANCEL; }

        const u32 format = AcquirePixelFormat(&desc->ddpfPixelFormat);

        if (format != RENDERER_PIXEL_FORMAT_NONE
            && (GRAPHICS_RESOLUTION_640 - 1) < desc->dwWidth && (GRAPHICS_RESOLUTION_480 - 1) < desc->dwHeight)
        {
            const u32 bits = desc->ddpfPixelFormat.dwRGBBitCount;

            if (bits != GRAPHICS_BITS_PER_PIXEL_16 && bits != GRAPHICS_BITS_PER_PIXEL_32) { return DDENUMRET_OK; }

            const u32 mask = (bits == GRAPHICS_BITS_PER_PIXEL_16) ? DEPTH_BIT_MASK_16_BIT : DEPTH_BIT_MASK_32_BIT;

            if (State.Device.Capabilities.RendererDeviceDepthBits & mask)
            {
                const u32 bytes = ((bits < GRAPHICS_BITS_PER_PIXEL_16) ? GRAPHICS_BITS_PER_PIXEL_16 : bits) >> 3;

                const u32 count = State.Settings.MaxAvailableMemory / (desc->dwHeight * desc->dwWidth * bytes);

                ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Width = desc->dwWidth;
                ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Height = desc->dwHeight;
                ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Bits =
                    format == RENDERER_PIXEL_FORMAT_16_BIT_555 ? (GRAPHICS_BITS_PER_PIXEL_16 - 1) : bits;
                ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].IsActive = TRUE;

                if (count < 4) // TODO
                {
                    ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Unk03 = count;
                    ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Unk04 = count - 1;
                }
                else
                {
                    ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Unk03 = 3;
                    ModuleDescriptor.Capabilities.Capabilities[ModuleDescriptor.Capabilities.Count].Unk04 = 3;
                }

                ModuleDescriptor.Capabilities.Count = ModuleDescriptor.Capabilities.Count + 1;
            }
        }

        return DDENUMRET_OK;
    }

    // 0x60003960
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

    // 0x600021f0
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        IDirectDraw* instance = NULL;
        State.DX.Code = DirectDrawCreate(State.Device.Identifier, &instance, NULL);
        State.DX.Code = DirectDrawCreateEx(State.Device.Identifier, (void**)&State.DX.Instance, IID_IDirectDraw7, NULL);

        if (State.DX.Code == DD_OK)
        {
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

                    IDirectDrawSurface7* surface = NULL;
                    State.DX.Code = State.DX.Instance->CreateSurface(&desc, &surface, NULL);

                    if (State.DX.Code == DD_OK)
                    {
                        surface->GetSurfaceDesc(&desc);

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

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

                        if (State.DX.Instance->GetAvailableVidMem(&caps, &total, &free) == DD_OK)
                        {
                            State.Settings.MaxAvailableMemory = total;

                            ModuleDescriptor.VideoMemorySize = total;
                            ModuleDescriptor.TotalMemorySize = 3; // TODO
                        }
                        else
                        {
                            State.Settings.MaxAvailableMemory = 0;

                            ModuleDescriptor.VideoMemorySize = 0;
                            ModuleDescriptor.TotalMemorySize = 0; // TODO
                        }
                    }

                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

                        caps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;

                        const HRESULT result = State.DX.Instance->GetAvailableVidMem(&caps, &total, &free);

                        State.Settings.MaxAvailableMemory = result == DD_OK
                            ? height * pitch + total
                            : MIN_RENDERER_DEVICE_AVAIABLE_VIDEO_MEMORY;

                        ModuleDescriptor.VideoMemorySize = State.Settings.MaxAvailableMemory;
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

                    hel.dwSize = sizeof(DDCAPS);

                    if (State.DX.Instance->GetCaps(&hal, &hel) == DD_OK)
                    {
                        State.Device.Capabilities.IsAccelerated = hal.dwCaps & DDCAPS_3D;
                    }
                }

                AcquireRendererDeviceCapabilities();
            }
        }

        SetEvent(State.Mutex);

        *result = State.DX.Code;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002e90
    HRESULT CALLBACK EnumerateDirectDrawAcceleratedDevices(LPSTR description, LPSTR name, LPD3DDEVICEDESC7 desc, LPVOID context)
    {
        const u32 result = (desc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) == 0;

        if (!result) { State.Device.Capabilities.RendererDeviceDepthBits = desc->dwDeviceRenderBitDepth; }

        return result;
    }

    // 0x60002d00
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDeviceSurfaces();

        State.Lambdas.Lambdas.SelectInstance(NULL);

        if (State.DX.Instance != NULL)
        {
            State.DX.Instance->Release();
            State.DX.Instance = NULL;
        }

        SetEvent(State.Mutex);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003610
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

    // 0x60002850
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result)
    {
        ReleaseRendererDevice();
        ReleaseRendererDeviceSurfaces();

        if (!State.Settings.IsWindowMode)
        {
            State.Window.Width = ModuleDescriptor.Capabilities.Capabilities[wp].Width;
            State.Window.Height = ModuleDescriptor.Capabilities.Capabilities[wp].Height;
        }
        else
        {
            if (!State.Settings.IsWindowModeActive)
            {
                RECT rect;
                GetClientRect(State.Window.HWND, &rect);

                State.Window.Width = rect.right - rect.left;
                State.Window.Height = rect.bottom - rect.top;
            }
            else
            {
                HDC hdc = GetDC(hwnd);

                State.Window.Width = GetDeviceCaps(hdc, HORZRES);
                State.Window.Height = GetDeviceCaps(hdc, VERTRES);

                ReleaseDC(hwnd, hdc);
            }

            if (State.Window.Width == 0 && State.Window.Height == 0)
            {
                State.Window.Width = ModuleDescriptor.Capabilities.Capabilities[wp].Width;
                State.Window.Height = ModuleDescriptor.Capabilities.Capabilities[wp].Height;
            }
        }

        const u32 bits = ModuleDescriptor.Capabilities.Capabilities[wp].Bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1)
            ? GRAPHICS_BITS_PER_PIXEL_16 : ModuleDescriptor.Capabilities.Capabilities[wp].Bits;

        if (!State.Settings.IsWindowMode)
        {
            State.DX.Code = State.DX.Instance->SetDisplayMode(ModuleDescriptor.Capabilities.Capabilities[wp].Width,
                ModuleDescriptor.Capabilities.Capabilities[wp].Height, bits, 0, DDSDM_NONE);

            if (State.DX.Code == DD_OK)
            {
                DDSURFACEDESC2 desc;
                ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                desc.dwSize = sizeof(DDSURFACEDESC2);

                if (lp - 1 == 0)
                {
                    State.DX.Active.Surfaces.Active.Back = NULL;
                    State.DX.Active.Surfaces.Back = NULL;
                    State.DX.Surfaces.Active[2] = NULL;
                }
                else
                {
                    desc.dwFlags = DDSD_BACKBUFFERCOUNT | DDSD_CAPS;
                    desc.dwBackBufferCount = lp - 1;
                }

                const u32 options = (lp - 1 == 0)
                    ? DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE
                    : DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

                desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
                    ? options | DDSCAPS_VIDEOMEMORY
                    : options | DDSCAPS_SYSTEMMEMORY;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);

                while (State.DX.Code == DDERR_OUTOFVIDEOMEMORY && 1 < desc.dwBackBufferCount)
                {
                    desc.dwBackBufferCount = desc.dwBackBufferCount - 1;

                    State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Main, NULL);
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    if (State.DX.Surfaces.Back == NULL && desc.dwBackBufferCount != 0)
                    {
                        DDSCAPS2 caps;
                        ZeroMemory(&caps, sizeof(DDSCAPS2));

                        caps.dwCaps = DDSCAPS_BACKBUFFER;

                        State.DX.Code = State.DX.Surfaces.Main->GetAttachedSurface(&caps, &State.DX.Surfaces.Back);
                    }
                }

                if (State.DX.Surfaces.Main != NULL)
                {
                    State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface7, (void**)&State.DX.Surfaces.Active[1]); // TODO
                }

                if (State.DX.Surfaces.Back != NULL)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface7, (void**)&State.DX.Surfaces.Active[2]); // TODO

                    {
                        ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                        desc.dwSize = sizeof(DDSURFACEDESC2);

                        State.DX.Code = State.DX.Surfaces.Active[2]->GetSurfaceDesc(&desc);
                        State.Device.Capabilities.IsVideoMemoryCapable = (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0;
                    }
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
                    State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawSurface7, (void**)&State.DX.Surfaces.Active[1]); // TODO
                }

                desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
                desc.dwHeight = State.Window.Height;
                desc.dwWidth = State.Window.Width;
                desc.ddsCaps.dwCaps = RendererDeviceType == RENDERER_MODULE_DEVICE_TYPE_1_ACCELERATED
                    ? DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN
                    : DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;

                State.DX.Code = State.DX.Instance->CreateSurface(&desc, &State.DX.Surfaces.Back, NULL);

                if (State.DX.Code == DD_OK)
                {
                    State.DX.Surfaces.Back->QueryInterface(IID_IDirectDrawSurface7, (void**)&State.DX.Surfaces.Active[2]); // TODO

                    {
                        ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                        desc.dwSize = sizeof(DDSURFACEDESC2);

                        State.DX.Code = State.DX.Surfaces.Active[2]->GetSurfaceDesc(&desc);
                        State.Device.Capabilities.IsVideoMemoryCapable = (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0;
                    }
                }
                else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { LOGWARNING("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
                else { LOGWARNING("CreateSurface for window back buffer failed %s.\n", AcquireRendererMessage(State.DX.Code)); }
            }
            else if (State.DX.Code == DDERR_OUTOFMEMORY || State.DX.Code == DDERR_OUTOFVIDEOMEMORY) { LOGWARNING("There was not enough video memory to create the rendering surface.\nTo run this program in a window of this size, please adjust your display settings for a smaller desktop area or a lower palette size and restart the program.\n"); }
            else { LOGWARNING("CreateSurface for window front buffer failed %s.\n", AcquireRendererMessage(State.DX.Code)); }
        }

        InitializeRendererDeviceAcceleration();

        if (State.DX.GammaControl != NULL)
        {
            if (State.DX.GammaControl->GetGammaRamp(0, &State.Settings.GammaControl) != DD_OK)
            {
                State.DX.GammaControl->Release();
                State.DX.GammaControl = NULL;
            }
        }

        if (State.Lambdas.Lambdas.AcquireWindow != NULL)
        {
            SetEvent(State.Mutex);

            *result = State.DX.Code;
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60009680
    // a.k.a. createD3D
    u32 InitializeRendererDeviceAcceleration(void)
    {
        State.DX.Active.Instance = State.DX.Instance;

        State.DX.Active.Surfaces.Main = State.DX.Surfaces.Main;
        State.DX.Active.Surfaces.Back = State.DX.Surfaces.Back;

        State.DX.Active.Surfaces.Active.Main = State.DX.Surfaces.Active[1]; // TODO
        State.DX.Active.Surfaces.Active.Back = State.DX.Surfaces.Active[2]; // TODO

        State.DX.Surfaces.Main->QueryInterface(IID_IDirectDrawGammaControl, (void**)&State.DX.GammaControl);

        {
            const HRESULT result = State.DX.Instance->QueryInterface(IID_IDirect3D7, (void**)&State.DX.DirectX);

            if (result != DD_OK)
            {
                LOGERROR("Creation of IDirect3D7 failed.\nCheck DX7 installed.\n (%s)\n", AcquireRendererMessage(result));
            }
        }

        InitializeConcreteRendererDevice();

        D3DDEVICEDESC7 caps;
        ZeroMemory(&caps, sizeof(D3DDEVICEDESC7));

        if (State.DX.Device->GetCaps(&caps) != DD_OK) { LOGERROR("GetCaps of IDirect3DDevice7 Failed\n"); } // ORIGINAL: IDirect3D3

        State.Device.Capabilities.IsAccelerated = ((caps.dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) != 0);

        if (caps.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
        {
            ModuleDescriptor.VideoMemorySize = ModuleDescriptor.VideoMemorySize / caps.wMaxSimultaneousTextures;
        }

        State.Device.Capabilities.IsTransformLightBufferSystemMemoryAvailable = (caps.dwDevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY) != 0;
        State.Device.Capabilities.IsTransformLightBufferVideoMemoryAvailable = (caps.dwDevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY) != 0;
        State.Device.Capabilities.MaxActiveLights = caps.dwMaxActiveLights;
        State.Device.Capabilities.MaxVertexBlendMatrices = caps.wMaxVertexBlendMatrices;
        State.Device.Capabilities.MaxUserClipPlanes = caps.wMaxUserClipPlanes;

        State.Device.Capabilities.MaxTextureRepeat = (f32)caps.dwMaxTextureRepeat;
        if (isnan(State.Device.Capabilities.MaxTextureRepeat) != (State.Device.Capabilities.MaxTextureRepeat == 0.0f))
        {
            State.Device.Capabilities.MaxTextureRepeat = 65535.0f;
        }

        State.Device.Capabilities.MaxTextureWidth = caps.dwMaxTextureWidth;
        State.Device.Capabilities.MinTextureWidth = caps.dwMinTextureWidth;

        State.Device.Capabilities.MaxTextureHeight = caps.dwMaxTextureHeight;
        State.Device.Capabilities.MinTextureHeight = caps.dwMinTextureHeight;

        State.Device.Capabilities.IsSquareOnlyTextures = (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0;

        State.Device.Capabilities.MultipleTextureWidth = 1;
        State.Device.Capabilities.MultipleTextureHeight = 1;

        State.Device.Capabilities.IsPowerOfTwoTexturesHeight = (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) != 0;
        State.Device.Capabilities.IsPowerOfTwoTexturesWidth = (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) != 0;

        State.Device.Capabilities.IsTextureIndependentUVs = (caps.dpcTriCaps.dwTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV) != 0;

        State.Device.Capabilities.Capabilities.Unk01 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_NEVER) != 0;
        State.Device.Capabilities.Capabilities.Unk02 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_LESS) != 0;
        State.Device.Capabilities.Capabilities.Unk03 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_EQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk04 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk05 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_GREATER) != 0;
        State.Device.Capabilities.Capabilities.Unk06 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_NOTEQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk07 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_GREATEREQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk08 = (caps.dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_ALWAYS) != 0;

        State.Device.Capabilities.Capabilities.Unk09 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_NEVER) != 0;
        State.Device.Capabilities.Capabilities.Unk10 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_LESS) != 0;
        State.Device.Capabilities.Capabilities.Unk11 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_EQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk12 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk13 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_GREATER) != 0;
        State.Device.Capabilities.Capabilities.Unk14 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk15 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL) != 0;
        State.Device.Capabilities.Capabilities.Unk16 = (caps.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_ALWAYS) != 0;

        State.Device.Capabilities.IsAlphaComparisonAvailable =
            (caps.dpcTriCaps.dwAlphaCmpCaps == D3DPCMPCAPS_ALWAYS || caps.dpcTriCaps.dwAlphaCmpCaps == D3DPCMPCAPS_NEVER) ? FALSE : TRUE;

        DDPIXELFORMAT format;
        ZeroMemory(&format, sizeof(DDPIXELFORMAT));

        format.dwSize = sizeof(DDPIXELFORMAT);

        State.DX.Active.Surfaces.Active.Main->GetPixelFormat(&format);

        State.Device.Capabilities.IsGreenAllowSixBits = format.dwGBitMask == 0x7e0;
        State.Device.Capabilities.RendererBits = caps.dwDeviceRenderBitDepth;
        State.Device.Capabilities.RendererDepthBits = caps.dwDeviceZBufferBitDepth;

        State.Device.Capabilities.AntiAliasing = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT) != 0;

        if (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
        {
            State.Device.Capabilities.AntiAliasing = State.Device.Capabilities.AntiAliasing | RENDERER_MODULE_ANTIALIASING_SORT_INDEPENDENT;
        }

        State.Device.Capabilities.IsAntiAliasEdges = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES) != 0;

        State.Device.Capabilities.IsAnisotropyAvailable = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0;

        if (State.Device.Capabilities.IsAnisotropyAvailable)
        {
            State.Device.Capabilities.MaxAnisotropy = caps.dwMaxAnisotropy;
        }

        State.Device.Capabilities.IsMipMapBiasAvailable = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) != 0;
        State.Device.Capabilities.IsWBufferAvailable = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WBUFFER) != 0;
        State.Device.Capabilities.IsWFogAvailable = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WFOG) != 0;
        State.Device.Capabilities.IsDitherAvailable = (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER) != 0;
        State.Device.Capabilities.IsAlphaTextures = (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA) != 0;
        State.Device.Capabilities.IsPerspectiveTextures = (caps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) != 0;

        {
            const u32 phong = caps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND;
            const u32 gouraud = caps.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_ALPHAFLATBLEND);

            State.Device.Capabilities.IsAlphaFlatBlending = (phong || gouraud) ? TRUE : FALSE;
        }

        State.Device.Capabilities.IsSpecularGouraudBlending = (caps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB) != 0;

        {
            const u32 phong = caps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND;
            const u32 gouraud = caps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND;

            State.Device.Capabilities.IsAlphaProperBlending = (phong || gouraud) ? TRUE : FALSE;
        }

        State.Device.Capabilities.IsModulateBlending = (caps.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA) != 0;

        if ((caps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) == 0)
        {
            State.Device.Capabilities.IsSourceAlphaBlending = TRUE;

            if ((caps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE) == 0)
            {
                State.Device.Capabilities.IsSourceAlphaBlending = FALSE;
            }
        }

        State.Device.Capabilities.IsColorBlending = (caps.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_COLORPHONGRGB | D3DPSHADECAPS_COLORGOURAUDRGB)) != 0;
        State.Device.Capabilities.IsSpecularBlending = (caps.dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_SPECULARPHONGRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB)) != 0;

        if (isnan(caps.dvGuardBandLeft) == (caps.dvGuardBandLeft == 0.0))
        {
            State.Device.Capabilities.GuardBandLeft = caps.dvGuardBandLeft;
            State.Device.Capabilities.GuardBandRight = caps.dvGuardBandRight;
            State.Device.Capabilities.GuardBandTop = caps.dvGuardBandTop;
            State.Device.Capabilities.GuardBandBottom = caps.dvGuardBandBottom;
        }
        else
        {
            State.Device.Capabilities.GuardBandBottom = 0.0f;
            State.Device.Capabilities.GuardBandTop = 0.0f;
            State.Device.Capabilities.GuardBandRight = 0.0f;
            State.Device.Capabilities.GuardBandLeft = 0.0f;
        }

        if (State.Window.Bits == GRAPHICS_BITS_PER_PIXEL_32
            && (caps.dwDeviceZBufferBitDepth & (DEPTH_BIT_MASK_24_BIT | DEPTH_BIT_MASK_32_BIT)) == 0)
        {
            State.Window.Bits = GRAPHICS_BITS_PER_PIXEL_16;
        }

        for (u32 x = 0; x < ModuleDescriptor.Capabilities.Count; x++)
        {
            BOOL found = FALSE;
            u32 value = 0;

            switch (ModuleDescriptor.Capabilities.Capabilities[x].Bits)
            {
            case GRAPHICS_BITS_PER_PIXEL_8: { value = caps.dwDeviceRenderBitDepth & DEPTH_BIT_MASK_8_BIT; found = TRUE; break; }
            case GRAPHICS_BITS_PER_PIXEL_16: { value = caps.dwDeviceRenderBitDepth & DEPTH_BIT_MASK_16_BIT; found = TRUE; break; }
            case GRAPHICS_BITS_PER_PIXEL_24: { value = caps.dwDeviceRenderBitDepth & DEPTH_BIT_MASK_24_BIT; found = TRUE; break; }
            case GRAPHICS_BITS_PER_PIXEL_32: { value = caps.dwDeviceRenderBitDepth & DEPTH_BIT_MASK_32_BIT; found = TRUE; break; }
            }

            if (found && value == 0)
            {
                ModuleDescriptor.Capabilities.Capabilities[x].Width = 0;
                ModuleDescriptor.Capabilities.Capabilities[x].Height = 0;
                ModuleDescriptor.Capabilities.Capabilities[x].Bits = 0;
                ModuleDescriptor.Capabilities.Capabilities[x].Format = RENDERER_PIXEL_FORMAT_NONE;
                ModuleDescriptor.Capabilities.Capabilities[x].IsActive = FALSE;
                ModuleDescriptor.Capabilities.Capabilities[x].Unk03 = 0;
                ModuleDescriptor.Capabilities.Capabilities[x].Unk04 = 0;
            }
        }

        State.Device.Capabilities.MaximumSimultaneousTextures = caps.wMaxSimultaneousTextures;

        DAT_6005ab50 = 0; // TODO
        DAT_6005ab5c = 1; // TODO

        ModuleDescriptor.SubType = 1; // TODO

        {
            DDCAPS hal;
            ZeroMemory(&hal, sizeof(DDCAPS));

            hal.dwSize = sizeof(DDCAPS);

            DDCAPS hel;
            ZeroMemory(&hel, sizeof(DDCAPS));

            hel.dwSize = sizeof(DDCAPS);

            const HRESULT result = State.DX.Active.Instance->GetCaps(&hal, &hel);

            if (result != DD_OK) { LOGERROR("GetCaps of IDirectDraw7 Failed %s\n", AcquireRendererMessage(result)); }

            State.Device.Capabilities.IsWindowMode = (hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) != 0;

            State.Device.Capabilities.IsNoVerticalSyncAvailable = (hal.dwCaps2 & DDCAPS2_FLIPNOVSYNC) != 0;

            State.Device.Capabilities.IsPrimaryGammaAvailable = (hal.dwCaps2 & DDCAPS2_PRIMARYGAMMA) != 0;
        }

        State.Device.Capabilities.IsTrilinearInterpolationAvailable = AcquireRendererDeviceTrilinearInterpolationCapabilities();

        State.Device.Capabilities.IsDepthBufferRemovalAvailable = AcquireRendererDeviceDepthBufferRemovalCapabilities();

        if ((State.Device.Capabilities.RendererDeviceDepthBits & DEPTH_BIT_MASK_32_BIT) == 0)
        {
            if ((State.Device.Capabilities.RendererDepthBits & DEPTH_BIT_MASK_24_BIT) == 0)
            {
                if ((State.Device.Capabilities.RendererDepthBits & DEPTH_BIT_MASK_16_BIT) == 0)
                {
                    State.Device.Capabilities.RendererDepthBits = (State.Device.Capabilities.RendererDepthBits >> 8) & 8;
                }
                else
                {
                    State.Device.Capabilities.RendererDepthBits = GRAPHICS_BITS_PER_PIXEL_16;
                }
            }
            else
            {
                State.Device.Capabilities.RendererDepthBits = GRAPHICS_BITS_PER_PIXEL_24;
            }
        }
        else
        {
            State.Device.Capabilities.RendererDepthBits = GRAPHICS_BITS_PER_PIXEL_32;
        }

        State.Device.Capabilities.IsDepthAvailable = FALSE;

        if (State.Window.Bits != 0 && InitializeRendererDeviceDepthSurfaces(State.Window.Width, State.Window.Height, NULL, NULL))
        {
            State.Device.Capabilities.IsDepthAvailable = TRUE;
        }

        State.DX.Device->Release();

        InitializeConcreteRendererDevice();

        AcquireRendererDeviceTextureFormats();

        D3DVIEWPORT7 vp;
        ZeroMemory(&vp, sizeof(D3DVIEWPORT7));

        vp.dwHeight = State.Window.Height;
        vp.dwWidth = State.Window.Width;

        vp.dvMinZ = 0.0f;
        vp.dvMaxZ = 0.9999847f;

        State.DX.Device->SetViewport(&vp);

        D3DMATERIAL7 material;
        ZeroMemory(&material, sizeof(D3DMATERIAL7));

        State.DX.Device->SetMaterial(&material);

        InitializeRendererState();

        if (State.Device.Capabilities.MaximumSimultaneousTextures != 0)
        {
            const BOOL op1 = (caps.dwTextureOpCaps & (D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_SELECTARG2 | D3DTEXOPCAPS_SELECTARG1)) != 0;
            const BOOL op2 = (caps.dwTextureOpCaps & (D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_SELECTARG2 | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_DISABLE)) != 0;

            for (u32 x = 0; x < State.Device.Capabilities.MaximumSimultaneousTextures; x++)
            {
                State.Textures.Stages[x].Unk11 = op1; // TODO
                State.Textures.Stages[x].Unk12 = op2; // TODO
            }
        }

        InitializeViewPort();

        State.DX.Active.IsInit = TRUE;

        return TRUE;
    }

    // 0x60009f50
    void InitializeConcreteRendererDevice(void)
    {
        HRESULT result = DD_OK;

        State.DX.Active.IsActive = FALSE;

        {
            const char* value = getenv(RENDERER_MODULE_DEVICE_TYPE_ENVIRONMENT_PROPERTY_NAME);

            if (value != NULL) { RendererDeviceType = atoi(value); }
        }

        if (SettingsState.Accelerate)
        {
            const HRESULT res = State.DX.DirectX->CreateDevice(State.Settings.Acceleration == RENDERER_MODULE_ACCELERATION_NORMAL ? IID_IDirect3DHALDevice : IID_IDirect3DTnLHalDevice,
                State.DX.Active.Surfaces.Back == NULL ? State.DX.Active.Surfaces.Main : State.DX.Active.Surfaces.Back, &State.DX.Device);

            if (res == DD_OK) { State.DX.Active.IsActive = TRUE; return; }
        }

        switch (RendererDeviceType)
        {
        case RENDERER_MODULE_DEVICE_TYPE_1_RGB:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRGBDevice, State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_1_REF:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRefDevice, State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_1_RAMP:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DRampDevice, State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_1_MMX:
        {
            State.DX.Active.IsSoft = TRUE;

            result = State.DX.DirectX->CreateDevice(IID_IDirect3DMMXDevice, State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        case RENDERER_MODULE_DEVICE_TYPE_1_INVALID:
        case RENDERER_MODULE_DEVICE_TYPE_1_ACCELERATED:
        {
            State.DX.Active.IsSoft = FALSE;

            result = State.DX.DirectX->CreateDevice(State.Settings.Acceleration == RENDERER_MODULE_ACCELERATION_NORMAL ? IID_IDirect3DHALDevice : IID_IDirect3DTnLHalDevice,
                State.DX.Active.Surfaces.Back == NULL ? State.DX.Active.Surfaces.Main : State.DX.Active.Surfaces.Back, &State.DX.Device);

            break;
        }
        default: { LOGERROR("D3D device type not recognised\n"); return; }
        }

        if (result == DD_OK) { State.DX.Active.IsActive = TRUE; }
        else { LOGERROR(AcquireRendererMessage(result)); }
    }

    // 0x6000aa60
    BOOL AcquireRendererDeviceTrilinearInterpolationCapabilities(void)
    {
        D3DDEVICEDESC7 caps;
        ZeroMemory(&caps, sizeof(D3DDEVICEDESC7));

        State.DX.Device->GetCaps(&caps);

        return (caps.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR) != 0;
    }

    // 0x6000aab0
    BOOL AcquireRendererDeviceDepthBufferRemovalCapabilities(void)
    {
        D3DDEVICEDESC7 caps;
        ZeroMemory(&caps, sizeof(D3DDEVICEDESC7));

        State.DX.Device->GetCaps(&caps);

        return (caps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR) != 0;
    }

    // 0x6000a490
    void InitializeRendererState(void)
    {
        State.DX.Device->BeginScene();

        SelectRendererTransforms(1.0, 65535.0);

        State.DX.Device->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);

        if (State.Device.Capabilities.IsDepthAvailable)
        {
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
        }
        else
        {
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
        }

        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE);

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
        State.DX.Device->SetRenderState(D3DRENDERSTATE_CLIPPING, 0);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_LIGHTING, 0);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_EXTENTS, 0);

        {
            const char* value = getenv(RENDERER_MODULE_WIRE_FRAME_DX7_ENVIRONMENT_PROPERTY_NAME);

            State.DX.Device->SetRenderState(D3DRENDERSTATE_FILLMODE,
                (value == NULL || atoi(value) == 0) ? D3DFILL_SOLID : D3DFILL_WIREFRAME);
        }

        State.DX.Device->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 0xff0000);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);

        {
            const f32 value = 0.0f;
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD*)(&value));
        }

        {
            const f32 value = 1.0f; // ORIGINAL: 1.4013e-45f
            State.DX.Device->SetRenderState(D3DRENDERSTATE_FOGEND, *(DWORD*)(&value));
        }

        State.DX.Device->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, GRAPCHICS_COLOR_WHITE);

        ZeroMemory(State.Textures.Stages, MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT * sizeof(TextureStage));

        State.Scene.IsActive = TRUE;

        SelectState(RENDERER_MODULE_STATE_SELECT_TEXTURE_FILTER_STATE, (void*)RENDERER_MODULE_TEXTURE_FILTER_LENEAR);
        SelectState(RENDERER_MODULE_STATE_SELECT_TEXTURE_MIP_FILTER_STATE, (void*)RENDERER_MODULE_TEXTURE_MIP_FILTER_LINEAR);

        for (u32 x = 0; x < State.Device.Capabilities.MaximumSimultaneousTextures; x++)
        {
            const u32 stage = MAKETEXTURESTAGEMASK(x); // NOTE: Originally a switch statement.

            const u32 message = stage | RENDERER_MODULE_STATE_SELECT_TEXTURE_STAGE_BLEND_STATE;

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_TEXTURE_ALPHA);

                DWORD value = 0;
                State.Textures.Stages[x].Unk09 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD);

                DWORD value = 0;
                State.Textures.Stages[x].Unk01 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD_SIGNED);

                DWORD value = 0;
                State.Textures.Stages[x].Unk08 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE);

                DWORD value = 0;
                State.Textures.Stages[x].Unk03 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_SUBTRACT);

                DWORD value = 0;
                State.Textures.Stages[x].Unk02 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE_2X);

                DWORD value = 0;
                State.Textures.Stages[x].Unk04 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE_4X);

                DWORD value = 0;
                State.Textures.Stages[x].Unk05 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD_SIGNED);

                DWORD value = 0;
                State.Textures.Stages[x].Unk08 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            {
                SelectState(message, (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_DOT_PRODUCT_3);

                DWORD value = 0;
                State.Textures.Stages[x].Unk10 = State.DX.Device->ValidateDevice(&value) == DD_OK;
            }

            SelectState(message, (stage == RENDERER_TEXTURE_STAGE_0 || stage == RENDERER_TEXTURE_STAGE_1)
                ? (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_NORMAL : (void*)RENDERER_MODULE_TEXTURE_STAGE_BLEND_DISABLE);
        }

        SelectState(RENDERER_MODULE_STATE_SELECT_TEXTURE_MIP_FILTER_STATE, (void**)RENDERER_MODULE_TEXTURE_MIP_FILTER_POINT);

        State.DX.Device->EndScene();

        State.Scene.IsActive = FALSE;
    }

    // 0x60001460
    void InitializeViewPort(void)
    {
        State.ViewPort.X0 = 0;
        State.ViewPort.Y0 = 0;
        State.ViewPort.X1 = 0;
        State.ViewPort.Y1 = 0;
    }
    
    // 0x60006f10
    void AttemptRenderScene(void)
    {
        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }
    }

    // 0x60006f20
    void RendererRenderScene(void)
    {
        if (State.Data.Vertexes.Count != 0 && State.Data.Indexes.Count != 0)
        {
            if (!State.Scene.IsActive) { BeginRendererScene(); }

            InitializeVertexes(State.Data.Vertexes.Vertexes, State.Data.Vertexes.Count);

            State.DX.Device->DrawIndexedPrimitive(RendererModuleValues::RendererPrimitiveType, RendererVertexType,
                State.Data.Vertexes.Vertexes, State.Data.Vertexes.Count,
                State.Data.Indexes.Indexes, State.Data.Indexes.Count, 0);

            State.Data.Vertexes.Count = 0;
            State.Data.Indexes.Count = 0;
        }
    }

    // 0x60008810
    BOOL BeginRendererScene(void)
    {
        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        State.Scene.IsActive = State.DX.Device->BeginScene() == DD_OK;

        return State.Scene.IsActive;
    }

    // 0x60008860
    BOOL EndRendererScene(void)
    {
        if (State.Data.Vertexes.Count != 0) { RendererRenderScene(); }

        if (State.Scene.IsActive)
        {
            RendererDepthBias = 0.0f;

            const HRESULT result = State.DX.Device->EndScene();

            State.Scene.IsActive = FALSE;

            return result == DD_OK;
        }

        return TRUE;
    }

    // 0x6000ab00
    u32 SelectRendererTransforms(const f32 zNear, const f32 zFar)
    {
        if ((zFar <= zNear) != (zFar == zNear)) { return TRUE; }

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

    //0x6000c110
    void AcquireRendererDeviceTextureFormats(void)
    {
        State.Textures.Formats.Count = 0;

        s32 count = INVALID_TEXTURE_FORMAT_COUNT;
        State.DX.Device->EnumTextureFormats(EnumerateRendererDeviceTextureFormats, &count);

        State.Textures.Formats.Indexes[0] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[1] = AcquireRendererDeviceTextureFormatIndex(4, 0, 0, 0, 0, 0, 0);
        State.Textures.Formats.Indexes[2] = AcquireRendererDeviceTextureFormatIndex(8, 0, 0, 0, 0, 0, 0);
        State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 1, 5, 5, 5, 0, 0);
        State.Textures.Formats.Indexes[4] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 6, 5, 0, 0);
        State.Textures.Formats.Indexes[5] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[6] = AcquireRendererDeviceTextureFormatIndex(0, 8, 8, 8, 8, 0, 0);
        State.Textures.Formats.Indexes[7] = AcquireRendererDeviceTextureFormatIndex(0, 4, 4, 4, 4, 0, 0);
        State.Textures.Formats.Indexes[12] = AcquireRendererDeviceTextureFormatIndex(0, 0, 0, 0, 0, 1, 0);
        State.Textures.Formats.Indexes[13] = AcquireRendererDeviceTextureFormatIndex(0, 0, 0, 0, 0, 3, 0);
        State.Textures.Formats.Indexes[14] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[15] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[16] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[17] = INVALID_TEXTURE_FORMAT_INDEX;
        State.Textures.Formats.Indexes[18] = AcquireRendererDeviceTextureFormatIndex(0, 0, 8, 8, 0, 0, 1);
        State.Textures.Formats.Indexes[19] = AcquireRendererDeviceTextureFormatIndex(0, 0, 8, 8, 8, 0, 2);
        State.Textures.Formats.Indexes[20] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 5, 6, 0, 3);

        RendererTextureFormatStates[0] = 0; // TODO
        RendererTextureFormatStates[1] = State.Textures.Formats.Indexes[1] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[2] = State.Textures.Formats.Indexes[2] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[4] = State.Textures.Formats.Indexes[4] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[5] = State.Textures.Formats.Indexes[5] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[6] = State.Textures.Formats.Indexes[6] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[7] = State.Textures.Formats.Indexes[7] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[12] = State.Textures.Formats.Indexes[12] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[13] = State.Textures.Formats.Indexes[13] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[14] = State.Textures.Formats.Indexes[14] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[15] = State.Textures.Formats.Indexes[15] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[16] = State.Textures.Formats.Indexes[16] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[17] = State.Textures.Formats.Indexes[17] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[18] = State.Textures.Formats.Indexes[18] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[19] = State.Textures.Formats.Indexes[19] != INVALID_TEXTURE_FORMAT_INDEX;
        RendererTextureFormatStates[20] = State.Textures.Formats.Indexes[20] != INVALID_TEXTURE_FORMAT_INDEX;

        if (State.Textures.Formats.Indexes[3] == INVALID_TEXTURE_FORMAT_INDEX)
        {
            State.Textures.Formats.Indexes[3] = AcquireRendererDeviceTextureFormatIndex(0, 0, 5, 5, 5, 0, 0);
            RendererTextureFormatStates[3] = (State.Textures.Formats.Indexes[3] != INVALID_TEXTURE_FORMAT_INDEX) ? 5 : 0; // TODO
        }
        else
        {
            RendererTextureFormatStates[3] = 1;
        }
    }

    // 0x6000c450
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDPIXELFORMAT format, LPVOID context)
    {
        ZeroMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count], sizeof(TextureFormat));

        CopyMemory(&State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat, format, sizeof(DDPIXELFORMAT));

        State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTF = DXT_FORMAT_NONE;

        if (format->dwFlags & DDPF_FOURCC)
        {
            if (format->dwFourCC == FOURCC_DXT1)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTF = DXT_FORMAT_DXT1;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTN = FOURCC_DXT1;
            }
            else if (format->dwFourCC == FOURCC_DXT2)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTF = DXT_FORMAT_DXT2;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTN = FOURCC_DXT2;
            }
            else if (format->dwFourCC == FOURCC_DXT3)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTF = DXT_FORMAT_DXT3;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTN = FOURCC_DXT3;
            }
        }

        if (format->dwFlags & DDPF_PALETTEINDEXED8)
        {
            State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
            State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 8;
            State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = 0; // TODO
        }
        else
        {
            if (format->dwFlags & DDPF_PALETTEINDEXED4)
            {
                State.Textures.Formats.Formats[State.Textures.Formats.Count].IsPalette = TRUE;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 4;
                State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = 0; // TODO
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
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 0;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = 0; // TODO
                    }
                    else
                    {
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].AlphaBitCount = 0;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].PaletteColorBits = 0;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = 0; // TODO
                    }
                }
                else
                {
                    if (format->dwFlags & DDPF_BUMPLUMINANCE)
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

                        State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = (red == 5 && green == 5 && blue == 6) ? 3 : 2; // TODO
                    }
                    else if (format->dwFlags & DDPF_BUMPDUDV)
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

                        State.Textures.Formats.Formats[State.Textures.Formats.Count].RedBitCount = red;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].GreenBitCount = green;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].BlueBitCount = 0;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].DXTT = 1; // TODO
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat.dwRGBBitCount = GRAPHICS_BITS_PER_PIXEL_16;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat.dwRBitMask = 0xff;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat.dwGBitMask = 0xff00;
                        State.Textures.Formats.Formats[State.Textures.Formats.Count].Descriptor.ddpfPixelFormat.dwBBitMask = 0;
                    }
                }
            }
        }

        if (*(s32*)context == INVALID_TEXTURE_FORMAT_COUNT)
        {
            *(s32*)context = State.Textures.Formats.Count;
        }

        // NOTE: Added check to avoid writing outside the array boundaries.
        if (MAX_TEXTURE_FORMAT_COUNT <= State.Textures.Formats.Count + 1) { return DDENUMRET_CANCEL; }

        State.Textures.Formats.Count = State.Textures.Formats.Count + 1;

        return DDENUMRET_OK;
    }

    // 0x6000c3c0
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue, const u32 dxtf, const u32 dxtt)
    {
        for (u32 x = 0; x < State.Textures.Formats.Count; x++)
        {
            const TextureFormat* format = &State.Textures.Formats.Formats[x];

            if (format->RedBitCount == red && format->GreenBitCount == green && format->BlueBitCount == blue)
            {
                if (format->PaletteColorBits == palette)
                {
                    if (format->AlphaBitCount == alpha && format->DXTF == dxtf && format->DXTT == dxtt) { return x; }
                    else if (format->PaletteColorBits == palette && palette != 0) { return x; }
                }
            }
            else if (format->PaletteColorBits == palette && palette != 0) { return x; }
        }

        return INVALID_TEXTURE_FORMAT_INDEX;
    }

    // 0x6000a130
    // a.k.a. createzbuffer
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height, IDirectDrawSurface7* depth, IDirectDrawSurface7* surf)
    {
        if (State.Device.Capabilities.IsDepthBufferRemovalAvailable) { return TRUE; }

        IDirectDrawSurface7* ds = surf == NULL ? State.DX.Surfaces.Depth : depth;

        DDPIXELFORMAT format;
        ZeroMemory(&format, sizeof(DDPIXELFORMAT));

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

        State.Device.Capabilities.IsStencilBuffer = (format.dwFlags & DDPF_STENCILBUFFER) != 0;

        desc.ddsCaps.dwCaps = State.Device.Capabilities.IsAccelerated
            ? DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY
            : DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;

        {
            const HRESULT result = State.DX.Active.Instance->CreateSurface(&desc, &ds, NULL);

            if (result != DD_OK)
            {
                LOGERROR("CreateSurface failure! %s\n", AcquireRendererMessage(result));

                if (result != DDERR_OUTOFMEMORY && result != DDERR_OUTOFVIDEOMEMORY)
                {
                    LOGWARNING("CreateSurface for Z-buffer failed %s.\n", AcquireRendererMessage(result));

                    return FALSE;
                }

                LOGWARNING("There was not enough video memory to create the Z-buffer surface.\nPlease restart the program and try another fullscreen mode with less resolution or lower bit depth.\n");

                return FALSE;
            }
        }

        {
            IDirectDrawSurface7* s = surf != NULL ? surf
                : (State.DX.Active.Surfaces.Back == NULL ? State.DX.Active.Surfaces.Main : State.DX.Active.Surfaces.Back);

            HRESULT result = s->AddAttachedSurface(ds);

            if (result == DD_OK)
            {
                DDSURFACEDESC2 desc;
                ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                desc.dwSize = sizeof(DDSURFACEDESC2);

                result = ds->GetSurfaceDesc(&desc);

                if (result == DD_OK)
                {
                    State.Device.Capabilities.IsDepthVideoMemoryCapable = (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0;

                    if (!State.Device.Capabilities.IsAccelerated || State.Device.Capabilities.IsDepthVideoMemoryCapable)
                    {
                        if (ds != NULL) { ds->Release(); }

                        return TRUE;
                    }

                    LOGWARNING("Could not fit the Z-buffer in video memory for this hardware device.\n");
                }
                else { LOGWARNING("Failed to get surface description of Z buffer %d.\n", result); }
            }
            else { LOGWARNING("AddAttachedBuffer failed for Z-Buffer %d.\n", result); }
        }

        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
        State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

        if (ds != NULL) { ds->Release(); }

        return FALSE;
    }

    // 0x6000a450
    HRESULT CALLBACK EnumerateRendererDevicePixelFormats(LPDDPIXELFORMAT format, LPVOID context)
    {
        if ((format->dwFlags & DDPF_ZBUFFER) && format->dwRGBBitCount == State.Window.Bits)
        {
            CopyMemory(context, format, sizeof(DDPIXELFORMAT));

            return D3DENUMRET_CANCEL;
        }

        return D3DENUMRET_OK;
    }

    // 0x60001f20
    void InitializeTextureStateStates(void)
    {
        ZeroMemory(State.Textures.StageStates, MAX_RENDERER_MODULE_TEXTURE_STATE_STATE_COUNT * sizeof(TextureStageState));
    }

    // 0x60009670
    BOOL AcquireRendererDeviceState(void)
    {
        return State.DX.Active.IsInit;
    }
    
    // 0x60005dc0
    void ReleaseRendererDevice(void)
    {
        if (AcquireRendererDeviceState() && State.Scene.IsActive)
        {
            FlushGameWindow();
            SyncGameWindow(0);
            Idle();

            State.Scene.IsActive = FALSE;
        }

        if (State.DX.Clipper != NULL)
        {
            State.DX.Clipper->Release();
            State.DX.Clipper = NULL;
        }

        if (State.DX.GammaControl != NULL)
        {
            {
                const f32 value = 1.0f;

                SelectState(RENDERER_MODULE_STATE_SELECT_GAMMA_CONTROL_STATE, (void*)(u32)(*(u32*)&value));
            }

            State.DX.GammaControl->Release();
            State.DX.GammaControl = NULL;
        }

        ResetTextures();

        State.DX.Active.Instance = NULL;

        State.DX.Active.Surfaces.Main = NULL;
        State.DX.Active.Surfaces.Back = NULL;
        State.DX.Active.Surfaces.Active.Main = NULL;
        State.DX.Active.Surfaces.Active.Back = NULL;

        State.DX.Surfaces.Window = NULL;

        State.Window.Index = 0;

        if (State.DX.Surfaces.Depth != NULL)
        {
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
            State.DX.Device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

            State.DX.Surfaces.Depth->Release();
            State.DX.Surfaces.Depth = NULL;
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

        State.Scene.IsActive = FALSE;

        ReleaseRendererWindows();
    }

    // 0x60001980
    void ReleaseRendererWindows(void)
    {
        for (u32 x = 0; x < State.Window.Count + RENDERER_WINDOW_OFFSET; x++)
        {
            if (State.Windows[x + RENDERER_WINDOW_OFFSET].Surface != NULL)
            {
                State.Windows[x + RENDERER_WINDOW_OFFSET].Surface->Release();
                State.Windows[x + RENDERER_WINDOW_OFFSET].Surface = NULL;
            }
        }

        State.Window.Count = 0;
    }
}