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

#include "Renderer.hxx"
#include "RendererValues.hxx"

using namespace RendererModuleValues;

namespace RendererModule
{
    RendererModuleState State;

    // 0x60001830
    void SelectRendererDevice(void)
    {
        if (RendererDeviceIndex < DEFAULT_RENDERER_DEVICE_INDEX
            && (State.Lambdas.Lambdas.AcquireWindow != NULL || State.Window.HWND != NULL))
        {
            const char* value = getenv(RENDERER_MODULE_DISPLAY_ENVIRONEMNT_PROPERTY_NAME);

            SelectDevice(value == NULL ? DEFAULT_RENDERER_DEVICE_INDEX : atoi(value));
        }
    }

    // 0x60001800
    u32 AcquireRendererDeviceCount(void)
    {
        State.Devices.Count = 0;
        State.Device.Identifier = NULL;

        u32 indx = DEFAULT_RENDERER_DEVICE_INDEX;
        DirectDrawEnumerateA(EnumerateRendererDevices, &indx);

        return State.Devices.Count;
    }

    // 0x60001970
    BOOL CALLBACK EnumerateRendererDevices(GUID* uid, LPSTR name, LPSTR description, LPVOID context)
    {
        if (uid == NULL)
        {
            State.Devices.Indexes[State.Devices.Count] = NULL;
        }
        else
        {
            State.Devices.Indexes[State.Devices.Count] = &State.Devices.Identifiers[State.Devices.Count];
            State.Devices.Identifiers[State.Devices.Count] = *uid;
        }

        if (State.Devices.Count == *(u32*)context)
        {
            State.Device.Identifier = State.Devices.Indexes[State.Devices.Count];
        }

        strncpy(State.Devices.Names[State.Devices.Count], name, MAX_DEVICE_NAME_LENGTH);

        // NOTE: Additional extra check to prevent writes outside of the array bounds.
        if (MAX_RENDERER_DEVICE_COUNT <= (State.Devices.Count + 1)) { return FALSE; }

        State.Devices.Count = State.Devices.Count + 1;

        return TRUE;
    }
}