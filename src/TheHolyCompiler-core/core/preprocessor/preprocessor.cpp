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

#include <core/preprocessor/preprocessor.h>
#include <core/parsing/line.h>
#include <util/utils.h>
#include <util/log.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;
using namespace parsing;

unsigned int FindLine(const String& string, size_t index) {
	unsigned int line = 1;
	size_t curr = 0;

	while ((curr = string.Find("\n", curr+1)) != ~0) {
		if (curr > index)
			break;

		line++;
	}

	return line;
}

void PreProcessor::RemoveComments(String& code) {
	size_t index = 0;

	while ((index = code.Find("/*")) != ~0) {
		size_t next = code.Find("*/", index);

		if (next != ~0) {
			code.Remove(index, next+1);
		} else {
			Log::CompilerError(fileName.str, FindLine(code, index) , "Multiline comment is missing end");
		}
	}
}

void PreProcessor::ProcessInclude(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessDefine(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessUndef(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessIf(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessDefined(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessIfdef(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessMessage(List<parsing::Line>& lines) {

}

void PreProcessor::ProcessError(List<parsing::Line>& lines) {

}

PreProcessor::CodeUnit PreProcessor::Process() {
	CodeUnit d;

	List<Line> lines = Line::GetLinesFromString(code, fileName);

	for (size_t i = 0; i < lines.GetCount(); i++) {
		const Line& line = lines[i];

	}

	return d;
}

PreProcessor::PreProcessor(const String& code, const String& fileName) : code(code), fileName(fileName) {
	RemoveComments(this->code);
}

String PreProcessor::Run(const String& code, const String& fileName) {
	PreProcessor pp(code, fileName);
	
	CodeUnit cu = pp.Process();

	return Line::ToString(cu.lines);
}

String PreProcessor::Run(const String& fileName) {
	return Run(Utils::ReadFile(fileName), fileName);
}


}
}
}