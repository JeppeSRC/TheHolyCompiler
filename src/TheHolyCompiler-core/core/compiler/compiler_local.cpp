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
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace instruction;
using namespace utils;
using namespace parsing;

Compiler::VariableStack::VariableStack(Compiler* compiler, List<Parameter*>& parameters) : compiler(compiler) {
	PushStack();
	parameters.Add(parameters);
	//PushStack(); Test
}

Compiler::VariableStack::~VariableStack() {

}

uint64 Compiler::VariableStack::PushStack() {
	uint64 tmp = variables.GetCount();
	offsets.Add(tmp);
	return tmp;
}

bool Compiler::VariableStack::CheckName(const Token& name) {
	return CheckName(name.string, name);
}

bool Compiler::VariableStack::CheckName(const String& name, const Token& token) {
	uint64 stackOffset = offsets[offsets.GetCount() - 1];

	bool res = true;

	for (uint64 i = stackOffset; i < variables.GetCount(); i++) {
		Variable* var = variables[i];
		if (var->name == name) {
			Log::CompilerError(token, "Redefinition of variable \"%s\"", name.str);
		}
	}

	for (int64 i = stackOffset - 1; i >= (int64)0; i--) {
		Variable* var = variables[i];
		if (var->name == name) {
			Log::CompilerWarning(token, "Overriding local variable \"%s\"", name.str);
			res = false;
		}
	}

	for (uint64 i = 0; i < parameters.GetCount(); i++) {
		Parameter* var = parameters[i];
		if (var->name == name) {
			Log::CompilerWarning(token, "Overriding parameter \"%s\"", name.str);
			res = false;
		}
	}

	if (!compiler->CheckGlobalName(name)) {
		Log::CompilerWarning(token, "Overriding global variable \"%s\"", name.str);
	}

	return res;
}

void Compiler::VariableStack::PopStack() {
	uint64 of = offsets.RemoveAt(offsets.GetCount() - 1);
	uint64 count = variables.GetCount();

	if (of == count) return;

	variables.Remove(of, count - 1);
}

void Compiler::VariableStack::AddVariable(Variable* variable, InstBase* inst) {
	variables.Add(variable);
	variableInstructions.Add(inst);
}

Compiler::Variable* Compiler::VariableStack::GetVariable(const String& name) {
	for (int64 i = variables.GetCount() - 1; i >= 0; i--) {
		Variable* var = variables[i];
		if (var->name == name) return var;
	}

	return nullptr;
}

uint64 Compiler::VariableStack::GetSize() const {
	return variables.GetCount();
}

uint64 Compiler::VariableStack::GetStackSize(uint64 stack) const {
	THC_ASSERT(stack < offsets.GetCount());
	return GetSize() - offsets[stack];
}

uint64 Compiler::VariableStack::GetCurrentStackSize() const {
	return GetStackSize(offsets.GetCount() - 1);
}

}
}
}