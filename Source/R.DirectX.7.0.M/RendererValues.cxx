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
#include "RendererValues.hxx"

using namespace Renderer;
using namespace RendererModule;

namespace RendererModuleValues
{
    s32 RendererState = DEFAULT_RENDERER_STATE;
    s32 RendererDeviceIndex = INVALID_RENDERER_DEVICE_INDEX;

    u32 RendererVertexType = D3DFVF_TLVERTEX;
    u32 RendererVertexSize = sizeof(RTLVX);

    D3DPRIMITIVETYPE RendererPrimitiveType = D3DPT_TRIANGLELIST;

    u32 RendererVersion = RENDERER_MODULE_VERSION_107;
    u32 RendererLineVertexSize = sizeof(RTLVX);

    u32 RendererIndexSize = RENDERER_MODULE_INDEX_SIZE_4;

    f32 RendererDepthBias;

    s32 RendererDeviceType;

    RendererModule::MinMax MinMax[6] =
    {
        { 0x0,   0x4a },
        { 0x64,  0x6e },
        { 0xc8,  0xd7 },
        { 0x12c, 0x13b },
        { 0x190, 0x19f },
        { 0x2bc, 0x2cb }
    }; // TODO

    RendererModuleDescriptor ModuleDescriptor;

    s32 RendererTextureFormatStates[MAX_USABLE_TEXTURE_FORMAT_COUNT] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1
    }; // TODO
}