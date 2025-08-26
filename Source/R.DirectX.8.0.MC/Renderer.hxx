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

#pragma once

#include "DirectX.hxx"
#include "RendererModule.Export.hxx"

#define CLEAR_DEPTH_VALUE (1.0f)
#define DEFAULT_DEVICE_INDEX 0
#define DEFAULT_FOG_COLOR 0x00FFFFFF
#define DEFAULT_FOG_END (1.0f)
#define DEFAULT_TEXTURE_PALETTE_VALUE 0
#define ENVIRONMENT_SECTION_NAME "DX8"
#define INVALID_TEXTURE_PALETTE_VALUE 0xFFFF
#define MAX_ACTIVE_SURFACE_COUNT 8
#define MAX_ACTIVE_UNKNOWN_COUNT 10
#define MAX_DEVICE_CAPABILITIES_COUNT 256
#define MAX_ENUMERATE_DEVICE_NAME_COUNT 60 /* ORIGINAL: 10 */
#define MAX_IMAGE_FORMAT_DESCRIPTOR_COUNT 30
#define MAX_INPUT_FOG_ALPHA_COUNT 64
#define MAX_OUTPUT_FOG_ALPHA_COUNT 256
#define MAX_OUTPUT_FOG_ALPHA_VALUE 255
#define MAX_RENDER_PACKET_COUNT 10000
#define MAX_TEXTURE_FORMAT_COUNT 34
#define MAX_TEXTURE_PALETTE_COLOR_COUNT 256
#define MAX_TEXTURE_PALETTE_INDEX_COUNT 512
#define MAX_TEXTURE_STAGE_COUNT 8
#define MAX_TEXTURE_STATE_STATE_COUNT 120
#define MAX_USABLE_TEXTURE_FORMAT_COUNT 34
#define MAX_USABLE_TEXTURE_FORMAT_INDEX_COUNT 34
#define MAX_VERTEX_BUFFER_SIZE 2621400
#define MAX_VERTEX_COUNT 65535
#define MAX_WINDOW_COUNT 256
#define MIN_ACTUAL_DEVICE_CAPABILITIES_INDEX 2
#define MIN_SIMULTANEOUS_TEXTURE_COUNT 1
#define MIN_WINDOW_INDEX 8
#define RENDERER_DEVICE_DEPTH_FORMAT_COUNT 5
#define RENDERER_DEVICE_FORMAT_COUNT 2

#define RENDERER_CULL_MODE_CLOCK_WISE           0x00000000
#define RENDERER_CULL_MODE_NONE                 0x00000001
#define RENDERER_CULL_MODE_COUNTER_CLOCK_WISE   0x80000000

#define MAKEPIXELFORMAT(x) (x & 0xFF)

#if !defined(__WATCOMC__) && _MSC_VER <= 1200
inline void LOGERROR(...) { }
inline void LOGWARNING(...) { }
inline void LOGMESSAGE(...) { }
#else
#define LOGERROR(...) Message(RENDERER_MODULE_MESSAGE_SEVERITY_ERROR, __VA_ARGS__)
#define LOGWARNING(...) Message(RENDERER_MODULE_MESSAGE_SEVERITY_WARNING, __VA_ARGS__)
#define LOGMESSAGE(...) Message(RENDERER_MODULE_MESSAGE_SEVERITY_MESSAGE, __VA_ARGS__)
#endif

namespace Renderer
{
    struct RendererTexture
    {
        DWORD Handle;
        u32 Width;
        u32 Height;
        u32 PixelFormat;
        u32 PaletteMode;
        u32 MipMapCount;
        u32 Stage;
        RendererTexture* Previous;
        u32 PixelSize;
        D3DFORMAT TextureFormat;
        u32 MemoryType;
        BOOL Is16Bit;
        IDirect3DTexture8* Texture;
        u32 Palette;
        u32 Colors;
    };
}

namespace RendererModule
{
    extern u32 DAT_6001eedc; // 0x6001eedc
    extern u32 DAT_6001eee0; // 0x6001eee0

    struct MinMax
    {
        u32 Min;
        u32 Max;
    };

    typedef enum RendererDeviceFormatSize
    {
        RendererDeviceFormatSizeBits = 0,
        RendererDeviceFormatSizeBytes = 1
    } RendererDeviceFormatSize;

    struct TextureStageState
    {
        s32 Values[MAX_TEXTURE_STAGE_COUNT];
    };

    struct RendererPacket
    {
        D3DPRIMITIVETYPE Type;
        u32 Count;
        u32 Size;
    };

    struct TextureStage
    {
        s32 Unk01; // TODO
        s32 Unk02; // TODO
        s32 Unk03; // TODO
        s32 Unk04; // TODO
        s32 Unk05; // TODO
        s32 Unk06; // TODO
        s32 Unk07; // TODO
        s32 Unk08; // TODO
        s32 Unk09; // TODO
        s32 Unk10; // TODO
        s32 Unk11; // TODO
        s32 Unk12; // TODO
    };

    struct RendererModuleWindow
    {
        IDirect3DTexture8* Texture;
        IDirect3DSurface8* Stencil;
        Renderer::RendererTexture Details;
    };

    struct RendererModuleState
    {
        struct
        {
            BOOL IsInit; // 0x6001ed64

            IDirect3D8* Instance; // 0x6001ed24
            IDirect3DDevice8* Device; // 0x6001ed28

            struct
            {
                u32 Width; // 0x6001eec8
                u32 Height; // 0x6001eecc
                u32 Bits; // 0x6001eed0

                IDirect3DSurface8* Surfaces[MAX_ACTIVE_SURFACE_COUNT]; // 0x6001ed2c
            } Surfaces;

            struct
            {
                D3DGAMMARAMP Gamma; // 0x600209a0
            } State;

            struct
            {
                D3DFORMAT Back; // 0x6001ed70
            } Formats;
        } DX;

        struct
        {
            u32 MaxPrimitiveCount; // 0x6001dab8

            struct
            {
                u32 Count; // 0x6001dab4

                RendererPacket Packets[MAX_RENDER_PACKET_COUNT]; // 0x60021000
            } Packets;

            struct
            {
                u32 Count; // 0x6001dac4

                IDirect3DVertexBuffer8* Buffer; // 0x6001dabc

                u32 StartIndex; // 0x6001dac0
            } Vertexes;
        } Data;

        struct
        {
            u32 Index; // 0x6001ed60

            BOOL IsInit; // 0x6001ed6c

            u32 SubType; // 0x6001eec4

            D3DPRESENT_PARAMETERS Presentation; // 0x60020fa0

            RendererModuleDeviceCapabilities8 Capabilities; // 0x600201c0
        } Device;

        struct
        {
            u32 Count; // 0x6001ed5c

            struct
            {
                char Names[MAX_ENUMERATE_DEVICE_NAME_COUNT][MAX_RENDERER_MODULE_DEVICE_LONG_NAME_LENGTH]; // 0x6001e9e8
            } Enumeration;
        } Devices;

        struct
        {
            RENDERERMODULELOGLAMBDA Log; // 0x6001c108

            RENDERERMODULEALLOCATEMEMORYLAMBDA AllocateMemory; // 0x6001eefc
            RENDERERMODULERELEASEMEMORYLAMBDA ReleaseMemory; // 0x6001ef00

            RENDERERMODULESELECTSTATELAMBDA SelectState; // 0x6001eef8

            RendererModuleLambdaContainer Lambdas; // 0x60020fe0

            RENDERERMODULETOGGLERENDERERLAMBDA ToggleRenderer; // 0x6001ede0
            RENDERERMODULETOGGLERENDERERWAITLAMBDA ToggleRendererWait; // 0x6001ede4
            void* ToggleRendererWaitContext; // 0x6001ede8
        } Lambdas;

        struct
        {
            BOOL IsActive; // 0x6001da80

            IDirect3DSurface8* Surface; // 0x6001da8c

            RendererModuleWindowLock State; // 0x6001da68
        } Lock;

        struct
        {
            HANDLE Device; // 0x6001ed58
            HANDLE Surface; // 0x6001ed68
        } Mutexes;

        struct
        {
            BOOL IsActive; // 0x6001da7c
        } Scene;

        struct
        {
            BOOL IsWindowMode; // 0x6001ed50

            u32 Cull; // 0x6001ef04

            BOOL IsFogActive; // 0x6001edd0
        } Settings;

        struct
        {
            u32 Triangles; // 0x6001dac8
            u32 Quads; // 0x6001dacc
            u32 Lines; // 0x6001dad0
            u32 Strips; // 0x6001dad4
            u32 Fans; // 0x6001dad8
            u32 Points; // 0x6001dadc
        } Statistics;

        struct
        {
            BOOL Illegal; // 0x6001edec

            Renderer::RendererTexture* Current; // 0x6001ef14

            D3DFORMAT Formats[MAX_TEXTURE_FORMAT_COUNT]; // 0x6001efa0

            TextureStage Stages[MAX_TEXTURE_STAGE_COUNT]; // 0x60020040

            TextureStageState StageStates[MAX_TEXTURE_STATE_STATE_COUNT]; // 0x6001dae8
        } Textures;

        struct
        {
            u32 X0; // 0x6001da9c
            u32 Y0; // 0x6001daa0
            u32 X1; // 0x6001daa4
            u32 Y1; // 0x6001daa8
        } ViewPort;

        struct
        {
            u32 Count; // 0x6001dab0

            u32 Index; // 0x6001da98

            struct
            {
                HWND HWND; // 0x6001eed8
            } Parent;

            HWND HWND; // 0x6001ed4c

            BOOL IsWindow; // 0x6001da84
            BOOL IsStencil; // 0x6001da88

            IDirect3DSurface8* Surface; // 0x6001da90
            IDirect3DSurface8* Stencil; // 0x6001da94
        } Window;

        RendererModuleWindow Windows[MAX_WINDOW_COUNT]; // 0x6003e4c0
    };

    extern RendererModuleState State;

    void Message(const u32 severity, const char* format, ...);

    BOOL AcquireRendererDeviceCapabilities(void);
    BOOL AcquireRendererDeviceDepthFormat(const u32 device, const D3DFORMAT adapter, const D3DFORMAT target, D3DFORMAT* result);
    BOOL AcquireRendererDeviceDepthFormat(u32* bits, D3DFORMAT* result);
    BOOL AcquireRendererDeviceDepthWindowFormat(u32* width, u32* height, u32* bits, D3DFORMAT* format);
    BOOL AreRenderPacketsComplete(const D3DPRIMITIVETYPE type, const u32 count);
    BOOL AttemptRenderPackets(void);
    BOOL BeginRendererScene(void);
    BOOL RestoreRendererSurfaces(void);
    BOOL SelectRendererMaterial(const u32 color);
    BOOL SelectRendererState(const D3DRENDERSTATETYPE type, const DWORD value);
    BOOL SelectRendererTextureStage(const u32 stage, const D3DTEXTURESTAGESTATETYPE type, const DWORD value);
    BOOL UpdateRendererTexture(Renderer::RendererTexture* tex, const u32* pixels, const u32* palette);
    BOOL UpdateRendererTexturePalette(Renderer::RendererTexture* tex, const u32* palette);
    BOOL UpdateRendererTextureRectangle(Renderer::RendererTexture* tex, const u32* pixels, const u32* palette, const s32 x, const s32 y, const s32 width, const s32 height, const s32 size, const s32 level);
    D3DFORMAT AcquireRendererTextureFormat(const u32 indx);
    HRESULT InitializeRendererTexture(Renderer::RendererTexture* tex);
    inline u32 AcquireNormal(const f32x3* a, const f32x3* b, const f32x3* c) { const s32 value = (s32)((b->X - a->X) * (c->Y - a->Y) - (c->X - a->X) * (b->Y - a->Y)); return *(u32*)&value; }
    Renderer::RendererTexture* AllocateRendererTexture(const u32 size);
    Renderer::RendererTexture* AllocateRendererTexture(void);
    s32 AcquireSettingsValue(const s32 value, const char* section, const char* name);
    s32 AcquireTextureStateStageIndex(const u32 state);
    u32 AcquireRendererDeviceCount(void);
    u32 AcquireRendererDeviceFormat(const D3DFORMAT format);
    u32 AcquireRendererDeviceFormatSize(const D3DFORMAT format, const RendererDeviceFormatSize size);
    u32 AcquireTexturePalette();
    u32 ClearRendererViewPort(const u32 x0, const u32 y0, const u32 x1, const u32 y1, const BOOL window);
    u32 DisposeRendererTexture(Renderer::RendererTexture* tex);
    u32 SelectBasicRendererState(const u32 state, void* value);
    u32 SelectRendererTransforms(const f32 zNear, const f32 zFar);
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 ToggleRenderer(void);
    void AcquireRendererDeviceFormats(void);
    void AcquireRendererDeviceMemorySize(void);
    void AcquireRendererModuleDescriptor(RendererModuleDescriptor* desc, const char* section);
    void AcquireRendererTextureFormats(const D3DFORMAT format);
    void InitializeRendererModuleState(const u32 mode, const u32 pending, const u32 depth, const char* section);
    void InitializeRendererState(void);
    void InitializeRenderState55(void);
    void InitializeTexturePalette(void);
    void InitializeTextureStateStates(void);
    void InitializeVertexBuffer(void);
    void InitializeViewPort(void);
    void ModifyRendererSurface(IDirect3DSurface8* surface, const u32 count);
    void ReleaseRendererObjects(void);
    void ReleaseRendererWindows(void);
    void ReleaseTexturePalette(const s32 palette);
    void RenderPackets(void);
    void RenderScene(void);
    void SelectRendererFogAlphas(const u8* input, u8* output);
    void SelectRendererStateValue(const u32 state, void* value);
    void UpdateVertex(Renderer::RTLVX* vertex);
}