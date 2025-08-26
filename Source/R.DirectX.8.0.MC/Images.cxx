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
#include "Images.hxx"
#include "Renderer.hxx"

#include <stdlib.h>

namespace Images
{
    // 0x6001864c
    ImageCalls AbstractImageDXTCalls = { (RELEASEIMAGECALL)&ReleaseAbstractImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x600186dc
    ImageCalls ImageA8P8Self = { TODO, TODO, TODO };

    // 0x60018658
    ImageCalls ImageR8G8B8Self = { TODO, TODO, TODO };

    // 0x60018664
    ImageCalls ImageA8R8G8B8Self = { TODO, TODO, TODO };

    // 0x60018670
    ImageCalls ImageX8R8G8B8Self = { TODO, TODO, TODO };

    // 0x6001867c
    ImageCalls ImageR5G6B5Self = { TODO, TODO, TODO };

    // 0x60018688
    ImageCalls ImageX1R5G5B5Self = { TODO, TODO, TODO };

    // 0x60018694
    ImageCalls ImageA1R5G5B5Self = { TODO, TODO, TODO };

    // 0x600186a0
    ImageCalls ImageA4R4G4B4Self = { TODO, TODO, TODO };

    // 0x600186ac
    ImageCalls ImageR3G3B2Self = { TODO, TODO, TODO };

    // 0x600186b8
    ImageCalls ImageA8Self = { TODO, TODO, TODO };

    // 0x600186c4
    ImageCalls ImageA8R3G3B2Self = { TODO, TODO, TODO };

    // 0x600186d0
    ImageCalls ImageX4R4G4B4Self = { TODO, TODO, TODO };

    // 0x60018724
    ImageCalls ImageL6V5U5Self = { TODO, TODO, TODO };

    // 0x600186e8
    ImageCalls ImageP8Self = { TODO, TODO, TODO };

    // 0x600186f4
    ImageCalls ImageL8Self = { TODO, TODO, TODO };

    // 0x60018700
    ImageCalls ImageA8L8Self = { TODO, TODO, TODO };

    // 0x6001870c
    ImageCalls ImageA4L4Self = { TODO, TODO, TODO };

    // 0x60018718
    ImageCalls ImageV8U8Self = { TODO, TODO, TODO };

    // 0x60018730
    ImageCalls ImageX8L8V8U8Self = { TODO, TODO, TODO };

    // 0x6001873c
    ImageCalls ImageQ8W8V8U8Self = { TODO, TODO, TODO };

    // 0x60018748
    ImageCalls ImageV16U16Self = { TODO, TODO, TODO };

    // 0x60018754
    ImageCalls ImageW11V11U10Self = { TODO, TODO, TODO };

    // 0x60018760
    ImageCalls ImageYUVYSelf = { TODO, TODO, TODO };

    // 0x6001876c
    ImageCalls ImageYUY2Self = { TODO, TODO, TODO };

    // 0x60018778
    ImageCalls ImageDXT1Self = { (RELEASEIMAGECALL)&ReleaseImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x60018784
    ImageCalls ImageDXT2Self = { (RELEASEIMAGECALL)&ReleaseImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x60018790
    ImageCalls ImageDXT3Self = { (RELEASEIMAGECALL)&ReleaseImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x6001879c
    ImageCalls ImageDXT4Self = { (RELEASEIMAGECALL)&ReleaseImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x600187a8
    ImageCalls ImageDXT5Self = { (RELEASEIMAGECALL)&ReleaseImageDXT, (READIMAGECALL)&ReadImageDXT, (WRITEDIMAGECALL)&WriteImageDXT };

    // 0x60009ad0
    void InitializeImageContainer(ImageContainer* img)
    {
        img->Destination = NULL;
        img->Source = NULL;
    }

    // 0x60009ada
    void ReleaseImageContainer(ImageContainer* img)
    {
        if (img->Destination != NULL) { img->Destination->Self->Release(img->Destination, IMAGE_RELEASE_DISPOSE); }
        if (img->Source != NULL) { img->Source->Self->Release(img->Source, IMAGE_RELEASE_DISPOSE); }
    }

    // 0x6000feda
    AbstractImage* InitializeAbstractImage(ImageContainerArgs* args)
    {
        AbstractImage* result = NULL;

        switch (args->Format)
        {
        case D3DFMT_R8G8B8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_24, 1);

                result->Self = &ImageR8G8B8Self;

                return result;
            }

            break;
        }
        case D3DFMT_A8R8G8B8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 1);

                result->Self = &ImageA8R8G8B8Self;

                return result;
            }

            break;
        }
        case D3DFMT_X8R8G8B8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 1);

                result->Self = &ImageX8R8G8B8Self;

                return result;
            }

            break;
        }
        case D3DFMT_R5G6B5:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageR5G6B5Self;

                return result;
            }

            break;
        }
        case D3DFMT_X1R5G5B5:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageX1R5G5B5Self;

                return result;
            }

            break;
        }
        case D3DFMT_A1R5G5B5:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageA1R5G5B5Self;

                return result;
            }

            break;
        }
        case D3DFMT_A4R4G4B4:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageA4R4G4B4Self;

                return result;
            }

            break;
        }
        case D3DFMT_R3G3B2:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_8, 1);

                result->Self = &ImageR3G3B2Self;

                return result;
            }

            break;
        }
        case D3DFMT_A8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_8, 1);

                result->Self = &ImageA8Self;

                return result;
            }

            break;
        }
        case D3DFMT_A8R3G3B2:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageA8R3G3B2Self;

                return result;
            }

            break;
        }
        case D3DFMT_X4R4G4B4:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageX4R4G4B4Self;

                return result;
            }

            break;
        }
        case D3DFMT_A8P8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 2);

                result->Self = &ImageA8P8Self;

                return result;
            }

            break;
        }
        case D3DFMT_P8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_8, 2);

                result->Self = &ImageP8Self;

                return result;
            }

            break;
        }
        case D3DFMT_L8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_8, 1);

                result->Self = &ImageL8Self;

                return result;
            }

            break;
        }
        case D3DFMT_A8L8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 1);

                result->Self = &ImageA8L8Self;

                return result;
            }

            break;
        }
        case D3DFMT_A4L4:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_8, 1);

                result->Self = &ImageA4L4Self;

                return result;
            }

            break;
        }
        case D3DFMT_V8U8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 3);

                result->Self = &ImageV8U8Self;

                return result;
            }

            break;
        }
        case D3DFMT_L6V5U5:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_16, 3);

                result->Self = &ImageL6V5U5Self;

                return result;
            }

            break;
        }
        case D3DFMT_X8L8V8U8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 3);

                result->Self = &ImageX8L8V8U8Self;

                return result;
            }

            break;
        }
        case D3DFMT_Q8W8V8U8:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 3);

                result->Self = &ImageQ8W8V8U8Self;

                return result;
            }

            break;
        }
        case D3DFMT_V16U16:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 3);

                result->Self = &ImageV16U16Self;

                return result;
            }

            break;
        }
        case D3DFMT_W11V11U10:
        {
            result = (AbstractImage*)malloc(sizeof(ImageBitMap));

            if (result != NULL)
            {
                InitializeImageBitMap(result, args, GRAPHICS_BITS_PER_PIXEL_32, 3);

                result->Self = &ImageW11V11U10Self;

                return result;
            }

            break;
        }
        case D3DFMT_DXT1:
        {
            result = (AbstractImage*)malloc(sizeof(ImageDXT));

            if (result != NULL)
            {
                InitializeImageDXT((ImageDXT*)result, args);

                result->Self = &ImageDXT1Self;

                return result;
            }

            break;
        }
        case D3DFMT_DXT2:
        {
            result = (AbstractImage*)malloc(sizeof(ImageDXT));

            if (result != NULL)
            {
                InitializeImageDXT((ImageDXT*)result, args);

                result->Self = &ImageDXT2Self;

                return result;
            }

            break;
        }
        case D3DFMT_DXT3:
        {
            result = (AbstractImage*)malloc(sizeof(ImageDXT));

            if (result != NULL)
            {
                InitializeImageDXT((ImageDXT*)result, args);

                result->Self = &ImageDXT3Self;

                return result;
            }

            break;
        }
        case D3DFMT_DXT4:
        {
            result = (AbstractImage*)malloc(sizeof(ImageDXT));

            if (result != NULL)
            {
                InitializeImageDXT((ImageDXT*)result, args);

                result->Self = &ImageDXT4Self;

                return result;
            }

            break;
        }
        case D3DFMT_DXT5:
        {
            result = (AbstractImage*)malloc(sizeof(ImageDXT));

            if (result != NULL)
            {
                InitializeImageDXT((ImageDXT*)result, args);

                result->Self = &ImageDXT5Self;

                return result;
            }

            break;
        }
        case D3DFMT_UYVY:
        {
            result = (AbstractImage*)malloc(sizeof(ImageYUVY));

            if (result != NULL)
            {
                InitializeImageYUVY(result, args);

                result->Self = &ImageYUVYSelf;

                return result;
            }

            break;
        }
        case D3DFMT_YUY2:
        {
            result = (AbstractImage*)malloc(sizeof(ImageYUVY));

            if (result != NULL)
            {
                InitializeImageYUVY(result, args);

                result->Self = &ImageYUY2Self;

                return result;
            }

            break;
        }
        }

        return result;
    }

    // 0x6000cce0
    u32 UpdateImageContainer(ImageContainer* img, ImageContainerArgs* dst, ImageContainerArgs* src, const u32 param_4)
    {
        img->Destination = NULL;
        img->Source = NULL;
        img->Unk02 = param_4;

        if ((param_4 & 0xffff) == 0 || 5 < (param_4 & 0xffff) || (param_4 & 0xfff00000) != 0) { return 0x8876086c; } // TODO

        dst->Unk16 = param_4 & 0x80000; // TODO

        img->Destination = InitializeAbstractImage(dst);

        if (img->Destination != NULL)
        {
            img->Source = InitializeAbstractImage(src);

            if (img->Source != NULL && TODO)
            {
                if (img->Destination != NULL) { img->Destination->Self->Release(img->Destination, IMAGE_RELEASE_DISPOSE); img->Destination = NULL; }
                if (img->Source != NULL) { img->Source->Self->Release(img->Source, IMAGE_RELEASE_DISPOSE); img->Source = NULL; }

                return D3D_OK;
            }
        }

        if (img->Destination != NULL) { img->Destination->Self->Release(img->Destination, IMAGE_RELEASE_DISPOSE); img->Destination = NULL; }
        if (img->Source != NULL) { img->Source->Self->Release(img->Source, IMAGE_RELEASE_DISPOSE); img->Source = NULL; }

        return DDERR_GENERIC;
    }

    // 0x6000f63b
    ImageDXT* InitializeImageDXT(ImageDXT* image, ImageContainerArgs* args)
    {
        InitializeImageBitMap((ImageBitMap*)image, args, 0, 1);

        image->Self = &AbstractImageDXTCalls;

        TODO
    }

    // 0x60010543
    void* ReleaseImageDXT(ImageDXT* image, const u32 mode)
    {
        DisposeAbstractImageDXT(image);

        if (mode & IMAGE_RELEASE_DISPOSE) { free(image); }

        return image;
    }

    // 0x60010506
    void* ReleaseAbstractImageDXT(ImageDXT* image, const u32 mode)
    {
        DisposeAbstractImageDXT(image);

        if (mode & IMAGE_RELEASE_DISPOSE) { free(image); }

        return image;
    }

    // 0x6000fe86
    void DisposeAbstractImageDXT(ImageDXT* image)
    {
        image->Self = &AbstractImageDXTCalls;

        FUN_6000f752(image);

        for (u32 x = 0; x < 4; x++)
        {
            if (image->field4193_0x1064[x] != NULL) { free(image->field4193_0x1064[x]); }
        }

        FUN_6000cdf2(image);
    }
}