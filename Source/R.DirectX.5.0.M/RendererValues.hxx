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

namespace RendererModuleValues
{
    extern s32 RendererDeviceIndex; // 0x60010ef4

    extern u32 RendererFogColor; // 0x60010f08
    extern f32 RendererFogDensity; // 0x60010f0c
    extern f32 RendererDepthBias; // 0x60010f10

    extern s32 RendererDeviceType; // 0x60010f20

    extern s32 RendererTextureFormatStates[MAX_USABLE_TEXTURE_FORMAT_COUNT]; // 0x60010e44

    extern s32 UnknownArray06[6]; // 0x60010e70 // TODO

    extern u32 UnknownFormatValues[MAX_OTHER_USABLE_TEXTURE_FORMAT_COUNT]; // 0x60010f98

    extern u8 RendererFogAlphas[MAX_OUTPUT_FOG_ALPHA_COUNT]; // 0x600128b0

    extern Renderer::RendererTexture* CurrentRendererTexture; // 0x60012a00

    extern RendererModule::RendererModuleDescriptor ModuleDescriptor; // 0x60012010
    extern RendererModule::RendererModuleDescriptorDeviceCapabilities ModuleDescriptorDeviceCapabilities[MAX_RENDERER_MODULE_DEVICE_CAPABILITIES_COUNT]; // 0x60019e00
}