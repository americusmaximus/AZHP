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
#include "RendererValues.hxx"
#include "Settings.hxx"

using namespace Renderer;
using namespace RendererModuleValues;
using namespace Settings;

namespace RendererModule
{
    // 0x60001020
    // a.k.a. THRASH_about
    DLLAPI RendererModuleDescriptor* STDCALLAPI AcquireDescriptor(void)
    {
        ModuleDescriptor.Signature = RENDERER_MODULE_SIGNATURE_D3D5;
        ModuleDescriptor.Unk1 = 128;
        ModuleDescriptor.Version = RENDERER_MODULE_VERSION_104;
        ModuleDescriptor.MinimumTextureWidth = 8;
        ModuleDescriptor.MaximumTextureWidth = 256;
        ModuleDescriptor.MultipleTextureWidth = 1;
        ModuleDescriptor.Caps = ModuleDescriptor.Caps | RENDERER_MODULE_CAPS_TEXTURE_HEIGHT_POW2 | RENDERER_MODULE_CAPS_TEXTURE_WIDTH_POW2 | RENDERER_MODULE_CAPS_TEXTURE_SQUARE | RENDERER_MODULE_CAPS_LINE_WIDTH;
        ModuleDescriptor.MinimumTextureHeight = 8;
        ModuleDescriptor.MaximumTextureHeight = 256;
        ModuleDescriptor.MultipleTextureHeight = 1;
        ModuleDescriptor.ClipAlign = 0;
        ModuleDescriptor.Unk4 = 9; // TODO
        ModuleDescriptor.TextureFormatStates = RendererTextureFormatStates;
        ModuleDescriptor.Unk5 = 4;
        ModuleDescriptor.Capabilities.Capabilities = ModuleDescriptorDeviceCapabilities;
        ModuleDescriptor.Unk6 = UnknownArray06;

        SelectRendererDevice();

        return &ModuleDescriptor;
    }

    // 0x60001118
    // a.k.a. THRASH_clearwindow
    DLLAPI u32 STDCALLAPI ClearGameWindow(void)
    {
        return ClearRendererViewPort(State.ViewPort.X0, State.ViewPort.Y0, State.ViewPort.X1, State.ViewPort.Y1);
    }

    // 0x60001188
    // a.k.a. THRASH_clip
    DLLAPI u32 STDCALLAPI ClipGameWindow(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        State.ViewPort.X0 = x0;
        State.ViewPort.Y0 = y0;
        State.ViewPort.X1 = x1;
        State.ViewPort.Y1 = y1;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600014a0
    // a.k.a. THRASH_drawline
    DLLAPI void STDCALLAPI DrawLine(RVX* a, RVX* b)
    {
        RTLVX vertexes[2];

        CopyMemory(&vertexes[0], a, sizeof(RTLVX));
        CopyMemory(&vertexes[1], b, sizeof(RTLVX));

        RenderLines(vertexes, 2);
    }

    // 0x600014fc
    // a.k.a. THRASH_drawlinemesh
    DLLAPI void STDCALLAPI DrawLineMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        const RTLVX* vs = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { DrawLine((RVX*)&vs[indexes[x * 2 + 0]], (RVX*)&vs[indexes[x * 2 + 1]]); }
    }

    // 0x60001538
    // a.k.a. THRASH_drawlinestrip
    DLLAPI void STDCALLAPI DrawLineStrip(const u32 count, RVX* vertexes)
    {
        const RTLVX* vs = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { DrawLine((RVX*)&vs[x + 0], (RVX*)&vs[x + 1]); }
    }

    // 0x60001564
    // a.k.a. THRASH_drawpoint
    DLLAPI void STDCALLAPI DrawPoint(RVX* vertex)
    {
        RenderPoints((RTLVX*)vertex, 1);
    }

    // 0x60001578
    // a.k.a. THRASH_drawpointmesh
    DLLAPI void STDCALLAPI DrawPointMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        const RTLVX* vs = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { RenderPoints((RTLVX*)&vs[indexes[x]], 1); }
    }

    // 0x600015b0
    // a.k.a. THRASH_drawpointstrip
    DLLAPI void STDCALLAPI DrawPointStrip(const u32 count, RVX* vertexes)
    {
        const RTLVX* vs = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { RenderPoints((RTLVX*)&vs[x], 1); }
    }

    // 0x6000146c
    // a.k.a. THRASH_drawquad
    DLLAPI void STDCALLAPI DrawQuad(RVX* a, RVX* b, RVX* c, RVX* d)
    {
        RenderQuad((RTLVX*)a, (RTLVX*)b, (RTLVX*)c, (RTLVX*)d);
    }

    // 0x60001488
    // a.k.a. THRASH_drawquadmesh
    DLLAPI void STDCALLAPI DrawQuadMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        RenderQuadMesh((RTLVX*)vertexes, indexes, count);
    }

    // 0x600013d0
    // a.k.a. THRASH_drawtri
    DLLAPI void STDCALLAPI DrawTriangle(RVX* a, RVX* b, RVX* c)
    {
        RenderTriangle((RTLVX*)a, (RTLVX*)b, (RTLVX*)c);
    }

    // 0x60001414
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFan(const u32 count, RVX* vertexes)
    {
        for (u32 x = 0; x < count; x++)
        {
            State.Data.Indexes.Small[x * 3 + 0] = 0;
            State.Data.Indexes.Small[x * 3 + 1] = x + 0;
            State.Data.Indexes.Small[x * 3 + 2] = x + 1;
        }

        RenderTriangleFan((RTLVX*)vertexes, count + 2, count, State.Data.Indexes.Small);
    }

    // 0x600013e8
    // a.k.a. THRASH_drawtrimesh
    DLLAPI void STDCALLAPI DrawTriangleMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        RenderTriangleMesh((RTLVX*)vertexes, indexes, count);
    }

    // 0x60001400
    // a.k.a. THRASH_drawtristrip
    DLLAPI void STDCALLAPI DrawTriangleStrip(const u32 count, RVX* vertexes)
    {
        RenderTriangleStrip((RTLVX*)vertexes, count + 2);
    }

    // 0x6000113c
    // a.k.a. THRASH_flushwindow
    DLLAPI u32 STDCALLAPI FlushGameWindow(void)
    {
        return EndRendererScene();
    }

    // 0x60001138
    // a.k.a. THRASH_idle
    DLLAPI void STDCALLAPI Idle(void) { }

    // 0x60002498
    // a.k.a. THRASH_init
    DLLAPI u32 STDCALLAPI Init(void)
    {
        InitializeSettings();

        AcquireRendererDeviceCount();

        return State.Devices.Count;
    }

    // 0x60002774
    // a.k.a. THRASH_is
    DLLAPI u32 STDCALLAPI Is(void)
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

                    return RENDERER_MODULE_DX5_ACCELERATION_AVAILABLE;
                }
            }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600011d4
    // a.k.a. THRASH_lockwindow
    DLLAPI RendererModuleWindowLock* STDCALLAPI LockGameWindow(void)
    {
        if (State.Lock.IsActive) { Error("D3D lock called while locked\n"); }

        if (State.Scene.IsActive) { EndRendererScene(); }

        if (State.Lambdas.LockWindow != NULL) { State.Lambdas.LockWindow(TRUE); }

        State.Lock.Surface = State.DX.Surfaces.Window;

        DDSURFACEDESC desc;
        ZeroMemory(&desc, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);

        State.DX.Code = State.Lock.Surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

        if (State.DX.Code != DD_OK) { return NULL; }

        if (State.Settings.IsWindowMode)
        {
            RECT rect;
            GetClientRect(State.Window.HWND, &rect);

            POINT point;
            ZeroMemory(&point, sizeof(POINT));

            ClientToScreen(State.Window.HWND, &point);

            OffsetRect(&rect, point.x, point.y);

            if (State.Lock.Surface == State.DX.Surfaces.Active[1]) // TODO
            {
                desc.lpSurface = (void*)((addr)desc.lpSurface + (addr)(rect.left * 2 + desc.lPitch * rect.top));
            }
        }

        State.Lock.State.Stride = desc.lPitch;
        State.Lock.State.Width = State.Window.Width;
        State.Lock.State.Height = State.Window.Height;

        if (State.Lock.Surface == State.DX.Active.Surfaces.Active.Depth)
        {
            State.Lock.State.Format = desc.ddpfPixelFormat.dwRGBBitCount;
        }
        else if (desc.ddpfPixelFormat.dwRGBBitCount == GRAPHICS_BITS_PER_PIXEL_16)
        {
            State.Lock.State.Format = (desc.ddpfPixelFormat.dwGBitMask == 0x7e0)
                ? RENDERER_PIXEL_FORMAT_16_BIT_565
                : RENDERER_PIXEL_FORMAT_UNKNOWN_11;
        }
        else if (desc.ddpfPixelFormat.dwRGBBitCount == GRAPHICS_BITS_PER_PIXEL_32)
        {
            State.Lock.State.Format = RENDERER_PIXEL_FORMAT_24_BIT;
        }

        State.Lock.IsActive = TRUE;

        State.Lock.State.Data = desc.lpSurface;

        return &State.Lock.State;
    }

    // 0x60001144
    // a.k.a. THRASH_pageflip
    DLLAPI u32 STDCALLAPI ToggleGameWindow(void)
    {
        if (State.Scene.IsActive) { EndRendererScene(); }

        if (State.Lock.IsActive) { Error("D3D pageflip called in while locked\n"); }

        return ToggleRenderer();
    }

    // 0x60002814
    // a.k.a. THRASH_readrect
    DLLAPI u32 STDCALLAPI ReadRectangle(const u32 x, const u32 y, const u32 width, const u32 height, u32* data)
    {
        RendererModuleWindowLock* state = LockGameWindow();

        if (state == NULL) { return RENDERER_MODULE_FAILURE; }

        const u32 multiplier = state->Format == RENDERER_PIXEL_FORMAT_24_BIT ? 4 : 2;
        const u32 length = multiplier * width;

        for (u32 xx = 0; xx < height; xx++)
        {
            const addr address = (xx * state->Stride) + (state->Stride * y) + (multiplier * x);

            CopyMemory(&data[xx * length], (void*)((addr)state->Data + address), length);
        }

        return UnlockGameWindow(state);
    }

    // 0x600024a4
    // a.k.a. THRASH_restore
    DLLAPI u32 STDCALLAPI RestoreGameWindow(void)
    {
        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        ReleaseRendererDevice();

        RendererDeviceIndex = INVALID_RENDERER_DEVICE_INDEX;

        if (State.Lambdas.AcquireWindow != NULL)
        {
            ReleaseRendererWindow();

            return RENDERER_MODULE_SUCCESS;
        }

        ReleaseRendererDeviceInstance();

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60001654
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
            name = State.Devices.Names[DEFAULT_RENDERER_DEVICE_INDEX];
        }
        else
        {
            RendererDeviceIndex = indx;
            State.Device.Identifier = State.Devices.Indexes[indx];
            name = State.Devices.Names[indx];
        }

        strcpy(ModuleDescriptor.Name, name);

        if (State.Lambdas.AcquireWindow == NULL)
        {
            InitializeRendererDevice();
        }
        else
        {
            InitializeRendererDeviceLambdas();
        }

        {
            const char* value = getenv(RENDERER_MODULE_DEVICE_TYPE_ENVIRONMENT_PROPERTY_NAME);

            if (value != NULL)
            {
                if (atoi(value) != RENDERER_MODULE_DEVICE_TYPE_0_ACCELERATED) { return RENDERER_MODULE_SUCCESS; }
            }
        }

        SelectState(RENDERER_MODULE_STATE_SELECT_DEVICE_TYPE, (void*)RENDERER_MODULE_DEVICE_TYPE_0_ACCELERATED);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60002b58
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectState(const u32 state, void* value)
    {
        switch (state)
        {
        case RENDERER_MODULE_STATE_NONE:
        case RENDERER_MODULE_STATE_SELECT_DITHER_STATE:
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_MIP_FILTER_STATE:
        case RENDERER_MODULE_STATE_SELECT_LINE_WIDTH:
        case RENDERER_MODULE_STATE_SELECT_FLAT_FANS_STATE:
        case RENDERER_MODULE_STATE_SELECT_FOG_START:
        case RENDERER_MODULE_STATE_SELECT_FOG_END:
        case RENDERER_MODULE_STATE_401: { return RENDERER_MODULE_SUCCESS; }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE:
        {
            SelectTexture((RendererTexture*)value);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_CULL_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_CULL_NONE: { SelectRendererState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE); break; }
            case RENDERER_MODULE_CULL_COUNTER_CLOCK_WISE: { SelectRendererState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW); break; }
            case RENDERER_MODULE_CULL_CLOCK_WISE: { SelectRendererState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW); break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_MATERIAL:
        {
            const u32 color = (u32)value;

            const f32 r = ((color >> 16) & 0xff) / 255.0f;
            const f32 g = ((color >> 8) & 0xff) / 255.0f;
            const f32 b = ((color >> 0) & 0xff) / 255.0f;

            SelectRendererMaterial(r, g, b);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_STATE:
        {
            if ((u32)value == RENDERER_MODULE_DEPTH_DISABLE)
            {
                SelectRendererState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
                SelectRendererState(D3DRENDERSTATE_ZENABLE, FALSE);
                SelectRendererState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
            }
            else
            {
                SelectRendererState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
                SelectRendererState(D3DRENDERSTATE_ZENABLE, TRUE);
                SelectRendererState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_SHADE_STATE:
        {
            SelectRendererState(D3DRENDERSTATE_SHADEMODE,
                (u32)value == RENDERER_MODULE_SHADE_FLAT ? D3DSHADE_FLAT : D3DSHADE_GOURAUD);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_FILTER_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_FILTER_POINT:
            {
                SelectRendererState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
                SelectRendererState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);

                break;
            }
            case RENDERER_MODULE_TEXTURE_FILTER_LENEAR:
            {
                SelectRendererState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
                SelectRendererState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_ANTI_ALIAS_STATE:
        {
            if (!SelectRendererState(D3DRENDERSTATE_ANTIALIAS, (DWORD)value)) { return RENDERER_MODULE_FAILURE; }

            if ((u32)value != RENDERER_MODULE_ANTIALIASING_NONE
                && State.Device.Capabilities.ResterOperationCaps == 0)
            {
                return RENDERER_MODULE_FAILURE;
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_ALPHA_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_ALPHA_BLEND_NONE: { SelectRendererState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE); break; }
            case RENDERER_MODULE_ALPHA_BLEND_UNKNOWN_1: { return RENDERER_MODULE_FAILURE; }
            case RENDERER_MODULE_ALPHA_BLEND_ACTIVE: { SelectRendererState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE); break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_ADDRESS_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_ADDRESS_CLAMP: { SelectRendererState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP); break; }
            case RENDERER_MODULE_TEXTURE_ADDRESS_WRAP: { SelectRendererState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP); break; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_DENSITY:
        {
            RendererFogDensity = 1.0f / *(f32*)&value;

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_COLOR:
        {
            RendererFogColor = ((u32)value) & RENDERER_MODULE_FOG_COLOR_MASK;

            SelectRendererState(D3DRENDERSTATE_FOGCOLOR, RendererFogColor);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_WINDOW_MODE_STATE:
        {
            State.Settings.IsWindowMode = (BOOL)value;

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_LAMBDAS:
        {
            if (value != NULL)
            {
                const RendererModuleLambdaContainer* lambdas = (RendererModuleLambdaContainer*)value;

                CopyMemory(&State.Lambdas, lambdas, sizeof(RendererModuleLambdaContainer));
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_FOG_INACTIVE:
            {
                SelectRendererState(D3DRENDERSTATE_FOGENABLE, FALSE);
                SelectRendererState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE:
            {
                SelectRendererState(D3DRENDERSTATE_FOGENABLE, TRUE);
                SelectRendererState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE_EXP:
            {
                AcquireRendererFogValues(RendererFogExponentialValues, RendererFogDensity);
                SelectRendererFogAlphas(RendererFogExponentialValues, RendererFogAlphas);

                break;
            }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BIAS_STATE:
        {
            if (RendererDepthBias != *(f32*)&value)
            {
                AttemptRenderScene();

                RendererDepthBias = *(f32*)&value * 0.000030517578f;
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_WINDOW:
        {
            State.Window.HWND = (HWND)value;

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_RESTORE_RENDERER_SURFACES:
        {
            RestoreRendererSurfaces();

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_ALPHA_TEST_STATE:
        {
            if ((u32)value == RENDERER_MODULE_ALPHA_TEST_0)
            {
                SelectRendererState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
                SelectRendererState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_ALWAYS);
            }
            else
            {
                SelectRendererState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
                SelectRendererState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

                {
                    f32 alpha = 0.0f;
                    SelectRendererState(D3DRENDERSTATE_ALPHAREF, *(DWORD*)&alpha);
                }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA_INVERSE_SOURCE_ALPHA:
            {
                SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
                SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

                break;
            }
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA_ONE:
            {
                SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
                SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

                break;
            }
            case RENDERER_MODULE_BLEND_ZERO_INVERSE_SOURCE_ALPHA:
            {
                SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
                SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

                break;
            }
            case RENDERER_MODULE_BLEND_DESTINATION_COLOR_ZERO:
            {
                SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
                SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

                break;
            }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_ALPHAS:
        {
            SelectRendererFogAlphas((u8*)value, RendererFogAlphas);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BUFFER_WRITE_STATE_ALTERNATIVE:
        {
            SelectRendererState(D3DRENDERSTATE_ZWRITEENABLE, ((u32)value) != 0 ? TRUE : FALSE);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_DEVICE_TYPE:
        {
            SelectRendererDeviceType((u32)value);

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_ACQUIRE_DEVICE_CAPABILITIES:
        {
            if (value == NULL) { return NULL; }

            RendererModuleDeviceCapabilities5* result = (RendererModuleDeviceCapabilities5*)value;

            result->IsAccelerated = State.Device.Capabilities.IsAccelerated;
            result->DepthBits = State.Device.Capabilities.DepthBits;
            result->RenderBits = State.Device.Capabilities.RendererBits;
            result->IsPerspectiveTextures = State.Device.Capabilities.IsPerspectiveTextures;
            result->IsAlphaFlatBlending = State.Device.Capabilities.IsAlphaBlend;
            result->IsAlphaProperBlending = State.Device.Capabilities.IsNonFlatAlphaBlend;
            result->IsAlphaTextures = State.Device.Capabilities.IsAlphaTextures;
            result->IsModulateBlending = State.Device.Capabilities.IsModulateBlending;
            result->IsSourceAlphaBlending = State.Device.Capabilities.IsSourceAlphaBlending;
            result->IsColorBlending = State.Device.Capabilities.IsColorBlending;
            result->IsSpecularBlending = State.Device.Capabilities.IsSpecularBlending;

            return (u32)result;
        }
        case RENDERER_MODULE_STATE_SELECT_SOURCE_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_BLEND_ONE: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE); break; }
            case RENDERER_MODULE_BLEND_ZERO: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO); break; }
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_ALPHA: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCALPHA); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_ALPHA: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_ALPHA: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVDESTALPHA); break; }
            case RENDERER_MODULE_BLEND_SOURCE_COLOR: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_COLOR: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_COLOR: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVSRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_COLOR: { SelectRendererState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVDESTCOLOR); break; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        case RENDERER_MODULE_STATE_SELECT_DESTINATION_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_BLEND_ONE: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE); break; }
            case RENDERER_MODULE_BLEND_ZERO: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO); break; }
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_ALPHA: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_ALPHA: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_DESTALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_ALPHA: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVDESTALPHA); break; }
            case RENDERER_MODULE_BLEND_SOURCE_COLOR: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_COLOR: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_DESTCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_COLOR: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_COLOR: { SelectRendererState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVDESTCOLOR); break; }
            }

            return RENDERER_MODULE_SUCCESS;
        }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600034d4
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectTexture(RendererTexture* tex)
    {
        if (tex != NULL) { return SelectRendererTexture(tex); }

        SelectRendererTexture(CurrentRendererTexture);
        return SelectRendererTexture(NULL);
    }

    // 0x600024f0
    // a.k.a. THRASH_setvideomode
    DLLAPI u32 STDCALLAPI SelectVideoMode(const u32 mode, const u32 pending, const u32 depth)
    {
        if (State.Scene.IsActive) { Error("D3D Setting videomode in 3d scene !!!!\n"); }

        SelectRendererDevice();

        if (State.DX.Instance == NULL) { return RENDERER_MODULE_FAILURE; }

        if (State.Lambdas.AcquireWindow == NULL)
        {
            InitializeRendererDeviceSurfacesExecute(0, State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending, NULL);
        }
        else
        {
            SetForegroundWindow(State.Window.HWND);
            PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, mode, pending);
            WaitForSingleObject(State.Mutex, INFINITE);
        }

        if (State.DX.Code != DD_OK) { Error("D3D_setdisplay - error\n"); }

        return State.DX.Code == DD_OK;
    }

    // 0x60001180
    // a.k.a. THRASH_sync
    DLLAPI u32 STDCALLAPI SyncGameWindow(const u32) { return RENDERER_MODULE_SUCCESS; }

    // 0x60003534
    // a.k.a. THRASH_talloc
    DLLAPI RendererTexture* STDCALLAPI AllocateTexture(const u32 width, const u32 height, const u32 format, void* p4, const u32 options)
    {
        if (State.Textures.Formats.Indexes[format] < 0)
        {
            Error("Tried to allocate an unsupported texture format\n");

            return NULL;
        }

        if (State.Textures.Illegal) { return NULL; }

        RendererTexture* tex = InitializeRendererTexture();

        tex->Width = width;
        tex->Height = height;

        tex->UnknownFormatIndexValue = UnknownFormatValues[format];
        tex->FormatIndex = State.Textures.Formats.Indexes[format];
        tex->FormatIndexValue = format;

        tex->Unk10 = (format == RENDERER_PIXEL_FORMAT_16_BIT_555 || format == RENDERER_PIXEL_FORMAT_16_BIT_444) ? 1 : 0; // TODO

        tex->Handle = 0;
        tex->MemoryType = RENDERER_MODULE_TEXTURE_LOCATION_SYSTEM_MEMORY;
        tex->Surface1 = NULL;
        tex->Texture1 = NULL;
        tex->Surface2 = NULL;
        tex->Texture2 = NULL;
        tex->Palette = NULL;
        tex->Unk06 = p4;
        tex->Colors = 0;

        if (InitializeRendererTextureDetails(tex))
        {
            tex->Previous = State.Textures.Current;
            State.Textures.Current = tex;

            if (CurrentRendererTexture == NULL && tex->Unk10 == 0) { CurrentRendererTexture = tex; } // TODO

            return tex;
        }

        ReleaseRendererTexture(tex);

        State.Textures.Illegal = TRUE;

        return NULL;
    }

    // 0x600036c8
    // a.k.a. THRASH_treset
    DLLAPI u32 STDCALLAPI ResetTextures(void)
    {
        if (State.Scene.IsActive)
        {
            FlushGameWindow();
            SyncGameWindow(0);
            Idle();

            State.Scene.IsActive = FALSE;
        }

        while (State.Textures.Current != NULL)
        {
            if (State.Textures.Current->Surface1 != NULL) { State.Textures.Current->Surface1->Release(); }
            if (State.Textures.Current->Texture1 != NULL) { State.Textures.Current->Texture1->Release(); }
            if (State.Textures.Current->Surface2 != NULL) { State.Textures.Current->Surface2->Release(); }
            if (State.Textures.Current->Texture2 != NULL) { State.Textures.Current->Texture2->Release(); }
            if (State.Textures.Current->Palette != NULL) { State.Textures.Current->Palette->Release(); }

            ReleaseRendererTexture(State.Textures.Current);

            State.Textures.Current = State.Textures.Current->Previous;
        }

        CurrentRendererTexture = NULL;
        State.Textures.Current = NULL;

        State.Textures.Illegal = FALSE;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003674
    // a.k.a. THRASH_tupdate
    DLLAPI RendererTexture* STDCALLAPI UpdateTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        if (tex == NULL) { return NULL; }

        return UpdateRendererTexture(tex, pixels, palette) ? tex : NULL;
    }

    // 0x60001368
    // a.k.a. THRASH_unlockwindow
    DLLAPI u32 STDCALLAPI UnlockGameWindow(const RendererModuleWindowLock* state)
    {
        if (State.Lock.IsActive && State.Lock.Surface != NULL)
        {
            State.Lock.Surface->Unlock(state == NULL ? NULL : state->Data);

            State.Lock.Surface = NULL;
            State.Lock.IsActive = FALSE;

            if (State.Lambdas.LockWindow != NULL) { State.Lambdas.LockWindow(FALSE); }
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600010e0
    // a.k.a. THRASH_window
    DLLAPI u32 STDCALLAPI SelectGameWindow(const u32 indx)
    {
        State.DX.Surfaces.Window = indx == 3
            ? State.DX.Active.Surfaces.Active.Depth
            : State.DX.Surfaces.Active[indx]; // TODO

        return State.DX.Surfaces.Window != NULL;
    }

    // 0x600028b0
    // a.k.a. THRASH_writerect
    DLLAPI u32 STDCALLAPI WriteRectangle(const u32 x, const u32 y, const u32 width, const u32 height, const u32* data)
    {
        RendererModuleWindowLock* state = LockGameWindow();

        if (state == NULL) { return RENDERER_MODULE_FAILURE; }

        const u32 multiplier = state->Format == RENDERER_PIXEL_FORMAT_24_BIT ? 4 : 2;
        const u32 length = multiplier * width;

        for (u32 xx = 0; xx < height; xx++)
        {
            const addr address = (xx * state->Stride) + (state->Stride * y) + (multiplier * x);

            CopyMemory((void*)((addr)state->Data + address), &data[xx * length], length);
        }

        return UnlockGameWindow(state);
    }
}