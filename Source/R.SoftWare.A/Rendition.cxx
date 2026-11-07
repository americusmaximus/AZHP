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

// NOTE. Pixel Adjustment logic in the original renderer differs from one here.
// The original logic performs double-precision addition of a horizontal pixel coordinate
// with value of 1.0307922E+11, then reads the low 4 bytes as if it is an integer.
// Adjustment up then adds 0xFFFF and shifts the new value to the right by 16.
// Adjustment down decreases the lower 4 bytes of a double by 1 and then shifts to the right by 16.
// I couldn't find an elegant way to implement the same without using inline assembly,
// therefore, quick and dirty aproach will serve just fine, for now.

#define ADJUSTPIXELUP(X)    (s32)((X + .5f))
#define ADJUSTPIXELDOWN(X)  (s32)((X - .5f))

namespace RendererModule
{
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
            while (y0 <= y1)
            {
                // Apply subpixel adjustments and fixed-point conversion for span bounds
                s32 x0 = ADJUSTPIXELUP(longEdgeX);
                s32 x1 = ADJUSTPIXELDOWN(upperShortEdgeX);

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
                y0++;
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
            while (y0 <= y1)
            {
                s32 x0 = ADJUSTPIXELUP(longEdgeX);
                s32 x1 = ADJUSTPIXELDOWN(lowerShortEdgeX);

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
                y0++;
            }
        }
        else
        {
            // --- Sub-case B: Long edge is on the right (mirror logic) ---
            while (y0 <= y1)
            {
                s32 x0 = ADJUSTPIXELUP(upperShortEdgeX);
                s32 x1 = ADJUSTPIXELDOWN(longEdgeX);

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
                y0++;
            }

            // Transition to lower half for right-side long edge
            shortEdgeSlope = (1.0f / (c->XYZ.Y - end->XYZ.Y)) * (c->XYZ.X - end->XYZ.X);
            y0 = (s32)(end->XYZ.Y + 0.5f);
            y1 = (s32)(c->XYZ.Y - 0.5f);

            if (c->XYZ.Y - (f32)y1 == 0.0f) { y1 = y1 - 1; }

            if (y0 < State.ViewPort.Y0) { y0 = State.ViewPort.Y0; }
            if (State.ViewPort.Y1 < y1) { y1 = State.ViewPort.Y1; }

            f32 lowerShortEdgeX = ((f32)y0 - end->XYZ.Y) * shortEdgeSlope + end->XYZ.X;

            while (y0 <= y1)
            {
                s32 x0 = ADJUSTPIXELUP(lowerShortEdgeX);
                s32 x1 = ADJUSTPIXELDOWN(longEdgeX);

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
                y0++;
            }
        }
    }

    // 0x60033a80
    void SetRendererUnknownFunction(void* lambda)
    {
        UnknownFunction = lambda;
    }
}
