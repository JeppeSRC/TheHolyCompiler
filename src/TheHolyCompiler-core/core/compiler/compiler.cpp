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


#include "compiler.h"
#include <util/utils.h>
#include <core/compiler/preprocessor/preprocessor.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;

bool Compiler::Process() {
	lines = preprocessor::PreProcessor::Run(code, filename, defines, includes);


}

Compiler::Compiler(const String& code, const String& filename, const List<String>& defines, const List<String>& includes) : code(code), filename(filename), defines(defines), includes(includes) {

}

bool Compiler::Run(const String& code, const String& filename, const List<String>& defines, const List<String>& includes) {
	Compiler c(code, filename, defines, includes);

	bool res = c.Process();

	return res;
}

bool Compiler::Run(const String& filename, const List<String>& defines, const List<String>& includes) {
	return Run(Utils::ReadFile(filename), filename, defines, includes);
}

}
}
}