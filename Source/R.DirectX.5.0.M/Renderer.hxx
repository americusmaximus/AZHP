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

#pragma once

#ifdef __WATCOMC__
#include <RendererModule.Export.hxx>
#else
#include "RendererModule.Export.hxx"
#endif

#define DIRECTDRAW_VERSION 0x500
#include <ddraw.h>

#ifdef __WATCOMC__
#undef PURE
#define PURE
#endif

#define DIRECT3D_VERSION 0x500
#include <d3d.h>

#define DDGDI_NONE 0
#define DDEDM_NONE 0
#define D3DDP_NONE 0
#define D3DVBCAPS_NONE 0
#define D3DVBOPTIMIZE_NONE 0
#define DDSDM_NONE 0
#define D3DCOLOR_NONE 0

#define D3DRENDERSTATE_FOGSTART (D3DRENDERSTATETYPE)36
#define D3DRENDERSTATE_FOGEND (D3DRENDERSTATETYPE)37
#define D3DRENDERSTATE_FOGDENSITY (D3DRENDERSTATETYPE)38

#define MAX_RENDERER_VERTEX_COUNT 253
#define MAX_RENDERER_INDEX_COUNT 8096

#define DEPTH_BIT_MASK_32_BIT 0x100
#define DEPTH_BIT_MASK_24_BIT 0x200
#define DEPTH_BIT_MASK_16_BIT 0x400
#define DEPTH_BIT_MASK_8_BIT 0x800

#define INVALID_TEXTURE_FORMAT_COUNT (-1)
#define INVALID_TEXTURE_FORMAT_INDEX (-1)

#define MAX_TEXTURE_FORMAT_COUNT 128 /* ORIGINAL: 12 */

#define MAX_RENDERER_DEVICE_COUNT 16 /* ORIGINAL: 10 */

#define MAX_DEVICE_NAME_LENGTH 32

#define MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT 128 /* ORIGINAL: 100 */
#define MAX_USABLE_TEXTURE_FORMAT_COUNT 11
#define MAX_OTHER_USABLE_TEXTURE_FORMAT_COUNT 12

#define MAX_ACTIVE_SURFACE_COUNT 8

#define MIN_RENDERER_DEVICE_AVAIABLE_VIDEO_MEMORY (16 * 1024 * 1024) /* ORIGINAL: 0x20000 (2 MB) */

#define MAX_TEXTURE_PALETTE_COLOR_COUNT 256

#define MAX_INPUT_FOG_ALPHA_COUNT 64
#define MAX_OUTPUT_FOG_ALPHA_VALUE 255
#define MAX_OUTPUT_FOG_ALPHA_COUNT 256

#define DEFAULT_FOG_DINSITY (0.5f)
#define DEFAULT_FOG_COLOR 0x00FFFFFF

namespace Renderer
{
    struct RendererTexture
    {
        u32 Width;
        u32 Height;

        u32 UnknownFormatIndexValue; // TODO
        s32 FormatIndex; // TODO
        s32 FormatIndexValue; // TODO

        void* Unk06; // TODO

        D3DTEXTUREHANDLE Handle;
        RendererTexture* Previous;
        u32 MemoryType;

        s32 Unk10; // TODO

        IDirectDrawSurface3* Surface1;
        IDirect3DTexture2* Texture1;
        IDirectDrawSurface3* Surface2;
        IDirect3DTexture2* Texture2;
        IDirectDrawPalette* Palette;

        DDSURFACEDESC Descriptor;

        u32 Colors;
    };
}

namespace RendererModule
{
    struct TextureFormat
    {
        DDSURFACEDESC Descriptor;

        BOOL IsPalette;
        u32 RedBitCount;
        u32 BlueBitCount;
        u32 GreenBitCount;
        u32 PaletteColorBits;
        u32 AlphaBitCount;
    };

    struct RendererModuleState
    {
        BOOL IsError; // 0x60010f54

        struct
        {
            HRESULT Code; // 0x60010ea4

            IDirectDraw2* Instance; // 0x60010eb8

            IDirect3D2* DirectX; // 0x60010f44
            IDirect3DDevice2* Device; // 0x60010f48
            IDirect3DViewport2* ViewPort; // 0x60010f4c
            IDirect3DMaterial2* Material; // 0x60010f50

            struct
            {
                BOOL IsInit; // 0x60010f24
                IDirectDraw2* Instance; // 0x60010f28

                struct
                {
                    struct
                    {
                        IDirectDrawSurface2* Main; // 0x60010f34
                        IDirectDrawSurface2* Back; // 0x60010f38

                        IDirectDrawSurface2* Depth; // 0x60010f40
                    } Active;

                    IDirectDrawSurface* Main; // 0x60010f2c
                    IDirectDrawSurface* Back; // 0x60010f30

                    IDirectDrawSurface* Depth; // 0x60010f3c
                } Surfaces;
            } Active;

            struct
            {
                IDirectDrawSurface* Main; // 0x60010ebc
                IDirectDrawSurface* Back; // 0x60010ec0

                IDirectDrawSurface2* Active[MAX_ACTIVE_SURFACE_COUNT]; // 0x60010ec4

                IDirectDrawSurface2* Window; // 0x60010ee4
            } Surfaces;
        } DX;

        struct
        {
            GUID* Identifier; // 0x60010eec

            struct
            {
                BOOL IsAccelerated; // 0x60012ba8
                u32 DepthBits; // 0x60012bac
                u32 RendererBits; // 0x60012bb0

                BOOL IsDepthAvailable; // 0x60012bb8

                BOOL IsPerspectiveTextures; // 0x60012bc0
                BOOL IsAlphaBlend; // 0x60012bc4
                BOOL IsNonFlatAlphaBlend; // 0x60012bc8
                BOOL IsAlphaTextures; // 0x60012bcc
                BOOL IsModulateBlending; // 0x60012bd0
                BOOL IsSourceAlphaBlending; // 0x60012bd4
                u32 ResterOperationCaps; // 0x60012bd8
                BOOL IsColorBlending; // 0x60012bdc
                BOOL IsSpecularBlending; // 0x60012be0
            } Capabilities;
        } Device;

        struct
        {
            u32 Count; // 0x60010ef0

            GUID* Indexes[MAX_RENDERER_DEVICE_COUNT]; // 0x60012860
            GUID Identifiers[MAX_RENDERER_DEVICE_COUNT]; // 0x60012680
            char Names[MAX_RENDERER_DEVICE_COUNT][MAX_DEVICE_NAME_LENGTH]; // 0x60012720
        } Devices;

        RendererModuleLambdaContainer Lambdas; // 0x60012888

        struct
        {
            BOOL IsActive; // 0x60010e8c

            IDirectDrawSurface2* Surface; // 0x60010e90

            RendererModuleWindowLock State; // 0x60012590
        } Lock;

        HANDLE Mutex; // 0x60010ee8

        struct
        {
            BOOL IsActive; // 0x60010e88
        } Scene;

        struct
        {
            u32 CooperativeLevel; // 0x60010ea8
            BOOL IsWindowMode; // 0x60010eac

            u32 MaxAvailableMemory; // 0x60010eb4
        } Settings;

        struct
        {
            struct
            {
                u32 Count; // 0x60010f6c

                u16 Indexes[MAX_RENDERER_INDEX_COUNT]; // 0x60014b90
            } Indexes;

            struct
            {
                u32 Count; // 0x60010f68

                Renderer::RTLVX Vertexes[MAX_RENDERER_VERTEX_COUNT]; // 0x6001d5d8
            } Vertexes;
        } Data;

        struct
        {
            Renderer::RendererTexture* Current; // 0x60010f18
            BOOL Illegal; // 0x60010f1c

            struct
            {
                u32 Count; // 0x60010f94

                TextureFormat Formats[MAX_TEXTURE_FORMAT_COUNT]; // 0x60018ae8
                s32 Indexes[MAX_OTHER_USABLE_TEXTURE_FORMAT_COUNT]; // 0x60019118
            } Formats;
        } Textures;

        struct
        {
            u32 X0; // 0x60010e94
            u32 Y0; // 0x60010e98
            u32 X1; // 0x60010e9c
            u32 Y1; // 0x60010ea0
        } ViewPort;

        struct
        {
            HWND HWND; // 0x600128a8

            u32 Height; // 0x60012be4
            u32 Width; // 0x60012be8
        } Window;
    };

    extern RendererModuleState State;

    void Message(const char* format, ...);
    BOOL Error(const char* format, ...);

    BOOL BeginRendererScene(void);
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context);
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height);
    BOOL InitializeRendererTextureDetails(Renderer::RendererTexture* tex);
    BOOL SelectRendererState(const D3DRENDERSTATETYPE type, const DWORD value);
    BOOL SelectRendererTexture(Renderer::RendererTexture* tex);
    BOOL UpdateRendererTexture(Renderer::RendererTexture* tex, const u32* pixels, const u32* palette);
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC desc, LPVOID context);
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDSURFACEDESC desc, LPVOID context);
    Renderer::RendererTexture* InitializeRendererTexture(void);
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue);
    u32 AcquirePixelFormat(const DDPIXELFORMAT* format);
    u32 AcquireRendererDeviceCount(void);
    u32 ClearRendererViewPort(const u32 x0, const u32 y0, const u32 x1, const u32 y1);
    u32 EndRendererScene(void);
    u32 InitializeRendererDevice(void);
    u32 InitializeRendererDeviceAcceleration(void);
    u32 InitializeRendererDeviceLambdas(void);
    u32 ReleaseRendererDeviceInstance(void);
    u32 ReleaseRendererWindow(void);
    u32 RendererRenderScene(void);
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 ToggleRenderer(void);
    void AcquireRendererDeviceTextureFormats(void);
    void AttemptRenderScene(void);
    void InitializeConcreteRendererDevice(void);
    void InitializeRendererState(void);
    void InitializeViewPort(void);
    void ReleaseRendererDevice(void);
    void ReleaseRendererDeviceSurfaces(void);
    void ReleaseRendererTexture(Renderer::RendererTexture* tex);
    void RendererLockWindow(const BOOL mode);
    void RestoreRendererSurfaces(void);
    void RestoreRendererTextures(void);
    void SelectRendererDevice(void);
    void SelectRendererDeviceType(const u32 type);
    void SelectRendererFogAlphas(const u8* input, u8* output);
    void SelectRendererMaterial(const f32 r, const f32 g, const f32 b);
}