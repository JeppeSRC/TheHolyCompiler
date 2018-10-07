/*
MIT License

Copyright (c) 2018 Jesper Hammarström

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

#include <Windows.h>
#include "string.h"

#ifdef THC_DEBUG
#define THC_ASSERT(x) if (!(x)) { printf("Assertion Failed (%s) -> File: \"%s\" Function: \"%s\" Line: %u\n", ##x, __FILE__, __FUNCSIG__, __LINE__); *((int*)0x0) = 0; }
#else
#define THC_ASSERT(x) 
#endif

namespace thc {
namespace utils {

enum class LogLevel {
	Debug,
	Info,
	Warning,
	Error
};

typedef void(*LogCallback)(LogLevel, const String& message);

class Log {
private:
	static HANDLE logHandle;

	static LogCallback logCallback;

	static void LogInternal(LogLevel level, const char* const message, va_list list);
public:
	static void Info(const char* const message...);
	static void Debug(const char* const message...);
	static void Warning(const char* const message...);
	static void Error(const char* const message...);

	static void SetOutputHandle(HANDLE logHandle);
	static void SetLogCallback(LogCallback logCallback);
};

}
}
