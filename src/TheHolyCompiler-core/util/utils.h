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

#define CLAMP(x, min, max) (x > max ? max : x < min ? min : x)

#include "string.h"
#include <core/parsing/line.h>

namespace thc {
namespace utils {

enum CompareOperation {
	Equal,
	NotEqual,
	Or,
};

enum class ValueResultType {
	Error,

	Float,
	Int,
};

struct ValueResult {
	ValueResultType type;

	uint8 sign;

	union {
		float32 fvalue;
		uint32 value;
	};
};

class Utils {
private:

public:
	static void CopyString(char*& dst, const char* const src);
	static String GetPathFromFile(const String& filepath);
	static uint64 FindLine(const String& string, uint64 index, uint64* col = nullptr);
	static void RemoveWhitespace(String& string);
	static uint64 FindMatchingSymbol(const String& code, const char start, const char end);
	static uint64 StringToUint64(const char* string, uint64* length = nullptr);
	static ValueResult StringToValue(const char* string, uint64* length, const core::parsing::Line& line, uint64 column);
	
	static String ReadFile(const String& filename);

	

	template<typename First, typename ...T>
	static bool CompareEnums(First first, CompareOperation op, T... args) {
		constexpr uint64 num = sizeof...(args);

		First arr[] = {args...};

		if (op == CompareOperation::Equal) {
			for (uint64 i = 0; i < num; i++) {
				if (first != arr[i]) return false;
			}

			return true;
		} else if (op == CompareOperation::NotEqual) {
			for (uint64 i = 0; i < num; i++) {
				if (first == arr[i]) return false;
			}

			return true;
		} else if (op == CompareOperation::Or) {
			for (uint64 i = 0; i < num; i++) {
				if (first == arr[i]) return true;
			}

			return false;
		}
		

		return false;
	}
};

}
}