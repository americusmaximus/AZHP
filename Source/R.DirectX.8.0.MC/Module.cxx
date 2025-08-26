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
#include "Mathematics.Basic.hxx"
#include "Module.hxx"
#include "RendererValues.hxx"
#include "Settings.hxx"

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
        ModuleDescriptor.Version = RendererVersion;

        ModuleDescriptor.Caps = RENDERER_MODULE_CAPS_UNKNOWN | RENDERER_MODULE_CAPS_WINDOWED | RENDERER_MODULE_CAPS_LINE_WIDTH
            | (((State.Device.Capabilities.IsPowerOfTwoTexturesHeight & 1) << 3) | ((State.Device.Capabilities.IsPowerOfTwoTexturesWidth & 1) << 2) | ((State.Device.Capabilities.IsSquareOnlyTextures & 1) << 1));

        ModuleDescriptor.MinimumTextureWidth = State.Device.Capabilities.MinTextureWidth;
        ModuleDescriptor.MaximumTextureWidth = State.Device.Capabilities.MaxTextureWidth;
        ModuleDescriptor.MultipleTextureWidth = State.Device.Capabilities.MultipleTextureWidth;
        ModuleDescriptor.MinimumTextureHeight = State.Device.Capabilities.MinTextureHeight;
        ModuleDescriptor.MaximumTextureHeight = State.Device.Capabilities.MaxTextureHeight;
        ModuleDescriptor.MultipleTextureHeight = State.Device.Capabilities.MultipleTextureHeight;
        ModuleDescriptor.MaximumSimultaneousTextures = State.Device.Capabilities.MaximumSimultaneousTextures;

        ModuleDescriptor.SubType = State.Device.SubType;

        ModuleDescriptor.Signature = RENDERER_MODULE_SIGNATURE_D3D8;

        ModuleDescriptor.Size = sizeof(RendererModuleDescriptor2);
        ModuleDescriptor.ClipAlign = 0;
        ModuleDescriptor.DXV = RENDERER_MODULE_VERSION_DX8;
        ModuleDescriptor.Author = RENDERER_MODULE_AUTHOR;
        ModuleDescriptor.ActiveTextureFormatStatesCount = MAX_USABLE_TEXTURE_FORMAT_COUNT;
        ModuleDescriptor.TextureFormatStates = RendererTextureFormatStates;
        ModuleDescriptor.ActiveUnknownValuesCount = MAX_ACTIVE_UNKNOWN_COUNT;
        ModuleDescriptor.UnknownValues = UnknownArray06;
        ModuleDescriptor.Capabilities.Capabilities = ModuleDescriptorDeviceCapabilities;

        strcpy(ModuleDescriptor.Name, RENDERER_MODULE_NAME);

        AcquireRendererModuleDescriptor((RendererModuleDescriptor*)&ModuleDescriptor, ENVIRONMENT_SECTION_NAME);

        return (RendererModuleDescriptor*)&ModuleDescriptor;
    }

    // 0x600012d0
    // a.k.a. THRASH_clearwindow
    DLLAPI u32 STDCALLAPI ClearGameWindow(void)
    {
        return ClearRendererViewPort(
            State.ViewPort.X0, State.ViewPort.Y0, State.ViewPort.X1, State.ViewPort.Y1, State.Window.IsWindow);
    }

    // 0x600013f0
    // a.k.a. THRASH_clip
    DLLAPI u32 STDCALLAPI ClipGameWindow(const u32 x0, const u32 y0, const u32 x1, const u32 y1)
    {
        State.ViewPort.X0 = x0;
        State.ViewPort.Y0 = y0;
        State.ViewPort.X1 = x1 - x0;
        State.ViewPort.Y1 = y1 - y0;

        RenderPackets();
        AttemptRenderPackets();

        D3DVIEWPORT8 vp;
        ZeroMemory(&vp, sizeof(D3DVIEWPORT8));

        vp.X = x0;
        vp.Y = y0;
        vp.Width = x1 - x0;
        vp.Height = y1 - y0;
        vp.MinZ = 0.0f;
        vp.MaxZ = 1.0f;

        if (State.DX.Device->SetViewport(&vp) != D3D_OK)
        {
            BeginRendererScene();

            return RENDERER_MODULE_FAILURE;
        }

        BeginRendererScene();

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60001630
    // a.k.a. THRASH_createwindow
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI CreateGameWindow(const u32 width, const u32 height, const u32 format, const u32 options)
    {
        if (State.DX.Device == NULL) { return RENDERER_MODULE_FAILURE; }
        if ((MAX_WINDOW_COUNT - 1) < State.Window.Count + MIN_WINDOW_INDEX) { return RENDERER_MODULE_FAILURE; }
        if (State.Device.Capabilities.Unk32 == 0) { return RENDERER_MODULE_FAILURE; }

        D3DFORMAT sformat = State.Device.Presentation.BackBufferFormat;
        D3DFORMAT tformat = AcquireRendererTextureFormat(format);
        D3DFORMAT aformat = (D3DFORMAT)format;

        D3DFORMAT stformat = D3DFMT_D16;

        if (options & 1) // TODO
        {
            aformat = (D3DFORMAT)AcquireRendererDeviceFormat(sformat);
            tformat = sformat;
            stformat = State.Device.Presentation.AutoDepthStencilFormat;
        }

        if (options & 2) { stformat = D3DFMT_UNKNOWN; } // TODO

        if (State.DX.Instance->CheckDeviceFormat(0, D3DDEVTYPE_HAL, sformat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, tformat) != D3D_OK) { return RENDERER_MODULE_FAILURE; }

        if (stformat != D3DFMT_UNKNOWN)
        {
            if (State.DX.Instance->CheckDeviceFormat(0, D3DDEVTYPE_HAL, sformat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, stformat) != D3D_OK) { return RENDERER_MODULE_FAILURE; }
        }

        IDirect3DTexture8* tex = NULL;
        if (State.DX.Device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, tformat, D3DPOOL_DEFAULT, &tex) != D3D_OK) { return RENDERER_MODULE_FAILURE; }

        const u32 indx = State.Window.Count + MIN_WINDOW_INDEX;

        State.Window.Count = State.Window.Count + 1;

        State.Windows[indx].Texture = tex;

        if (stformat != D3DFMT_UNKNOWN)
        {
            IDirect3DSurface8* stencil = NULL;
            if (State.DX.Device->CreateDepthStencilSurface(width, height, stformat, D3DMULTISAMPLE_NONE, &stencil) != D3D_OK)
            {
                DestroyGameWindow(indx);

                return RENDERER_MODULE_FAILURE;
            }

            State.Windows[indx].Stencil = stencil;
        }

        ZeroMemory(&State.Windows[indx].Details, sizeof(RendererTexture));

        State.Windows[indx].Details.Width = width;
        State.Windows[indx].Details.Height = height;
        State.Windows[indx].Details.PixelFormat = aformat;
        State.Windows[indx].Details.PixelSize = AcquireRendererDeviceFormatSize(tformat, RendererDeviceFormatSizeBytes);
        State.Windows[indx].Details.TextureFormat = tformat;
        State.Windows[indx].Details.Texture = State.Windows[indx].Texture;
        State.Windows[indx].Details.Palette = INVALID_TEXTURE_PALETTE_VALUE;

        return indx;
    }

    // 0x60001840
    // a.k.a. THRASH_destroywindow
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI DestroyGameWindow(const u32 indx)
    {
        if (State.DX.Device != NULL && (MIN_WINDOW_INDEX - 1) < indx && indx < (State.Window.Count + MIN_WINDOW_INDEX))
        {
            if (State.Windows[indx].Stencil != NULL)
            {
                while (State.Windows[indx].Stencil->Release() != D3D_OK) {}

                State.Windows[indx].Stencil = NULL;
            }

            if (State.Windows[indx].Texture != NULL)
            {
                while (State.Windows[indx].Texture->Release() != D3D_OK) {}

                State.Windows[indx].Texture = NULL;

                ZeroMemory(&State.Windows[indx].Details, sizeof(RendererTexture));
            }

            return RENDERER_MODULE_SUCCESS;
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60002aa0
    // a.k.a. THRASH_drawline
    DLLAPI void STDCALLAPI DrawLine(RVX* a, RVX* b)
    {
        State.Statistics.Lines = State.Statistics.Lines + 1;

        if (AreRenderPacketsComplete(D3DPT_LINELIST, 2)) { RenderScene(); }

        if (State.Data.Packets.Count == 0)
        {
            State.Data.Packets.Packets[0].Type = D3DPT_LINELIST;
            State.Data.Packets.Packets[0].Count = 1;
            State.Data.Packets.Packets[0].Size = 2;

            State.Data.Packets.Count = 1;
        }
        else if (State.Data.Packets.Packets[State.Data.Packets.Count].Type == D3DPT_LINELIST)
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Count =
                State.Data.Packets.Packets[State.Data.Packets.Count].Count + 1;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size =
                State.Data.Packets.Packets[State.Data.Packets.Count].Size + 2;
        }
        else
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_LINELIST;
            State.Data.Packets.Packets[State.Data.Packets.Count].Count = 1;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size = 2;

            State.Data.Packets.Count = State.Data.Packets.Count + 1;
        }

        BYTE* lock;
        State.Data.Vertexes.Buffer->Lock(
            State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * 2, &lock, D3DLOCK_NOOVERWRITE);

        RTLVX* va = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 0));
        RTLVX* vb = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 1));

        CopyMemory(va, a, RendererVertexSize);
        CopyMemory(vb, b, RendererVertexSize);

        UpdateVertex(va);
        UpdateVertex(vb);

        State.Data.Vertexes.Buffer->Unlock();

        State.Data.Vertexes.Count = State.Data.Vertexes.Count + 2;
    }

    // 0x60002be0
    // a.k.a. THRASH_drawlinemesh
    DLLAPI void STDCALLAPI DrawLineMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Lines = State.Statistics.Lines + count;

        u32 indxA = 0, indxB = 1;

        for (u32 x = 0; x < count; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1:
                {
                    indxA = ((u8*)indexes)[x * 2 + 0];
                    indxB = ((u8*)indexes)[x * 2 + 1]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8:
                {
                    indxA = ((u16*)indexes)[x * 2 + 0];
                    indxB = ((u16*)indexes)[x * 2 + 1]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_4:
                {
                    indxA = ((u32*)indexes)[x * 2 + 0];
                    indxB = ((u32*)indexes)[x * 2 + 1]; break;
                }
                }
            }

            RVX* a = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxA));
            RVX* b = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxB));

            DrawLine(a, b);

            if (indexes == NULL) { indxA = indxA + 2; indxB = indxB + 2; }
        }
    }

    // 0x60003000
    // a.k.a. THRASH_drawlinestrip
    DLLAPI void STDCALLAPI DrawLineStrip(const u32 count, RVX* vertexes)
    {
        RTLVX* points = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { DrawLine((RVX*)&points[x + 0], (RVX*)&points[x + 1]); }
    }

    // 0x60002c70
    // a.k.a. THRASH_drawlinestrip
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawLineStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Lines = State.Statistics.Lines + 1;

        if (count <= 1) { return; }

        if ((count % 2) == 0)
        {
            if (AreRenderPacketsComplete(D3DPT_LINESTRIP, count + 1)) { RenderScene(); }

            State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_LINESTRIP;
            State.Data.Packets.Packets[State.Data.Packets.Count].Count = count;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size = count + 1;

            State.Data.Packets.Count = State.Data.Packets.Count + 1;

            BYTE* lock;
            State.Data.Vertexes.Buffer->Lock(
                State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * (count + 1), &lock, D3DLOCK_NOOVERWRITE);

            u32 indx = 0;

            for (u32 x = 0; x < count + 1; x++)
            {
                if (indexes != NULL)
                {
                    switch (RendererIndexSize)
                    {
                    case RENDERER_MODULE_INDEX_SIZE_1: { indx = ((u8*)indexes)[x]; break; }
                    case RENDERER_MODULE_INDEX_SIZE_2:
                    case RENDERER_MODULE_INDEX_SIZE_8: { indx = ((u16*)indexes)[x]; break; }
                    case RENDERER_MODULE_INDEX_SIZE_4: { indx = ((u32*)indexes)[x]; break; }
                    }
                }

                RTLVX* src = (RTLVX*)((addr)vertexes + (addr)(RendererVertexSize * indx));
                RTLVX* dst = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * x));

                CopyMemory(dst, src, RendererVertexSize);
                UpdateVertex(dst);

                if (indexes == NULL) { indx = indx + 1; }
            }

            State.Data.Vertexes.Buffer->Unlock();

            State.Data.Vertexes.Count = State.Data.Vertexes.Count + count + 1;
        }
    }

    // 0x60002dc0
    // a.k.a. THRASH_drawpoint
    DLLAPI void STDCALLAPI DrawPoint(RVX* vertex)
    {
        State.Statistics.Points = State.Statistics.Points + 1;

        if (AreRenderPacketsComplete(D3DPT_POINTLIST, 1)) { RenderScene(); }

        if (State.Data.Packets.Count == 0)
        {
            State.Data.Packets.Packets[0].Type = D3DPT_POINTLIST;
            State.Data.Packets.Packets[0].Count = 1;
            State.Data.Packets.Packets[0].Size = 1;

            State.Data.Packets.Count = 1;
        }
        else if (State.Data.Packets.Packets[State.Data.Packets.Count].Type == D3DPT_POINTLIST)
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Count =
                State.Data.Packets.Packets[State.Data.Packets.Count].Count + 1;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size =
                State.Data.Packets.Packets[State.Data.Packets.Count].Size + 1;
        }
        else
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_POINTLIST;
            State.Data.Packets.Packets[State.Data.Packets.Count].Count = 1;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size = 1;

            State.Data.Packets.Count = State.Data.Packets.Count + 1;
        }

        BYTE* lock;
        State.Data.Vertexes.Buffer->Lock(
            State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize, &lock, D3DLOCK_NOOVERWRITE);

        CopyMemory(lock, vertex, RendererVertexSize);

        UpdateVertex((RTLVX*)lock);

        State.Data.Vertexes.Buffer->Unlock();

        State.Data.Vertexes.Count = State.Data.Vertexes.Count + 1;
    }

    // 0x60002ec0
    // a.k.a. THRASH_drawpointmesh
    DLLAPI void STDCALLAPI DrawPointMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Points = State.Statistics.Points + count;

        u32 indx = 0;

        for (u32 x = 0; x < count; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1: { indx = ((u8*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8: { indx = ((u16*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_4: { indx = ((u32*)indexes)[x]; break; }
                }
            }

            DrawPoint((RVX*)((addr)vertexes + (addr)(RendererVertexSize * indx)));

            if (indexes == NULL) { indx = indx + 1; }
        }
    }

    // 0x60003030
    // a.k.a. THRASH_drawpointstrip
    DLLAPI void STDCALLAPI DrawPointStrip(const u32 count, RVX* vertexes)
    {
        RTLVX* points = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { DrawPoint((RVX*)&points[x]); }
    }

    // 0x60002790
    // a.k.a. THRASH_drawquad
    DLLAPI void STDCALLAPI DrawQuad(RVX* a, RVX* b, RVX* c, RVX* d)
    {
        State.Statistics.Quads = State.Statistics.Quads + 1;

        if (AreRenderPacketsComplete(D3DPT_TRIANGLELIST, 6)) { RenderScene(); }

        if (State.Data.Packets.Count == 0)
        {
            State.Data.Packets.Packets[0].Type = D3DPT_TRIANGLELIST;
            State.Data.Packets.Packets[0].Count = 2;
            State.Data.Packets.Packets[0].Size = 6;

            State.Data.Packets.Count = 1;
        }
        else if (State.Data.Packets.Packets[State.Data.Packets.Count].Type == D3DPT_TRIANGLELIST)
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Count =
                State.Data.Packets.Packets[State.Data.Packets.Count].Count + 2;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size =
                State.Data.Packets.Packets[State.Data.Packets.Count].Size + 6;
        }
        else
        {
            State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_TRIANGLELIST;
            State.Data.Packets.Packets[State.Data.Packets.Count].Count = 2;
            State.Data.Packets.Packets[State.Data.Packets.Count].Size = 6;

            State.Data.Packets.Count = State.Data.Packets.Count + 1;
        }

        BYTE* lock;
        State.Data.Vertexes.Buffer->Lock(
            State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * 6, &lock, D3DLOCK_NOOVERWRITE);

        {
            RTLVX* va = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 0));
            RTLVX* vb = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 1));
            RTLVX* vc = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 2));

            CopyMemory(va, a, RendererVertexSize);
            CopyMemory(vb, b, RendererVertexSize);
            CopyMemory(vc, c, RendererVertexSize);

            UpdateVertex(va);
            UpdateVertex(vb);
            UpdateVertex(vc);
        }

        {
            RTLVX* va = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 3));
            RTLVX* vc = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 4));
            RTLVX* vd = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 5));

            CopyMemory(va, a, RendererVertexSize);
            CopyMemory(vc, c, RendererVertexSize);
            CopyMemory(vc, d, RendererVertexSize);

            UpdateVertex(va);
            UpdateVertex(vc);
            UpdateVertex(vd);
        }

        State.Data.Vertexes.Buffer->Unlock();

        State.Data.Vertexes.Count = State.Data.Vertexes.Count + 6;
    }

    // 0x600029b0
    // a.k.a. THRASH_drawquadmesh
    DLLAPI void STDCALLAPI DrawQuadMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Quads = State.Statistics.Quads + count;

        u32 indxA = 0, indxB = 1, indxC = 2, indxD = 3;

        for (u32 x = 0; x < count; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1:
                {
                    indxA = ((u8*)indexes)[x * 4 + 0];
                    indxB = ((u8*)indexes)[x * 4 + 1];
                    indxC = ((u8*)indexes)[x * 4 + 2];
                    indxD = ((u8*)indexes)[x * 4 + 3]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8:
                {
                    indxA = ((u16*)indexes)[x * 4 + 0];
                    indxB = ((u16*)indexes)[x * 4 + 1];
                    indxC = ((u16*)indexes)[x * 4 + 2];
                    indxD = ((u16*)indexes)[x * 4 + 3]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_4:
                {
                    indxA = ((u32*)indexes)[x * 4 + 0];
                    indxB = ((u32*)indexes)[x * 4 + 1];
                    indxC = ((u32*)indexes)[x * 4 + 2];
                    indxD = ((u32*)indexes)[x * 4 + 3]; break;
                }
                }
            }

            RVX* a = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxA));
            RVX* b = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxB));
            RVX* c = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxC));
            RVX* d = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxD));

            DrawQuad(a, b, c, d);

            if (indexes == NULL)
            {
                indxA = indxA + 4; indxB = indxB + 4;
                indxC = indxC + 4; indxD = indxD + 4;
            }
        }
    }

    // 0x60001960
    // a.k.a. THRASH_drawsprite
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawSprite(RVX* a, RVX* b)
    {
        RTLVX2 c, d;

        CopyMemory(&c, b, RendererVertexSize);
        CopyMemory(&d, b, RendererVertexSize);

        c.XYZ.Y = ((RTLVX*)a)->XYZ.Y;
        c.UV1.Y = ((RTLVX*)a)->UV.Y;

        d.XYZ.X = ((RTLVX*)a)->XYZ.X;
        d.UV1.X = ((RTLVX*)a)->UV.X;

        DrawQuad(a, (RVX*)&c, b, (RVX*)&d);
    }

    // 0x600019e0
    // a.k.a. THRASH_drawspritemesh
    // NOTE: Never being called by the application.
    DLLAPI void STDCALLAPI DrawSpriteMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        for (u32 x = 0; x < count; x++)
        {
            u32 indxA = 0, indxB = 0;

            switch (RendererIndexSize)
            {
            case RENDERER_MODULE_INDEX_SIZE_1:
            {
                indxA = ((u8*)indexes)[x * 2 + 0];
                indxB = ((u8*)indexes)[x * 2 + 1]; break;
            }
            case RENDERER_MODULE_INDEX_SIZE_2:
            case RENDERER_MODULE_INDEX_SIZE_8:
            {
                indxA = ((u16*)indexes)[x * 2 + 0];
                indxB = ((u16*)indexes)[x * 2 + 1]; break;
            }
            case RENDERER_MODULE_INDEX_SIZE_4:
            {
                indxA = ((u32*)indexes)[x * 2 + 0];
                indxB = ((u32*)indexes)[x * 2 + 1]; break;
            }
            }

            DrawSprite((RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxA)),
                (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxB)));
        }
    }

    // 0x60002290
    // a.k.a. THRASH_drawtri
    DLLAPI void STDCALLAPI DrawTriangle(RVX* a, RVX* b, RVX* c)
    {
        if (State.Settings.Cull == RENDERER_CULL_MODE_NONE
            || (AcquireNormal((f32x3*)a, (f32x3*)b, (f32x3*)c) & RENDERER_CULL_MODE_COUNTER_CLOCK_WISE) != State.Settings.Cull)
        {
            State.Statistics.Triangles = State.Statistics.Triangles + 1;

            if (AreRenderPacketsComplete(D3DPT_TRIANGLELIST, 3)) { RenderScene(); }

            if (State.Data.Packets.Count == 0)
            {
                State.Data.Packets.Packets[0].Type = D3DPT_TRIANGLELIST;
                State.Data.Packets.Packets[0].Count = 1;
                State.Data.Packets.Packets[0].Size = 3;

                State.Data.Packets.Count = 1;
            }
            else if (State.Data.Packets.Packets[State.Data.Packets.Count].Type == D3DPT_TRIANGLELIST)
            {
                State.Data.Packets.Packets[State.Data.Packets.Count].Count =
                    State.Data.Packets.Packets[State.Data.Packets.Count].Count + 1;
                State.Data.Packets.Packets[State.Data.Packets.Count].Size =
                    State.Data.Packets.Packets[State.Data.Packets.Count].Size + 3;
            }
            else
            {
                State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_TRIANGLELIST;
                State.Data.Packets.Packets[State.Data.Packets.Count].Count = 1;
                State.Data.Packets.Packets[State.Data.Packets.Count].Size = 3;

                State.Data.Packets.Count = State.Data.Packets.Count + 1;
            }

            BYTE* lock;
            State.Data.Vertexes.Buffer->Lock(
                State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * 3, &lock, D3DLOCK_NOOVERWRITE);

            RTLVX* va = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 0));
            RTLVX* vb = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 1));
            RTLVX* vc = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * 2));

            CopyMemory(va, a, RendererVertexSize);
            CopyMemory(vb, b, RendererVertexSize);
            CopyMemory(vc, c, RendererVertexSize);

            UpdateVertex(va);
            UpdateVertex(vb);
            UpdateVertex(vc);

            State.Data.Vertexes.Buffer->Unlock();

            State.Data.Vertexes.Count = State.Data.Vertexes.Count + 3;
        }
    }

    // 0x60002fd0
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFan(const u32 count, RVX* vertexes)
    {
        const RTLVX* points = (RTLVX*)vertexes;

        for (u32 x = 0; x < count; x++) { DrawTriangle(vertexes, (RVX*)&points[x + 1], (RVX*)&points[x + 2]); }
    }

    // 0x60002650
    // a.k.a. THRASH_drawtrifan
    DLLAPI void STDCALLAPI DrawTriangleFans(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Fans = State.Statistics.Fans + 1;

        if (AreRenderPacketsComplete(D3DPT_TRIANGLEFAN, count + 2)) { RenderScene(); }

        State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_TRIANGLEFAN;
        State.Data.Packets.Packets[State.Data.Packets.Count].Count = count;
        State.Data.Packets.Packets[State.Data.Packets.Count].Size = count + 2;

        State.Data.Packets.Count = State.Data.Packets.Count + 1;

        BYTE* lock;
        State.Data.Vertexes.Buffer->Lock(
            State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * (count + 2), &lock, D3DLOCK_NOOVERWRITE);

        u32 indx = 0;

        for (u32 x = 0; x < count + 2; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1: { indx = ((u8*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8: { indx = ((u16*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_4: { indx = ((u32*)indexes)[x]; break; }
                }
            }

            RTLVX* src = (RTLVX*)((addr)vertexes + (addr)(RendererVertexSize * indx));
            RTLVX* dst = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * x));

            CopyMemory(dst, src, RendererVertexSize);
            UpdateVertex(dst);

            if (indexes == NULL) { indx = indx + 1; }
        }

        State.Data.Vertexes.Buffer->Unlock();
        State.Data.Vertexes.Count = State.Data.Vertexes.Count + count + 2;
    }

    // 0x60002460
    // a.k.a. THRASH_drawtrimesh
    DLLAPI void STDCALLAPI DrawTriangleMesh(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Triangles = State.Statistics.Triangles + count;

        u32 indxA = 0, indxB = 1, indxC = 2;

        for (u32 x = 0; x < count; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1:
                {
                    indxA = ((u8*)indexes)[x * 3 + 0];
                    indxB = ((u8*)indexes)[x * 3 + 1];
                    indxC = ((u8*)indexes)[x * 3 + 2]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8:
                {
                    indxA = ((u16*)indexes)[x * 3 + 0];
                    indxB = ((u16*)indexes)[x * 3 + 1];
                    indxB = ((u16*)indexes)[x * 3 + 2]; break;
                }
                case RENDERER_MODULE_INDEX_SIZE_4:
                {
                    indxA = ((u32*)indexes)[x * 3 + 0];
                    indxB = ((u32*)indexes)[x * 3 + 1];
                    indxB = ((u32*)indexes)[x * 3 + 2]; break;
                }
                }
            }

            RVX* a = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxA));
            RVX* b = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxB));
            RVX* c = (RVX*)((addr)vertexes + (addr)(RendererVertexSize * indxC));

            DrawTriangle(a, b, c);

            if (indexes == NULL) { indxA = indxA + 3; indxB = indxB + 3; indxC = indxC + 3; }
        }
    }

    // 0x60002f80
    // a.k.a. THRASH_drawtristrip
    DLLAPI void STDCALLAPI DrawTriangleStrip(const u32 count, RVX* vertexes)
    {
        u32 offset = 0;

        for (u32 x = 0; x < count; x = x + 2)
        {
            {
                RTLVX* a = &((RTLVX*)vertexes)[offset + 0];
                RTLVX* b = &((RTLVX*)vertexes)[offset + 1];
                RTLVX* c = &((RTLVX*)vertexes)[offset + 2];

                DrawTriangle((RVX*)a, (RVX*)b, (RVX*)c);
            }

            if (x + 2 < count) { return; }

            {
                RTLVX* a = &((RTLVX*)vertexes)[offset + 0];
                RTLVX* b = &((RTLVX*)vertexes)[offset + 3];
                RTLVX* c = &((RTLVX*)vertexes)[offset + 2];

                DrawTriangle((RVX*)a, (RVX*)b, (RVX*)c);
            }

            offset = offset + 2;
        }
    }

    // 0x60002510
    // a.k.a. THRASH_drawtristrip
    DLLAPI void STDCALLAPI DrawTriangleStrips(const u32 count, RVX* vertexes, const u32* indexes)
    {
        State.Statistics.Strips = State.Statistics.Strips + 1;

        if (AreRenderPacketsComplete(D3DPT_TRIANGLESTRIP, count + 2)) { RenderScene(); }

        State.Data.Packets.Packets[State.Data.Packets.Count].Type = D3DPT_TRIANGLESTRIP;
        State.Data.Packets.Packets[State.Data.Packets.Count].Count = count;
        State.Data.Packets.Packets[State.Data.Packets.Count].Size = count + 2;

        State.Data.Packets.Count = State.Data.Packets.Count + 1;

        BYTE* lock;
        State.Data.Vertexes.Buffer->Lock(
            State.Data.Vertexes.Count * RendererVertexSize, RendererVertexSize * (count + 2), &lock, D3DLOCK_NOOVERWRITE);

        u32 indx = 0;

        for (u32 x = 0; x < count + 2; x++)
        {
            if (indexes != NULL)
            {
                switch (RendererIndexSize)
                {
                case RENDERER_MODULE_INDEX_SIZE_1: { indx = ((u8*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_2:
                case RENDERER_MODULE_INDEX_SIZE_8: { indx = ((u16*)indexes)[x]; break; }
                case RENDERER_MODULE_INDEX_SIZE_4: { indx = ((u32*)indexes)[x]; break; }
                }
            }

            RTLVX* src = (RTLVX*)((addr)vertexes + (addr)(RendererVertexSize * indx));
            RTLVX* dst = (RTLVX*)((addr)lock + (addr)(RendererVertexSize * x));

            CopyMemory(dst, src, RendererVertexSize);
            UpdateVertex(dst);

            if (indexes == NULL) { indx = indx + 1; }
        }

        State.Data.Vertexes.Buffer->Unlock();
        State.Data.Vertexes.Count = State.Data.Vertexes.Count + count + 2;
    }

    // 0x60001300
    // a.k.a. THRASH_flushwindow
    DLLAPI u32 STDCALLAPI FlushGameWindow(void)
    {
        if (!State.Scene.IsActive) { return RENDERER_MODULE_FAILURE; }

        if (State.DX.Device != NULL)
        {
            RenderPackets();
            State.Scene.IsActive = State.DX.Device->EndScene() != D3D_OK;
        }

        return State.Scene.IsActive;
    }
    
    // 0x600030d0
    // a.k.a. THRASH_getstate
    DLLAPI u32 STDCALLAPI AcquireState(const u32 state)
    {
        const s32 indx = AcquireTextureStateStageIndex(state & RENDERER_MODULE_SELECT_STATE_MASK);

        return  (-1 < indx) ? State.Textures.StageStates[indx].Values[MAKETEXTURESTAGEVALUE(state)] : indx;
    }

    // 0x60001800
    // a.k.a. THRASH_getwindowtexture
    DLLAPI RendererTexture* STDCALLAPI AcquireGameWindowTexture(const u32 indx)
    {
        return (indx >= MIN_WINDOW_INDEX
            || indx < (State.Window.Count + MIN_WINDOW_INDEX)
            || State.Windows[indx].Texture == NULL)
            ? NULL : &State.Windows[indx].Details;
    }

    // 0x60003060
    // a.k.a. THRASH_idle
    DLLAPI void STDCALLAPI Idle(void) { return; }

    // 0x60003740
    // a.k.a. THRASH_init
    DLLAPI u32 STDCALLAPI Init(void)
    {
        InitializeSettings();

        if (State.DX.Instance == NULL)
        {
            State.DX.Instance = Direct3DCreate8(D3D_SDK_VERSION);

            if (State.DX.Instance == NULL) { return RENDERER_MODULE_FAILURE; }
        }

        AcquireRendererDeviceCount();

        AcquireRendererDeviceFormats();

        InitializeTextureStateStates();

        if (State.Lambdas.Lambdas.Execute != NULL)
        {
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceExecute);
            State.Lambdas.Lambdas.Execute(RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, (RENDERERMODULEEXECUTECALLBACK)InitializeRendererDeviceSurfacesExecute);
        }

        atexit(ReleaseRendererModule);

        for (u32 x = 0; x < MAX_OUTPUT_FOG_ALPHA_COUNT; x++)
        {
            RendererFogAlphas[x] = MAX_OUTPUT_FOG_ALPHA_VALUE - x;
        }

        for (u32 x = 0; x < MAX_WINDOW_COUNT; x++)
        {
            State.Windows[x].Texture = NULL;
            State.Windows[x].Stencil = NULL;

            ZeroMemory(&State.Windows[x].Details, sizeof(RendererTexture));
        }

        State.DX.IsInit = TRUE;

        return State.Devices.Count;
    }

    // 0x60004f10
    // a.k.a. THRASH_is
    DLLAPI u32 STDCALLAPI Is(void)
    {
        return RENDERER_MODULE_DX8_ACCELERATION_AVAILABLE;
    }

    // 0x600014c0
    // a.k.a. THRASH_lockwindow
    DLLAPI RendererModuleWindowLock* STDCALLAPI LockGameWindow(void)
    {
        if (State.Window.Surface == NULL || State.Window.Index == 1
            || State.Window.Index == 3 || MIN_WINDOW_INDEX <= State.Window.Index) {
            return NULL;
        }

        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        if (State.Scene.IsActive) { AttemptRenderPackets(); }

        if (State.Lambdas.Lambdas.LockWindow != NULL) { State.Lambdas.Lambdas.LockWindow(TRUE); }

        IDirect3DSurface8* surface = State.DX.Surfaces.Surfaces[State.Window.Index];

        D3DLOCKED_RECT lr;
        ZeroMemory(&lr, sizeof(D3DLOCKED_RECT));

        if (surface->LockRect(&lr, NULL, D3DLOCK_NONE) == D3D_OK)
        {
            D3DSURFACE_DESC desc;
            ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));

            surface->GetDesc(&desc);

            State.Lock.State.Data = lr.pBits;
            State.Lock.State.Stride = lr.Pitch;
            State.Lock.State.Format = AcquireRendererDeviceFormat(desc.Format);
            State.Lock.State.Width = desc.Width;
            State.Lock.State.Height = desc.Height;

            State.Lock.Surface = surface;

            State.Lock.IsActive = TRUE;

            return &State.Lock.State;
        }

        return NULL;
    }

    // 0x60001310
    // a.k.a. THRASH_pageflip
    DLLAPI void STDCALLAPI ToggleGameWindow(void)
    {
        FlushGameWindow();

        if (State.Lock.IsActive) { LOGERROR("pageflip called while buffer was locked\n"); }

        ToggleRenderer();

        State.Statistics.Triangles = 0;
        State.Statistics.Quads = 0;
        State.Statistics.Lines = 0;
        State.Statistics.Strips = 0;
        State.Statistics.Fans = 0;
        State.Statistics.Points = 0;

        BeginRendererScene();
    }

    // 0x60003070
    // a.k.a. THRASH_readrect
    DLLAPI u32 STDCALLAPI ReadRectangle(const u32 x, const u32 y, const u32 width, const u32 height, u32* pixels)
    {
        return ReadRectangles(x, y, width, height, pixels, 0);
    }

    // 0x60004f20
    // a.k.a. THRASH_readrect
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI ReadRectangles(const u32 x, const u32 y, const u32 width, const u32 height, u32* pixels, const u32 stride)
    {
        const RendererModuleWindowLock* state = LockGameWindow();

        if (state == NULL) { return RENDERER_MODULE_FAILURE; }

        const u32 multiplier = state->Format == RENDERER_PIXEL_FORMAT_A8R8G8B8
            ? 4 : (state->Format == RENDERER_PIXEL_FORMAT_R8G8B8 ? 3 : 2);

        const u32 length = stride != 0 ? stride : multiplier * width;

        for (u32 xx = 0; xx < height; xx++)
        {
            const addr offset = (xx * state->Stride) + (state->Stride * y) + (multiplier * x);

            CopyMemory(&pixels[xx * length], (void*)((addr)state->Data + (addr)offset), length);
        }

        return UnlockGameWindow(NULL);
    }

    // 0x60003a00
    // a.k.a. THRASH_restore
    DLLAPI u32 STDCALLAPI RestoreGameWindow(void)
    {
        if (!State.DX.IsInit) { return RENDERER_MODULE_FAILURE; }

        HWND hwnd = NULL;

        if (State.Lambdas.Lambdas.Execute != NULL)
        {
            if (GetWindowThreadProcessId(State.Window.Parent.HWND, NULL) != GetCurrentThreadId())
            {
                hwnd = State.Lambdas.Lambdas.AcquireWindow();

                State.Mutexes.Surface = CreateEventA(NULL, FALSE, FALSE, NULL);

                SetForegroundWindow(hwnd);
                PostMessageA(hwnd, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_SURFACES, 0, 0);

                if (WaitForSingleObject(State.Mutexes.Surface, 10000) == WAIT_OBJECT_0)
                {
                    State.DX.IsInit = FALSE;

                    return RENDERER_MODULE_SUCCESS;
                }
            }
        }

        InitializeRendererDeviceSurfacesExecute(0, hwnd, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, 0, 0, 0);

        State.DX.IsInit = FALSE;

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60003800
    // a.k.a. THRASH_selectdisplay
    DLLAPI u32 STDCALLAPI SelectDevice(const s32 indx)
    {
        if (!State.Device.IsInit)
        {
            s32 actual = indx;

            if (State.Lambdas.Lambdas.AcquireWindow != NULL || State.Window.HWND != NULL)
            {
                const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONMENT_PROPERTY_NAME);

                if (value != NULL) { actual = atoi(value); }
            }

            State.Device.Index = DEFAULT_DEVICE_INDEX;

            if ((DEFAULT_DEVICE_INDEX - 1) < indx && indx < State.Devices.Count) { State.Device.Index = indx; }

            strncpy(ModuleDescriptor.DeviceName, State.Devices.Enumeration.Names[indx], MAX_RENDERER_MODULE_DEVICE_LONG_NAME_LENGTH);

            State.Device.IsInit = TRUE;
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60005490
    // a.k.a. THRASH_setstate
    DLLAPI u32 STDCALLAPI SelectState(const u32 state, void* value)
    {
        const u32 actual = state & RENDERER_MODULE_SELECT_STATE_MASK;
        const u32 stage = MAKETEXTURESTAGEVALUE(state);

        u32 result = RENDERER_MODULE_FAILURE;

        switch (actual)
        {
        case RENDERER_MODULE_STATE_NONE:
        case RENDERER_MODULE_STATE_49:
        case RENDERER_MODULE_STATE_50:
        case RENDERER_MODULE_STATE_51:
        case RENDERER_MODULE_STATE_SELECT_VERTEX_MIN_DEPTH:
        case RENDERER_MODULE_STATE_SELECT_VERTEX_MAX_DEPTH: { return RENDERER_MODULE_FAILURE; }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE:
        {
            SelectTexture((RendererTexture*)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_CULL_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_CULL_NONE:
            {
                SelectRendererState(D3DRS_CULLMODE, D3DCULL_NONE);

                State.Settings.Cull = RENDERER_CULL_MODE_NONE;

                break;
            }
            case RENDERER_MODULE_CULL_COUNTER_CLOCK_WISE:
            {
                SelectRendererState(D3DRS_CULLMODE, D3DCULL_CCW);

                State.Settings.Cull = RENDERER_CULL_MODE_COUNTER_CLOCK_WISE;

                break;
            }
            case RENDERER_MODULE_CULL_CLOCK_WISE:
            {
                SelectRendererState(D3DRS_CULLMODE, D3DCULL_CW);

                State.Settings.Cull = RENDERER_CULL_MODE_CLOCK_WISE;

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_MATERIAL:
        {
            SelectRendererMaterial((u32)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_STATE:
        {
            if (!State.Device.Capabilities.IsDepthAvailable) { return RENDERER_MODULE_FAILURE; }

            if (State.DX.Surfaces.Bits == 0) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_DEPTH_INACTIVE:
            {
                SelectRendererState(D3DRS_ZWRITEENABLE, FALSE);
                SelectRendererState(D3DRS_ZENABLE, D3DZB_FALSE);
                SelectRendererState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

                break;
            }
            case RENDERER_MODULE_DEPTH_ACTIVE:
            {
                SelectRendererState(D3DRS_ZWRITEENABLE, TRUE);
                SelectRendererState(D3DRS_ZENABLE, D3DZB_TRUE);
                SelectRendererState(D3DRS_ZFUNC, RendererDepthFunction);

                break;
            }
            case RENDERER_MODULE_DEPTH_ACTIVE_W:
            {
                if (State.Device.Capabilities.IsWBufferAvailable)
                {
                    SelectRendererState(D3DRS_ZENABLE, D3DZB_TRUE);
                    SelectRendererState(D3DRS_ZWRITEENABLE, TRUE);
                    SelectRendererState(D3DRS_ZFUNC, RendererDepthFunction);
                    SelectRendererState(D3DRS_ZENABLE, D3DZB_USEW);
                }
                else
                {
                    SelectRendererState(D3DRS_ZWRITEENABLE, TRUE);
                    SelectRendererState(D3DRS_ZENABLE, D3DZB_TRUE);
                    SelectRendererState(D3DRS_ZFUNC, RendererDepthFunction);

                    return RENDERER_MODULE_FAILURE;
                }

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DITHER_STATE:
        {
            SelectRendererState(D3DRS_DITHERENABLE, ((u32)value) != 0 ? TRUE : FALSE);

            if (!State.Device.Capabilities.IsDitherAvailable) { return RENDERER_MODULE_FAILURE; }

            result = State.Device.Capabilities.IsDitherAvailable; break;
        }
        case RENDERER_MODULE_STATE_SELECT_SHADE_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_SHADE_FLAT:
            case RENDERER_MODULE_SHADE_GOURAUD:
            {
                if ((u32)value == RENDERER_MODULE_SHADE_FLAT && SettingsState.FlatShading)
                {
                    SelectRendererState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
                    SelectRendererState(D3DRS_SPECULARENABLE, FALSE);

                    RendererShadeMode = RENDERER_MODULE_SHADE_FLAT;
                }
                else
                {
                    SelectRendererState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
                    SelectRendererState(D3DRS_SPECULARENABLE, FALSE);

                    RendererShadeMode = RENDERER_MODULE_SHADE_GOURAUD;
                }

                break;
            }
            case RENDERER_MODULE_SHADE_GOURAUD_SPECULAR:
            {
                if (!State.Device.Capabilities.IsSpecularGouraudBlending) { return RENDERER_MODULE_FAILURE; }

                SelectRendererState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
                SelectRendererState(D3DRS_SPECULARENABLE, TRUE);

                RendererShadeMode = RENDERER_MODULE_SHADE_GOURAUD;

                break;
            }
            case RENDERER_MODULE_SHADE_3: { return RENDERER_MODULE_FAILURE; }
            case RENDERER_MODULE_SHADE_4:
            {
                SelectRendererState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
                SelectRendererState(D3DRS_SPECULARENABLE, FALSE);

                RendererShadeMode = RENDERER_MODULE_SHADE_4;

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_FILTER_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_FILTER_POINT:
            {
                SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
                SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_POINT);

                break;
            }
            case RENDERER_MODULE_TEXTURE_FILTER_LENEAR:
            {
                SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

                break;
            }
            case RENDERER_MODULE_TEXTURE_FILTER_ANISOTROPY:
            {
                if (!State.Device.Capabilities.IsAnisotropyAvailable) { return RENDERER_MODULE_FAILURE; }

                SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
                SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_MIP_MAP_LOD_BIAS_STATE:
        {
            if (!State.Device.Capabilities.IsMipMapBiasAvailable) { return RENDERER_MODULE_FAILURE; }

            SelectRendererTextureStage(stage, D3DTSS_MIPMAPLODBIAS, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_ANTI_ALIAS_STATE:
        case RENDERER_MODULE_STATE_SELECT_LOG_LAMBDA:
        case RENDERER_MODULE_STATE_SELECT_EXECUTE_LAMBDA:
        case RENDERER_MODULE_STATE_SELECT_LOCK_WINDOW_LAMBDA:
        case RENDERER_MODULE_STATE_SELECT_WINDOW_LOCK_STATE:
        case RENDERER_MODULE_STATE_SELECT_LINE_VERTEX_SIZE:
        case RENDERER_MODULE_STATE_SELECT_VERSION:
        case RENDERER_MODULE_STATE_SELECT_MEMORY_ALLOCATE_LAMBDA:
        case RENDERER_MODULE_STATE_SELECT_MEMORY_RELEASE_LAMBDA:
        case RENDERER_MODULE_STATE_34:
        case RENDERER_MODULE_STATE_37:
        case RENDERER_MODULE_STATE_ACQUIRE_RENDERER_INSTANCE:
        case RENDERER_MODULE_STATE_SELECT_SELECT_STATE_LAMBDA:
        case RENDERER_MODULE_STATE_SELECT_BACK_BUFFER_STATE:
        case RENDERER_MODULE_STATE_SELECT_PALETTE:
        case RENDERER_MODULE_STATE_45:
        case RENDERER_MODULE_STATE_55:
        case RENDERER_MODULE_STATE_SELECT_DISPLAY_STATE:
        case RENDERER_MODULE_STATE_SELECT_GAME_WINDOW_INDEX:
        case RENDERER_MODULE_STATE_SELECT_SHAMELESS_PLUG_STATE:
        case RENDERER_MODULE_STATE_62:
        case RENDERER_MODULE_STATE_SELECT_WINDOW_MODE_ACTIVE_STATE:
        case RENDERER_MODULE_STATE_INDEX_SIZE:
        case RENDERER_MODULE_STATE_SELECT_LOG_STATE:
        case RENDERER_MODULE_STATE_72:
        case RENDERER_MODULE_STATE_77:
        case RENDERER_MODULE_STATE_78:
        case RENDERER_MODULE_STATE_79:
        case RENDERER_MODULE_STATE_80:
        case RENDERER_MODULE_STATE_81:
        case RENDERER_MODULE_STATE_82:
        case RENDERER_MODULE_STATE_83:
        case RENDERER_MODULE_STATE_84:
        case RENDERER_MODULE_STATE_85:
        case RENDERER_MODULE_STATE_86:
        case RENDERER_MODULE_STATE_87:
        case RENDERER_MODULE_STATE_88:
        case RENDERER_MODULE_STATE_89:
        case RENDERER_MODULE_STATE_90:
        case RENDERER_MODULE_STATE_91:
        case RENDERER_MODULE_STATE_92:
        case RENDERER_MODULE_STATE_93:
        case RENDERER_MODULE_STATE_94:
        case RENDERER_MODULE_STATE_95:
        case RENDERER_MODULE_STATE_96:
        case RENDERER_MODULE_STATE_97:
        case RENDERER_MODULE_STATE_98:
        case RENDERER_MODULE_STATE_99:
        case RENDERER_MODULE_STATE_100:
        case RENDERER_MODULE_STATE_SELECT_TOGGLE_STATE_ALTERNATIVE:
        {
            if (SelectBasicRendererState(RENDERER_MODULE_STATE_SELECT_WINDOW, value) == RENDERER_MODULE_FAILURE) { return RENDERER_MODULE_FAILURE; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_ALPHA_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_ALPHA_BLEND_NONE: { SelectRendererState(D3DRS_ALPHABLENDENABLE, FALSE); break; }
            case RENDERER_MODULE_ALPHA_BLEND_UNKNOWN_1:
            case RENDERER_MODULE_ALPHA_BLEND_UNKNOWN_3: { break; }
            case RENDERER_MODULE_ALPHA_BLEND_ACTIVE: { SelectRendererState(D3DRS_ALPHABLENDENABLE, TRUE); break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_MIP_FILTER_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_MIP_FILTER_NONE: { SelectRendererTextureStage(stage, D3DTSS_MIPFILTER, D3DTEXF_NONE); break; }
            case RENDERER_MODULE_TEXTURE_MIP_FILTER_POINT: { SelectRendererTextureStage(stage, D3DTSS_MIPFILTER, D3DTEXF_POINT); break; }
            case RENDERER_MODULE_TEXTURE_MIP_FILTER_LINEAR:
            {
                if (!State.Device.Capabilities.IsInterpolationAvailable) { return RENDERER_MODULE_FAILURE; }

                SelectRendererTextureStage(stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_CHROMATIC_COLOR:
        case RENDERER_MODULE_STATE_SELECT_LINE_WIDTH:
        case RENDERER_MODULE_STATE_SELECT_FLAT_FANS_STATE:
        case RENDERER_MODULE_STATE_SELECT_TOGGLE_STATE: { result = RENDERER_MODULE_SUCCESS; break; }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_ADDRESS_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_ADDRESS_CLAMP:
            {
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

                break;
            }
            case RENDERER_MODULE_TEXTURE_ADDRESS_WRAP:
            {
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

                break;
            }
            case RENDERER_MODULE_TEXTURE_ADDRESS_MIRROR:
            {
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR);
                SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_DENSITY:
        {
            const f32 density = *(f32*)&value;

            // NOTE: 0xfffff000 takes the high part of the float.
            const BOOL indivisible = (density == 0.0f || (((u32)value) & 0xfffff000) != 0);

            RendererFogDensity = indivisible ? density : (1.0f / density);

            SelectRendererState(D3DRS_FOGDENSITY, *(DWORD*)&RendererFogDensity);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_COLOR:
        {
            RendererFogColor = ((u32)value) & RENDERER_MODULE_FOG_COLOR_MASK;

            SelectRendererState(D3DRS_FOGCOLOR, RendererFogColor);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_WINDOW_MODE_STATE:
        {
            State.Settings.IsWindowMode = (BOOL)value;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_LAMBDAS:
        {
            if (value == NULL)
            {
                ZeroMemory(&State.Lambdas.Lambdas, sizeof(RendererModuleLambdaContainer));
            }
            else
            {
                const RendererModuleLambdaContainer* lambdas = (RendererModuleLambdaContainer*)value;

                State.Lambdas.Log = lambdas->Log;

                CopyMemory(&State.Lambdas.Lambdas, lambdas, sizeof(RendererModuleLambdaContainer));
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_ALPHAS:
        case RENDERER_MODULE_STATE_SELECT_FOG_ALPHAS_ALTERNATIVE:
        {
            if (!State.Device.Capabilities.IsWFogAvailable) { return RENDERER_MODULE_FAILURE; }

            if (value != NULL) { SelectRendererFogAlphas((u8*)value, RendererFogAlphas); }

            SelectRendererState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);

            RendererFogState = RENDERER_MODULE_FOG_ACTIVE_ALPHAS;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_STATE:
        {
            if (!State.Device.Capabilities.IsWFogAvailable) { RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_FOG_INACTIVE:
            {
                SelectRendererState(D3DRS_FOGENABLE, FALSE);

                State.Settings.IsFogActive = FALSE;

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE:
            {
                SelectRendererState(D3DRS_FOGENABLE, TRUE);

                State.Settings.IsFogActive = TRUE;

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE_LINEAR:
            {
                SelectRendererState(D3DRS_FOGSTART, *(DWORD*)&RendererFogStart);
                SelectRendererState(D3DRS_FOGEND, *(DWORD*)&RendererFogEnd);
                SelectRendererState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
                SelectRendererState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
                SelectRendererState(D3DRS_FOGCOLOR, RendererFogColor);

                RendererFogState = (u32)value;

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE_EXP:
            {
                SelectRendererState(D3DRS_FOGDENSITY, *(DWORD*)&RendererFogDensity);
                SelectRendererState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
                SelectRendererState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
                SelectRendererState(D3DRS_FOGCOLOR, RendererFogColor);
                
                RendererFogState = (u32)value;

                break;
            }
            case RENDERER_MODULE_FOG_ACTIVE_EXP2:
            {
                SelectRendererState(D3DRS_FOGDENSITY, *(DWORD*)&RendererFogDensity);
                SelectRendererState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
                SelectRendererState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
                SelectRendererState(D3DRS_FOGCOLOR, RendererFogColor);
                
                RendererFogState = (u32)value;

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_START:
        {
            RendererFogStart = *(f32*)&value;

            SelectRendererState(D3DRS_FOGSTART, *(DWORD*)&RendererFogStart);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_FOG_END:
        {
            RendererFogEnd = *(f32*)&value;

            SelectRendererState(D3DRS_FOGEND, *(DWORD*)&RendererFogEnd);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BIAS_STATE:
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BIAS_STATE_ALTERNATIVE:
        {
            SelectRendererState(D3DRS_ZBIAS, Clamp<LONG>((LONG)value, 0, 16));

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_WINDOW:
        {
            State.Window.HWND = (HWND)value;

            SelectBasicRendererState(RENDERER_MODULE_STATE_SELECT_WINDOW, value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_RESTORE_RENDERER_SURFACES:
        {
            if (!RestoreRendererSurfaces()) { return RENDERER_MODULE_FAILURE; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_ALPHA_TEST_STATE:
        {
            if ((u32)value == RENDERER_MODULE_ALPHA_TEST_0)
            {
                SelectRendererState(D3DRS_ALPHATESTENABLE, FALSE);
                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
            }
            else
            {
                SelectRendererState(D3DRS_ALPHATESTENABLE, TRUE);
                SelectRendererState(D3DRS_ALPHAFUNC, RendererAlphaFunction);
                SelectRendererState(D3DRS_ALPHAREF, (DWORD)value);
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELCT_DEPTH_FUNCTION:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_DEPTH_FUNCTION_NEVER:
            {
                RendererDepthFunction = D3DCMP_NEVER;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_NEVER);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_LESS:
            {
                RendererDepthFunction = D3DCMP_LESS;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_LESS);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_EQUAL:
            {
                RendererDepthFunction = D3DCMP_EQUAL;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_EQUAL);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_LESS_EQUAL:
            {
                RendererDepthFunction = D3DCMP_LESSEQUAL;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_GREATER:
            {
                RendererDepthFunction = D3DCMP_GREATER;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_GREATER);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_NOT_EQUAL:
            {
                RendererDepthFunction = D3DCMP_NOTEQUAL;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_NOTEQUAL);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_GREATER_EQUAL:
            {
                RendererDepthFunction = D3DCMP_GREATEREQUAL;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);

                break;
            }
            case RENDERER_MODULE_DEPTH_FUNCTION_ALWAYS:
            {
                RendererDepthFunction = D3DCMP_ALWAYS;

                SelectRendererState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

                break;
            }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_STAGE_BLEND_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_NORMAL:
            {
                if (stage == RENDERER_TEXTURE_STAGE_0)
                {
                    if (!State.Device.Capabilities.IsModulateBlending)
                    {
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    }
                    else
                    {
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                    }
                }
                else if (State.Device.Capabilities.MaximumSimultaneousTextures < 2)
                {
                    SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
                }
                else
                {
                    SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
                }

                result = RENDERER_MODULE_SUCCESS; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_ADD);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_ADD);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);

                result = State.Textures.Stages[stage].Unk01; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_DISABLE:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

                result = RENDERER_MODULE_SUCCESS; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);

                result = State.Textures.Stages[stage].Unk03;

                break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_SUBTRACT:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_SUBTRACT);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

                result = State.Textures.Stages[stage].Unk02; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_ARG2:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_COMPLEMENT | D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

                result = State.Textures.Stages[stage].Unk07; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE_2X:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);

                result = State.Textures.Stages[stage].Unk04; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_MODULATE_4X:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTSS_ALPHAARG2);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);

                result = State.Textures.Stages[stage].Unk05; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_TEXTURE_ALPHA:
            {
                if (stage == RENDERER_TEXTURE_STAGE_0)
                {
                    if (!State.Device.Capabilities.IsModulateBlending)
                    {
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    }
                    else
                    {
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                        SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                    }

                    result = State.Textures.Stages[RENDERER_TEXTURE_STAGE_0].Unk09;
                }
                else
                {
                    SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

                    result = State.Textures.Stages[stage].Unk09;
                }

                break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD_SIGNED:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);

                result = State.Textures.Stages[stage].Unk08; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_DOT_PRODUCT_3:
            {
                if (stage == RENDERER_TEXTURE_STAGE_0)
                {
                    SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
                    SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                    SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                }
                else
                {
                    SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
                    SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                    SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                }

                result = State.Textures.Stages[stage].Unk10; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_BUMP_ENV_MAP:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

                result = State.Textures.Stages[stage].Unk11; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_BUMP_ENV_MAP_LUMINANCE:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, (stage != RENDERER_TEXTURE_STAGE_0) ? D3DTA_CURRENT : D3DTA_DIFFUSE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

                result = State.Textures.Stages[stage].Unk12; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_ADD_BLEND_FACTOR_ALPHA:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_ADD);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

                result = RENDERER_MODULE_SUCCESS; break;
            }
            case RENDERER_MODULE_TEXTURE_STAGE_BLEND_BLEND_FACTOR_ALPHA_ARG1:
            {
                SelectRendererTextureStage(stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                SelectRendererTextureStage(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

                result = RENDERER_MODULE_SUCCESS; break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            if (result == RENDERER_MODULE_FAILURE) { return RENDERER_MODULE_FAILURE; }

            break;
        }
        case RENDERER_MODULE_STATE_SELECT_VERTEX_TYPE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_VERTEX_TYPE_RTLVX:
            {
                RendererVertexSize = sizeof(RTLVX);
                RendererCurrentShader = D3DFVF_TEX1 | D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW;

                State.DX.Device->SetVertexShader(RendererCurrentShader);

                SelectState(RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE, (void*)0);
                SelectState(MAKETEXTURESTAGEMASK(RENDERER_TEXTURE_STAGE_1) | RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE, (void*)0);

                break;
            }
            case RENDERER_MODULE_VERTEX_TYPE_RTLVX2:
            {
                RendererVertexSize = sizeof(RTLVX2);
                RendererCurrentShader = D3DFVF_TEX2 | D3DFVF_SPECULAR | D3DFVF_DIFFUSE | D3DFVF_XYZRHW;

                State.DX.Device->SetVertexShader(RendererCurrentShader);

                SelectState(RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE, (void*)0);
                SelectState(MAKETEXTURESTAGEMASK(RENDERER_TEXTURE_STAGE_1) | RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE, (void*)1);

                break;
            }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_GAMMA_CONTROL_STATE:
        case RENDERER_MODULE_STATE_SELECT_GAMMA_CONTROL_STATE_ALTERNATIVE:
        {
            if (!State.Device.Capabilities.IsGammaAvailable) { return RENDERER_MODULE_FAILURE; }
            if (State.DX.Device == NULL) { return RENDERER_MODULE_FAILURE; }
            if (State.Settings.IsWindowMode) { return RENDERER_MODULE_FAILURE; }

            const f32 modifier = Clamp(*(f32*)&value, 0.0f, 4.0f);
            const f32 multiplier = modifier < 4.0f ? (modifier * 257.0f) : (modifier * 1023.0f);

            D3DGAMMARAMP gamma;

            for (u32 x = 0; x < MAX_TEXTURE_PALETTE_COLOR_COUNT; x++)
            {
                gamma.red[x] = (u16)Clamp<u32>((u32)(State.DX.State.Gamma.red[x] * multiplier + 0.5f), U16_MIN, U16_MAX);
                gamma.green[x] = (u16)Clamp<u32>((u32)(State.DX.State.Gamma.green[x] * multiplier + 0.5f), U16_MIN, U16_MAX);
                gamma.blue[x] = (u16)Clamp<u32>((u32)(State.DX.State.Gamma.blue[x] * multiplier + 0.5f), U16_MIN, U16_MAX);
            }

            State.DX.Device->SetGammaRamp(0, &gamma);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_STENCIL_STATE:
        {
            if (!State.Device.Capabilities.IsStencilBufferAvailable) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_INACTIVE: { SelectRendererState(D3DRS_STENCILENABLE, FALSE); break; }
            case RENDERER_MODULE_STENCIL_ACTIVE: { SelectRendererState(D3DRS_STENCILENABLE, TRUE); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_STENCIL_FUNCTION:
        {
            if (!State.Device.Capabilities.IsStencilBufferAvailable) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_FUNCTION_NEVER: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_NEVER); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_LESS: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_LESS); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_EQUAL: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_EQUAL); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_LESS_EQUAL: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_GREATER: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_GREATER); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_NOT_EQUAL: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_GREATER_EQUAL: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_GREATEREQUAL); break; }
            case RENDERER_MODULE_STENCIL_FUNCTION_ALWAYS: { SelectRendererState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_STENCIL_FAIL_STATE:
        {
            if (!State.Device.Capabilities.IsStencilBufferAvailable) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_FAIL_KEEP: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP); break; }
            case RENDERER_MODULE_STENCIL_FAIL_ZERO: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO); break; }
            case RENDERER_MODULE_STENCIL_FAIL_REPLACE: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE); break; }
            case RENDERER_MODULE_STENCIL_FAIL_INCREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT); break; }
            case RENDERER_MODULE_STENCIL_FAIL_DECREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_DECRSAT); break; }
            case RENDERER_MODULE_STENCIL_FAIL_INVERT: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_INVERT); break; }
            case RENDERER_MODULE_STENCIL_FAIL_INCREMENT: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR); break; }
            case RENDERER_MODULE_STENCIL_FAIL_DECREMENT: { SelectRendererState(D3DRS_STENCILFAIL, D3DSTENCILOP_DECR); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_STENCIL_DEPTH_FAIL_STATE:
        {
            if (!State.Device.Capabilities.IsStencilBufferAvailable) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_KEEP: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_ZERO: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_REPLACE: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_INCREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_DECREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_INVERT: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INVERT); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_INCREMENT: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR); break; }
            case RENDERER_MODULE_STENCIL_DEPTH_FAIL_DECREMENT: { SelectRendererState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_STENCIL_PASS_STATE:
        {
            if (!State.Device.Capabilities.IsStencilBufferAvailable) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_PASS_KEEP: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP); break; }
            case RENDERER_MODULE_STENCIL_PASS_ZERO: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO); break; }
            case RENDERER_MODULE_STENCIL_PASS_REPLACE: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE); break; }
            case RENDERER_MODULE_STENCIL_PASS_INCREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT); break; }
            case RENDERER_MODULE_STENCIL_PASS_DECREMENT_CLAMP: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_DECRSAT); break; }
            case RENDERER_MODULE_STENCIL_PASS_INVERT: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_INVERT); break; }
            case RENDERER_MODULE_STENCIL_PASS_INCREMENT: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR); break; }
            case RENDERER_MODULE_STENCIL_PASS_DECREMENT: { SelectRendererState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_BLEND_STATE:
        case RENDERER_MODULE_STATE_SELECT_BLEND_STATE_ALTERNATIVE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_ALPHA_BLEND_SOURCE_INVERSE_SOURCE:
            {
                SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

                break;
            }
            case RENDERER_MODULE_ALPHA_BLEND_SOURCE_ONE:
            {
                SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_ONE);

                break;
            }
            case RENDERER_MODULE_ALPHA_BLEND_ZERO_INVERSE_SOURCE:
            {
                SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

                break;
            }
            case RENDERER_MODULE_ALPHA_BLEND_DESTINATION_COLOR_ZERO:
            {
                SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
                SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BUFFER_WRITE_STATE:
        case RENDERER_MODULE_STATE_SELECT_DEPTH_BUFFER_WRITE_STATE_ALTERNATIVE:
        {
            SelectRendererState(D3DRS_ZWRITEENABLE, (u32)value == 0 ? FALSE : TRUE);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_ACQUIRE_GUARD_BANDS:
        {
            if (value != NULL)
            {
                const s32 left = (s32)State.Device.Capabilities.GuardBandLeft;
                const s32 right = (s32)State.Device.Capabilities.GuardBandRight;
                const s32 top = (s32)State.Device.Capabilities.GuardBandTop;
                const s32 bottom = (s32)State.Device.Capabilities.GuardBandBottom;

                if (left != 0 && right != 0 && top != 0 && bottom != 0)
                {
                    RendererModuleGuardBands* output = (RendererModuleGuardBands*)value;

                    output->Left = left;
                    output->Right = right;
                    output->Top = top;
                    output->Bottom = bottom;

                    return RENDERER_MODULE_SUCCESS;
                }
            }

            return RENDERER_MODULE_FAILURE;
        }
        case RENDERER_MODULE_STATE_SELCT_ALPHA_FUNCTION:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_ALPHA_FUNCTION_NEVER:
            {
                RendererAlphaFunction = D3DCMP_NEVER;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_NEVER);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_LESS:
            {
                RendererAlphaFunction = D3DCMP_LESS;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_LESS);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_EQUAL:
            {
                RendererAlphaFunction = D3DCMP_EQUAL;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_EQUAL);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_LESS_EQUAL:
            {
                RendererAlphaFunction = D3DCMP_LESSEQUAL;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_GREATER:
            {
                RendererAlphaFunction = D3DCMP_GREATER;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_NOT_EQUAL:
            {
                RendererAlphaFunction = D3DCMP_NOTEQUAL;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_GREATER_EQUAL:
            {
                RendererAlphaFunction = D3DCMP_GREATEREQUAL;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

                break;
            }
            case RENDERER_MODULE_ALPHA_FUNCTION_ALWAYS:
            {
                RendererAlphaFunction = D3DCMP_ALWAYS;

                SelectRendererState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);

                break;
            }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_ADDRESS_STATE_U:
        {
            if (!State.Device.Capabilities.IsTextureIndependentUVs) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_ADDRESS_CLAMP: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP); break; }
            case RENDERER_MODULE_TEXTURE_ADDRESS_WRAP: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP); break; }
            case RENDERER_MODULE_TEXTURE_ADDRESS_MIRROR: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR); break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_ADDRESS_STATE_V:
        {
            if (!State.Device.Capabilities.IsTextureIndependentUVs) { return RENDERER_MODULE_FAILURE; }

            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_ADDRESS_CLAMP: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP); break; }
            case RENDERER_MODULE_TEXTURE_ADDRESS_WRAP: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP); break; }
            case RENDERER_MODULE_TEXTURE_ADDRESS_MIRROR: { SelectRendererTextureStage(stage, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR); break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_CLEAR_DEPTH_STATE:
        {
            RendererClearDepth = *(f32*)&value;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE:
        {
            SelectRendererTextureStage(stage, D3DTSS_TEXCOORDINDEX, (DWORD)value);

            return RENDERER_MODULE_FAILURE;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_MIN_FILTER_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_FILTER_POINT: { SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_POINT); break; }
            case RENDERER_MODULE_TEXTURE_FILTER_LENEAR: { SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR); break; }
            case RENDERER_MODULE_TEXTURE_FILTER_ANISOTROPY:
            {
                if (!State.Device.Capabilities.IsAnisotropyAvailable) { return RENDERER_MODULE_FAILURE; }

                SelectRendererTextureStage(stage, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_MAG_FILTER_STATE:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_TEXTURE_FILTER_POINT: { SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT); break; }
            case RENDERER_MODULE_TEXTURE_FILTER_LENEAR: { SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR); break; }
            case RENDERER_MODULE_TEXTURE_FILTER_ANISOTROPY:
            {
                if (!State.Device.Capabilities.IsAnisotropyAvailable) { return RENDERER_MODULE_FAILURE; }

                SelectRendererTextureStage(stage, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);

                break;
            }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_SOURCE_BLEND_STATE_DX8:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_BLEND_ONE: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_ONE); break; }
            case RENDERER_MODULE_BLEND_ZERO: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_ZERO); break; }
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_ALPHA: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_ALPHA: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_ALPHA: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA); break; }
            case RENDERER_MODULE_BLEND_SOURCE_COLOR: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_COLOR: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_COLOR: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_INVSRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_COLOR: { SelectRendererState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DESTINATION_BLEND_STATE_DX8:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_BLEND_ONE: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_ONE); break; }
            case RENDERER_MODULE_BLEND_ZERO: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_ZERO); break; }
            case RENDERER_MODULE_BLEND_SOURCE_ALPHA: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_ALPHA: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_ALPHA: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_ALPHA: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA); break; }
            case RENDERER_MODULE_BLEND_SOURCE_COLOR: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_DESTINATION_COLOR: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_SOURCE_COLOR: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR); break; }
            case RENDERER_MODULE_BLEND_INVERSE_DESTINATION_COLOR: { SelectRendererState(D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR); break; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_FACTOR_DX8:
        {
            SelectRendererState(D3DRS_TEXTUREFACTOR, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_DESTINATION_BLEND_STATE:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            const RendererModuleTextureStageBumpMappingMatrix* matrix = (RendererModuleTextureStageBumpMappingMatrix*)value;

            SelectRendererTextureStage(stage, D3DTSS_BUMPENVMAT00, *(DWORD*)&matrix->M00);
            SelectRendererTextureStage(stage, D3DTSS_BUMPENVMAT01, *(DWORD*)&matrix->M01);
            SelectRendererTextureStage(stage, D3DTSS_BUMPENVMAT10, *(DWORD*)&matrix->M10);
            SelectRendererTextureStage(stage, D3DTSS_BUMPENVMAT11, *(DWORD*)&matrix->M11);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_BUMP_MAPPING_LUMINANCE_SCALE_DX8:
        {
            SelectRendererTextureStage(stage, D3DTSS_BUMPENVLSCALE, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_BUMP_MAPPING_LUMINANCE_OFFSET_DX8:
        {
            SelectRendererTextureStage(stage, D3DTSS_BUMPENVLOFFSET, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_RENDER_PACKET_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            const RendererModulePacket* packet = (RendererModulePacket*)value;

            if (packet->FVF == 0 || packet->Vertexes == NULL || packet->VertexCount == 0) { return RENDERER_MODULE_FAILURE; }

            u32 count = 0;

            switch (packet->Type)
            {
            case RendererModulePrimitiveTypePointList: { count = packet->VertexCount; break; }
            case RendererModulePrimitiveTypeLineList: { count = packet->VertexCount / 2; break; }
            case RendererModulePrimitiveTypeLineStrip: { count = packet->VertexCount - 1; break; }
            case RendererModulePrimitiveTypeTriangleList: { count = packet->VertexCount / 3; break; }
            case RendererModulePrimitiveTypeTriangleStrip:
            case RendererModulePrimitiveTypeTriangleFan: { count = packet->VertexCount - 2; break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            BeginRendererScene();

            State.DX.Device->SetVertexShader(packet->FVF);

            if (packet->Indexes == NULL || packet->IndexCount == 0)
            {
                result = State.DX.Device->DrawPrimitiveUP((D3DPRIMITIVETYPE)packet->Type,
                    count, packet->Vertexes, VertexStreamStride) == D3D_OK;
            }
            else
            {
                result = State.DX.Device->DrawIndexedPrimitiveUP((D3DPRIMITIVETYPE)packet->Type,
                    0, packet->IndexCount + 1, count, packet->Indexes, D3DFMT_INDEX16,
                    packet->Vertexes, VertexStreamStride) == D3D_OK;
            }

            State.DX.Device->SetVertexShader(RendererCurrentShader);

            if (result == RENDERER_MODULE_FAILURE) { return RENDERER_MODULE_FAILURE; }

            break;
        }
        case RENDERER_MODULE_STATE_SELECT_TEXTURE_COORDINATE_INDEX_STATE_DX8:
        {
            if (RendererCurrentShader & D3DFVF_TEX1)
            {
                SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_TEXCOORDINDEX, 0);
                SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_1, D3DTSS_TEXCOORDINDEX, 0);
            }
            else if (RendererCurrentShader & D3DFVF_TEX2)
            {
                SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_0, D3DTSS_TEXCOORDINDEX, 0);
                SelectRendererTextureStage(RENDERER_TEXTURE_STAGE_1, D3DTSS_TEXCOORDINDEX, 1);
            }

            return RENDERER_MODULE_FAILURE;
        }
        case RENDERER_MODULE_STATE_ACQUIRE_MAX_ANISOTROPY_DX8:
        {
            if (!State.Device.Capabilities.IsAnisotropyAvailable) { return RENDERER_MODULE_FAILURE; }

            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            *(u32*)value = State.Device.Capabilities.MaxAnisotropy;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_MAX_ANISOTROPY_DX8:
        {
            if (!State.Device.Capabilities.IsAnisotropyAvailable) { return RENDERER_MODULE_FAILURE; }

            if (State.Device.Capabilities.MaxAnisotropy < (u32)value)
            {
                State.Device.Capabilities.MaxAnisotropy = (u32)value;
            }

            SelectRendererTextureStage(stage, D3DTSS_MAXANISOTROPY, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_VERTEX_STREAM_STRIDE_DX8:
        {
            if ((u32)value < 1) { return RENDERER_MODULE_FAILURE; }

            VertexStreamStride = (u32)value;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case 0x19d:
        case 0x19e:
        case 0x19f:
        case 0x1a0:
        case 0x1a1:
        case 0x1a2:
        case 0x1a3:
        case 0x1a7:
        case 0x1a8:
        case 0x1a9:
        case 0x1aa:
        case 0x1ab:
        case 0x1ac:
        case 0x1ad:
        case 0x1b6:
        case 0x1b7:
        case 0x1ba:
        case 0x1bb:
        case 0x1bc:
        case 0x1bd:
        case 0x1be:
        case 0x1bf:
        case 0x1c0:
        case 0x1c1:
        case 0x1c2:
        case 0x1c3:
        case 0x1c4:
        case 0x1c5:
        case 0x1c6:
        case 0x1c7:
        case 0x1c8:
        case 0x1c9:
        case 0x1ca:
        case 0x1cb:
        case 0x1cc:
        case 0x1cd:
        case 0x1ce:
        case 0x1cf:
        case 0x1d0:
        case 0x1d1:
        case 0x1d2:
        case 0x1d3:
        case 0x1d4:
        case 0x1d5:
        case 0x1d6:
        case 0x1d7:
        case 0x1d8:
        case 0x1d9:
        case 0x1da:
        case 0x1db:
        case 0x1dc:
        case 0x1dd:
        case 0x1de:
        case 0x1df:
        case 0x1e0:
        case 0x1e1:
        case 0x1e2:
        case 0x1e3:
        case 0x1e4:
        case 0x1e5:
        case 0x1e6:
        case 0x1e7:
        case 0x1e8:
        case 0x1e9:
        case 0x1ea:
        case 0x1eb:
        case 0x1ec:
        case 0x1ed:
        case 0x1ee:
        case 0x1ef:
        case 0x1f0:
        case 0x1f1:
        case 0x1f2:
        {
            if (SelectBasicRendererState(actual, value) == RENDERER_MODULE_FAILURE) { return RENDERER_MODULE_FAILURE; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TOGGLE_RENDERER_LAMBDA_DX8:
        {
            if (value != NULL) { State.Lambdas.ToggleRenderer = (RENDERERMODULETOGGLERENDERERLAMBDA)value; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TOGGLE_RENDERER_WAIT_LAMBDA_DX8:
        {
            if (value != NULL) { State.Lambdas.ToggleRendererWait = (RENDERERMODULETOGGLERENDERERWAITLAMBDA)value; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_TOGGLE_RENDERER_WAIT_LAMBDA_VALUE_DX8:
        {
            if (value != NULL) { State.Lambdas.ToggleRendererWaitContext = value; }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_430_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 0; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_431_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 1; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_432_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 2; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_433_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 3; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_434_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 4; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_435_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 5; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_436_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 6; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_437_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            ((u32*)value)[6] = 7; // TODO

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_ACQUIRE_RENDERER_DEVICE_DX8:
        {
            if (value == NULL) { return RENDERER_MODULE_FAILURE; }

            *(IDirect3DDevice8**)value = State.DX.Device;

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_WINDOW_STENCIL_STATE_DX8:
        {
            switch ((u32)value)
            {
            case RENDERER_MODULE_STENCIL_ACTIVE: { State.Window.IsStencil = TRUE; break; }
            case RENDERER_MODULE_STENCIL_UNKNOWN: { State.Window.IsStencil = FALSE; break; }
            default: { return RENDERER_MODULE_FAILURE; }
            }

            result = RENDERER_MODULE_SUCCESS; break;
        }
        case RENDERER_MODULE_STATE_SELECT_CLIPPING_STATE_DX8:
        {
            RenderPackets();

            State.DX.Device->SetRenderState(D3DRS_CLIPPING, (DWORD)value);

            result = RENDERER_MODULE_SUCCESS; break;
        }
        default:
        {
            if (value == NULL) { return NULL; }

            CopyMemory(value, &State.Device.Capabilities, sizeof(RendererModuleDeviceCapabilities8));

            return (addr)value;
        }
        }

        SelectRendererStateValue(state, value);

        return result;
    }

    // 0x60001a50
    // a.k.a. THRASH_settexture
    DLLAPI u32 STDCALLAPI SelectTexture(RendererTexture* tex)
    {
        RendererTexture* current = (RendererTexture*)AcquireState(RENDERER_MODULE_STATE_SELECT_TEXTURE);

        BeginRendererScene();
        RenderPackets();

        u32 palette = DEFAULT_TEXTURE_PALETTE_VALUE;

        if (tex == NULL)
        {
            for (u32 x = 0; x < State.Device.Capabilities.MaximumSimultaneousTextures; x++)
            {
                State.DX.Device->SetTexture(x, NULL);
            }
        }
        else
        {
            if ((u32)tex < 16) // TODO
            {
                State.DX.Device->SetTexture((u32)tex - 1, NULL);

                SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_TEXTURE, tex);

                return RENDERER_MODULE_SUCCESS;
            }

            if (current == tex)
            {
                SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_TEXTURE, tex);

                return RENDERER_MODULE_SUCCESS;
            }

            State.DX.Device->SetTexture(tex->Stage, tex->Texture);

            palette = tex->Palette;
        }

        if (palette != INVALID_TEXTURE_PALETTE_VALUE)
        {
            State.DX.Device->SetCurrentTexturePalette(palette);
        }

        SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_TEXTURE, tex);

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600038b0
    // a.k.a. THRASH_setvideomode
    DLLAPI u32 STDCALLAPI SelectVideoMode(const u32 mode, const u32 pending, const u32 depth)
    {
        s32 actual = (s32)mode;

        if (actual < 0 || ModuleDescriptor.Capabilities.Count <= actual
            || !ModuleDescriptor.Capabilities.Capabilities[actual].IsActive) { actual = 1; } // TODO
        
        if (depth == 1) { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_16; } // TODO
        else if (depth == 2) { State.DX.Surfaces.Bits = GRAPHICS_BITS_PER_PIXEL_32; } // TODO
        else { State.DX.Surfaces.Bits = 0; }

        u32 result = RENDERER_MODULE_FAILURE;

        if (State.Lambdas.Lambdas.Execute != NULL)
        {
            if (GetWindowThreadProcessId(State.Window.Parent.HWND, NULL) != GetCurrentThreadId())
            {
                State.Window.HWND = NULL;
                State.Window.HWND = State.Lambdas.Lambdas.AcquireWindow();

                State.Mutexes.Device = CreateEventA(NULL, FALSE, FALSE, NULL);

                SetForegroundWindow(State.Window.HWND);
                PostMessageA(State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, actual, pending);
                
                result = WaitForSingleObject(State.Mutexes.Device, 10000) == WAIT_OBJECT_0;
            }
        }
        else
        {
            InitializeRendererDeviceExecute(0, State.Window.HWND, RENDERER_MODULE_WINDOW_MESSAGE_INITIALIZE_DEVICE, actual, pending, NULL);
        }

        BeginRendererScene();

        InitializeRendererModuleState(actual, pending, depth, ENVIRONMENT_SECTION_NAME);
        SelectBasicRendererState(RENDERER_MODULE_STATE_62, (void*)(DAT_6001eee0 + 1));

        InitializeRendererState();

        SelectGameWindow(2); // TODO

        UnlockGameWindow(NULL);

        SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_WINDOW_LOCK_STATE, LockGameWindow());

        return result;
    }

    // 0x60001380
    // a.k.a. THRASH_sync
    DLLAPI u32 STDCALLAPI SyncGameWindow(const u32 type)
    {
        switch (type)
        {
        case RENDERER_MODULE_SYNC_NORMAL:
        {
            LockGameWindow();
            UnlockGameWindow(NULL);
            break;
        }
        case RENDERER_MODULE_SYNC_VBLANK:
        {
            if (State.DX.Device != NULL)
            {
                D3DRASTER_STATUS status;
                ZeroMemory(&status, sizeof(D3DRASTER_STATUS));

                while (State.DX.Device->GetRasterStatus(&status) == D3D_OK)
                {
                    if (status.InVBlank) { return RENDERER_MODULE_FAILURE; }
                }
            }

            break;
        }
        }

        return RENDERER_MODULE_FAILURE;
    }

    // 0x600077c0
    // a.k.a. THRASH_talloc
    DLLAPI RendererTexture* STDCALLAPI AllocateTexture(const u32 width, const u32 height, const u32 format, const BOOL palette, const u32 state)
    {
        if (width == 0 || height == 0
            || State.Textures.Formats[format] == D3DFMT_UNKNOWN || State.Textures.Illegal) {
            return NULL;
        }

        RendererTexture* tex = tex = AllocateRendererTexture();

        if (tex == NULL) { return NULL; }

        tex->Width = width;
        tex->Height = height;

        tex->PixelFormat = MAKEPIXELFORMAT(format);
        tex->TextureFormat = State.Textures.Formats[format];
        tex->PixelSize = PixelFormatSizes[format];

        tex->Stage = MAKETEXTURESTAGEVALUE(state);
        tex->MipMapCount = MAKETEXTUREMIPMAPVALUE(state) != 0 ? (MAKETEXTUREMIPMAPVALUE(state) + 1) : 0;

        tex->Texture = NULL;

        tex->Is16Bit = tex->PixelFormat == RENDERER_PIXEL_FORMAT_R5G5B5 || tex->PixelFormat == RENDERER_PIXEL_FORMAT_R4G4B4;
        tex->PaletteMode = palette;
        tex->MemoryType = RENDERER_MODULE_TEXTURE_LOCATION_SYSTEM_MEMORY;
        tex->Palette = INVALID_TEXTURE_PALETTE_VALUE;
        tex->Colors = 0;

        {
            const HRESULT result = InitializeRendererTexture(tex);

            if (result != D3D_OK)
            {
                ReleaseTexture(tex);

                if (result == D3DERR_OUTOFVIDEOMEMORY) { State.Textures.Illegal = TRUE; }

                return NULL;
            }
        }

        tex->Previous = State.Textures.Current;
        State.Textures.Current = tex;

        if (palette == RENDERER_MODULE_PALETTE_ACQUIRE) { tex->Palette = AcquireTexturePalette(); }

        return tex;
    }

    // 0x60007950
    // a.k.a. THRASH_tfree
    DLLAPI u32 STDCALLAPI ReleaseTexture(RendererTexture* tex)
    {
        if (State.Textures.Current == NULL) { return RENDERER_MODULE_FAILURE; }

        RendererTexture* prev = NULL;
        RendererTexture* current = State.Textures.Current;
        RendererTexture* next = NULL;

        do
        {
            if (current == tex)
            {
                if (current == State.Textures.Current) { State.Textures.Current = current->Previous; }
                else if (next != NULL)
                {
                    if (current->Previous == NULL) { next->Previous = NULL; }
                    else { next->Previous = current->Previous; }
                }

                if (current->Texture != NULL)
                {
                    while (current->Texture->Release() != D3D_OK) {}

                    current->Texture = NULL;
                }

                if (current->Palette != INVALID_TEXTURE_PALETTE_VALUE)
                {
                    ReleaseTexturePalette(current->Palette);

                    current->Palette = INVALID_TEXTURE_PALETTE_VALUE;
                }

                DisposeRendererTexture(current);

                return RENDERER_MODULE_SUCCESS;
            }

            prev = current->Previous;
            next = current;
            current = prev;
        } while (prev != NULL);

        return RENDERER_MODULE_FAILURE;
    }

    // 0x60007a80
    // a.k.a. THRASH_treset
    DLLAPI u32 STDCALLAPI ResetTextures(void)
    {
        if (State.DX.Device == NULL) { return RENDERER_MODULE_FAILURE; }

        SelectTexture(NULL);

        if (State.Scene.IsActive) { AttemptRenderPackets(); }

        while (State.Textures.Current != NULL)
        {
            RendererTexture* tex = State.Textures.Current;

            State.Textures.Current = State.Textures.Current->Previous;

            ReleaseTexture(tex);
        }

        State.Textures.Current = NULL;

        State.Textures.Illegal = FALSE;

        BeginRendererScene();

        InitializeRenderState55();

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600079e0
    // a.k.a. THRASH_tupdate
    DLLAPI RendererTexture* STDCALLAPI UpdateTexture(RendererTexture* tex, const u32* pixels, const u32* palette)
    {
        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        if (tex != NULL && pixels != NULL) { return UpdateRendererTexture(tex, pixels, palette) ? tex : NULL; }

        return NULL;
    }

    // 0x60007a20
    // a.k.a. THRASH_tupdaterect
    DLLAPI RendererTexture* STDCALLAPI UpdateTextureRectangle(RendererTexture* tex, const u32* pixels, const u32* palette, const s32 x, const s32 y, const s32 width, const s32 height, const s32 size, const s32 level)
    {
        if (State.Lock.IsActive) { UnlockGameWindow(NULL); }

        if (tex != NULL && pixels != NULL)
        {
            return UpdateRendererTextureRectangle(tex, pixels, palette, x, y, width, height, size, level) ? tex : NULL;
        }

        return NULL;
    }
    
    // 0x600015c0
    // a.k.a. THRASH_unlockwindow
    DLLAPI u32 STDCALLAPI UnlockGameWindow(const RendererModuleWindowLock* state)
    {
        if (State.Lock.IsActive && State.Lock.Surface != NULL)
        {
            if (State.Lock.Surface->UnlockRect() != D3D_OK) { return RENDERER_MODULE_FAILURE; }

            ZeroMemory(&State.Lock.State, sizeof(RendererModuleWindowLock));

            State.Lock.Surface = NULL;

            State.Lock.IsActive = FALSE;

            if (State.Lambdas.Lambdas.LockWindow != NULL) { State.Lambdas.Lambdas.LockWindow(FALSE); }

            BeginRendererScene();
        }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x60001160
    // a.k.a. THRASH_window
    DLLAPI u32 STDCALLAPI SelectGameWindow(const u32 indx)
    {
        if (State.DX.Device == NULL) { return RENDERER_MODULE_FAILURE; }

        if (State.Window.Index == indx) { return RENDERER_MODULE_SUCCESS; }

        SelectRendererStateValue(RENDERER_MODULE_STATE_SELECT_GAME_WINDOW_INDEX, (void*)indx);

        State.Window.IsWindow = FALSE;

        if (indx == 3 || indx == 5) // TODO
        {
            State.Window.IsWindow = TRUE;

            return RENDERER_MODULE_SUCCESS;
        }

        if (indx == 1) { return RENDERER_MODULE_FAILURE; } // TODO

        IDirect3DSurface8* surface = NULL;
        IDirect3DSurface8* stencil = NULL;

        if (indx < 1) // TODO
        {
            if (indx < MIN_WINDOW_INDEX) // TODO
            {
                State.Window.IsWindow = FALSE;

                return RENDERER_MODULE_FAILURE;
            }

            if (State.Window.Count + MIN_WINDOW_INDEX <= indx) // TODO
            {
                State.Window.IsWindow = FALSE;

                return RENDERER_MODULE_FAILURE;
            }

            State.Windows[indx].Texture->GetSurfaceLevel(0, &surface);

            stencil = State.Windows[indx].Stencil;
        }
        else if (indx < MIN_WINDOW_INDEX) // TODO
        {
            surface = State.DX.Surfaces.Surfaces[indx];
            stencil = State.DX.Surfaces.Surfaces[3]; // TODO
        }

        if (surface == NULL) { return RENDERER_MODULE_FAILURE; }

        if (State.Window.IsStencil) { stencil = State.Window.Stencil; }

        ModifyRendererSurface(State.Window.Surface, 2);

        if (State.Window.Stencil != NULL) { ModifyRendererSurface(State.Window.Stencil, 2); }

        if (State.DX.Device->SetRenderTarget(surface, stencil) != D3D_OK) { return RENDERER_MODULE_STENCIL_FAIL_REPLACE; }

        State.Window.Index = indx;

        State.Window.Surface = surface;
        State.Window.Stencil = stencil;

        ModifyRendererSurface(State.Window.Surface, 1);

        if (State.Window.Stencil != NULL) { ModifyRendererSurface(State.Window.Stencil, 1); }

        return RENDERER_MODULE_SUCCESS;
    }

    // 0x600030a0
    // a.k.a. THRASH_writerect
    DLLAPI u32 STDCALLAPI WriteRectangle(const u32 x, const u32 y, const u32 width, const u32 height, const u32* pixels)
    {
        return WriteRectangles(x, y, width, height, pixels, 0);
    }

    // 0x60004fd0
    // a.k.a. THRASH_writerect
    // NOTE: Never being called by the application.
    DLLAPI u32 STDCALLAPI WriteRectangles(const u32 x, const u32 y, const u32 width, const u32 height, const u32* pixels, const u32 stride)
    {
        RendererModuleWindowLock* state = LockGameWindow();

        if (state == NULL) { return RENDERER_MODULE_FAILURE; }

        const u32 multiplier = state->Format == RENDERER_PIXEL_FORMAT_A8R8G8B8
            ? 4 : (state->Format == RENDERER_PIXEL_FORMAT_R8G8B8 ? 3 : 2);

        const u32 length = multiplier * width;

        for (u32 xx = 0; xx < height; xx++)
        {
            const addr offset = (xx * state->Stride) + (state->Stride * y) + (multiplier * x);

            CopyMemory((void*)((addr)state->Data + (addr)offset), &pixels[xx * length], length);
        }

        return UnlockGameWindow(NULL);
    }
}