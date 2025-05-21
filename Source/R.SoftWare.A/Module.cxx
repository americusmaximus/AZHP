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

#include "Graphics.Basic.hxx"
#include "Mathematics.Basic.hxx"
#include "Module.hxx"
#include "RendererValues.hxx"

#include <math.h>
#include <stdlib.h>

using namespace Mathematics;
using namespace Renderer;
using namespace RendererModuleValues;

namespace RendererModule
{
    // 0x60002330
    // a.k.a. THRASH_about
    DLLAPI RendererModuleDescriptor* STDCALLAPI AcquireDescriptor(void)
    {
        ModuleDescriptor.Signature = RENDERER_MODULE_SIGNATURE_STRI;
        ModuleDescriptor.Size = sizeof(RendererModuleDescriptor);
        ModuleDescriptor.Version = RENDERER_MODULE_VERSION_104;

        ModuleDescriptor.MinimumTextureWidth = 16;
        ModuleDescriptor.MaximumTextureWidth = 256;
        ModuleDescriptor.MultipleTextureWidth = 1;

        ModuleDescriptor.Caps = ModuleDescriptor.Caps
            | RENDERER_MODULE_CAPS_SOFTWARE | RENDERER_MODULE_CAPS_TEXTURE_HEIGHT_POW2
            | RENDERER_MODULE_CAPS_TEXTURE_WIDTH_POW2 | RENDERER_MODULE_CAPS_TEXTURE_SQUARE | RENDERER_MODULE_CAPS_LINE_WIDTH;
        
        ModuleDescriptor.MinimumTextureHeight = 16;
        ModuleDescriptor.MaximumTextureHeight = 256;
        ModuleDescriptor.MultipleTextureHeight = 1;

        ModuleDescriptor.VideoMemorySize = 0;
        ModuleDescriptor.TotalMemorySize = MIN_DEVICE_AVAIABLE_VIDEO_MEMORY;

        ModuleDescriptor.ClipAlign = 0;

        ModuleDescriptor.ActiveTextureFormatStatesCount = MAX_ACTIVE_USABLE_TEXTURE_FORMAT_COUNT;
        ModuleDescriptor.ActiveUnknownValuesCount = MAX_ACTIVE_UNKNOWN_COUNT;

        ModuleDescriptor.UnknownValues = UnknownArray06;
        ModuleDescriptor.TextureFormatStates = RendererTextureFormatStates;
        
        strcpy(ModuleDescriptor.Name, RENDERER_MODULE_NAME);

        ModuleDescriptor.SubType = 0;

        return &ModuleDescriptor;
    }
    
    // 0x60003470
    // a.k.a. THRASH_clearwindow
    DLLAPI u32 STDCALLAPI ClearGameWindow(void)
    {
        return RendererClearGameWindow();
    }

    // 0x60003490
    // a.k.a. THRASH_clip
    DLLAPI u32 STDCALLAPI ClipGameWindow(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        const u32 x = (s32)x0 < 0 ? 0 : x0;
        const u32 y = (s32)y0 < 0 ? 0 : y0;

        const u32 width = State.Renderer.Settings.Width < x1 ? State.Renderer.Settings.Width : x1;
        const u32 height = State.Renderer.Settings.Height < y1 ? State.Renderer.Settings.Height : y1;

        State.ViewPort.X = x;
        State.ViewPort.Y = y;

        State.ViewPort.Left = width - x;
        State.ViewPort.Right = width - 1;
        State.ViewPort.Top = height - y;
        State.ViewPort.Bottom = height - 1;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60005020
    // a.k.a. THRASH_drawline
    DLLAPI void STDCALLAPI DrawLine(RVX* a, RVX* b)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600051f0
    // a.k.a. THRASH_drawlinemesh
    DLLAPI void STDCALLAPI DrawLineMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60005230
    // a.k.a. THRASH_drawlinestrip
    DLLAPI void STDCALLAPI DrawLineStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60005260
    // a.k.a. THRASH_drawpoint
    DLLAPI void STDCALLAPI DrawPoint(RVX* vertex)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600052b0
    // a.k.a. THRASH_drawpointmesh
    DLLAPI void STDCALLAPI DrawPointMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600052f0
    // a.k.a. THRASH_drawpointstrip
    DLLAPI void STDCALLAPI DrawPointStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004ea0
    // a.k.a. THRASH_drawquad
    DLLAPI void STDCALLAPI DrawQuad(RVX* a, RVX* b, RVX* c, RVX* d)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004f30
    // a.k.a. THRASH_drawquadmesh
    DLLAPI void STDCALLAPI DrawQuadMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004da0
    // a.k.a. THRASH_drawtri
    DLLAPI void STDCALLAPI DrawTriangle(RVX* a, RVX* b, RVX* c)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004fe0
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFan(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004e50
    // a.k.a. THRASH_drawtrimesh
    DLLAPI void STDCALLAPI DrawTriangleMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60004f90
    // a.k.a. THRASH_drawtristrip
    // NOTE: Triangle strip vertex order: 0 1 2, 1 3 2, 2 3 4, 3 5 4, 4 5 6, ...
    DLLAPI void STDCALLAPI DrawTriangleStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60003480
    // a.k.a. THRASH_flushwindow
    DLLAPI u32 STDCALLAPI FlushGameWindow(void) { return RENDERER_MODULE_SUCCESS; }

    // 0x60003480
    // a.k.a. THRASH_idle
    DLLAPI void STDCALLAPI Idle(void) { }

    // 0x60002bd0
    // a.k.a. THRASH_init
    DLLAPI u32 STDCALLAPI Init(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002f50
    // a.k.a. THRASH_is
    DLLAPI u32 STDCALLAPI Is(void) { return RENDERER_MODULE_SW_ACCELERATION_AVAILABLE; }

    // 0x60003530
    // a.k.a. THRASH_lockwindow
    DLLAPI RendererModuleWindowLock* STDCALLAPI LockGameWindow(void)
    {
        if (State.DX.Surfaces.Window == State.DX.Surfaces.Active[2])
        {
            State.Lock.State.Data = AcquireRendererSurface();
            State.Lock.State.Stride = RendererSurfaceStride;

            State.Lock.State.Format = State.DX.Surfaces.Bits == (GRAPHICS_BITS_PER_PIXEL_16 - 1)
                ? RENDERER_PIXEL_FORMAT_R5G5B5 : RENDERER_PIXEL_FORMAT_R5G6B5;

            State.Lock.State.Width = State.Window.Width;
            State.Lock.State.Height = State.Window.Height;

            return &State.Lock.State;
        }

        if (State.DX.Surfaces.Window == State.DX.Surfaces.Active[1])
        {
            RECT rect;
            GetClientRect(State.Window.HWND, &rect);

            POINT point;
            ZeroMemory(&point, sizeof(POINT));

            ClientToScreen(State.Window.HWND, &point);
            OffsetRect(&rect, point.x, point.y);

            State.Lambdas.Lambdas.LockWindow(TRUE);

            State.Lock.Surface = State.DX.Surfaces.Window;

            DDSURFACEDESC desc;
            ZeroMemory(&desc, sizeof(DDSURFACEDESC));

            desc.dwSize = sizeof(DDSURFACEDESC);

            State.DX.Code = State.DX.Surfaces.Window->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

            if (State.DX.Code == DD_OK)
            {
                State.Lock.State.Data = (void*)((addr)desc.lpSurface + (addr)(rect.left * sizeof(u16) + rect.top * desc.lPitch));

                State.Lock.State.Width = State.Window.Width;
                State.Lock.State.Height = State.Window.Height;

                State.Lock.State.Stride = desc.lPitch;

                if (desc.ddpfPixelFormat.dwRGBBitCount == GRAPHICS_BITS_PER_PIXEL_16)
                {
                    State.Lock.State.Format = desc.ddpfPixelFormat.dwGBitMask == 0x7e0
                        ? RENDERER_PIXEL_FORMAT_R5G6B5 : RENDERER_PIXEL_FORMAT_R5G5B5;
                }
                else if (desc.ddpfPixelFormat.dwRGBBitCount == GRAPHICS_BITS_PER_PIXEL_32) { State.Lock.State.Format = RENDERER_PIXEL_FORMAT_R8G8B8; }

                State.Lock.IsActive = TRUE;

                return &State.Lock.State;
            }
        }

        return NULL;
    }

    // 0x60003750
    // a.k.a. THRASH_pageflip
    DLLAPI void STDCALLAPI ToggleGameWindow(void)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60002f60
    // a.k.a. THRASH_readrect
    DLLAPI u32 STDCALLAPI ReadRectangle(const u32 x, const u32 y, const u32 width, const u32 height, u32* pixels)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60002bf0
    // a.k.a. THRASH_restore
    DLLAPI u32 STDCALLAPI RestoreGameWindow(void)
    {
        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        RendererVideoMode = DEFAULT_RENDERER_MODE;

        return ReleaseRendererWindow() == DD_OK;
    }

    // 0x60002c20
    // a.k.a. THRASH_selectdisplay
    DLLAPI u32 STDCALLAPI SelectDevice(const s32 indx)
    {
        RendererDeviceIndex = indx;

        s32 index = DEFAULT_DEVICE_INDEX;

        DirectDrawEnumerateA(EnumerateRendererDevices, &index);

        return index == INVALID_DEVICE_INDEX;
    }

    // 0x600030c0
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectState(const u32 state, void* value)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003f30
    // a.k.a. THRASH_settexture
    DLLAPI u32 STDCALLAPI SelectTexture(RendererTexture* tex)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60002c80
    // a.k.a. THRASH_setvideomode
    DLLAPI u32 STDCALLAPI SelectVideoMode(const u32 mode, const u32 pending, const u32 depth)
    {
        if (mode == RendererVideoMode) { return RENDERER_MODULE_SUCCESS; }

        if (State.DX.Instance == NULL) { InitializeRendererDeviceLambdas(); }
        else if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        if (State.DX.Instance == NULL) { return RENDERER_MODULE_FAILURE; }

        SetForegroundWindow(State.Window.HWND);
        PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending);
        WaitForSingleObject(State.Mutex, INFINITE);

        if (State.DX.Code != DD_OK) { Message("SOFTTRI_setdisplaymode - ERROR CODE (softtristatus) %8x\n", State.DX.Code); }

        SelectRendererSettings(State.Window.Width, State.Window.Height, State.DX.Surfaces.Bits);

        SelectGameWindow(1); // TODO

        RendererModuleWindowLock* lock = LockGameWindow();

        if (lock != NULL)
        {
            switch (lock->Format)
            {
            case RENDERER_PIXEL_FORMAT_P8: { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_8; break; }
            case RENDERER_PIXEL_FORMAT_R5G5B5:
            case RENDERER_PIXEL_FORMAT_A1R5G5B5: { State.DX.Surfaces.Bits = (GRAPHICS_BITS_PER_PIXEL_16 - 1); break; }
            case RENDERER_PIXEL_FORMAT_R8G8B8: { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_24; break; }
            case RENDERER_PIXEL_FORMAT_A8R8G8B8: { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_32; break; }
            default: { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_16; break; }
            }

            UnlockGameWindow(NULL);

            State.DX.Bits = State.DX.Surfaces.Bits;

            SelectRendererColorMasks(State.DX.Surfaces.Bits);
        }

        return State.DX.Code == DD_OK ? RENDERER_MODULE_SUCCESS : RENDERER_MODULE_FAILURE;
    }

    // 0x60003460
    // a.k.a. THRASH_sync
    DLLAPI u32 STDCALLAPI SyncGameWindow(const u32) { return RENDERER_MODULE_FAILURE; }

    // 0x60003c50
    // a.k.a. THRASH_talloc
    DLLAPI RendererTexture* STDCALLAPI AllocateTexture(const u32 width, const u32 height, const u32 format, const BOOL palette, const u32 state)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x60004080
    // a.k.a. THRASH_treset
    DLLAPI u32 STDCALLAPI ResetTextures(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003dc0
    // a.k.a. THRASH_tupdate
    DLLAPI RendererTexture* STDCALLAPI UpdateTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x600036d0
    // a.k.a. THRASH_unlockwindow
    DLLAPI u32 STDCALLAPI UnlockGameWindow(const RendererModuleWindowLock* state)
    {
        if (State.DX.Surfaces.Window == State.DX.Surfaces.Active[2]) { return RENDERER_MODULE_SUCCESS; }

        if (State.DX.Surfaces.Window != State.DX.Surfaces.Active[1]) { return RENDERER_MODULE_FAILURE; }

        if (!State.Lock.IsActive || State.Lock.Surface == NULL)
        {
            Message("SOFTTRI_unlockwindow - ATTEMPTING TO UNLOCK WINDOW THAT ISN`T LOCKED.\n");

            return RENDERER_MODULE_SUCCESS;
        }

        State.Lock.Surface->Unlock(state == NULL ? NULL : state->Data);

        State.Lock.Surface = NULL;
        State.Lock.IsActive = FALSE;

        State.Lambdas.Lambdas.LockWindow(FALSE);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600033f0
    // a.k.a. THRASH_window
    DLLAPI u32 STDCALLAPI SelectGameWindow(const u32 indx)
    {
        State.DX.Surfaces.Window = State.DX.Surfaces.Active[indx];

        if (State.DX.Surfaces.Window == State.DX.Surfaces.Active[2])
        {
            State.Renderer.Surface.Surface = State.Renderer.Active.Surface;

            RendererSurfaceStride = State.Renderer.Active.Stride;

            State.Renderer.Settings.Width = State.Renderer.Active.Width;
            State.Renderer.Settings.Height = State.Renderer.Active.Height;

            State.DX.Surfaces.Bits = State.DX.Bits;

            SelectRendererColorMasks(State.DX.Bits);
        }

        return State.DX.Surfaces.Window == NULL ? RENDERER_MODULE_FAILURE : RENDERER_MODULE_SUCCESS;
    }

    // 0x60003000
    // a.k.a. THRASH_writerect
    DLLAPI u32 STDCALLAPI WriteRectangle(const u32 x, const u32 y, const u32 width, const u32 height, const u32* pixels)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }
}