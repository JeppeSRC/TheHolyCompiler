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

#include "line.h"
#include <util/utils.h>

namespace thc {
namespace core {
namespace parsing {

using namespace utils;

Line::Line() : string(""), sourceFile(""), lineNumber(0) { }

Line::Line(const String& string, const String& file, uint32 line) : string(string), sourceFile(file), lineNumber(line) { }

Line::Line(const Line& other) {
	string = other.string;
	sourceFile = other.sourceFile;
	lineNumber = other.lineNumber;
}

Line::Line(Line&& other) {
	string = std::move(other.string);
	sourceFile = std::move(other.sourceFile);
	lineNumber = other.lineNumber;
}

Line& Line::operator=(const Line& other) {
	if (this != &other) {
		string = other.string;
		sourceFile = other.sourceFile;
		lineNumber = other.lineNumber;
	}

	return *this;
}

Line& Line::operator=(Line&& other) {
	if (this != &other) {
		string = std::move(other.string);
		sourceFile = std::move(other.sourceFile);
		lineNumber = other.lineNumber;
	}

	return *this;
}

List<Line> Line::GetLinesFromString(const String& string, const String& file) {
	List<String> lines = string.Split("\n");
	List<Line> res(lines.GetCount());

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		res.Emplace(lines[i], file, i+1);
	}

	return res;
}

List<Line> Line::GetLinesFromFile(const String& fileName) {
	String string = Utils::ReadFile(fileName);
	return GetLinesFromString(string, fileName);
}

String Line::ToString(const List<Line>& lines) {
	String string;

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		string.Append(lines[i].string + "\n");
	}

	return string;
}

}
}
}