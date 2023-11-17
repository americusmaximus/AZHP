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

#include "Logger.hxx"
#include "Native.Basic.hxx"

#include <stdarg.h>
#include <stdio.h>

#ifdef __WATCOMC__
#define vsnprintf_s _vsnprintf
#endif

#if MSC_VER <= 1200
#define vsnprintf_s _vsnprintf
#endif

namespace Logger
{
    LoggerContainer LoggerState;

    // 0x00401010
    void Error(const char* format, ...)
    {
        if (LoggerState.IsActive) { return; }

        LoggerState.IsActive = TRUE;

        if (format == NULL) { LoggerState.Message[0] = NULL; }
        else
        {
            va_list args;
            va_start(args, format);
            vsnprintf_s(LoggerState.Message, MAX_LOGGER_MESSAGE_BUFFER_SIZE, format, args);
            va_end(args);
        }

        if (LoggerState.IsDebugMessageActive)
        {
            OutputDebugStringA(LoggerState.Message);

            if (LoggerState.IsDebugStopActive) { DebugBreak(); }
        }

        {
            // TODO NOT IMPLEMENTED

            ShowCursor(TRUE);

            if (MessageBoxA(NULL, LoggerState.Message, "Abort Message", MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK)
            {
                // TODO NOT IMPLEMENTED
            }

            // TODO NOT IMPLEMENTED
        }

        LoggerState.IsActive = FALSE;
    }

    // 0x004df6c0
    void Message(const char* format, ...)
    {
        // TODO NOT IMPLEMENTED
    }
}