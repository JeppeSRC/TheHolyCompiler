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


#define THC_LOG_COLOR_INFO 0b00001111
#define THC_LOG_COLOR_DEBUG 0b00001010
#define THC_LOG_COLOR_WARNING 0b00001110
#define THC_LOG_COLOR_ERROR 0b00001100

#include "log.h"
#include <core/compiler/compiler.h>

namespace thc {
namespace utils {

using namespace core::compiler;
using namespace core::parsing;

HANDLE Log::logHandle = INVALID_HANDLE_VALUE;
LogCallback Log::logCallback = nullptr;

void Log::LogInternal(LogLevel level, const char* const message, va_list list) {
	if (logHandle != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO info;

		GetConsoleScreenBufferInfo(logHandle, &info);
		
		switch (level) {
			case LogLevel::Info:
				SetConsoleTextAttribute(logHandle, THC_LOG_COLOR_INFO);
				printf("INFO: ");
				vprintf(message, list);
				printf("\n");
				break;
			case LogLevel::Debug:
				SetConsoleTextAttribute(logHandle, THC_LOG_COLOR_DEBUG);
				printf("DEBUG: ");
				vprintf(message, list);
				printf("\n");
				break;
			case LogLevel::Warning:
				SetConsoleTextAttribute(logHandle, THC_LOG_COLOR_WARNING);
				printf("WARNING: ");
				vprintf(message, list);
				printf("\n");
				break;
			case LogLevel::Error:
				SetConsoleTextAttribute(logHandle, THC_LOG_COLOR_ERROR);
				printf("ERROR: ");
				vprintf(message, list);
				printf("\n");
				break;
		}

		SetConsoleTextAttribute(logHandle, info.wAttributes);
	}

	if (logCallback) {
		char buffer[4096] = { 0 };
		vsprintf(buffer, message, list);

		logCallback(level, buffer);
	}
}

void Log::Info(const char* const message...) {
	va_list list;
	va_start(list, message);
	LogInternal(LogLevel::Info, message, list);
	va_end(list);
}

void Log::Debug(const char* const message...) {
	va_list list;
	va_start(list, message);
	LogInternal(LogLevel::Debug, message, list);
	va_end(list);
}

void Log::Warning(const char* const message...) {
	va_list list;
	va_start(list, message);
	LogInternal(LogLevel::Warning, message, list);
	va_end(list);
}

void Log::Error(const char* const message...) {
	va_list list;
	va_start(list, message);
	LogInternal(LogLevel::Error, message, list);
	va_end(list);
}

void Log::CompilerLog(LogLevel level, const char* filename, uint64 line, uint64 col, const char* message, va_list args) {
	char buf[2048] = { 0 };

	sprintf(buf, "%s [%llu:%llu] -> %s", filename, line, col, message);

	LogInternal(level, buf, args);
}

void Log::CompilerInfo(const char* const filename, uint64 line, uint64 col, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Info, filename, line, col, message, list);
	va_end(list);
}

void Log::CompilerDebug(const char* const filename, uint64 line, uint64 col, const char* const message...) {
	if (!CompilerOptions::DebugMessages()) return;

	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Debug, filename, line, col, message, list);
	va_end(list);
}

void Log::CompilerWarning(const char* const filename, uint64 line, uint64 col, const char* const message...) {
	if (!CompilerOptions::WarningMessages()) return;
	
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Warning, filename, line, col, message, list);
	va_end(list);
}

void Log::CompilerError(const char* const filename, uint64 line, uint64 col, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Error, filename, line, col, message, list);
	va_end(list);

	if (CompilerOptions::StopOnError()) {
		exit(1);
	}
}

void Log::CompilerInfo(const Line& line, uint64 col, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Info, line.sourceFile.str, line.lineNumber, col, message, list);
	va_end(list);
}

void Log::CompilerDebug(const Line& line, uint64 col, const char* const message...) {
	if (!CompilerOptions::DebugMessages()) return;

	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Debug, line.sourceFile.str, line.lineNumber, col, message, list);
	va_end(list);
}

void Log::CompilerWarning(const Line& line, uint64 col, const char* const message...) {
	if (!CompilerOptions::WarningMessages()) return;

	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Warning, line.sourceFile.str, line.lineNumber, col, message, list);
	va_end(list);
}

void Log::CompilerError(const Line& line, uint64 col, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Error, line.sourceFile.str, line.lineNumber, col, message, list);
	va_end(list);

	if (CompilerOptions::StopOnError()) {
		exit(1);
	}
}


void Log::CompilerInfo(const Token& token, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Info, token.line.sourceFile.str, token.line.lineNumber, token.column, message, list);
	va_end(list);
}

void Log::CompilerDebug(const Token& token, const char* const message...) {
	if (!CompilerOptions::DebugMessages()) return;

	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Debug, token.line.sourceFile.str, token.line.lineNumber, token.column, message, list);
	va_end(list);
}

void Log::CompilerWarning(const Token& token, const char* const message...) {
	if (!CompilerOptions::WarningMessages()) return;

	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Warning, token.line.sourceFile.str, token.line.lineNumber, token.column, message, list);
	va_end(list);
}

void Log::CompilerError(const Token& token, const char* const message...) {
	va_list list;
	va_start(list, message);
	CompilerLog(LogLevel::Error, token.line.sourceFile.str, token.line.lineNumber, token.column, message, list);
	va_end(list);

	if (CompilerOptions::StopOnError()) {
		exit(1);
	}
}

void Log::SetOutputHandle(HANDLE logHandle) {
	Log::logHandle = logHandle;
	SetStdHandle(STD_OUTPUT_HANDLE, logHandle);
}

void Log::SetLogCallback(LogCallback logCallback) {
	Log::logCallback = logCallback;
}


}
}