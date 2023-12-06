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
#include "Mathematics.Basic.hxx"
#include "Module.hxx"
#include "RendererValues.hxx"
#include "Settings.hxx"

#include <math.h>
#include <stdlib.h>

using namespace Mathematics;
using namespace Renderer;
using namespace RendererModuleValues;
using namespace Settings;

namespace RendererModule
{
    // 0x60001000
    // a.k.a. THRASH_about
    DLLAPI RendererModuleDescriptor* STDCALLAPI AcquireDescriptor(void)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x600012e0
    // a.k.a. THRASH_clearwindow
    DLLAPI u32 STDCALLAPI ClearGameWindow()
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600013c0
    // a.k.a. THRASH_clip
    DLLAPI u32 STDCALLAPI ClipGameWindow(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        State.ViewPort.X0 = x0;
        State.ViewPort.Y0 = y0;
        State.ViewPort.X1 = x1 - x0;
        State.ViewPort.Y1 = y1 - y0;

        AttemptRenderScene();

        if (State.Scene.IsActive) { EndRendererScene(); }

        D3DVIEWPORT7 vp;
        ZeroMemory(&vp, sizeof(D3DVIEWPORT7));

        vp.dwX = x0;
        vp.dwY = y0;
        vp.dvMinZ = 0.0f;
        vp.dvMaxZ = 1.0f;
        vp.dwWidth = x1 - x0;
        vp.dwHeight = y1 - y0;

        State.DX.Device->SetViewport(&vp);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60001880
    // a.k.a. THRASH_createwindow
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI CreateGameWindow(const u32 width, const u32 height, const u32 format, void*)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001910
    // a.k.a. THRASH_destroywindow
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI DestroyGameWindow(const u32 indx)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001b10
    // // a.k.a. THRASH_drawline
    DLLAPI void STDCALLAPI DrawLine(RVX* a, RVX* b)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001b30
    // a.k.a. THRASH_drawlinemesh
    DLLAPI void STDCALLAPI DrawLineMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001d20
    // a.k.a. THRASH_drawlinestrip
    DLLAPI void STDCALLAPI DrawLineStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001b50
    // a.k.a. THRASH_drawlinestrip
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawLineStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001b90
    // a.k.a. THRASH_drawpoint
    DLLAPI void STDCALLAPI DrawPoint(RVX* vertex)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001bb0
    // a.k.a. THRASH_drawpointmesh
    DLLAPI void STDCALLAPI DrawPointMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001d50
    // a.k.a. THRASH_drawpointstrip
    DLLAPI void STDCALLAPI DrawPointStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001a90
    // a.k.a. THRASH_drawquad
    DLLAPI void STDCALLAPI DrawQuad(RVX* a, RVX* b, RVX* c, RVX* d)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001af0
    // a.k.a. THRASH_drawquadmesh
    DLLAPI void STDCALLAPI DrawQuadMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001bf0
    // a.k.a. THRASH_drawsprite
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawSprite(RVX* a, RVX* b)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001c50
    // a.k.a. THRASH_drawspritemesh
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawSpriteMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600019d0
    // a.k.a. THRASH_drawtri
    DLLAPI void STDCALLAPI DrawTriangle(RVX* a, RVX* b, RVX* c)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001cf0
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFan(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001a70
    // a.k.a. THRASH_drawtrifan
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawTriangleFans(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001a30
    // a.k.a. THRASH_drawtrimesh
    DLLAPI void STDCALLAPI DrawTriangleMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001ca0
    // a.k.a. THRASH_drawtristrip
    // NOTE: Triangle strip vertex order: 0 1 2, 1 3 2, 2 3 4, 3 5 4, 4 5 6, ...
    DLLAPI void STDCALLAPI DrawTriangleStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001a50
    // a.k.a. THRASH_drawtristrip
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawTriangleStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001320
    // a.k.a. THRASH_flushwindow
    DLLAPI u32 STDCALLAPI FlushGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001d80
    // a.k.a. THRASH_getstate
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI AcquireState(const u32 stage)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001960
    // a.k.a. THRASH_getwindowtexture
    // NOTE: Never being called by the application.
    DLLAPI RendererTexture* STDCALLAPI AcquireGameWindowTexture(const u32 indx)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x60001370
    // a.k.a. THRASH_idle
    DLLAPI void STDCALLAPI Idle(void) { }

    // 0x60003670
    // a.k.a. THRASH_init
    DLLAPI u32 STDCALLAPI Init(void)
    {
        RendererState = RENDERER_STATE_INACTIVE;

        InitializeSettings();

        AcquireRendererDeviceCount();

        InitializeTextureStateStates();

        atexit(ReleaseRendererModule);

        return State.Devices.Count;
    }

    // 0x60003a50
    // a.k.a. THRASH_is
    DLLAPI u32 STDCALLAPI Is(void)
    {
        HWND hwnd = GetDesktopWindow();
        HDC hdc = GetWindowDC(hwnd);

        if (GetDeviceCaps(hdc, BITSPIXEL) < GRAPHICS_BITS_PER_PIXEL_8)
        {
            ReleaseDC(hwnd, hdc);

            return RENDERER_MODULE_FAILURE;
        }

        IDirectDraw7* instance = NULL;
        HRESULT result = DirectDrawCreateEx(NULL, (void**)&instance, IID_IDirectDraw7, NULL);

        if (result == DD_OK)
        {
            DDCAPS caps;
            ZeroMemory(&caps, sizeof(DDCAPS));

            caps.dwSize = sizeof(DDCAPS);

            result = instance->GetCaps(&caps, NULL);

            if ((caps.dwCaps & DDCAPS_3D) && result == DD_OK)
            {
                IDirect3D7* dx = NULL;
                result = instance->QueryInterface(IID_IDirect3D7, (void**)&dx);

                instance->Release();

                if (result == DD_OK && dx != NULL)
                {
                    dx->Release();

                    return RENDERER_MODULE_DX7_ACCELERATION_AVAILABLE;
                }

                if (dx != NULL) { dx->Release(); }
            }
        }

        if (instance != NULL) { instance->Release(); }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001480
    // a.k.a. THRASH_lockwindow
    DLLAPI RendererModuleWindowLock* STDCALLAPI LockGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001330
    // a.k.a. THRASH_pageflip
    DLLAPI u32 STDCALLAPI ToggleGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003b60
    // a.k.a. THRASH_readrect
    DLLAPI u32 STDCALLAPI ReadRectangle(const u32 x, const u32 y, const u32 width, const u32 height, u32* data)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600036c0
    // a.k.a. THRASH_restore
    DLLAPI u32 STDCALLAPI RestoreGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600020c0
    // a.k.a. THRASH_selectdisplay
    DLLAPI u32 STDCALLAPI SelectDevice(const s32 indx)
    {
        State.Device.Identifier = NULL;

        if (State.DX.Instance != NULL) { RestoreGameWindow(); }

        const char* name = NULL;

        if (indx < DEFAULT_RENDERER_DEVICE_INDEX || State.Devices.Count <= indx)
        {
            RendererDeviceIndex = DEFAULT_RENDERER_DEVICE_INDEX;
            State.Device.Identifier = State.Devices.Indexes[DEFAULT_RENDERER_DEVICE_INDEX];
            name = State.Devices.Enumeration.Names[DEFAULT_RENDERER_DEVICE_INDEX];
        }
        else
        {
            RendererDeviceIndex = indx;
            State.Device.Identifier = State.Devices.Indexes[indx];
            name = State.Devices.Enumeration.Names[indx];
        }

        strncpy(State.Device.Name, name, MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH);

        if (State.Lambdas.Lambdas.Execute != NULL)
        {
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceExecute);
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_RELEASE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)ReleaseRendererDeviceExecute);
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceSurfacesExecute);

            if (State.Lambdas.Lambdas.Execute != NULL)
            {
                if (GetWindowThreadProcessId(State.Window.Parent.HWND, NULL) != GetCurrentThreadId())
                {
                    InitializeRendererDeviceLambdas();

                    return RENDERER_MODULE_SUCCESS;
                }
            }
        }

        InitializeRendererDevice();

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003ca0
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectState(const u32 state, void* value)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60008980
    // a.k.a. THRASH_settexture
    DLLAPI u32 STDCALLAPI SelectTexture(RendererTexture* tex)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003750
    // a.k.a. THRASH_setvideomode
    DLLAPI u32 STDCALLAPI SelectVideoMode(const u32 mode, const u32 pending, const u32 depth)
    {
        State.Window.Bits = depth;

        if (depth == 1) { State.Window.Bits = GRAPHICS_BITS_PER_PIXEL_16; } // TODO
        else if (depth == 2) { State.Window.Bits = GRAPHICS_BITS_PER_PIXEL_32; } // TODO

        if (State.Scene.IsActive) { LOGWARNING("D3D Setting videomode in 3d scene !!!!\n"); }

        SelectRendererDevice();

        if (ModuleDescriptor.Capabilities.Capabilities[mode].Bits < State.Window.Bits) { State.Window.Bits = GRAPHICS_BITS_PER_PIXEL_16; }

        s32 result = RENDERER_MODULE_FAILURE;

        if (State.DX.Instance != NULL)
        {
            if (State.Lambdas.Lambdas.AcquireWindow == NULL)
            {
                InitializeRendererDeviceSurfacesExecute(0, State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending, NULL);
            }
            else
            {
                if (GetWindowThreadProcessId(State.Window.Parent.HWND, NULL) == GetCurrentThreadId())
                {
                    InitializeRendererDeviceSurfacesExecute(0, State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending, NULL);
                }
                else
                {
                    SetForegroundWindow(State.Window.HWND);
                    PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending);
                    WaitForSingleObject(State.Mutex, INFINITE);
                }
            }

            if (State.DX.Code == DD_OK)
            {
                DDSURFACEDESC2 desc;
                ZeroMemory(&desc, sizeof(DDSURFACEDESC2));

                desc.dwSize = sizeof(DDSURFACEDESC2);

                const HRESULT code = State.DX.Surfaces.Active[1]->GetSurfaceDesc(&desc);

                if (code != DD_OK) { LOGWARNING("DX7_setvideomode:  Error getting Surface Description %8x\n", code); }
            }
            else { LOGERROR("DX7_setdisplay - error\n"); }

            result = State.DX.Code == DD_OK;
        }

        InitializeRendererModuleState(mode, pending, depth, RENDERER_MODULE_ENVIRONMENT_SECTION_NAME);
        SelectBasicRendererState(RENDERER_MODULE_STATE_62, (void*)(DAT_60058df8 + 1));

        SelectGameWindow(1); // TODO

        const RendererModuleWindowLock* lock = LockGameWindow();

        UnlockGameWindow(lock);

        SelectGameWindow(0); // TODO

        ZeroMemory(&State.Settings.Lock, sizeof(RendererModuleWindowLock));

        if (lock != NULL)
        {
            State.Settings.Lock.Data = NULL;

            State.Settings.Lock.Stride = lock->Stride;
            State.Settings.Lock.Format = lock->Format;
            State.Settings.Lock.Width = lock->Width;
            State.Settings.Lock.Height = lock->Height;

            SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_WINDOW_LOCK_STATE, &State.Settings.Lock);
        }

        ResetTextures();

        RendererState = RENDERER_STATE_INACTIVE;

        return result;
    }

    // 0x60001380
    // a.k.a. THRASH_sync
    DLLAPI u32 STDCALLAPI SyncGameWindow(const u32)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60008e70
    // a.k.a. THRASH_talloc
    DLLAPI RendererTexture* STDCALLAPI AllocateTexture(const u32 width, const u32 height, const u32 format, void* p4, const u32 options)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x60008eb0
    // a.k.a. THRASH_tfree
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI ReleaseTexture(RendererTexture* tex)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60008fc0
    // a.k.a. THRASH_treset
    DLLAPI u32 STDCALLAPI ResetTextures(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60008f20
    // a.k.a. THRASH_tupdate
    DLLAPI RendererTexture* STDCALLAPI UpdateTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x60008f60
    // a.k.a. THRASH_tupdaterect
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI UpdateTextureRectangle(RendererTexture* tex, const u32* pixels, const u32* palette, const u32 x, const u32 y, const s32 width, const s32 height, const u32 size, void*)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001830
    // a.k.a.  _THRASH_unlockwindow
    DLLAPI u32 STDCALLAPI UnlockGameWindow(const RendererModuleWindowLock* state)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600011a0
    // a.k.a. THRASH_window
    DLLAPI u32 STDCALLAPI SelectGameWindow(const u32 indx)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003c00
    // a.k.a. THRASH_writerect
    DLLAPI u32 STDCALLAPI WriteRectangle(const u32 x, const u32 y, const u32 width, const u32 height, const u32* data)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }
}