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
			} else if (c0 == '=') {
				tokens.Emplace(TokenType::OperatorAssign, "=", l, j);
			} else if (c0 >= '0' && c0 <= '9') {
				uint64 len = 0;
				uint64 value = 0;

				value = Utils::StringToUint64(line.str + i, &len);

				tokens.Emplace(TokenType::Value, value, line.SubString(i, i + len - 1), l,  j);
			} else {
				uint64 end = ~0;

				for (uint64 c = j; c < tokens.GetCount(); c++) {
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

				j = end;
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

			const Token& specifier = tokens[i + offset++];

			if (specifier.type != TokenType::Name || !(specifier.string == "location" || specifier.string == "set" || specifier.string == "binding")) {
				Log::CompilerError(specifier, "Unexpected symbol \"%s\" expected \"location, set or binding\"", specifier.string.str);
				return;
			}

			if (specifier.string == "location") {
			//Is in/out data

				const Token& equal = tokens[i + offset++];

				if (equal.type != TokenType::OperatorAssign) {
					Log::CompilerError(equal, "Unexpected symbol \"%s\" expected \"=\"", equal.string.str);
					return;
				}

				const Token& value = tokens[i + offset++];

				if (value.type != TokenType::Value) {
					Log::CompilerError(value, "Unexpected symbol \"%s\" expected an integer scalar");
					return;
				}

				const Token& parenthesisClose = tokens[i + offset++];

				if (parenthesisClose.type != TokenType::ParenthesisClose) {
					Log::CompilerError(parenthesisClose, "Unexpected symbol \"%s\" expected \")\"", parenthesisClose.string.str);
					return;
				}

				const Token& inout = tokens[i + offset++];

				if (inout.type != TokenType::DataIn || inout.type != TokenType::DataOut) {
					Log::CompilerError(inout, "Unexxpected symbol \"%s\" expected \"in or out\"", inout.string.str);
					return;
				}

				const Token& type = tokens[i + offset++];

				if (!Token::ValidInOutType(type.type)) {
					Log::CompilerError(type, "Unexpected symbol \"%s\" expected a valid type", type.string.str);
					return;
				}

				VariablePrimitive tmp;

				if (tokens[i + offset].type == TokenType::OperatorLess) {
					offset++;
					const Token& t = tokens[i + offset++];

					if (!Token::ValidInOutType(t.type)) {
						Log::CompilerError(t, "Unexpected symbol \"%s\" expected a valid type", t.string.str);
						return;
					}

					tmp.dataType = t.type;
					tmp.bits = t.bits;

					const Token& close = tokens[i + offset++];

					if (close.type != TokenType::OperatorGreater) {
						Log::CompilerError(close, "Unexpected symbol \"%s\" expected \">\"", close.string.str);
					}
				} else {
					tmp.dataType = TokenType::TypeFloat;
					tmp.bits = 32;
				}

				const Token& name = tokens[i + offset++];

				if (name.type != TokenType::Name) {
					Log::CompilerError(name, "Unexpected symbol \"%s\" expected a name", name.string.str);
					return;
				}

				const Token& semiColon = tokens[i + offset++];

				if (semiColon.type != TokenType::SemiColon) {
					Log::CompilerError(semiColon, "Unexpected symbol \"%s\" expected \";\"", semiColon.string.str);
					return;
				}

				VariablePrimitive* var = new VariablePrimitive;

				var->scope = inout.type == TokenType::DataIn ? VariableScope::In : VariableScope::Out;
				var->type = VariableType::Primitive;
				var->name = name.string;
				var->dataType = type.type;
				var->componentType = tmp.dataType;
				var->bits = tmp.bits;
				var->rows = type.rows;
				var->columns = type.columns;

				TypeBase* t = nullptr;

				switch (type.type) {
					case TokenType::TypeUint:
						t = new TypeInt(var->bits, 0);
						break;
					case TokenType::TypeInt:
						t = new TypeInt(var->bits, 1);
						break;
					case TokenType::TypeFloat:
						t = new TypeFloat(var->bits);
						break;
					case TokenType::TypeVec:
					{
						TypeBase* tt = nullptr;

						switch (var->componentType) {
							case TokenType::TypeFloat:
								tt = new TypeFloat(var->bits);
								break;
							case TokenType::TypeUint:
								tt = new TypeInt(var->bits, 0);
								break;
							case TokenType::TypeInt:
								tt = new TypeInt(var->bits, 1);
								break;
						}

						CheckTypeExists(&tt);

						t = new TypeVector(var->columns, tt->id);

					}
						break;
					case TokenType::TypeMat:
					{
						TypeBase* tt = nullptr;

						switch (var->componentType) {
							case TokenType::TypeFloat:
								tt = new TypeFloat(var->bits);
								break;
							case TokenType::TypeUint:
								tt = new TypeInt(var->bits, 0);
								break;
							case TokenType::TypeInt:
								tt = new TypeInt(var->bits, 1);
								break;
						}

						CheckTypeExists(&tt);

						uint32 compId = tt->id;

						tt = new TypeVector(var->rows, compId);

						CheckTypeExists(&tt);

						compId = tt->id;

						t = new TypeMatrix(var->columns, compId);
					}

				}

				CheckTypeExists(&t);

				TypeBase* pointer = new TypePointer(var->scope == VariableScope::In ? THC_SPIRV_STORAGE_CLASS_INPUT : THC_SPIRV_STORAGE_CLASS_OUTPUT, t->id);

				CheckTypeExists(&pointer);

				InstVariable* variable = new InstVariable(pointer->id, ((TypePointer*)pointer)->storageClass, 0);

				types.Add(variable);
				variables.Add(var);

				var->id = variable->id;

				debugInstructions.Add(new InstName(variable->id, name.string.str));
				annotationIstructions.Add(new InstDecorate(variable->id, THC_SPIRV_DECORATION_LOCATION, (uint32*)&value.value, 1));


				for (uint64 j = 0; j < offset; j++) {
					tokens.RemoveAt(i);
				}

				i--;
			} else if (specifier.string == "binding" || specifier.string == "set") {
			//Uniform
				

			}
		}
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