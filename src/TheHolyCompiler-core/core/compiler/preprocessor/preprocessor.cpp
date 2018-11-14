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

#include <core/compiler/preprocessor/preprocessor.h>
#include <core/compiler/parsing/line.h>
#include <util/utils.h>
#include <util/log.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;
using namespace parsing;

void PreProcessor::RemoveComments(String& code) {
	uint64 index = 0;

	while ((index = code.Find("/*")) != ~0) {
		uint64 next = code.Find("*/", index);

		if (next != ~0) {
			code.Remove(index, next+1);
		} else {
			Log::CompilerError(fileName.str, Utils::FindLine(code, index) , "Multiline comment is missing end");
		}
	}
}

void PreProcessor::ProcessInclude(uint64 lineNumber) {

}

void PreProcessor::ProcessDefine(uint64 lineNumber) {

}

void PreProcessor::ProcessUndef(uint64 lineNumber) {

}

void PreProcessor::ProcessIf(uint64 lineNumber) {

}

void PreProcessor::ProcessDefined(uint64 lineNumber) {

}

void PreProcessor::ProcessIfdef(uint64 lineNumber) {

}

void PreProcessor::ProcessMessage(uint64 lineNumber) {

}

void PreProcessor::ProcessError(uint64 lineNumber) {

}

void PreProcessor::Process() {

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		const Line& line = lines[i];

	}

}

PreProcessor::PreProcessor(String code, const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) : fileName(fileName), includeDirectories(includeDirs) {
	this->defines.Reserve(defines.GetCount());

	for (uint64 i = 0; i < defines.GetCount(); i++) {
		this->defines.Emplace(defines[i], "");
	}

	RemoveComments(code);

	lines = Line::GetLinesFromString(code, fileName);
}

String PreProcessor::Run(const String& code, const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) {
	PreProcessor pp(code, fileName, defines, includeDirs);
	
	pp.Process();

	return Line::ToString(pp.lines);
}

String PreProcessor::Run(const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) {
	return Run(Utils::ReadFile(fileName), fileName, defines, includeDirs);
}


}
}
}