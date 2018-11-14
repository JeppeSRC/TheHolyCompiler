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

#include <util/string.h>
#include <core/thctypes.h>

namespace thc {
namespace core {
namespace parsing {

class Line {
public:
	utils::String string;
	utils::String sourceFile;
	uint32 lineNumber;

public:
	Line();
	Line(const utils::String& string, const utils::String& file, uint32 line);
	Line(const Line& other);
	Line(Line&& other);

	Line& operator=(const Line& other);
	Line& operator=(Line&& other);

public:
	static utils::String ToString(const utils::List<Line>& lines);
	static utils::List<Line> GetLinesFromString(const utils::String& string, const utils::String& file);
	static utils::List<Line> GetLinesFromFile(const utils::String& fileName);
};

}
}
}