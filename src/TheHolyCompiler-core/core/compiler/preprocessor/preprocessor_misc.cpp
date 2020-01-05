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

#include "preprocessor.h"
#include <util/log.h>
#include <util/utils.h>
#include <stdio.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;
using namespace parsing;

String PreProcessor::FindFile(const String& fileName, String parentDir) {
	parentDir.Append(fileName);

	if (includedFiles.Find(parentDir) != ~0) {
		return "AlreadyIncluded";
	}

	FILE* f = fopen(parentDir.str, "rb");

	if (f != nullptr) {
		fclose(f);
		includedFiles.Add(parentDir);
		return parentDir;
	}

	for (uint64 i = 0; i < includeDirectories.GetCount(); i++) {
		String path = includeDirectories[i];
		
		if (path.EndsWith("/")) {
			path.Append(fileName);
		} else {
			path.Append("/").Append(fileName);
		}

		if (includedFiles.Find(path) != ~0) {
			return "AlreadyIncluded";
		}

		f = fopen(path.str, "rb");

		if (f != nullptr) {
			continue;
		}

		fclose(f);

		includedFiles.Add(path);

		return path;
	}

	return "NotFound";
}

uint64 PreProcessor::IsDefined(const String& name) {
	uint64 index = defines.Find<String>(name, [](const Define& current, const String& name) -> bool {
		if (current.name == name) return true;
		return false;
	});
	
	return index;
}

uint64 PreProcessor::FindMatchingParenthesis(const List<Token>& tokens, uint64 start, const Line& line) {
	uint64 count = 0;

	for (uint64 i = start; i < tokens.GetCount(); i++) {
		const Token& t = tokens[i];

		if (t.type == TokenType::ParenthesisOpen) {
			count++;
		} else if (t.type == TokenType::ParenthesisClose) {
			if (count == 1) return i;
			else if (count == 0) {
				Log::CompilerError(line, t.column, "One lonley ')' found without a matching '('");
			}
			count--;
		}
	}

	return ~0;
}

uint64 PreProcessor::FindLineWith(const String& string, uint64 offset) {
	for (uint64 i = offset; i < lines.GetCount(); i++) {
		if (lines[i].string.Find(string) != ~0) {
			return i;
		}
	}

	return ~0;
}

void PreProcessor::ReplaceMacrosWithValue(String& code) {
	for (uint64 i = 0; i < defines.GetCount(); i++) {
		const String& name = defines[i].name;

		uint64 num = code.Count(name);

		for (uint64 j = 0; j < num; j++) {
			uint64 index = code.Find(name);

			code.Insert(index, index + name.length - 1, defines[i].value);
		}
	}
}

}
}
}