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

#include "Renderer.hxx"

#define DEFAULT_RENDERER_DEVICE_INDEX 0
#define INVALID_RENDERER_DEVICE_INDEX (-1)

#define INVALID_RENDERER_DEVICE_STATE (-1)

namespace RendererModuleValues
{
    extern s32 RendererDeviceIndex; // 0x6000fe3c
    extern s32 RendererDeviceState; // 0x6000fe40

    extern u32 RendererFogColor; // 0x6000fe50

    extern f32 RendererDepthBias; // 0x6000fe58

    extern s32 RendererDeviceType; // 0x6000fe68

    extern u32 MaximumRendererVertexCount; // 0x6000fea4

    extern s32 RendererTextureFormatStates[MAX_USABLE_TEXTURE_FORMAT_COUNT]; // 0x6000fd88

    extern s32 UnknownArray06[6]; // 0x6000fdb4 // TODO

    extern u32 UnknownFormatValues[MAX_OTHER_USABLE_TEXTURE_FORMAT_COUNT]; // 0x6000fee4

    extern u8 RendererFogAlphas[MAX_FOG_ALPHA_COUNT]; // 0x60011760

    extern Renderer::RendererTexture* CurrentRendererTexture; // 0x60011870

    extern RendererModule::RendererModuleDescriptor ModuleDescriptor; // 0x60011010
    extern RendererModule::RendererModuleDescriptorDeviceCapabilities ModuleDescriptorDeviceCapabilities[MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT]; // 0x60011090
}