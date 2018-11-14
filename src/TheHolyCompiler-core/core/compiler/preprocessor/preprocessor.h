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
#include <core/compiler/parsing/line.h>

namespace thc {
namespace core {
namespace preprocessor {

class PreProcessor {
private:
	struct Define {
		utils::String name;
		utils::String value;

		Define() {}
		Define(const utils::String& name, const utils::String& value);
		Define(const Define& other);
		Define(const Define* other);
		Define(Define&& other);

		Define& operator=(const Define& other);
		Define& operator=(Define&& other);

	};

private:
	utils::String fileName;
	utils::List<parsing::Line> lines;

	utils::List<Define> defines;
	utils::List<utils::String> includedFiles;
	utils::List<utils::String> includeDirectories;

	bool IsDefined(const utils::String& name);
	utils::String FindFile(const utils::String& fileName);

	void RemoveComments(utils::String& code);
	void ProcessInclude(size_t line);
	void ProcessDefine(size_t line);
	void ProcessUndef(size_t line);
	void ProcessIf(size_t line);
	void ProcessDefined(size_t line);
	void ProcessIfdef(size_t line);
	void ProcessMessage(size_t line);
	void ProcessError(size_t line);
	void Process();

private:
	PreProcessor(utils::String code, const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);

public:
	static utils::String Run(const utils::String& code, const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);
	static utils::String Run(const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);
};

}
}
}
