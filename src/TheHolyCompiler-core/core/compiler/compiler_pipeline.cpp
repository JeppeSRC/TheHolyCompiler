/*
MIT License

Copyright (c) 2018 Jesper Hammarstr�m

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
#include <core/preprocessor/preprocessor.h>
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;
using namespace instruction;

void Compiler::ParseLayout(List<Token>& tokens, uint64 start) {
	uint64 offset = 0;

	const Token& parenthesisOpen = tokens[++start + offset++];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	uint32 location = ~0;
	uint32 binding = ~0;
	uint32 set = ~0;

	auto GetValue = [&tokens, &offset, start]() -> uint32 {
		const Token& equal = tokens[start + offset++];

		if (equal.type != TokenType::OperatorAssign) {
			Log::CompilerError(equal, "Unexpected symbol \"%s\" expected \"=\"", equal.string.str);
		}

		const Token& value = tokens[start + offset++];

		if (value.type != TokenType::Value) {
			Log::CompilerError(value, "Unexpected symbol \"%s\" expected a valid value", value.string.str);
		}

		return (uint32)value.value;
	};

	while (true) {
		const Token& specifier = tokens[start + offset++];

		if (specifier.type != TokenType::Name && !(specifier.string == "location" || specifier.string == "set" || specifier.string == "binding")) {
			Log::CompilerError(specifier, "Unexpected symbol \"%s\" expected \"location, set or binding\"", specifier.string.str);
		}

		if (specifier.string == "location") {
			if (location != ~0) {
				Log::CompilerError(specifier, "Specifier \"location\" already specified once");
			}

			location = GetValue();
		} else if (specifier.string == "binding") {
			if (binding != ~0) {
				Log::CompilerError(specifier, "Specifier \"binding\" already specified once");
			}

			binding = GetValue();
		} else if (specifier.string == "set") {
			if (set != ~0) {
				Log::CompilerError(specifier, "Specifier \"set\" already specified once");
			}

			set = GetValue();
		}

		const Token& next = tokens[start + offset++];

		if (next.type == TokenType::Comma) {
			continue;
		} else if (next.type == TokenType::ParenthesisClose) {
			break;
		} else {
			Log::CompilerError(next, "Unexpected symbol \"%s\" expected \")\"", next.string.str);
		}
	}

	const Token& scope = tokens[start + offset++];

	Variable tmp;

	switch (scope.type) {
		case TokenType::DataIn:
			tmp.scope = VariableScope::In;

			if (binding != ~0) {
				Log::CompilerError(scope, "Specifier \"binding\" cannot be used on \"in\"");
			} else if (set != ~0) {
				Log::CompilerError(scope, "Specifier \"set\" cannot be used on \"in\"");
			} else if (location == ~0) {
				Log::CompilerError(scope, "Specifier \"location\" must be set");
			}

			break;
		case TokenType::DataOut:
			tmp.scope = VariableScope::Out;

			if (binding != ~0) {
				Log::CompilerError(scope, "Specifier \"binding\" cannot be used on \"out\"");
			} else if (set != ~0) {
				Log::CompilerError(scope, "Specifier \"set\" cannot be used on \"out\"");
			} else if (location == ~0) {
				Log::CompilerError(scope, "Specifier \"location\" must be set");
			}

			break;
		case TokenType::DataUniform:
			tmp.scope = VariableScope::Uniform;

			if (binding == ~0) {
				Log::CompilerError(scope, "Specifier \"binding\" must be set");
			} else if (set == ~0) {
				Log::CompilerError(scope, "Specifier \"set\" must be set");
			} else if (location != ~0) {
				Log::CompilerError(scope, "Specifier \"location\" cannot be used on \"uniform\"");
			}

			break;
		default:
			Log::CompilerError(scope, "Unexpected symbol \"%s\" expected \"in, out or uniform\"", scope.string.str);
	}

	Variable* var;

	if (tmp.scope == VariableScope::Uniform) {
		TypeStruct* str = CreateTypeStruct(tokens, start + offset++, nullptr);

		String name = str->typeString;
		str->typeString += "_uniform_type";

		var = CreateGlobalVariable(str, tmp.scope, name);

		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_BINDING, &binding, 1));
		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_DESCRIPTORSET, &set, 1));
	} else {
		TypePrimitive* type = CreateTypePrimitive(tokens, start + offset, nullptr);

		const Token& name = tokens[start + offset++];

		if (name.type != TokenType::Name) {
			Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
		}

		if (!CheckGlobalName(name.string)) {
			Log::CompilerError(name, "Redefinition of global variable \"%s\"", name.string.str);
		}

		const Token& semiColon = tokens[start + offset++];

		if (semiColon.type != TokenType::SemiColon) {
			Log::CompilerError(semiColon, "Unexpected symbol \"%s\" expected \";\"", semiColon.string.str);
		}

		var = CreateGlobalVariable(type, tmp.scope, name.string);

		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_LOCATION, &location, 1));
	}

	start--;

	tokens.Remove(start, start + offset - 1);
}

void Compiler::ParseInOut(List<Token>& tokens, uint64 start, VariableScope scope) {
	uint64 offset = 0;

	TypePrimitive* type = CreateTypePrimitive(tokens, ++start + offset, nullptr);

	const Token& name = tokens[start + offset++];

	if (name.type != TokenType::Name) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
	}

	const Token& assign = tokens[start + offset++];

	if (assign.type != TokenType::OperatorAssign) {
		Log::CompilerError(assign, "Unexpected symbol \"%s\" expected \"=\"", assign.string.str);
	}

	const Token& intrin = tokens[start + offset++];

	if (intrin.type != TokenType::Name) {
		Log::CompilerError(intrin, "Unexpected symbol \"%s\" expected a valid name", intrin.string.str);
	}

	const Token& semi = tokens[start + offset++];

	if (semi.type != TokenType::SemiColon) {
		Log::CompilerError(semi, "Unexpected symbol \"%s\" expected \";\"", semi.string.str);
	}

	Variable* var = CreateGlobalVariable(type, scope, name.string);

	if (intrin.string == "THSL_Position") {
		uint32 builtin = 0;

		if (var->scope != VariableScope::Out) {
			Log::CompilerError(intrin, "Builtin THSL_Position must be an output variable");
		}

		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_BUILTIN, &builtin, 1));
	}

	start--;

	tokens.Remove(start, start + offset);
}

}
}
}