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
#include "Module.hxx"
#include "Renderer.hxx"
#include "RendererValues.hxx"

using namespace Renderer;
using namespace RendererModuleValues;

namespace RendererModule
{
    // 0x60001000
    // a.k.a. THRASH_about
    DLLAPI RendererModuleDescriptor* STDCALLAPI AcquireDescriptor(void)
    {
        ModuleDescriptor.Version = RendererVersion;
        ModuleDescriptor.MaximumTextureWidth = 256;
        ModuleDescriptor.MaximumTextureHeight = 256;
        ModuleDescriptor.DXV = RENDERER_MODULE_VERSION_DX6; // ORIGINAL: 4
        ModuleDescriptor.Unk5 = 4; // TODO
        ModuleDescriptor.Caps = (State.DX.Active.IsSoft & 1) << 4
            | (RENDERER_MODULE_CAPS_WINDOWED | RENDERER_MODULE_CAPS_TEXTURE_HEIGHT_POW2 | RENDERER_MODULE_CAPS_TEXTURE_WIDTH_POW2 | RENDERER_MODULE_CAPS_TEXTURE_SQUARE | RENDERER_MODULE_CAPS_LINE_WIDTH);
        ModuleDescriptor.Author = RENDERER_MODULE_AUTHOR;
        ModuleDescriptor.Signature = RENDERER_MODULE_SIGNATURE_D3D6;
        ModuleDescriptor.Unk1 = 0x80; // TOOD
        ModuleDescriptor.MinimumTextureWidth = 8;
        ModuleDescriptor.MultipleTextureWidth = 1;
        ModuleDescriptor.MinimumTextureHeight = 8;
        ModuleDescriptor.MultipleTextureHeight = 1;
        ModuleDescriptor.ClipAlign = 0;
        ModuleDescriptor.Unk4 = 0xc; // TODO
        ModuleDescriptor.TextureFormatStates = RendererTextureFormatStates;

        ModuleDescriptor.Unk6 = UnknownArray06;

        ModuleDescriptor.Capabilities.Capabilities = ModuleDescriptorDeviceCapabilities;

        strcpy(ModuleDescriptor.Name, RENDERER_MODULE_NAME);

        SelectRendererDevice();

        return &ModuleDescriptor;
    }

    // 0x60001150
    // a.k.a. THRASH_clearwindow
    DLLAPI u32 STDCALLAPI ClearGameWindow()
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600011f0
    // a.k.a. THRASH_clip
    DLLAPI u32 STDCALLAPI ClipGameWindow(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600015f0
    // a.k.a. THRASH_drawline
    DLLAPI void STDCALLAPI DrawLine(RVX* a, RVX* b)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001630
    // a.k.a. THRASH_drawlinemesh
    DLLAPI void STDCALLAPI DrawLineMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600017a0
    // a.k.a. THRASH_drawlinestrip
    DLLAPI void STDCALLAPI DrawLineStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0z60001670
    // a.k.a. THRASH_drawlinestrip
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawLineStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600016b0
    // a.k.a. THRASH_drawpoint
    DLLAPI void STDCALLAPI DrawPoint(RVX* vertex)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600016d0
    // a.k.a. THRASH_drawpointmesh
    DLLAPI void STDCALLAPI DrawPointMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600017d0
    // a.k.a. THRASH_drawpointstrip
    DLLAPI void STDCALLAPI DrawPointStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001570
    // a.k.a. THRASH_drawquad
    DLLAPI void STDCALLAPI DrawQuad(RVX* a, RVX* b, RVX* c, RVX* d)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600015d0
    // a.k.a. THRASH_drawquadmesh
    DLLAPI void STDCALLAPI DrawQuadMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001480
    // a.k.a. THRASH_drawtri
    DLLAPI void STDCALLAPI DrawTriangle(RVX* a, RVX* b, RVX* c)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001760
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFan(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001510
    // a.k.a. THRASH_drawtrifan
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawTriangleFans(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600014d0
    // a.k.a. THRASH_drawtrimesh
    DLLAPI void STDCALLAPI DrawTriangleMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001700
    // a.k.a. THRASH_drawtristrip
    // NOTE: Triangle strip vertex order: 0 1 2, 1 3 2, 2 3 4, 3 5 4, 4 5 6, ...
    DLLAPI void STDCALLAPI DrawTriangleStrip(const u32 count, RVX* vertexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x600014f0
    // a.k.a. THRASH_drawtristrip
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawTriangleStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        // TODO NOT IMPLEMENTED
    }

    // 0x60001180
    // a.k.a. THRASH_flushwindow
    DLLAPI u32 STDCALLAPI FlushGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600011d0
    // a.k.a. THRASH_idle
    DLLAPI void STDCALLAPI Idle(void) { }

    // 0x60002740
    // a.k.a. THRASH_init
    DLLAPI u32 STDCALLAPI Init(void)
    {
        AcquireRendererDeviceCount();

        return State.Devices.Count;
    }

    // 0x60002ea0
    // a.k.a. THRASH_is
    DLLAPI u32 STDCALLAPI Is(void)
    {
        const HWND hwnd = GetDesktopWindow();
        const HDC hdc = GetWindowDC(hwnd);

        if ((GRAPHICS_BITS_PER_PIXEL_8 - 1) < GetDeviceCaps(hdc, BITSPIXEL))
        {
            IDirectDraw* instance = NULL;
            if (DirectDrawCreate(NULL, &instance, NULL) == DD_OK)
            {
                IDirectDraw2* dd = NULL;
                HRESULT result = instance->QueryInterface(IID_IDirectDraw2, (void**)&dd);

                instance->Release();

                if (result == DD_OK)
                {
                    IDirect3D2* dx = NULL;
                    result = dd->QueryInterface(IID_IDirect3D2, (void**)&dx);

                    dd->Release();

                    if (result == DD_OK && dx != NULL)
                    {
                        dx->Release();

                        return RENDERER_MODULE_DX6_ACCELERATION_AVAILABLE;
                    }
                }
            }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001240
    // a.k.a. THRASH_lockwindow
    DLLAPI RendererModuleWindowLock* STDCALLAPI LockGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x60001190
    // a.k.a. THRASH_pageflip
    DLLAPI u32 STDCALLAPI ToggleGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60002f60
    // a.k.a. THRASH_readrect
    DLLAPI u32 STDCALLAPI ReadRectangle(const u32 x, const u32 y, const u32 width, const u32 height, u32* data)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60002750
    // a.k.a. THRASH_restore
    DLLAPI u32 STDCALLAPI RestoreGameWindow(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001880
    // a.k.a. THRASH_selectdisplay
    DLLAPI u32 STDCALLAPI SelectDevice(const s32 indx)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003310
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectState(const u32 state, void* value)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600040f0
    // a.k.a. THRASH_settexture
    DLLAPI u32 STDCALLAPI SelectTexture(RendererTexture* tex)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600027a0
    // a.k.a. THRASH_setvideomode
    DLLAPI u32 STDCALLAPI SelectVideoMode(const u32 mode, const u32 pending, const u32 depth)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600011e0
    // a.k.a. THRASH_sync
    DLLAPI u32 STDCALLAPI SyncGameWindow(const u32)
    {
        UnlockGameWindow(LockGameWindow());

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60004130
    // a.k.a. THRASH_talloc
    DLLAPI RendererTexture* STDCALLAPI AllocateTexture(const u32 width, const u32 height, const u32 format, void* p4, const u32)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x600042a0
    // a.k.a. THRASH_treset
    DLLAPI u32 STDCALLAPI ResetTextures(void)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60004270
    // a.k.a. THRASH_tupdate
    DLLAPI RendererTexture* STDCALLAPI UpdateTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        // TODO NOT IMPLEMENTED

        return NULL;
    }

    // 0x600013e0
    // a.k.a.  _THRASH_unlockwindow
    DLLAPI u32 STDCALLAPI UnlockGameWindow(const RendererModuleWindowLock* state)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60001100
    // a.k.a. THRASH_window
    DLLAPI u32 STDCALLAPI SelectGameWindow(const u32 indx)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60003000
    // a.k.a. THRASH_writerect
    DLLAPI u32 STDCALLAPI WriteRectangle(const u32 x, const u32 y, const u32 width, const u32 height, const u32* data)
    {
        // TODO NOT IMPLEMENTED

        return RENDERER_MODULE_FAILURE;
    }
}