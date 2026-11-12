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
