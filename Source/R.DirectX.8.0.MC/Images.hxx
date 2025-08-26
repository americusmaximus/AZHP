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

#include "Basic.hxx"
#include "DirectX.hxx"

#define IMAGE_RELEASE_NONE 0
#define IMAGE_RELEASE_DISPOSE 1

#define IMAGE_CONTAINER_OPTIONS_NONE                0x00000000

#define IMAGE_CONTAINER_OPTIONS_COLOR               0x00000001
#define IMAGE_CONTAINER_OPTIONS_BUMP_MAP            0x00000002
#define IMAGE_CONTAINER_OPTIONS_UNKNOWN_4           0x00000004

#define IMAGE_CONTAINER_OPTIONS_LOW_VALUE_MASK      0x0000FFFF
#define IMAGE_CONTAINER_OPTIONS_HIGH_VALUE_MASK     0xFFF00000

#define IMAGE_CONTAINER_OPTIONS_GRADIENT            0x00080000

#define IMAGE_CONTAINER_OPTIONS_INVALID             0xFFFFFFFF

#define MAX_IMAGE_PALETTE_VALUES_COUNT 256

#define MAX_IMAGE_COLOR_MODIFIER_VALUES_COUNT 32

#define IMAGE_DXT_DIMENSION 4
#define IMAGE_DXT_DIMENSION_SEGMENT (IMAGE_DXT_DIMENSION * IMAGE_DXT_DIMENSION)

namespace Images
{
    enum ImageFormatDescriptorType
    {
        IMAGE_FORMAT_DESCRIPTOR_TYPE_RGB = 0,
        IMAGE_FORMAT_DESCRIPTOR_TYPE_PALETTE = 1,
        IMAGE_FORMAT_DESCRIPTOR_TYPE_LUMINANCE = 2,
        IMAGE_FORMAT_DESCRIPTOR_TYPE_UV = 3,
        IMAGE_FORMAT_DESCRIPTOR_TYPE_FORCE_DWORD = 0x7FFFFFFF
    };

    struct ImageFormatDescriptor
    {
        D3DFORMAT Format;

        ImageFormatDescriptorType Type;

        u32 Bits;

        u32 Luminance;
        union
        {
            u32 Alpha;
            u32 Q;
        };
        union
        {
            u32 Red;
            u32 W;
        };
        union
        {
            u32 Green;
            u32 V;
        };
        union
        {
            u32 Blue;
            u32 U;
        };

        u32 Unk08; // TODO
        u32 Unk09; // TODO
    };



    struct ImageColor
    {
        f32 R;
        f32 G;
        f32 B;
        f32 A;
    };

    struct ImageDimensions
    {
        u32 Left;
        u32 Top;
        u32 Right;
        u32 Bottom;
        u32 Min;
        u32 Max;
    };

    struct ImageContainerArgs
    {
        void* Pixels;
        D3DFORMAT Format;
        u32 Stride;
        u32 AreaStride;
        s32 Unk04; // TODO
        s32 Unk05; // TODO
        s32 Width;
        s32 Height;
        s32 Unk08; // TODO
        s32 Unk09; // TODO
        ImageDimensions Dimensions;
        BOOL IsGradient;
        u32 Color;
        const u8* Palette;
    };

    struct AbstractImage;

    typedef void* (*RELEASEIMAGECALL)(AbstractImage* image, const u32 mode);
    typedef void* (*READIMAGECALL)(AbstractImage* image, const u32 p2, const u32 p3, u32* pixels);
    typedef void* (*WRITEDIMAGECALL)(AbstractImage* image, const u32 p2, const u32 p3, u32* pixels);

    struct ImageCalls
    {
        RELEASEIMAGECALL Release;
        READIMAGECALL Read;
        WRITEDIMAGECALL Write;
    };

    struct AbstractImage
    {
        ImageCalls* Self;
    };

    struct ImageContainer
    {
        AbstractImage* Source;
        AbstractImage* Destination;
        u32 Unk02;
    };

    struct ImageContainerArgs
    {
        void* Pixels;
        D3DFORMAT Format;
        u32 Stride;
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
        s32 Unk13; // TODO
        s32 Unk14; // TODO
        s32 Unk15; // TODO
        s32 Unk16; // TODO
        s32 Unk17; // TODO
        s32 Unk18; // TODO
    };

    // INHERITANCE: AbstractImage
    struct ImageBitMap
    {
        ImageCalls* Self;

        char Unk00[0x1060];
    };

    // INHERITANCE: ImageBitMap
    struct ImageDXT
    {
        ImageCalls* Self;
        D3DFORMAT Format;

        char Unk00[0x10a0];
    };

    // INHERITANCE: ImageBitMap
    struct ImageYUVY
    {
        ImageCalls* Self;

        char Unk00[0x1090];
    };

    void InitializeImageContainer(ImageContainer* img);
    void ReleaseImageContainer(ImageContainer* img);
    AbstractImage* InitializeAbstractImage(ImageContainerArgs* args);
    u32 UpdateImageContainer(ImageContainer* img, ImageContainerArgs* dst, ImageContainerArgs* src, const u32 param_4);

    ImageDXT* InitializeImageDXT(ImageDXT* image, ImageContainerArgs* args);
    void DisposeAbstractImageDXT(ImageDXT* image);
    void ReadImageDXT(ImageDXT* image, const u32 p2, const u32 p3, u32* pixels);
    void WriteImageDXT(ImageDXT* image, const u32 p2, const u32 p3, u32* pixels);
    void* ReleaseAbstractImageDXT(ImageDXT* image, const u32 mode);
    void* ReleaseImageDXT(ImageDXT* image, const u32 mode);
}