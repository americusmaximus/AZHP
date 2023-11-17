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

#ifdef __WATCOMC__
#define D3DTFG_GAUSSIANCUBIC 4 /* D3DTEXTUREMAGFILTER */
#define D3DTFG_ANISOTROPIC   5 /* D3DTEXTUREMAGFILTER */
#endif

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

#define DEPTH_BIT_MASK_32_BIT 0x100
#define DEPTH_BIT_MASK_24_BIT 0x200
#define DEPTH_BIT_MASK_16_BIT 0x400
#define DEPTH_BIT_MASK_8_BIT 0x800

#define INVALID_TEXTURE_FORMAT_COUNT (-1)
#define INVALID_TEXTURE_FORMAT_INDEX (-1)

#define MAX_TEXTURE_FORMAT_COUNT 128 /* ORIGINAL: 12 */
#define MAX_USABLE_TEXTURE_FORMAT_COUNT 10
#define MAX_OTHER_USABLE_TEXTURE_FORMAT_COUNT 12

#define MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT 128 /* ORIGINAL: 32 */

#define MAX_RENDERER_DEVICE_COUNT 16 /* ORIGINAL: 10 */

#define MAX_DEVICE_NAME_LENGTH 32 /* ORIGINAL: 20 */

#define MAX_ACTIVE_SURFACE_COUNT 8

#define MAX_RENDERER_INDEX_COUNT 8100

#define MAX_TEXTURE_PALETTE_COLOR_COUNT 256

#define MAX_FOG_ALPHA_COUNT 256

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
        BOOL IsError; // 0x6000fe9c

        struct
        {
            HRESULT Code; // 0x6000fdec

            IDirectDraw2* Instance; // 0x6000fe00

            IDirect3D2* DirectX; // 0x6000fe8c
            IDirect3DDevice2* Device; // 0x6000fe90
            IDirect3DViewport2* ViewPort; // 0x6000fe94
            IDirect3DMaterial2* Material; // 0x6000fe98

            struct
            {
                BOOL IsInit; // 0x6000fe6c
                IDirectDraw2* Instance; // 0x6000fe70

                struct
                {
                    struct
                    {
                        IDirectDrawSurface2* Main; // 0x6000fe7c
                        IDirectDrawSurface2* Back; // 0x6000fe80

                        IDirectDrawSurface2* Depth; // 0x6000fe88
                    } Active;

                    IDirectDrawSurface* Main; // 0x6000fe74
                    IDirectDrawSurface* Back; // 0x6000fe78

                    IDirectDrawSurface* Depth; // 0x6000fe84
                } Surfaces;
            } Active;

            struct
            {
                IDirectDrawSurface2* Active[MAX_ACTIVE_SURFACE_COUNT]; // 0x6000fe0c

                IDirectDrawSurface* Main; // 0x6000fe04
                IDirectDrawSurface* Back; // 0x6000fe08

                IDirectDrawSurface2* Window; // 0x6000fe2c
            } Surfaces;
        } DX;

        struct
        {
            GUID* Identifier; // 0x6000fe34

            struct
            {
                BOOL IsAccelerated; // 0x60011880
                u32 DepthBits; // 0x60011884
                u32 RendererBits; // 0x60011888

                BOOL IsDepthAvailable; // 0x60011890

                BOOL IsPerspectiveTextures; // 0x60011898
                BOOL IsAlphaBlend; // 0x6001189c
                BOOL IsNonFlatAlphaBlend; // 0x600118a0
                BOOL IsAlphaTextures; // 0x600118a4
                BOOL IsModulateBlending; // 0x600118a8
                BOOL IsSourceAlphaBlending; // 0x600118ac
                BOOL IsColorBlending; // 0x600118b0
                BOOL IsSpecularBlending; // 0x600118b4
            } Capabilities;
        } Device;

        struct
        {
            u32 Count; // 0x6000fe38

            GUID* Indexes[MAX_RENDERER_DEVICE_COUNT]; // 0x60011718
            GUID Identifiers[MAX_RENDERER_DEVICE_COUNT]; // 0x600115b0
            char Names[MAX_RENDERER_DEVICE_COUNT][MAX_DEVICE_NAME_LENGTH]; // 0x60011650
        } Devices;

        RendererModuleLambdaContainer Lambdas; // 0x60011740

        struct
        {
            BOOL IsActive; // 0x6000fdcc
        } Scene;

        struct
        {
            u32 CooperativeLevel; // 0x6000fdf0
            BOOL IsWindowMode; // 0x6000fdf4
        } Settings;

        struct
        {
            BOOL IsActive; // 0x6000fdd0

            IDirectDrawSurface2* Surface; // 0x6000fdd4

            RendererModuleWindowLock State; // 0x60011590
        } Lock;

        HANDLE Mutex; // 0x6000fe30

        struct
        {
            struct
            {
                u32 Count; // 0x6000feb4

                u16 Indexes[MAX_RENDERER_INDEX_COUNT]; // 0x60015a08
            } Indexes;

            struct
            {
                u32 Count; // 0x6000feb0

                void* Vertexes; // 0x60011a68 // TODO
            } Vertexes;
        } Data;

        struct
        {
            BOOL Illegal; // 0x6000fe64

            Renderer::RendererTexture* Current; // 0x6000fe60

            struct
            {
                u32 Count; // 0x6000fee0

                TextureFormat Formats[MAX_TEXTURE_FORMAT_COUNT]; // 0x60019960
                s32 Indexes[MAX_USABLE_TEXTURE_FORMAT_COUNT]; // 0x60019f90
            } Formats;
        } Textures;

        struct
        {
            u32 X0; // 0x6000fdd8
            u32 Y0; // 0x6000fddc
            u32 X1; // 0x6000fde0
            u32 Y1; // 0x6000fde4
        } ViewPort;

        struct
        {
            HWND HWND; // 0x6000fde8

            u32 Width; // 0x60011a54
            u32 Height; // 0x60011a50
        } Window;
    };

    extern RendererModuleState State;

    void Message(const char* format, ...);
    BOOL Error(const char* format, ...);

    u32 AcquireRendererDeviceCount(void);
    void SelectRendererDevice(void);
    u32 EndRendererScene(void);
    u32 RendererRenderScene(void);
    u32 BeginRendererScene(void);
    u32 ClearRendererViewPort(const u32 x0, const u32 y0, const u32 x1, const u32 y1);
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context);
    u32 InitializeRendererDevice(void);
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC desc, LPVOID context);
    u32 AcquirePixelFormat(const DDPIXELFORMAT* format);
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    void ReleaseRendererDeviceSurfaces(void);
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 InitializeRendererDeviceAcceleration(void);
    void InitializeConcreteRendererDevice(void);
    void ReleaseRendererDevice(void);
    BOOL AcquireRendererDeviceState(void);
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height);
    void AcquireRendererDeviceTextureFormats(void);
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDSURFACEDESC desc, LPVOID context);
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue);
    void InitializeRendererState(void);
    void InitializeViewPort(void);
    u32 ReleaseRendererWindow(void);
    u32 ToggleRenderer(void);
    BOOL SelectRendererState(const D3DRENDERSTATETYPE type, const DWORD value);
    void SelectRendererMaterial(const f32 r, const f32 g, const f32 b);
    u32 AttemptRenderScene(void);
    void SelectRendererDeviceType(const u32 type);
    Renderer::RendererTexture* InitializeRendererTexture(void);
    BOOL InitializeRendererTextureDetails(Renderer::RendererTexture* tex);
    void ReleaseRendererTexture(Renderer::RendererTexture* tex);
    BOOL UpdateRendererTexture(Renderer::RendererTexture* tex, const u32* pixels, const u32* palette);
    BOOL SelectRendererTexture(Renderer::RendererTexture* tex);
}