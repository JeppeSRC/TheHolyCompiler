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
#include <core/compiler/parsing/line.h>
#include "string.h"

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

	static void CompilerLog(LogLevel level, const char* filename, int line, int col, const char* message, va_list args);
public:
	static void Info(const char* const message...);
	static void Debug(const char* const message...);
	static void Warning(const char* const message...);
	static void Error(const char* const message...);

	static void CompilerDebug(const char* const filename, int line, int col, const char* const message...);
	static void CompilerWarning(const char* const filename, int line, int col, const char* const message...);
	static void CompilerError(const char* const filename, int line, int col, const char* const message...);

	static void CompilerDebug(const core::parsing::Line& line, int col, const char* const message...);
	static void CompilerWarning(const core::parsing::Line& line, int col, const char* const message...);
	static void CompilerError(const core::parsing::Line& line, int col, const char* const message...);

	static void SetOutputHandle(HANDLE logHandle);
	static void SetLogCallback(LogCallback logCallback);
};

}
}
