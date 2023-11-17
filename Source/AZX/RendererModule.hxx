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

#include "RendererModule.Import.hxx"

namespace RendererModule
{
    struct RendererModuleContainer
    {
        struct
        {
            HMODULE* _Handle; // 0x00564a6c
        } Module;

        RENDERERACQUIREDESCRIPTOR* _AcquireDescriptor; // 0x009ef980
        RENDERERCLEARGAMEWINDOW* _ClearGameWindow; // 0x009ef970
        RENDERERCLIPGAMEWINDOW* _ClipGameWindow; // 0x009ef92c
        RENDERERDRAWLINE* _DrawLine; // 0x009ef928
        RENDERERDRAWLINEMESH* _DrawLineMesh; // 0x009ef90c
        RENDERERDRAWLINESTRIP* _DrawLineStrip; // 0x009ef908
        RENDERERDRAWPOINT* _DrawPoint; // 0x009ef910
        RENDERERDRAWPOINTSTRIP* _DrawPointStrip; // 0x009ef904
        RENDERERDRAWPOINTMESH* _DrawPointMesh; // 0x009ef900
        RENDERERDRAWQUAD* _DrawQuad; // 0x009ef94c
        RENDERERDRAWQUADMESH* _DrawQuadMesh; // 0x009ef930
        RENDERERDRAWTRIANGLE* _DrawTriangle; // 0x009ef974
        RENDERERDRAWTRIANGLEFAN* _DrawTriangleFan; // 0x009ef960
        RENDERERDRAWTRIANGLEMESH* _DrawTriangleMesh; // 0x009ef924
        RENDERERDRAWTRIANGLESTRIP* _DrawTriangleStrip; // 0x009ef91c
        RENDERERFLUSHGAMEWINDOW* _FlushGameWindow; // 0x009ef950
        RENDERERIDLE* _Idle; // 0x009ef93c
        RENDERERINIT* _Init; // 0x009ef978
        RENDERERIS* _Is; // 0x009ef984
        RENDERERLOCKGAMEWINDOW* _LockGameWindow; // 0x009ef954
        RENDERERTOGGLEGAMEWINDOW* _ToggleGameWindow; // 0x009ef940
        RENDERERREADRECTANGLE* _ReadRectangle; // 0x009ef920
        RENDERERRESTOREGAMEWINDOW* _RestoreGameWindow; // 0x009ef97c
        RENDERERSELECTDEVICE* _SelectDevice; // 0x009ef944
        RENDERERSELECTSTATE* _SelectState; // 0x009ef96c
        RENDERERSELECTTEXTURE* _SelectTexture; // 0x009ef934
        RENDERERSELECTVIDEOMODE* _SelectVideoMode; // 0x009ef958
        RENDERERSYNCGAMEWINDOW* _SyncGameWindow; // 0x009ef948
        RENDERERALLOCATETEXTURE* _AllocateTexture; // 0x009ef968
        RENDERERRESETTEXTURES* _ResetTextures; // 0x009ef918
        RENDERERUPDATETEXTURE* _UpdateTexture; // 0x009ef964
        RENDERERUNLOCKGAMEWINDOW* _UnlockGameWindow; // 0x009ef95c
        RENDERERSELECTGAMEWINDOW* _SelectGameWindow; // 0x009ef938
        RENDERERWRITERECTANGLE* _WriteRectangle; // 0x009ef914
    };

    extern RendererModuleContainer RendererModuleState;

    BOOL InitializeRendererModule(const char* name);
}