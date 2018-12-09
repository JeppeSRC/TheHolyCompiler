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
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;
using namespace instruction;

bool TokenTypeCmpFunc(const Token& token, const TokenType& type) {
	return token.type == type;
}

List<Token> Compiler::Tokenize() {
	List<Token> tokens;

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		const Line& l = lines[i];
		const String& line = l.string;

		for (uint64 j = 0; j < line.length; j++) {
			const char c0 = line[j];
			const char c1 = j < line.length-1 ? line[j+1] : 0;

			if (IsCharWhitespace(c0)) {
				continue;
			} else if (c0 == '(') {
				tokens.Emplace(TokenType::ParenthesisOpen, "(", l, j);
			} else if (c0 == ')') {
				tokens.Emplace(TokenType::ParenthesisClose, ")", l, j);
			} else if (c0 == '{') {
				tokens.Emplace(TokenType::CurlyBracketOpen, "{", l, j);
			} else if (c0 == '}') {
				tokens.Emplace(TokenType::CurlyBracketClose, "}", l, j);
			} else if (c0 == '[') {
				tokens.Emplace(TokenType::BracketOpen, "[", l, j);
			} else if (c0 == ']') {
				tokens.Emplace(TokenType::BracketClose, "]", l, j);
			} else if (c0 == ';') {
				tokens.Emplace(TokenType::SemiColon, 0, ";", l, j);
			} else if (c0 == '+' && c1 == '+') {
				tokens.Emplace(TokenType::OperatorIncrement, "++", l, j++);
			} else if (c0 == '-' && c1 == '-') {
				tokens.Emplace(TokenType::OperatorDecrement, "--", l, j++);
			} else if (c0 == '+' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundAdd, "+=", l, j++);
			} else if (c0 == '-' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundSub, "-=", l, j++);
			} else if (c0 == '*' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundMul, "*=", l, j++);
			} else if (c0 == '/' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundDiv, "/=", l, j++);
			} else if (c0 == '<' && c1 == '<') {
				tokens.Emplace(TokenType::OperatorLeftShift, "<<", l, j++);
			} else if (c0 == '>' && c1 == '>') {
				tokens.Emplace(TokenType::OperatorRightShift, ">>", l, j++);
			} else if (c0 == '<' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorLessEqual, "<=", l, j++);
			} else if (c0 == '>' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorGreaterEqual, ">=", l, j++);
			} else if (c0 == '&' && c1 == '&') {
				tokens.Emplace(TokenType::OperatorLogicalAnd, "&&", l, j++);
			} else if (c0 == '|' && c1 == '|') {
				tokens.Emplace(TokenType::OperatorLogicalOr, "||", l, j++);
			} else if (c0 == '=' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorLogicalEqual, "==", l, j++);
			} else if (c0 == '!' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorLogicalNotEqual, "!=", l, j++);
			} else if (c0 == '+') {
				tokens.Emplace(TokenType::OperatorAdd, "+", l, j);
			} else if (c0 == '-') {
				tokens.Emplace(TokenType::OperatorSub, "-", l, j);
			} else if (c0 == '*') {
				tokens.Emplace(TokenType::OperatorMul, "*", l, j);
			} else if (c0 == '/') {
				tokens.Emplace(TokenType::OperatorDiv, "/", l, j);
			} else if (c0 == '<') {
				tokens.Emplace(TokenType::OperatorLess, "<", l, j);
			} else if (c0 == '>') {
				tokens.Emplace(TokenType::OperatorGreater, ">", l, j);
			} else if (c0 == '!') {
				tokens.Emplace(TokenType::OperatorLogicalNot, "!", l, j);
			} else if (c0 == '&') {
				tokens.Emplace(TokenType::OperatorBitwiseAnd, "&", l, j);
			} else if (c0 == '|') {
				tokens.Emplace(TokenType::OperatorBitwiseOr, "|", l, j);
			} else if (c0 == '~') {
				tokens.Emplace(TokenType::OperatorBitwiseNot, "~", l, j);
			} else if (c0 == '^') {
				tokens.Emplace(TokenType::OperatorBitwiseXor, "^", l, j);
			} else if (c0 == '?') {
				tokens.Emplace(TokenType::OperatorTernary1, "?", l, j);
			} else if (c0 == ':') {
				tokens.Emplace(TokenType::OperatorTernary2, ":", l, j);
			} else if (c0 == '.') {
				tokens.Emplace(TokenType::OperatorSelector, ".", l, j);
			} else if (c0 == ',') {
				tokens.Emplace(TokenType::Comma, ",", l, j);
			} else if (c0 == '=') {
				tokens.Emplace(TokenType::OperatorAssign, "=", l, j);
			} else if (c0 >= '0' && c0 <= '9') {
				uint64 len = 0;
				uint64 value = 0;

				value = Utils::StringToUint64(line.str + j, &len);

				tokens.Emplace(TokenType::Value, value, line.SubString(j, j + len - 1), l,  j);
			} else {
				uint64 end = ~0;

				for (uint64 c = j; c < line.length; c++) {
					if (!IsCharAllowedInName(line[c], c == j ? true : false)) {
						end = c;
						break;
					}
				}

				if (end == j) {
					Log::CompilerError(l, j, "Unexpect symbol \"%c\"", c0);
					break;
				}

				tokens.Emplace(TokenType::Name, 0, line.SubString(j, end-1), l, j);

				j = end-1;
			}
		}
	}

	for (uint64 i = 0; i < tokens.GetCount(); i++) {
		Token& t = tokens[i];

		if (t.type == TokenType::Name) {
			ProcessName(t);
		} else if (t.type == TokenType::OperatorSub) {
			if (i == 0) {
				t.type = TokenType::OperatorNegate;
			} else {
				const Token& left = tokens[i-1];
				const Token& right = tokens[i+1];

				if ((right.type == TokenType::Value || right.type == TokenType::Name) && (left.type != TokenType::Name || left.type != TokenType::Value)) {
					t.type = TokenType::OperatorNegate;
				}
			}
		}
	}

	return tokens;
}

void Compiler::ParseTokens(List<Token>& tokens) {
	for (uint64 i = 0; i < tokens.GetCount(); i++) {
		Token& token = tokens[i];

		uint64 offset = 1;

		if (token.type == TokenType::DataLayout) {
			if (i >= tokens.GetCount() - 9) {
				Log::CompilerError(token, "Undefined symbol \"%s\"", token.string.str);
				return;
			}

			const Token& parenthesisOpen = tokens[i + offset++];

			if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
				Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
				return;
			}

			

			uint32 location = ~0;
			uint32 binding = ~0;
			uint32 set = ~0;

			auto GetValue = [&tokens, &offset, i]() -> uint32 {
				const Token& equal = tokens[i+offset++];

				if (equal.type != TokenType::OperatorAssign) {
					Log::CompilerError(equal, "Unexpected symbol \"%s\" expected \"=\"", equal.string.str);
					return ~0;
				}

				const Token& value = tokens[i+offset++];

				if (value.type != TokenType::Value) {
					Log::CompilerError(value, "Unexpected symbol \"%s\" expected a valid value", value.string.str);
					return ~0;
				}

				return (uint32)value.value;
			};

			while (true) {
				const Token& specifier = tokens[i + offset++];

				if (specifier.type != TokenType::Name && !(specifier.string == "location" || specifier.string == "set" || specifier.string == "binding")) {
					Log::CompilerError(specifier, "Unexpected symbol \"%s\" expected \"location, set or binding\"", specifier.string.str);
					return;
				}

				if (specifier.string == "location") {
					if (location != ~0) {
						Log::CompilerError(specifier, "Specifier \"location\" already specified once");
						return;
					}

					location = GetValue();
				} else if (specifier.string == "binding") {
					if (binding != ~0) {
						Log::CompilerError(specifier, "Specifier \"binding\" already specified once");
						return;
					}

					binding = GetValue();
				} else if (specifier.string == "set") {
					if (set != ~0) {
						Log::CompilerError(specifier, "Specifier \"set\" already specified once");
						return;
					}

					set = GetValue();
				}

				const Token& next = tokens[i + offset++];

				if (next.type == TokenType::Comma) {
					continue;
				} else if (next.type == TokenType::ParenthesisClose) {
					break;
				} else {
					Log::CompilerError(next, "Unexpected symbol \"%s\" expected \")\"", next.string.str);
					return;
				}
			}

			const Token& scope = tokens[i + offset++];

			Variable tmp;

			switch (scope.type) {
				case TokenType::DataIn:
					tmp.scope = VariableScope::In;

					if (binding != ~0) {
						Log::CompilerError(scope, "Specifier \"binding\" cannot be used on \"in\"");
						return;
					} else if (set != ~0) {
						Log::CompilerError(scope, "Specifier \"set\" cannot be used on \"in\"");
						return;
					} else if (location == ~0) {
						Log::CompilerError(scope, "Specifier \"location\" must be set");
						return;
					}

					break;
				case TokenType::DataOut:
					tmp.scope = VariableScope::Out;

					if (binding != ~0) {
						Log::CompilerError(scope, "Specifier \"binding\" cannot be used on \"out\"");
						return;
					} else if (set != ~0) {
						Log::CompilerError(scope, "Specifier \"set\" cannot be used on \"out\"");
						return;
					} else if (location == ~0) {
						Log::CompilerError(scope, "Specifier \"location\" must be set");
						return;
					}

					break;
				case TokenType::DataUniform:
					tmp.scope = VariableScope::Uniform;

					if (binding == ~0) {
						Log::CompilerError(scope, "Specifier \"binding\" must be set");
						return;
					} else if (set == ~0) {
						Log::CompilerError(scope, "Specifier \"set\" must be set");
						return;
					} else if (location != ~0) {
						Log::CompilerError(scope, "Specifier \"location\" cannot be used on \"uniform\"");
						return;
					}

					break;
				default:
					Log::CompilerError(scope, "Unexpected symbol \"%s\" expected \"in, out or uniform\"", scope.string.str);
					return;
			}

			Variable* var;

			if (tmp.scope == VariableScope::Uniform) {
				TypeStruct* str = CreateTypeStruct(tokens, i + offset);


				for (uint64 i = 0; i < str->members.GetCount(); i++) {
					if (!CheckGlobalName(str->members[i]->name)) {
						const Token& n = tokens[i + offset];
						Log::CompilerError(n, "Redefinition of global variable \"%s\" in \"%s\"", str->members[i]->name, n.string.str);
						return;
					}
				}

				var = CreateGlobalVariable(str, tmp.scope, "_uniform_buf");

				annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_BINDING, &binding, 1));
				annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_DESCRIPTORSET, &set, 1));
			} else {
				uint64 typeLocation = i + offset++;

				if (tokens[i + offset].type == TokenType::OperatorLess) {
					offset += 3;
				}

				const Token& name = tokens[i + offset++];

				if (name.type != TokenType::Name) {
					Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
					return;
				}

				if (!CheckGlobalName(name.string)) {
					Log::CompilerError(name, "Redefinition of global variable \"%s\"", name.string.str);
					return;
				}

				const Token& semiColon = tokens[i + offset++];

				if (semiColon.type != TokenType::SemiColon) {
					Log::CompilerError(semiColon, "Unexpected symbol \"%s\" expected \";\"", semiColon.string.str);
					return;
				}

				TypePrimitive* type = CreateTypePrimitive(tokens, typeLocation);

				var = CreateGlobalVariable(type, tmp.scope, name.string);

				annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_LOCATION, &location, 1));
			}
		} else if (token.type == TokenType::DataOut || token.type == TokenType::DataIn) {
			TypePrimitive* type = CreateTypePrimitive(tokens, i + offset++);

			const Token& name = tokens[i + offset++];

			if (name.type != TokenType::Name) {
				Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
				return;
			}

			const Token& assign = tokens[i + offset++];

			if (assign.type != TokenType::OperatorAssign) {
				Log::CompilerError(assign, "Unexpected symbol \"%s\" expected \"=\"", assign.string.str);
				return;
			}

			const Token& intrin = tokens[i + offset++];

			if (intrin.type != TokenType::Name) {
				Log::CompilerError(intrin, "Unexpected symbol \"%s\" expected a valid name", intrin.string.str);
				return;
			}

			const Token& semi = tokens[i + offset++];

			if (semi.type != TokenType::SemiColon) {
				Log::CompilerError(semi, "Unexpected symbol \"%s\" expected \";\"", semi.string.str);
				return;
			}

			Variable* var = CreateGlobalVariable(type, token.type == TokenType::DataOut ? VariableScope::Out : VariableScope::In, name.string);

			if (intrin.string == "THSL_Position") {
				uint32 builtin = 0;

				if (var->scope != VariableScope::Out) {
					Log::CompilerError(intrin, "Builtin THSL_Position must be an output variable");
					return;
				}

				annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_BUILTIN, &builtin, 1));
			}
		}

		tokens.Remove(i, i + offset-1);
		i--;
	}
}

void Compiler::ParseFunction(List<Token>& tokens, uint64 start) {
	const Token& returnType = tokens[start];
	const Token& name = tokens[start + 1];

	uint64 paramEnd = tokens.Find<TokenType>(TokenType::ParenthesisClose, TokenTypeCmpFunc, start + 2);

	if (paramEnd == ~0) {
		Log::CompilerError(tokens[start + 2], "Missing closing parenthesis");
		return;
	}

	for (uint64 i = start + 3; i < paramEnd; i++) {

	}
}

bool Compiler::Process() {
	lines = preprocessor::PreProcessor::Run(code, filename, defines, includes);

	List<Token> tokens = Tokenize();

	ParseTokens(tokens);

	return false;
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