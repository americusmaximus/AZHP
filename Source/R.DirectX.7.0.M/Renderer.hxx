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

#define DIRECTDRAW_VERSION 0x700
#include <ddraw.h>

#ifdef __WATCOMC__
#undef PURE
#define PURE
#endif

#define DIRECT3D_VERSION 0x700
#include <d3d.h>

#ifdef __WATCOMC__
#include <mmsystem.h>
#endif

#define DDGDI_NONE 0
#define DDEDM_NONE 0
#define D3DDP_NONE 0
#define D3DVBCAPS_NONE 0
#define D3DVBOPTIMIZE_NONE 0
#define DDSDM_NONE 0
#define D3DCOLOR_NONE 0

#define RENDERER_STATE_INACTIVE 0
#define RENDERER_STATE_ACTIVE 1

#define RENDERER_MODULE_ENVIRONMENT_SECTION_NAME "DX7"

#define MAX_ENUMERATE_RENDERER_DEVICE_COUNT 60 /* ORIGINAL: 16 */
#define MAX_ENUMERATE_RENDERER_DEVICE_NAME_COUNT 60 /* ORIGINAL: 10 */
#define MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH 80

#define MAX_ACTIVE_SURFACE_COUNT 8

#define MIN_RENDERER_DEVICE_AVAIABLE_VIDEO_MEMORY (16 * 1024 * 1024) /* ORIGINAL: 0x200000 (2 MB) */

#define MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT 128 /* ORIGINAL: 98 */

#define MAX_TEXTURE_FORMAT_COUNT 128 /* ORIGINAL: 32 */
#define MAX_USABLE_TEXTURE_FORMAT_COUNT 22

#define DEPTH_BIT_MASK_32_BIT 0x100
#define DEPTH_BIT_MASK_24_BIT 0x200
#define DEPTH_BIT_MASK_16_BIT 0x400
#define DEPTH_BIT_MASK_8_BIT 0x800

#define DXT_FORMAT_NONE 0
#define DXT_FORMAT_DXT1 1
#define DXT_FORMAT_DXT2 2
#define DXT_FORMAT_DXT3 3

#define INVALID_TEXTURE_FORMAT_COUNT (-1)
#define INVALID_TEXTURE_FORMAT_INDEX (-1)

#define MAX_RENDERER_INDEX_COUNT 65536

#define MAX_RENDERER_VERTEX_COUNT 32768

#define MAX_TEXTURE_DEPTH_FORMAT_COUNT 16 /* ORIGINAL: 6 */

#define MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT 8
#define MAX_RENDERER_MODULE_TEXTURE_STATE_STATE_COUNT 120

#define RENDERER_WINDOW_OFFSET 8
#define MAX_RENDERER_WINDOW_COUNT 65536
#define MAX_RENDERER_WINDOW_INDEX 65535

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

}

namespace RendererModule
{
    extern u32 DAT_6005ab50; // 0x6005ab50
    extern u32 DAT_6005ab5c; // 0x6005ab5c

    struct TextureStageState
    {
        s32 Values[MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT];
    };

    struct TextureFormat
    {
        DDSURFACEDESC2 Descriptor;

        BOOL IsPalette;
        u32 RedBitCount;
        u32 BlueBitCount;
        u32 GreenBitCount;
        u32 PaletteColorBits;
        u32 AlphaBitCount;

        u32 DXTF; // Format
        u32 DXTT; // Type
        u32 DXTN; // Name
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

    struct TriangulationCapabilities
    {
        BOOL Unk01; // TODO
        BOOL Unk02; // TODO
        BOOL Unk03; // TODO
        BOOL Unk04; // TODO
        BOOL Unk05; // TODO
        BOOL Unk06; // TODO
        BOOL Unk07; // TODO
        BOOL Unk08; // TODO
        BOOL Unk09; // TODO
        BOOL Unk10; // TODO
        BOOL Unk11; // TODO
        BOOL Unk12; // TODO
        BOOL Unk13; // TODO
        BOOL Unk14; // TODO
        BOOL Unk15; // TODO
        BOOL Unk16; // TODO
    };

    struct RendererModuleWindow
    {
        Renderer::RendererTexture* Texture;
        IDirectDrawSurface7* Surface;
    };

    struct RendererModuleState
    {
        struct
        {
            HRESULT Code; // 0x60018680

            IDirectDrawClipper* Clipper; // 0x60018698
            IDirectDrawGammaControl* GammaControl; // 0x6001869c

            IDirectDraw7* Instance; // 0x600186a0

            IDirect3D7* DirectX; // 0x60058d98
            IDirect3DDevice7* Device; // 0x60058d9c

            struct
            {
                BOOL IsSoft; // 0x60058d6c
                BOOL IsActive; // 0x6005abc0
                BOOL IsInit; // 0x60058d74

                IDirectDraw7* Instance; // 0x60058d80

                struct
                {
                    struct
                    {
                        IDirectDrawSurface7* Main; // 0x60058d8c
                        IDirectDrawSurface7* Back; // 0x60058d90
                    } Active;

                    IDirectDrawSurface7* Main; // 0x60058d84
                    IDirectDrawSurface7* Back; // 0x60058d88
                } Surfaces;
            } Active;

            struct
            {
                IDirectDrawSurface7* Main; // 0x600186a4
                IDirectDrawSurface7* Back; // 0x600186a8

                IDirectDrawSurface7* Active[MAX_ACTIVE_SURFACE_COUNT]; // 0x600186ac

                IDirectDrawSurface7* Window; // 0x600186cc

                IDirectDrawSurface7* Depth; // 0x60058d94
            } Surfaces;
        } DX;

        struct
        {
            struct
            {
                u32 Count; // 0x60058878

                u16 Indexes[MAX_RENDERER_INDEX_COUNT]; // 0x60038868
            } Indexes;

            struct
            {
                u32 Count; // 0x60058874

                u32 Vertexes[MAX_RENDERER_VERTEX_COUNT]; // 0x60018868
            } Vertexes;
        } Data;

        struct
        {
            GUID* Identifier; // 0x600186d8

            char Name[MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH]; // 0x600fc840

            struct
            {
                BOOL IsAccelerated; // 0x6005aae0
                u32 RendererDepthBits; // 0x6005aae4
                u32 RendererBits; // 0x6005aae8
                u32 RendererDeviceDepthBits; // 0x6005aaec
                BOOL IsDepthVideoMemoryCapable; // 0x6005aaf0
                BOOL IsDepthAvailable; // 0x6005aaf4
                BOOL IsGreenAllowSixBits; // 0x6005aaf8
                BOOL IsVideoMemoryCapable; // 0x6005aafc
                BOOL IsDitherAvailable; // 0x6005ab00
                BOOL IsNoVerticalSyncAvailable; // 0x6005ab04
                BOOL IsWBufferAvailable; // 0x6005ab08
                BOOL IsWFogAvailable; // 0x6005ab0c
                BOOL IsWindowMode; // 0x6005ab10
                BOOL IsTrilinearInterpolationAvailable; // 0x6005ab14
                BOOL IsDepthBufferRemovalAvailable; // 0x6005ab18
                BOOL IsPerspectiveTextures; // 0x6005ab1c
                BOOL IsAlphaFlatBlending; // 0x6005ab20
                BOOL IsAlphaProperBlending; // 0x6005ab24
                BOOL IsAlphaTextures; // 0x6005ab28
                BOOL IsModulateBlending; // 0x6005ab2c
                BOOL IsSourceAlphaBlending; // 0x6005ab30
                u32 AntiAliasing; // 0x6005ab34
                BOOL IsColorBlending; // 0x6005ab38
                BOOL IsAnisotropyAvailable; // 0x6005ab3c
                BOOL IsPrimaryGammaAvailable; // 0x6005ab40
                BOOL IsSpecularGouraudBlending; // 0x6005ab44
                BOOL IsStencilBuffer; // 0x6005ab48
                BOOL IsSpecularBlending; // 0x6005ab4c

                BOOL IsTextureIndependentUVs; // 0x6005ab54
                BOOL IsMipMapBiasAvailable; // 0x6005ab58

                u32 MinTextureWidth; // 0x6005ab60
                u32 MaxTextureWidth; // 0x6005ab64
                u32 MinTextureHeight; // 0x6005ab68
                u32 MaxTextureHeight; // 0x6005ab6c

                u32 MultipleTextureWidth; // 0x6005ab70
                BOOL IsPowerOfTwoTexturesWidth; // 0x6005ab74
                u32 MultipleTextureHeight; // 0x6005ab78
                BOOL IsPowerOfTwoTexturesHeight; // 0x6005ab7c
                u32 MaximumSimultaneousTextures; // 0x6005ab80
                BOOL IsSquareOnlyTextures; // 0x6005ab84
                BOOL IsAntiAliasEdges; // 0x6005ab88
                f32 GuardBandLeft; // 0x6005ab8c
                f32 GuardBandRight; // 0x6005ab90
                f32 GuardBandTop; // 0x6005ab94
                f32 GuardBandBottom; // 0x6005ab98
                f32 MaxTextureRepeat; // 0x6005ab9c
                BOOL IsAlphaComparisonAvailable; // 0x6005aba0
                u32 MaxAnisotropy; // 0x6005aba4

                u32 MaxActiveLights; // 0x6005abc4

                u32 MaxUserClipPlanes; // 0x6005abcc
                u32 MaxVertexBlendMatrices; // 0x6005abd0
                BOOL IsTransformLightBufferSystemMemoryAvailable; // 0x6005abd4
                BOOL IsTransformLightBufferVideoMemoryAvailable; // 0x6005abd8

                TriangulationCapabilities Capabilities; // 0x6005abe0
            } Capabilities;
        } Device;

        struct
        {
            u32 Count; // 0x600186e0

            GUID* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60018250

            struct
            {
                u32 Count; // 0x60059000
                BOOL IsAvailable; // 0x60059004

                char Names[MAX_ENUMERATE_RENDERER_DEVICE_NAME_COUNT][MAX_ENUMERATE_RENDERER_DEVICE_NAME_LENGTH]; // 0x60018340

                struct
                {
                    GUID* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058e40
                    GUID Identifiers[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058e80
                } Identifiers;

                struct
                {
                    HMONITOR* Indexes[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058f80
                    HMONITOR Monitors[MAX_ENUMERATE_RENDERER_DEVICE_COUNT]; // 0x60058fc0
                } Monitors;

                DDDEVICEIDENTIFIER Identifier; // 0x60059010
            } Enumeration;
        } Devices;

        struct
        {
            RENDERERMODULELOGLAMBDA Log; // 0x6001713c

            RendererModuleLambdaContainer Lambdas; // 0x6007b860
        } Lambdas;

        struct
        {
            BOOL IsActive; // 0x60018230
        } Lock;

        HANDLE Mutex; // 0x600186d4

        struct
        {
            BOOL IsActive; // 0x6001822c
        } Scene;

        struct
        {
            BOOL IsWindowModeActive; // 0x60018860

            u32 CooperativeLevel; // 0x60018684
            BOOL IsWindowMode; // 0x60018688

            u32 MaxAvailableMemory; // 0x60018690

            u32 Acceleration; // 0x60018844

            DDGAMMARAMP GammaControl; // 0x6007b240
        } Settings;

        struct
        {
            TextureStage Stages[MAX_RENDERER_MODULE_TEXTURE_STAGE_COUNT]; // 0x6005a940

            TextureStageState StageStates[MAX_RENDERER_MODULE_TEXTURE_STATE_STATE_COUNT]; // 0x6007b8a0

            struct
            {
                u32 Count; // 0x6005a840

                TextureFormat Formats[MAX_TEXTURE_FORMAT_COUNT]; // 0x60059440
                s32 Indexes[MAX_USABLE_TEXTURE_FORMAT_COUNT]; // 0x6005a860

                struct
                {
                    u32 Count; // 0x600186e8

                    u32 Formats[MAX_TEXTURE_DEPTH_FORMAT_COUNT]; // 0x6007b840
                } Depth;
            } Formats;
        } Textures;

        struct
        {
            u32 X0; // 0x60018238
            u32 Y0; // 0x6001823c
            u32 X1; // 0x60018240
            u32 Y1; // 0x60018244
        } ViewPort;

        struct
        {
            u32 Count; // 0x6001824c

            u32 Index; // 0x600186d0

            struct
            {
                HWND HWND; // 0x60058df0
            } Parent;

            HWND HWND; // 0x6007b880

            u32 Height; // 0x6005a934

            u32 Width; // 0x6005aac0

            u32 Bits; // 0x60058d7c
        } Window;

        RendererModuleWindow Windows[MAX_RENDERER_WINDOW_COUNT]; // 0x6007c7c0
    };

    extern RendererModuleState State;

    void Message(const u32 severity, const char* format, ...);

    BOOL AcquireRendererDeviceAccelerationState(const u32 indx);
    BOOL AcquireRendererDeviceDepthBufferRemovalCapabilities(void);
    BOOL AcquireRendererDeviceState(void);
    BOOL AcquireRendererDeviceTrilinearInterpolationCapabilities(void);
    BOOL BeginRendererScene(void);
    BOOL CALLBACK EnumerateDirectDrawDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context, HMONITOR monitor);
    BOOL EndRendererScene(void);
    BOOL InitializeRendererDeviceDepthSurfaces(const u32 width, const u32 height, IDirectDrawSurface7* depth, IDirectDrawSurface7* surf);
    const char* AcquireRendererMessage(const HRESULT code);
    const char* AcquireRendererMessageDescription(const HRESULT code);
    HRESULT CALLBACK EnumerateDirectDrawAcceleratedDevices(LPSTR description, LPSTR name, LPD3DDEVICEDESC7 desc, LPVOID context);
    HRESULT CALLBACK EnumerateRendererDeviceDepthPixelFormats(LPDDPIXELFORMAT format, LPVOID ctx);
    HRESULT CALLBACK EnumerateRendererDeviceModes(LPDDSURFACEDESC2 desc, LPVOID context);
    HRESULT CALLBACK EnumerateRendererDevicePixelFormats(LPDDPIXELFORMAT format, LPVOID context);
    HRESULT CALLBACK EnumerateRendererDeviceTextureFormats(LPDDPIXELFORMAT format, LPVOID context);
    s32 AcquireRendererDeviceTextureFormatIndex(const u32 palette, const u32 alpha, const u32 red, const u32 green, const u32 blue, const u32 dxtf, const u32 dxtt);
    s32 AcquireSettingsValue(const s32 value, const char* section, const char* name);
    u32 AcquireDirectDrawDeviceCount(GUID** uids, HMONITOR** monitors, const char* section);
    u32 AcquirePixelFormat(const DDPIXELFORMAT* format);
    u32 AcquireRendererDeviceCount(void);
    u32 InitializeRendererDevice(void);
    u32 InitializeRendererDeviceAcceleration(void);
    u32 InitializeRendererDeviceLambdas(void);
    u32 SelectRendererTransforms(const f32 zNear, const f32 zFar);
    u32 STDCALLAPI InitializeRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI InitializeRendererDeviceSurfacesExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    u32 STDCALLAPI ReleaseRendererDeviceExecute(const void*, const HWND hwnd, const u32 msg, const u32 wp, const u32 lp, HRESULT* result);
    void AcquireRendererDeviceCapabilities(void);
    void AcquireRendererDeviceTextureFormats(void);
    void AcquireWindowModeCapabilities(void);
    void AttemptRenderScene(void);
    void InitializeConcreteRendererDevice(void);
    void InitializeRendererState(void);
    void InitializeTextureStateStates(void);
    void InitializeVertexes(void* vertexes, const u32 count);
    void InitializeViewPort(void);
    void ReleaseRendererDevice(void);
    void ReleaseRendererDeviceSurfaces(void);
    void ReleaseRendererWindows(void);
    void RendererRenderScene(void);
}