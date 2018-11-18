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

#include "utils.h"
#include <memory>
#include <util/log.h>
#include <math.h>


namespace thc {
namespace utils {

void Utils::CopyString(char*& dst, const char* const src) {
	uint64 len = strlen(src)+1;

	dst = new char[len+1];
	memcpy(dst, src, len);
}

String Utils::GetPathFromFile(const String& filepath) {
	uint64 folders = filepath.Count("/") + filepath.Count("\\") + filepath.Count("\\\\");

	if (folders == 0) {
		Log::Error("[Utils::GetPathFromFile] Invalid parameter");
		return "";
	}

	return filepath.SubString(0, filepath.FindReversed("/"));
}

uint32 Utils::FindLine(const String& string, uint64 index, uint64* col) {
	uint32 line = 1;
	uint64 curr = 0;
	uint64 prev = 0;

	while ((curr = string.Find("\n", curr+1)) != ~0) {
		if (curr > index) {

			break;
		}

		prev = curr;

		line++;
	}

	if (col) {
		if (line == 1) {
			*col = index;
		} else {
			*col = index - prev;
		}
	}
	

	return line;
}

void Utils::RemoveWhiteSpace(String& string) {
#define REM_THING(x) while ((index = string.Find(x, index)) != ~0) { string.Remove(index, index); } index = 0;
	
	uint64 index = 0;

	REM_THING(" ");
	REM_THING("\t");
}

uint64 Utils::FindMatchingSymbol(const String& code, const char startSymbol, const char endSymbol) {
	uint64 firstSymbol = code.Find(startSymbol);
	uint64 firstSymbolCount = 0;

	for (uint64 i = firstSymbol+1; i < code.length; i++) {
		if (code[i] == startSymbol) {
			firstSymbolCount++;
		} else if (code[i] == endSymbol) {
			if (firstSymbolCount-- == 0) {
				return i;
			}
		}
	}

	return ~0;
}

uint64 Utils::StringToUint64(const char* string, uint64* length) {
	auto FindLength = [](const char* const string, int base) -> uint64 {
		for (uint64 i = 0;; i++) {
			if (base == 2) {
				if (!(string[i] >= '0' && string[i] <= '1')) {
					return i;
				}
			} else if (base == 8) {
				if (!(string[i] >= '0' && string[i] <= '7')) {
					return i;
				}
			} else if (base == 10) {
				if (!(string[i] >= '0' && string[i] <= '9')) {
					return i;
				}
			} else if (base == 16) {
				if (!((string[i] >= '0' && string[i] <= '9') || (string[i] >= 'a' && string[i] <= 'f') || (string[i] >= 'A' && string[i] <= 'F'))) {
					return i;
				}
			}
		}
	};

	auto GetValue = [](const char c) -> uint64 {
		switch (c) {
			case '0':
				return 0;
			case '1':
				return 1;
			case '2':
				return 2;
			case '3':
				return 3;
			case '4':
				return 4;
			case '5':
				return 5;
			case '6':
				return 6;
			case '7':
				return 7;
			case '8':
				return 8;
			case '9':
				return 9;
			case 'a':
			case 'A':
				return 10;
			case 'b':
			case 'B':
				return 11;
			case 'c':
			case 'C':
				return 12;
			case 'd':
			case 'D':
				return 13;
			case 'e':
			case 'E':
				return 14;
			case 'f':
			case 'F':
				return 15;
		}

		return ~0;
	};

	uint64 value = 0;

	bool sign = false;

	if (string[0] == '-') {
		sign = true;
		string++;
	} else if (string[0] == '+') {
		string++;
	}

	int base = 10;

	if (string[0] == '0') {
		if (string[1] == 'x' || string[1] == 'X') {
			base = 16;
			string += 2;
		} else if (string[1] == 'b' || string[1] == 'B') {
			base = 2;
			string += 2;
		} else if (string[1] >= '0' && string[1] <= '9'){
			base = 8;
			string++;
		}

	} 

	uint64 len = FindLength(string, base);

	for (uint64 i = 0; i < len; i++) {
		uint64 v = GetValue(string[len - i - 1]);
		value += v * (uint64)pow(base, i);

		if (v == ~0) {
			return ~0;
		}
	}

	if (sign) value *= -1;

	if (length) *length = len;

	return value;
}

}
}