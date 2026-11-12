/*
Copyright (c) 2026 Americus Maximus

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
#include "Renderer.hxx"
#include "RendererValues.hxx"

using namespace Renderer;
using namespace RendererModuleValues;

#define ADJUSTPIXELUP(X)    (s32)(X + 0.5f)
#define ADJUSTPIXELDOWN(X)  (s32)(X - 0.5f)

#define VALUE_103_079_215_104 (f64)+1.0307921510400000E+0011
#define VALUE_206_158_430_208 (f64)+2.0615843020800000E+0011

#define CALCULATE_WITH_103_079_215_104(RESULT, X)               \
    DrawTriangleValue = ((f64)(X) + VALUE_103_079_215_104);     \
    RESULT = ((u32*)&DrawTriangleValue)[0];

#define CALCULATE_WITH_103_079_215_104_UP(RESULT, X)            \
    DrawTriangleValue = ((f64)(X) + VALUE_103_079_215_104);     \
    RESULT = (((u32*)&DrawTriangleValue)[0] + 0xFFFF) >> 16;

#define CALCULATE_WITH_103_079_215_104_DOWN(RESULT, X)          \
    DrawTriangleValue = ((f64)(X) + VALUE_103_079_215_104);     \
    RESULT = (((u32*)&DrawTriangleValue)[0] - 1) >> 16;

#define CALCULATE_WITH_206_158_430_208(RESULT, X)               \
    DrawTriangleValue = ((f64)(X) + VALUE_206_158_430_208);     \
    RESULT = ((u32*)&DrawTriangleValue)[0];

namespace RendererModule
{
    // 0x6000ae90
    void DrawHorizontalLineTextureBlend1(u16* start, const u16* end, u32 x0, u32 x1, u32 u, u32 v)
    {
        for (; start <= end; start = start + 1)
        {
            u32 m1 = State.Textures.Selected.WidthOffset & x0;
            u32 m2 = State.Textures.Selected.UnknownSize & x1;

            x0 += u;
            x1 += v;

            u16 tex = (State.Textures.Selected.Pixels[(m1 + m2) >> 15] >> 2) & (u16)State.Renderer.Colors.Unknown3[4];

            *start = tex
                + ((*start >> 1) & (u16)State.Renderer.Colors.Unknown1[8])
                + ((*start >> 2) & (u16)State.Renderer.Colors.Unknown3[4]);
        }
    }

    // 0x6000af20
    void DrawTriangleTextureBlend1(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        // Local vertex pointers for sorting (Top, Mid, Bottom)
        RTLVX* vTop = a;
        RTLVX* vMid = b;
        RTLVX* vBottom = c;

        // -------------------------------------------------------------
        // STEP 1: Sort Vertices Vertically by Y-Coordinate (Ascending)
        // -------------------------------------------------------------
        if (vMid->XYZ.Y < vTop->XYZ.Y)
        {
            State.Vertexes.Start = vTop;
            RTLVX* temp = vTop; vTop = vMid; vMid = temp;
        }

        if (vBottom->XYZ.Y < vTop->XYZ.Y)
        {
            State.Vertexes.Start = vTop;
            RTLVX* temp = vTop; vTop = vBottom; vBottom = temp;
        }

        if (vBottom->XYZ.Y < vMid->XYZ.Y)
        {
            State.Vertexes.Start = vMid;
            RTLVX* temp = vMid; vMid = vBottom; vBottom = temp;
        }

        // -------------------------------------------------------------
        // STEP 2: Compute Edge Slopes and Total Heights
        // -------------------------------------------------------------

        f32 totalHeight = vBottom->XYZ.Y - vTop->XYZ.Y;
        f32 invHeightTotal = 1.0f / totalHeight;

        f32 dyTopMid = vMid->XYZ.Y - vTop->XYZ.Y;
        f32 invDyTopMid = 1.0f / dyTopMid;

        f32 dxdyLong = (vBottom->XYZ.X - vTop->XYZ.X) * invHeightTotal;
        f32 dxdyShortUpper = (vMid->XYZ.X - vTop->XYZ.X) * invDyTopMid;
        f32 xLongAtMid = dxdyLong * dyTopMid + vTop->XYZ.X;

        // -------------------------------------------------------------
        // STEP 3: Y-Viewport Clipping & Scanline Range Setup
        // -------------------------------------------------------------
        s32 scanlineY = ADJUSTPIXELUP(vTop->XYZ.Y);
        s32 scanlineMidY = ADJUSTPIXELDOWN(vMid->XYZ.Y);

        if (vMid->XYZ.Y - (f32)scanlineMidY == 0.0f) { scanlineMidY--; }

        if (scanlineY < State.ViewPort.Y0) { scanlineY = State.ViewPort.Y0; }
        if (State.ViewPort.Y1 < scanlineMidY) { scanlineMidY = State.ViewPort.Y1; }

        f32 yOffsetFromTop = (f32)scanlineY - vTop->XYZ.Y;
        addr surface = (addr)State.Renderer.Surface.Surface + RendererSurfaceStride * scanlineY;

        // -------------------------------------------------------------
        // STEP 4: Texture UV Gradients Setup
        // -------------------------------------------------------------
        f32 du_dy = (vBottom->UV.X - vTop->UV.X) * invHeightTotal;
        f32 dv_dy = (vBottom->UV.Y - vTop->UV.Y) * invHeightTotal;

        f32 uAtMid = du_dy * dyTopMid + vTop->UV.X;
        f32 vAtMid = dv_dy * dyTopMid + vTop->UV.Y;

        f32 invDeltaX = 1.0f / (xLongAtMid - vMid->XYZ.X);
        f32 xShortUpper = yOffsetFromTop * dxdyShortUpper + vTop->XYZ.X;
        f32 xLong = yOffsetFromTop * dxdyLong + vTop->XYZ.X;

        const f32 texWidth = State.Textures.Selected.WidthAsFloat;
        const f32 texWidthSq = State.Textures.Selected.WidthSquaredAsFloat;

        // Precompute texture gradient constants for span drawer
        CALCULATE_WITH_206_158_430_208(State.Textures.U, (uAtMid - vMid->UV.X) * invDeltaX * texWidth);
        CALCULATE_WITH_206_158_430_208(State.Textures.V, (vAtMid - vMid->UV.Y) * invDeltaX * texWidthSq);

        // -------------------------------------------------------------
        // STEP 5: PHASE 1 - Upper Half Rasterization (Top to Middle)
        // -------------------------------------------------------------
        if (xLongAtMid <= vMid->XYZ.X)
        {
            f32 scaledDuDy = du_dy * texWidth;
            f32 scaledDvDy = dv_dy * texWidthSq;
            f32 currentU = yOffsetFromTop * scaledDuDy + vTop->UV.X * texWidth;
            f32 currentV = yOffsetFromTop * scaledDvDy + vTop->UV.Y * texWidthSq;

            for (; scanlineY <= scanlineMidY; scanlineY++)
            {
                CALCULATE_WITH_103_079_215_104_UP(s32 spanStartX, xLong);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 spanEndX, xShortUpper);

                // X Viewport Clipping
                if (spanStartX < State.ViewPort.X0) { spanStartX = State.ViewPort.X0; }
                if (State.ViewPort.X1 < spanEndX) { spanEndX = State.ViewPort.X1; }

                // Draw horizontal pixel span with properly interpolated UVs
                const addr ls = surface + spanStartX * sizeof(u16);
                const addr le = surface + spanEndX * sizeof(u16);

                CALCULATE_WITH_206_158_430_208(u32 x0, currentU);
                CALCULATE_WITH_206_158_430_208(u32 y0, currentV);

                DrawHorizontalLineTextureBlend1((u16*)ls, (u16*)le, x0, y0, State.Textures.U, State.Textures.V);

                // Step edge interpolators and texture coordinates
                xShortUpper += dxdyShortUpper;
                xLong += dxdyLong;
                currentU += scaledDuDy;
                currentV += scaledDvDy;

                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // -------------------------------------------------------------
            // STEP 6: PHASE 2 - Lower Half Rasterization (Middle to Bottom)
            // -------------------------------------------------------------
            f32 dyBottomMid = vBottom->XYZ.Y - vMid->XYZ.Y;
            f32 dxdyShortLower = (dyBottomMid != 0.0f) ? (vBottom->XYZ.X - vMid->XYZ.X) / dyBottomMid : 0.0f;

            scanlineY = ADJUSTPIXELUP(vMid->XYZ.Y);
            s32 scanlineBottomY = ADJUSTPIXELDOWN(vBottom->XYZ.Y);

            if (vBottom->XYZ.Y - (f32)scanlineBottomY == 0.0f) { scanlineBottomY--; }

            if (scanlineY < State.ViewPort.Y0) { scanlineY = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < scanlineBottomY) { scanlineBottomY = State.ViewPort.Y1; }

            f32 dyFromMid = (f32)scanlineY - vMid->XYZ.Y;
            f32 xShortLower = dyFromMid * dxdyShortLower + vMid->XYZ.X;

            currentU = dyFromMid * scaledDuDy + uAtMid * texWidth;
            currentV = dyFromMid * scaledDvDy + vAtMid * texWidthSq;

            for (; scanlineY <= scanlineBottomY; scanlineY++)
            {
                CALCULATE_WITH_103_079_215_104_UP(s32 spanStartX, xLong);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 spanEndX, xShortLower);

                // X Viewport Clipping
                if (spanStartX < State.ViewPort.X0) { spanStartX = State.ViewPort.X0; }
                if (State.ViewPort.X1 < spanEndX) { spanEndX = State.ViewPort.X1; }

                // Draw horizontal pixel span with properly interpolated UVs
                const addr ls = surface + spanStartX * sizeof(u16);
                const addr le = surface + spanEndX * sizeof(u16);

                CALCULATE_WITH_206_158_430_208(u32 x0, currentU);
                CALCULATE_WITH_206_158_430_208(u32 y0, currentV);

                DrawHorizontalLineTextureBlend1((u16*)ls, (u16*)le, x0, y0, State.Textures.U, State.Textures.V);

                xShortLower += dxdyShortLower;
                xLong += dxdyLong;
                currentU += scaledDuDy;
                currentV += scaledDvDy;

                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
        else
        {
            // --- Sub-case B: Short edge on left, Long edge on right (Alternate Orientation) ---
            f32 du_dx_step = (vMid->UV.X - vTop->UV.X) * invDyTopMid * texWidth;
            f32 dv_dx_step = (vMid->UV.Y - vTop->UV.Y) * invDyTopMid * texWidthSq;

            f32 currentU = yOffsetFromTop * du_dx_step + vTop->UV.X * texWidth;
            f32 currentV = yOffsetFromTop * dv_dx_step + vTop->UV.Y * texWidthSq;

            for (; scanlineY <= scanlineMidY; scanlineY++)
            {
                CALCULATE_WITH_103_079_215_104_UP(s32 spanStartX, xShortUpper);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 spanEndX, xLong);

                // X Viewport Clipping
                if (spanStartX < State.ViewPort.X0) { spanStartX = State.ViewPort.X0; }
                if (State.ViewPort.X1 < spanEndX) { spanEndX = State.ViewPort.X1; }

                // Draw horizontal pixel span with properly interpolated UVs
                const addr ls = surface + spanStartX * sizeof(u16);
                const addr le = surface + spanEndX * sizeof(u16);

                CALCULATE_WITH_206_158_430_208(u32 x0, currentU);
                CALCULATE_WITH_206_158_430_208(u32 y0, currentV);

                DrawHorizontalLineTextureBlend1((u16*)ls, (u16*)le, x0, y0, State.Textures.U, State.Textures.V);

                xShortUpper += dxdyShortUpper;
                xLong += dxdyLong;
                currentU += du_dx_step;
                currentV += dv_dx_step;

                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // --- PHASE 2 (Sub-case B): Middle to Bottom ---
            f32 dyBottomMid = vBottom->XYZ.Y - vMid->XYZ.Y;
            f32 invDyBottomMid = (dyBottomMid != 0.0f) ? (1.0f / dyBottomMid) : 0.0f;
            f32 dxdyShortLower = (vBottom->XYZ.X - vMid->XYZ.X) * invDyBottomMid;

            f32 du_lower_step = (vBottom->UV.X - vMid->UV.X) * invDyBottomMid * texWidth;
            f32 dv_lower_step = (vBottom->UV.Y - vMid->UV.Y) * invDyBottomMid * texWidthSq;

            scanlineY = ADJUSTPIXELUP(vMid->XYZ.Y);
            s32 scanlineBottomY = ADJUSTPIXELDOWN(vBottom->XYZ.Y);

            if (vBottom->XYZ.Y - (f32)scanlineBottomY == 0.0f) { scanlineBottomY--; }
            if (scanlineY < State.ViewPort.Y0) { scanlineY = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < scanlineBottomY) { scanlineBottomY = State.ViewPort.Y1; }

            f32 dyFromMid = (f32)scanlineY - vMid->XYZ.Y;
            f32 xShortLower = dyFromMid * dxdyShortLower + vMid->XYZ.X;

            currentU = dyFromMid * du_lower_step + uAtMid * texWidth;
            currentV = dyFromMid * dv_lower_step + vAtMid * texWidthSq;

            for (; scanlineY <= scanlineBottomY; scanlineY++)
            {
                CALCULATE_WITH_103_079_215_104_UP(s32 spanStartX, xShortLower);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 spanEndX, xLong);

                // X Viewport Clipping
                if (spanStartX < State.ViewPort.X0) { spanStartX = State.ViewPort.X0; }
                if (State.ViewPort.X1 < spanEndX) { spanEndX = State.ViewPort.X1; }

                // Draw horizontal pixel span with properly interpolated UVs
                const addr ls = surface + spanStartX * sizeof(u16);
                const addr le = surface + spanEndX * sizeof(u16);

                CALCULATE_WITH_206_158_430_208(u32 x0, currentU);
                CALCULATE_WITH_206_158_430_208(u32 y0, currentV);

                DrawHorizontalLineTextureBlend1((u16*)ls, (u16*)le, x0, y0, State.Textures.U, State.Textures.V);

                xShortLower += dxdyShortLower;
                xLong += dxdyLong;
                currentU += du_lower_step;
                currentV += dv_lower_step;

                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
    }

    // 0x6002fcb0
    void DrawTriangleNone(RTLVX* a, RTLVX* b, RTLVX* c) { }

    // 0x6002fcc0
    void DrawHorizontalLineCurrentVertexColor(u16* start, const u16* end)
    {
        for (; start <= end; start = start + 1)
        {
            *start = VertexColor;
        }
    }

    // 0x6002fcf0
    void DrawTriangleVertexColor(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        RTLVX* begin = a;
        RTLVX* end = a;

        // -------------------------------------------------------------
        // STEP 1: Vertex Preprocessing / Color Setup
        // -------------------------------------------------------------

        CalculateVertexColor((s32)a->XYZ.X, (s32)b->XYZ.X, a->Color);

        // -------------------------------------------------------------
        // STEP 2: Sort Vertices Vertically by Y-Coordinate (Ascending)
        // We want: 
        //   - 'begin' = Top vertex (smallest Y)
        //   - 'end'   = Middle vertex
        //   - 'c'     = Bottom vertex (largest Y)
        // -------------------------------------------------------------

        if (b->XYZ.Y < begin->XYZ.Y)
        {
            end = b;
            b = begin;
            State.Vertexes.Start = begin;
        }

        begin = end;

        if (c->XYZ.Y < end->XYZ.Y)
        {
            begin = c;
            State.Vertexes.Start = end;
            c = end;
        }

        end = b;

        if (c->XYZ.Y < b->XYZ.Y)
        {
            end = c;
            State.Vertexes.Start = b;
            c = b;
        }

        // -------------------------------------------------------------
        // STEP 3: Compute Edge Slopes (dx/dy)
        // longEdgeSlope = Long edge slope (from Top vertex 'begin' to Bottom vertex 'c')
        // shortEdgeSlope = Upper short edge slope (from Top vertex 'begin' to Middle vertex 'end')
        // -------------------------------------------------------------

        f32 upperHeight = end->XYZ.Y - begin->XYZ.Y;
        f32 longEdgeSlope = (c->XYZ.X - begin->XYZ.X) * (1.0f / (c->XYZ.Y - begin->XYZ.Y));
        f32 shortEdgeSlope = (1.0f / upperHeight) * (end->XYZ.X - begin->XYZ.X);

        // -------------------------------------------------------------
        // STEP 4: Y-Viewport Clipping & Scanline Range Setup
        // Convert float Y coordinates to integer scanline boundaries (y0 to y1)
        // -------------------------------------------------------------

        s32 y0 = (s32)(begin->XYZ.Y + 0.5f);
        s32 y1 = (s32)(end->XYZ.Y - 0.5f);

        if (end->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

        // Clamp scanlines to viewport bounds
        if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
        if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

        // Initialize initial X span positions for the top-to-middle phase
        f32 yOffsetFromTop = (f32)y0 - begin->XYZ.Y;
        f32 upperShortEdgeX = yOffsetFromTop * shortEdgeSlope + begin->XYZ.X; // Upper short edge X
        f32 longEdgeX = yOffsetFromTop * longEdgeSlope + begin->XYZ.X; // Long edge X

        // Calculate initial surface frame buffer pointer offset for row y0
        addr surface = (addr)State.Renderer.Surface.Surface + RendererSurfaceStride * y0;

        // -------------------------------------------------------------
        // STEP 5: PHASE 1 - Top Half Rasterization (Top to Middle Vertex)
        // Checks triangle orientation (left-facing vs right-facing edge)
        // -------------------------------------------------------------
        if (longEdgeSlope * upperHeight + begin->XYZ.X <= end->XYZ.X)
        {
            // --- Sub-case A: Long edge is on the left ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, upperShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineCurrentVertexColor((u16*)ls, (u16*)le);

                // Step edge interpolators down by one scanline
                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // --- Transition to Phase 2: Middle to Bottom Vertex ---
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X); // New lower short edge slope (b -> c)
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 + -1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            // -------------------------------------------------------------
            // STEP 6: PHASE 2 - Bottom Half Rasterization (Middle to Bottom)
            // -------------------------------------------------------------
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, lowerShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineCurrentVertexColor((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
        else
        {
            // --- Sub-case B: Long edge is on the right (mirror logic) ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, upperShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineCurrentVertexColor((u16*)ls, (u16*)le);

                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // Transition to lower half for right-side long edge
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X);
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, lowerShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineCurrentVertexColor((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
    }
    
    // 0x60030200
    void DrawHorizontalLineUnknown2(u16* start, const u16* end)
    {
        for (; start <= end; start = start + 1)
        {
            *start = ((u16)State.Renderer.Colors.Unknown1[8] & (u16)(*start >> 1)) + State.Renderer.Colors.UnknownValue1;
        }
    }

    // 0x60030250
    void DrawTriangleUnknownMask2(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        RTLVX* begin = a;
        RTLVX* end = a;

        // -------------------------------------------------------------
        // STEP 1: Vertex Preprocessing / Color Setup
        // -------------------------------------------------------------

        CalculateVertexColor((s32)a->XYZ.X, (s32)b->XYZ.X, a->Color);

        // -------------------------------------------------------------
        // STEP 2: Sort Vertices Vertically by Y-Coordinate (Ascending)
        // We want: 
        //   - 'begin' = Top vertex (smallest Y)
        //   - 'end'   = Middle vertex
        //   - 'c'     = Bottom vertex (largest Y)
        // -------------------------------------------------------------

        if (b->XYZ.Y < begin->XYZ.Y)
        {
            end = b;
            b = begin;
            State.Vertexes.Start = begin;
        }

        begin = end;

        if (c->XYZ.Y < end->XYZ.Y)
        {
            begin = c;
            State.Vertexes.Start = end;
            c = end;
        }

        end = b;

        if (c->XYZ.Y < b->XYZ.Y)
        {
            end = c;
            State.Vertexes.Start = b;
            c = b;
        }

        // -------------------------------------------------------------
        // STEP 3: Compute Edge Slopes (dx/dy)
        // longEdgeSlope = Long edge slope (from Top vertex 'begin' to Bottom vertex 'c')
        // shortEdgeSlope = Upper short edge slope (from Top vertex 'begin' to Middle vertex 'end')
        // -------------------------------------------------------------

        f32 upperHeight = end->XYZ.Y - begin->XYZ.Y;
        f32 longEdgeSlope = (c->XYZ.X - begin->XYZ.X) * (1.0f / (c->XYZ.Y - begin->XYZ.Y));
        f32 shortEdgeSlope = (1.0f / upperHeight) * (end->XYZ.X - begin->XYZ.X);

        // -------------------------------------------------------------
        // STEP 4: Y-Viewport Clipping & Scanline Range Setup
        // Convert float Y coordinates to integer scanline boundaries (y0 to y1)
        // -------------------------------------------------------------

        s32 y0 = (s32)(begin->XYZ.Y + 0.5f);
        s32 y1 = (s32)(end->XYZ.Y - 0.5f);

        if (end->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

        // Clamp scanlines to viewport bounds
        if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
        if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

        // Initialize initial X span positions for the top-to-middle phase
        f32 yOffsetFromTop = (f32)y0 - begin->XYZ.Y;
        f32 upperShortEdgeX = yOffsetFromTop * shortEdgeSlope + begin->XYZ.X; // Upper short edge X
        f32 longEdgeX = yOffsetFromTop * longEdgeSlope + begin->XYZ.X; // Long edge X

        // Calculate initial surface frame buffer pointer offset for row y0
        addr surface = (addr)State.Renderer.Surface.Surface + RendererSurfaceStride * y0;

        // -------------------------------------------------------------
        // STEP 5: PHASE 1 - Top Half Rasterization (Top to Middle Vertex)
        // Checks triangle orientation (left-facing vs right-facing edge)
        // -------------------------------------------------------------
        if (longEdgeSlope * upperHeight + begin->XYZ.X <= end->XYZ.X)
        {
            // --- Sub-case A: Long edge is on the left ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, upperShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown2((u16*)ls, (u16*)le);

                // Step edge interpolators down by one scanline
                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // --- Transition to Phase 2: Middle to Bottom Vertex ---
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X); // New lower short edge slope (b -> c)
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 + -1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            // -------------------------------------------------------------
            // STEP 6: PHASE 2 - Bottom Half Rasterization (Middle to Bottom)
            // -------------------------------------------------------------
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, lowerShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown2((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
        else
        {
            // --- Sub-case B: Long edge is on the right (mirror logic) ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, upperShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown2((u16*)ls, (u16*)le);

                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // Transition to lower half for right-side long edge
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X);
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, lowerShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown2((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
    }

    // 0x60030760
    void DrawHorizontalLineUnknown3(u16* start, const u16* end)
    {
        for (; start <= end; start = start + 1)
        {
            *start = ((u16)(*start >> 2) & (u16)State.Renderer.Colors.Unknown3[4]) +
                ((u16)(*start >> 1) & (u16)State.Renderer.Colors.Unknown1[8]) + State.Renderer.Colors.UnknownValue2;
        }
    }

    // 0x600307b0
    void DrawTriangleUnknownMask3(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        RTLVX* begin = a;
        RTLVX* end = a;

        // -------------------------------------------------------------
        // STEP 1: Vertex Preprocessing / Color Setup
        // -------------------------------------------------------------

        CalculateVertexColor((s32)a->XYZ.X, (s32)b->XYZ.X, a->Color);

        // -------------------------------------------------------------
        // STEP 2: Sort Vertices Vertically by Y-Coordinate (Ascending)
        // We want: 
        //   - 'begin' = Top vertex (smallest Y)
        //   - 'end'   = Middle vertex
        //   - 'c'     = Bottom vertex (largest Y)
        // -------------------------------------------------------------

        if (b->XYZ.Y < begin->XYZ.Y)
        {
            end = b;
            b = begin;
            State.Vertexes.Start = begin;
        }

        begin = end;

        if (c->XYZ.Y < end->XYZ.Y)
        {
            begin = c;
            State.Vertexes.Start = end;
            c = end;
        }

        end = b;

        if (c->XYZ.Y < b->XYZ.Y)
        {
            end = c;
            State.Vertexes.Start = b;
            c = b;
        }

        // -------------------------------------------------------------
        // STEP 3: Compute Edge Slopes (dx/dy)
        // longEdgeSlope = Long edge slope (from Top vertex 'begin' to Bottom vertex 'c')
        // shortEdgeSlope = Upper short edge slope (from Top vertex 'begin' to Middle vertex 'end')
        // -------------------------------------------------------------

        f32 upperHeight = end->XYZ.Y - begin->XYZ.Y;
        f32 longEdgeSlope = (c->XYZ.X - begin->XYZ.X) * (1.0f / (c->XYZ.Y - begin->XYZ.Y));
        f32 shortEdgeSlope = (1.0f / upperHeight) * (end->XYZ.X - begin->XYZ.X);

        // -------------------------------------------------------------
        // STEP 4: Y-Viewport Clipping & Scanline Range Setup
        // Convert float Y coordinates to integer scanline boundaries (y0 to y1)
        // -------------------------------------------------------------

        s32 y0 = (s32)(begin->XYZ.Y + 0.5f);
        s32 y1 = (s32)(end->XYZ.Y - 0.5f);

        if (end->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

        // Clamp scanlines to viewport bounds
        if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
        if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

        // Initialize initial X span positions for the top-to-middle phase
        f32 yOffsetFromTop = (f32)y0 - begin->XYZ.Y;
        f32 upperShortEdgeX = yOffsetFromTop * shortEdgeSlope + begin->XYZ.X; // Upper short edge X
        f32 longEdgeX = yOffsetFromTop * longEdgeSlope + begin->XYZ.X; // Long edge X

        // Calculate initial surface frame buffer pointer offset for row y0
        addr surface = (addr)State.Renderer.Surface.Surface + RendererSurfaceStride * y0;

        // -------------------------------------------------------------
        // STEP 5: PHASE 1 - Top Half Rasterization (Top to Middle Vertex)
        // Checks triangle orientation (left-facing vs right-facing edge)
        // -------------------------------------------------------------
        if (longEdgeSlope * upperHeight + begin->XYZ.X <= end->XYZ.X)
        {
            // --- Sub-case A: Long edge is on the left ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, upperShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown3((u16*)ls, (u16*)le);

                // Step edge interpolators down by one scanline
                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // --- Transition to Phase 2: Middle to Bottom Vertex ---
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X); // New lower short edge slope (b -> c)
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 + -1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            // -------------------------------------------------------------
            // STEP 6: PHASE 2 - Bottom Half Rasterization (Middle to Bottom)
            // -------------------------------------------------------------
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, lowerShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown3((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
        else
        {
            // --- Sub-case B: Long edge is on the right (mirror logic) ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, upperShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown3((u16*)ls, (u16*)le);

                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // Transition to lower half for right-side long edge
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X);
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, lowerShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown3((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
    }

    // 0x60030cc0
    void DrawHorizontalLineUnknown1(u16* start, const u16* end)
    {
        for (; start <= end; start = start + 1)
        {
            *start = ((u16)State.Renderer.Colors.Unknown3[4] & (u16)(*start >> 2)) + State.Renderer.Colors.UnknownValue3;
        }
    }

    // 0x60030d10
    void DrawTriangleUnknownMask1(RTLVX* a, RTLVX* b, RTLVX* c)
    {
        RTLVX* begin = a;
        RTLVX* end = a;

        // -------------------------------------------------------------
        // STEP 1: Vertex Preprocessing / Color Setup
        // -------------------------------------------------------------

        CalculateVertexColor((s32)a->XYZ.X, (s32)b->XYZ.X, a->Color);

        // -------------------------------------------------------------
        // STEP 2: Sort Vertices Vertically by Y-Coordinate (Ascending)
        // We want: 
        //   - 'begin' = Top vertex (smallest Y)
        //   - 'end'   = Middle vertex
        //   - 'c'     = Bottom vertex (largest Y)
        // -------------------------------------------------------------

        if (b->XYZ.Y < begin->XYZ.Y)
        {
            end = b;
            b = begin;
            State.Vertexes.Start = begin;
        }

        begin = end;

        if (c->XYZ.Y < end->XYZ.Y)
        {
            begin = c;
            State.Vertexes.Start = end;
            c = end;
        }

        end = b;

        if (c->XYZ.Y < b->XYZ.Y)
        {
            end = c;
            State.Vertexes.Start = b;
            c = b;
        }

        // -------------------------------------------------------------
        // STEP 3: Compute Edge Slopes (dx/dy)
        // longEdgeSlope = Long edge slope (from Top vertex 'begin' to Bottom vertex 'c')
        // shortEdgeSlope = Upper short edge slope (from Top vertex 'begin' to Middle vertex 'end')
        // -------------------------------------------------------------

        f32 upperHeight = end->XYZ.Y - begin->XYZ.Y;
        f32 longEdgeSlope = (c->XYZ.X - begin->XYZ.X) * (1.0f / (c->XYZ.Y - begin->XYZ.Y));
        f32 shortEdgeSlope = (1.0f / upperHeight) * (end->XYZ.X - begin->XYZ.X);

        // -------------------------------------------------------------
        // STEP 4: Y-Viewport Clipping & Scanline Range Setup
        // Convert float Y coordinates to integer scanline boundaries (y0 to y1)
        // -------------------------------------------------------------

        s32 y0 = (s32)(begin->XYZ.Y + 0.5f);
        s32 y1 = (s32)(end->XYZ.Y - 0.5f);

        if (end->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

        // Clamp scanlines to viewport bounds
        if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
        if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

        // Initialize initial X span positions for the top-to-middle phase
        f32 yOffsetFromTop = (f32)y0 - begin->XYZ.Y;
        f32 upperShortEdgeX = yOffsetFromTop * shortEdgeSlope + begin->XYZ.X; // Upper short edge X
        f32 longEdgeX = yOffsetFromTop * longEdgeSlope + begin->XYZ.X; // Long edge X

        // Calculate initial surface frame buffer pointer offset for row y0
        addr surface = (addr)State.Renderer.Surface.Surface + RendererSurfaceStride * y0;

        // -------------------------------------------------------------
        // STEP 5: PHASE 1 - Top Half Rasterization (Top to Middle Vertex)
        // Checks triangle orientation (left-facing vs right-facing edge)
        // -------------------------------------------------------------
        if (longEdgeSlope * upperHeight + begin->XYZ.X <= end->XYZ.X)
        {
            // --- Sub-case A: Long edge is on the left ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, upperShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown1((u16*)ls, (u16*)le);

                // Step edge interpolators down by one scanline
                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // --- Transition to Phase 2: Middle to Bottom Vertex ---
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X); // New lower short edge slope (b -> c)
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 + -1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            // -------------------------------------------------------------
            // STEP 6: PHASE 2 - Bottom Half Rasterization (Middle to Bottom)
            // -------------------------------------------------------------
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, longEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, lowerShortEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown1((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
        else
        {
            // --- Sub-case B: Long edge is on the right (mirror logic) ---
            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, upperShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown1((u16*)ls, (u16*)le);

                upperShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }

            // Transition to lower half for right-side long edge
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X);
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            for (; y0 <= y1; y0++)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                CALCULATE_WITH_103_079_215_104_UP(s32 x0, lowerShortEdgeX);
                CALCULATE_WITH_103_079_215_104_DOWN(s32 x1, longEdgeX);

                // Horizontal viewport clipping (X0 to X1)
                if (x0 < State.ViewPort.X0) { x0 = State.ViewPort.X0; }
                if (State.ViewPort.X1 < x1) { x1 = State.ViewPort.X1; }

                // Draw the horizontal span across the framebuffer surface
                const addr ls = surface + x0 * sizeof(u16);
                const addr le = surface + x1 * sizeof(u16);
                DrawHorizontalLineUnknown1((u16*)ls, (u16*)le);

                lowerShortEdgeX += shortEdgeSlope;
                longEdgeX += longEdgeSlope;

                // Advance surface pointer by 1 row stride
                surface += State.Renderer.Settings.Width * sizeof(u16);
            }
        }
    }

    // 0x60033a80
    void SetRendererUnknownFunction(void* lambda)
    {
        UnknownFunction = lambda;
    }
}
