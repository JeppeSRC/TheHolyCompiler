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

void PreProcessor::ProcessInclude(uint64& index) {
	const Line& l = lines[index];
	const String& line = l.string;

	uint64 firstBracket = line.Find("<");
	uint64 secondBracket = line.Find(">", firstBracket+1);

	String file = line.SubString(firstBracket+1, secondBracket-1);

	String fullPath = FindFile(file, Utils::GetPathFromFile(fileName));

	if (fullPath == "AlreadyIncluded") {
		Log::CompilerDebug(l, "File \"%s\" has already been included", fullPath.str);
		return;
	} else if (fullPath == "NotFound") {
		Log::CompilerError(l, "File \"%s\" not found", file.str);
		return;
	}

	lines.RemoveAt(index);
	lines.InsertList(index, Line::GetLinesFromFile(fullPath));

	index--;
}

void PreProcessor::ProcessDefine(uint64& index) {
	const Line& l = lines[index];
	String& line = lines[index].string;


	uint64 nameStart = line.Find("#define")+7;
	uint64 nameEnd = line.Find(" ", nameStart+1);

	String name = line.SubString(nameStart, nameEnd);
	Utils::RemoveWhiteSpace(name);

	String value = line.SubString(nameEnd, line.length-1);

	uint64 defIndex = IsDefined(name);

	if (defIndex != ~0) {
		Log::CompilerWarning(l, "Macro redefinition \"%s\"", name.str);

		defines[defIndex].value = value;
	} else {
		defines.Emplace(name, value);
	}

	lines.RemoveAt(index--);
}

void PreProcessor::ProcessUndef(uint64& index) {
	const Line& l = lines[index];
	String& line = lines[index].string;

	uint64 nameStart = line.Find("#undef")+6;

	String name = line.SubString(nameStart, line.length-1);
	Utils::RemoveWhiteSpace(name);

	uint64 defIndex = IsDefined(name);

	if (defIndex != ~0) {
		defines.RemoveAt(defIndex);
	} else {
		Log::CompilerWarning(l, "No macro \"%s\" is not defined");
	}

	lines.RemoveAt(index--);
}

void PreProcessor::ProcessIf(uint64& index) {

}

void PreProcessor::ProcessDefined(uint64& index) {

}

void PreProcessor::ProcessIfdef(uint64& index) {

}

void PreProcessor::ProcessMessage(uint64& index) {

}

void PreProcessor::ProcessError(uint64& index) {

}

void PreProcessor::Process() {

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		const Line& l = lines[i];
		const String& line = l.string;
		
		if (line.Find("#include ") != ~0) {
			ProcessInclude(i);
		} else if (line.Find("#define ") != ~0) {
			ProcessDefine(i);
		} else if (line.Find("#undef ") != ~0) {
			ProcessUndef(i);
		} else if (line.Find("#if ") != ~0) {
			ProcessIf(i);
		}
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