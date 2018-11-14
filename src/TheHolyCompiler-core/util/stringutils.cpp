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

namespace thc {
namespace utils {

void Utils::CopyString(char*& dst, const char* const src) {
	size_t len = strlen(src)+1;

	dst = new char[len+1];
	memcpy(dst, src, len);
}

String Utils::GetPathFromFile(const String& filepath) {
	size_t folders = filepath.Count("/");

	if (folders == 0) {
		Log::Error("[Utils::GetPathFromFile] Invalid parameter");
		return "";
	}

	return filepath.SubString(0, filepath.FindReversed("/"));
}

unsigned int Utils::FindLine(const String& string, size_t index) {
	unsigned int line = 1;
	size_t curr = 0;

	while ((curr = string.Find("\n", curr+1)) != ~0) {
		if (curr > index)
			break;

		line++;
	}

	return line;
}

}
}