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

auto  CmpFunc = [](const Token& curr, const TokenType& c) -> bool {
	return curr.type == c;
};

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
				tokens.Emplace(TokenType::ParenthesisOpen, "(", l, j+1);
			} else if (c0 == ')') {
				tokens.Emplace(TokenType::ParenthesisClose, ")", l, j+1);
			} else if (c0 == '{') {
				tokens.Emplace(TokenType::CurlyBracketOpen, "{", l, j+1);
			} else if (c0 == '}') {
				tokens.Emplace(TokenType::CurlyBracketClose, "}", l, j+1);
			} else if (c0 == '[') {
				tokens.Emplace(TokenType::BracketOpen, "[", l, j+1);
			} else if (c0 == ']') {
				tokens.Emplace(TokenType::BracketClose, "]", l, j+1);
			} else if (c0 == ';') {
				tokens.Emplace(TokenType::SemiColon, 0, ";", l, j+1);
			} else if (c0 == '+' && c1 == '+') {
				tokens.Emplace(TokenType::OperatorIncrement, "++", l, ++j);
			} else if (c0 == '-' && c1 == '-') {
				tokens.Emplace(TokenType::OperatorDecrement, "--", l, ++j);
			} else if (c0 == '+' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundAdd, "+=", l, ++j);
			} else if (c0 == '-' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundSub, "-=", l, ++j);
			} else if (c0 == '*' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundMul, "*=", l, ++j);
			} else if (c0 == '/' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundDiv, "/=", l, ++j);
			} else if (c0 == '<' && c1 == '<') {
				tokens.Emplace(TokenType::OperatorLeftShift, "<<", l, ++j);
			} else if (c0 == '>' && c1 == '>') {
				tokens.Emplace(TokenType::OperatorRightShift, ">>", l, ++j);
			} else if (c0 == '<' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorLessEqual, "<=", l, ++j);
			} else if (c0 == '>' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorGreaterEqual, ">=", l, ++j);
			} else if (c0 == '&' && c1 == '&') {
				tokens.Emplace(TokenType::OperatorLogicalAnd, "&&", l, ++j);
			} else if (c0 == '|' && c1 == '|') {
				tokens.Emplace(TokenType::OperatorLogicalOr, "||", l, ++j);
			} else if (c0 == '=' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorEqual, "==", l, ++j);
			} else if (c0 == '!' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorNotEqual, "!=", l, ++j);
			} else if (c0 == '+') {
				tokens.Emplace(TokenType::OperatorAdd, "+", l, j+1);
			} else if (c0 == '*') {
				tokens.Emplace(TokenType::OperatorMul, "*", l, j+1);
			} else if (c0 == '/') {
				tokens.Emplace(TokenType::OperatorDiv, "/", l, j+1);
			} else if (c0 == '<') {
				tokens.Emplace(TokenType::OperatorLess, "<", l, j+1);
			} else if (c0 == '>') {
				tokens.Emplace(TokenType::OperatorGreater, ">", l, j+1);
			} else if (c0 == '!') {
				tokens.Emplace(TokenType::OperatorLogicalNot, "!", l, j+1);
			} else if (c0 == '&') {
				tokens.Emplace(TokenType::OperatorBitwiseAnd, "&", l, j+1);
			} else if (c0 == '|') {
				tokens.Emplace(TokenType::OperatorBitwiseOr, "|", l, j+1);
			} else if (c0 == '~') {
				tokens.Emplace(TokenType::OperatorBitwiseNot, "~", l, j+1);
			} else if (c0 == '^') {
				tokens.Emplace(TokenType::OperatorBitwiseXor, "^", l, j+1);
			} else if (c0 == '?') {
				tokens.Emplace(TokenType::OperatorTernary1, "?", l, j+1);
			} else if (c0 == ':') {
				tokens.Emplace(TokenType::OperatorTernary2, ":", l, j+1);
			} else if (c0 == '.') {
				tokens.Emplace(TokenType::OperatorSelector, ".", l, j+1);
			} else if (c0 == ',') {
				tokens.Emplace(TokenType::Comma, ",", l, j+1);
			} else if (c0 == '=') {
				tokens.Emplace(TokenType::OperatorAssign, "=", l, j+1);
			} else if (c0 >= '0' && c0 <= '9') {
				uint64 len = 0;
				
				ValueResult res = Utils::StringToValue(line.str+j-1, &len, l, j);

				Token tmp(TokenType::Value, res.value, line.SubString(j - (line[j - 1] == '-' ? 1 : 0), j + len - 1), l, j);

				switch (res.type) {
					case ValueResultType::Float:
						tmp.valueType = TokenType::TypeFloat;
						break;
					case ValueResultType::Int:
						tmp.valueType = TokenType::TypeInt;
						tmp.sign = res.sign;
				}

				tokens.Emplace(tmp);

				j += len-1;

			} else if (c0 == '-') {
				tokens.Emplace(TokenType::OperatorSub, "-", l, j+1);
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
				}

				tokens.Emplace(TokenType::Name, 0, line.SubString(j, end-1), l, j+1);

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
		const Token& token = tokens[i];

		if (token.type == TokenType::DataLayout) {
			ParseLayout(tokens, i--);
		} else if (token.type == TokenType::DataIn) {
			ParseInOut(tokens, i--, VariableScope::In);
		} else if (token.type == TokenType::DataOut) {
			ParseInOut(tokens, i--, VariableScope::Out);
		} else if (token.type == TokenType::DataStruct) {
			CreateTypeStruct(tokens, i + 1);
			tokens.RemoveAt(i--);
		} else if (token.type == TokenType::Name) {
			const Token& t2 = tokens[i + 1];
			if (t2.type == TokenType::ParenthesisOpen) {
				ParseFunction(tokens, --i);
				i--;
			} else {
				TypeBase* type = CreateType(tokens, i - 1, nullptr);

				Variable* var = CreateGlobalVariable(type, VariableScope::Private, token.string);
				var->isConstant = tokens[i - 1].type == TokenType::ModifierConst;

				if (t2.type == TokenType::SemiColon) {
					continue;
				} else if (t2.type == TokenType::OperatorAssign) {
					 //TODO:
				}
			} 
		}
	}
}

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
		TypeStruct* str = CreateTypeStruct(tokens, start + offset++);

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

	tokens.Remove(start, start + offset-1);
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

void Compiler::ParseFunction(List<Token>& tokens, uint64 start) {
	uint64 offset = 0;

	const Token& returnType = tokens[start];

	TypeBase* retType = CreateType(tokens, start, nullptr);

	if (retType == nullptr) {
		Log::CompilerError(returnType, "Unexpected symbol \"%s\" expected a valid return type");
	}

	const Token& name = tokens[start + offset++];

	if (name.type != TokenType::Name) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
	}

	FunctionDeclaration* decl = new FunctionDeclaration;

	decl->name = name.string;
	decl->returnType = retType;

	const Token& open = tokens[start + offset++];

	if (open.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(open, "Unexpected symbol \"%s\" expected \"(\"", open.string.str);
	}

	const Token& close = tokens[start + offset];

	if (close.type == TokenType::ParenthesisClose) {
		offset++;
		goto naughtyLabel;
	}

	while (true) {
		Variable* param = new Variable;

		param->scope = VariableScope::None;

		const Token& token = tokens[start + offset++];

		param->isConstant = token.type == TokenType::ModifierConst;

		if (!param->isConstant) {
			offset--;
		}

		TypeBase* type = CreateType(tokens, start + offset, nullptr);

		if (type == nullptr) {
			const Token& tmp = tokens[start + offset];
			Log::CompilerError(tmp, "Unexpected symbol \"%s\" expected valid type", tmp.string.str);
		}

		const Token& name = tokens[start + offset++];

		if (name.type == TokenType::Name) {
			param->name = name.string;
			if (!CheckGlobalName(name.string)) {
				Log::CompilerWarning(name, "Local parameter \"%s\" overriding global variable", name.string.str);
			}
		} else {
			offset--;
		}

		const Token& ref = tokens[start + offset++];

		if (ref.type == TokenType::ModifierReference) {
			param->type = CreateTypePointer(type, VariableScope::Function);
		} else {
			offset--;
			param->type = type;
		}

		decl->parameters.Add(param);

		const Token& delim = tokens[start + offset++];

		if (delim.type == TokenType::ParenthesisClose) {
			break;
		} else if (delim.type == TokenType::Comma) {
			continue;
		} else {
			Log::CompilerError(delim, "Unexpected symbol \"%s\" expected \")\" or \",\"", delim.string.str);
		}
	}

	naughtyLabel:

	auto cmp = [](FunctionDeclaration* const& curr, FunctionDeclaration* const& other) {
		return *curr == other;
	};

	const Token& bracket = tokens[start + offset++];

	uint64 index = functionDeclarations.Find<FunctionDeclaration*>(decl, cmp);

	decl->defined = false;

	if (index == ~0) {
		CreateFunctionDeclaration(decl);
	} else if (bracket.type == TokenType::SemiColon) {
		Log::CompilerError(name, "Redeclaration of function \"%s\"", name.string.str);
	}

	instructions.Add(decl->declInstructions);

	if (bracket.type == TokenType::SemiColon) {
		instructions.Add(new InstFunctionEnd);
	} else if (bracket.type == TokenType::CurlyBracketOpen) {
		if (index != ~0)  {
			FunctionDeclaration* old = decl;
			decl = functionDeclarations[index];

			for (uint64 i = 0; i < decl->parameters.GetCount(); i++) {
				decl->parameters[i]->name = old->parameters[i]->name;
			}
		}

		if (decl->defined) {
			Log::CompilerError(tokens[start], "Redefinition");
		}

		VariableStack localVariables(this, decl->parameters);

		instructions.Add(new InstLabel);

		uint64 index = instructions.GetCount();

		ParseBody(decl, tokens, start + offset, &localVariables);
		
		decl->defined = true;
	} else {
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \";\" or \"{\"", bracket.string.str);
	}

	tokens.Remove(start, start + offset - 1);
}

void Compiler::ParseBody(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start, VariableStack* localVariables) {
	uint64 closeBracket = ~0;

	for (uint64 i = start; i < tokens.GetCount(); i++) {
		const Token& token = tokens[i];

		if (token.type == TokenType::CurlyBracketClose) {
			//end of function
			closeBracket = i;
			break;
		} else if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeMatrix, TokenType::TypeVector)) {
			//variable declaration
			TypeBase* t = CreateType(tokens, i, nullptr);

			const Token& name = tokens[i];

			if (name.type != TokenType::Name) {
				Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
			}

			if (!localVariables->CheckName(name)) { }

			CreateLocalVariable(t, name.string, localVariables);

			const Token& next = tokens[i + 1];

			if (next.type == TokenType::SemiColon) {
				tokens.Remove(i, i + 1);
			}

			i--;
			
		} else if (token.type == TokenType::Name) {
			const Token& next = tokens[i + 1];

			uint64 index = typeDefinitions.Find<String>(token.string, findStructFunc);

			if (next.type == TokenType::ParenthesisOpen) {
				ParseInfo inf;
				inf.start = i;

				ParseFunctionCall(tokens, &inf, localVariables);

				tokens.Remove(i, i + inf.len - 1);
			} else if (index != ~0) {
				TypeStruct* str = (TypeStruct*)typeDefinitions[index];
				tokens.RemoveAt(i);

				const Token& name = tokens[i];

				if (name.type != TokenType::Name) {
					Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
				}

				if (!localVariables->CheckName(name)) { }

				CreateLocalVariable(str, name.string, localVariables);

				const Token& assign = tokens[i + 1];

				if (assign.type == TokenType::SemiColon) {
					tokens.Remove(i, i-- + 1);
				}  else {
					Log::CompilerError(assign, "Unexpected symbol \"%s\" expected \";\"", assign.string.str);
				}
			} else {
				ParseInfo inf;
				inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, i);

				if (inf.end-- == ~0) {
					Log::CompilerError(token, "Expression is missing \";\"");
				}

				inf.start = Utils::CompareEnums(tokens[i - 1].type, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement) ? i - 1 : i;

				ParseExpression(tokens, &inf, localVariables);

				i = inf.end+1;
			}
		} else if (token.type == TokenType::ControlFlowReturn) {
			const Token& next = tokens[i + 1];

			InstBase* operation = nullptr;

			bool returnVoid = declaration->returnType->type == Type::Void;

			if (next.type == TokenType::SemiColon) {
				if (!returnVoid) {
					Log::CompilerError(token, "Function must return something that matches the return type");
				}

				operation = new InstReturn;
			} else {
				if (returnVoid) {
					Log::CompilerError(token, "Unexpected symbol \"%s\" expected \";\". Function has return type void", next.string.str);
				}

				ParseInfo inf;
				inf.start = i + 1;
				inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, inf.start);

				if (inf.end-- == ~0) {
					Log::CompilerError(token, "Expression is missing \";\"");
				}

				ResultVariable res = ParseExpression(tokens, &inf, localVariables);

				TypeBase* type = res.type;
				TypeBase* retType = declaration->returnType;

				ID* operandId;

				if (res.isVariable) {
					InstLoad* load = new InstLoad(type->typeId, res.id, 0);
					instructions.Add(load);

					operandId = load->id;
				} else {
					operandId = res.id;
				}

				if (*type != retType) {
					ResultVariable tmp = Cast(retType, type, operandId);

					if (tmp.id == nullptr) {
						Log::CompilerError(next, "No suitable conversion between return type(%s) and \"%s\"", retType->typeString.str, type->typeString.str);
					} else {
						Log::CompilerWarning(next, "Implicit conversion from \"%s\" to return type(%s)", type->typeString.str, retType->typeString.str);
					}

					operandId = res.id;
				}
				

				operation = new InstReturnValue(operandId);
			}

			instructions.Add(operation);

		} else if (token.type == TokenType::ControlFlowIf) {
			ParseIf(declaration, tokens, i, localVariables);

		} /*else {
			uint64 end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, start);

			if (end == ~0) {
				Log::CompilerError(token, "Expression is missing \";\"");
			}

			ParseExpression(tokens, --i, end - 1, localVariables);

			tokens.Remove(i, end);
		}*/
	}

	tokens.Remove(start, closeBracket);
}

void Compiler::ParseIf(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start, VariableStack* localVariables) {
	const Token& parenthesisOpen = tokens[start + 1];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	ParseInfo inf;
	inf.start = start + 2;
	inf.end = FindMatchingToken(tokens, start, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (inf.end-- == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}


	ResultVariable res = ParseExpression(tokens, &inf, localVariables);

	if (!Utils::CompareEnums(res.type->type, CompareOperation::Or, Type::Int, Type::Float, Type::Bool)) {
		Log::CompilerError(tokens[start + 2], "Expression must result in a scalar bool, int or float type. Is \"%s\"", res.type->typeString.str);
	}

	if (res.isVariable) {
		InstLoad* load = new InstLoad(res.type->typeId, res.id, 0);
		instructions.Add(load);

		res.id = load->id;
	}

	if (res.type->type != Type::Bool) {
		TypeBase* tmp = res.type;
		res = Cast(CreateTypeBool(), res.type, res.id);

		if (res.id == nullptr) {
			Log::CompilerError(tokens[start+2], "No suitable conversion between \"bool\" and \"%s\"", tmp->typeString.str);
		} else {
			Log::CompilerWarning(tokens[start + 2], "Implicit conversion from \"%s\" to \"bool\"", tmp->typeString.str);
		}
	}

	tokens.Remove(start, inf.end+1);

	const Token& bracket = tokens[start];

	if (bracket.type != TokenType::CurlyBracketOpen) { //TODO: one line if statements
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \"{\"", bracket.string.str);
	}

	
}

Compiler::Variable* Compiler::ParseName(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	uint64 offset = 0;

	const Token& name = tokens[info->start + offset++];

	Variable* var = GetVariable(name.string, localVariables);
	Variable* result;

	if (var == nullptr) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected a variable", name.string.str);
	} 

	Token op = tokens[info->start + offset++];

	if (Utils::CompareEnums(op.type, CompareOperation::Or, TokenType::OperatorSelector, TokenType::BracketOpen)) {
		List<ID*> accessIds;
		
		String n = name.string;
		TypeBase* curr = var->type;

		while (true) {
			if (op.type == TokenType::OperatorSelector) {
				if (curr->type != Type::Struct) {
					Log::CompilerError(op, "Left of operator \".\" must be a struct");
				}

				const Token& member = tokens[info->start + offset++];

				if (member.type != TokenType::Name) {
					Log::CompilerError(member, "Right of operator \".\" must be a valid name");
				}

				TypeStruct* s = (TypeStruct*)curr;

				uint64 index = s->GetMemberIndex(member.string);

				if (index == ~0) {
					Log::CompilerError(member, "\"%s\" doesn't have a member named \"%s\"", n.str, member.string.str);
				}

				n.Append(".").Append(member.string);
				curr = s->members[index].type;

				accessIds.Add(CreateConstantS32((int32)index));
			} else if (op.type == TokenType::BracketOpen) {
				if (curr->type != Type::Array) {
					Log::CompilerError(op, "\"%s\" is not an array", n.str);
				}

				TypeArray* arr = (TypeArray*)curr;

				ParseInfo inf;
				inf.start = info->start+offset;
				inf.end = tokens.Find<TokenType>(TokenType::BracketClose, CmpFunc, info->start+offset);

				if (inf.end-- == ~0) {
					Log::CompilerError(op, "\"[\" needs a closing \"]\"");
				}

				ResultVariable index = ParseExpression(tokens, &inf, localVariables);

				if (index.type->type != Type::Int) {
					Log::CompilerError(op, "Array index must be a (signed) integer scalar");
				} else {
					TypePrimitive* p = (TypePrimitive*)index.type;

					if (!p->sign) {
						Log::CompilerWarning(op, "Array index is unsigned but will be treated as signed");
					}
				}

				if (index.isVariable) {
					InstLoad* load = new InstLoad(index.type->typeId, index.id, 0);
					instructions.Add(load);

					accessIds.Add(load->id);
				} else {
					accessIds.Add(index.id);
				}

				n.Append("[]");

				curr = arr->elementType;

				offset = (inf.end - info->start)+2;
			} else {
				break;
			}

			op = tokens[info->start + offset++];
		}
		
		if (accessIds.GetCount() != 0) {
			TypePointer* pointer = CreateTypePointer(curr, var->scope);

			InstAccessChain* access = new InstAccessChain(pointer->typeId, var->variableId, (uint32)accessIds.GetCount(), accessIds.GetData());

			instructions.Add(access);

			result = new Variable;

			result->scope = var->scope;
			result->name = n;
			result->type = curr;
			result->typePointerId = pointer->typeId;
			result->variableId = access->id;
			result->isConstant = var->isConstant;
		} 
	} else {
		result = var;
	}

	info->len = offset-1;

	return result;
}

Compiler::ResultVariable Compiler::ParseExpression(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	List<Expression> expressions;
	List<Variable*> tmpVariables;
	List<InstBase*> postIncrements;


	for (uint64 i = info->start; i <= info->end; i++) {
		const Token& t = tokens[i];
		Expression e = {ExpressionType::Undefined};

		if (t.type == TokenType::Name) {
			const Token& next = tokens[i + 1];
			if (next.type == TokenType::OperatorSelector || next.type == TokenType::BracketOpen) { //Member selection in a struct and/or array subscripting
				ParseInfo inf;
				inf.start = i;

				Variable* v = ParseName(tokens, &inf, localVariables);

				e.type = ExpressionType::Variable;
				e.variable = v;
				e.parent = tokens[i + inf.len - 1];

				tmpVariables.Add(v);

				i += inf.len-1;
			} else { //Normal variable
				e.type = ExpressionType::Variable;
				e.variable = GetVariable(t.string, localVariables);
				e.parent = t;

				if (e.variable == nullptr) {
					Log::CompilerError(t, "Unexpected symbol \"%s\" expected a variable or constant", t.string.str);
				}
			}
		} else if (t.type == TokenType::Value) {
			e.type = ExpressionType::Constant;
			e.constant.type = CreateTypePrimitiveScalar(ConvertToType(t.valueType), 32, t.sign);
			e.constant.id = CreateConstant(e.constant.type, (uint32)t.value);
			e.parent = t;
		} else if (t.type >= TokenType::OperatorIncrement && t.type <= TokenType::OperatorCompoundDiv) {
			e.type = ExpressionType::Operator;
			e.operatorType = t.type;
			e.parent = t;
		} else if (t.type >= TokenType::TypeVoid && t.type <= TokenType::TypeMatrix && info->start == info->end) {
			if (info->start == info->end) {//Type for a cast
				//Log::CompilerError(t, "Unexpected symbol \"%s\"", t.string.str);
				if (!Utils::CompareEnums(t.type, CompareOperation::Or, TokenType::TypeInt, TokenType::TypeFloat)) {
					Log::CompilerError(t, "Cast type must be scalar of type integer or float");
				}

				e.type = ExpressionType::Type;
				e.castType = CreateTypePrimitiveScalar(ConvertToType(t.type), t.bits, t.sign);
				e.parent = t;
			} else {
				uint64 parenthesisOpen = ~0;
				uint64 parenthesisClose = ~0;

				for (uint64 j = 1; j <= 4; j++) {
					if (tokens[i + j].type == TokenType::ParenthesisOpen) {
						parenthesisOpen = i+j;
						parenthesisClose = FindMatchingToken(tokens, parenthesisOpen, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

						if (parenthesisClose == ~0) {
							Log::CompilerError(tokens[parenthesisOpen], "\"(\" needs a closing \")\"");
						}

						break;
					}
				}

				if (parenthesisOpen == ~0) {
					Log::CompilerError(t, "Unexpected symbol \"%s\", did you mean to construct a type? If so you're missing the parentheses");
				}

				ParseInfo inf;

				inf.start = i;
				inf.end = parenthesisClose;

				e.type = ExpressionType::Result;
				e.result = ParseFunctionCall(tokens, &inf, localVariables);

				info->end -= parenthesisClose - inf.end;
			}

		} else if (t.type == TokenType::ParenthesisOpen) {
			ParseInfo inf;
			
			e.type = ExpressionType::Result;
			e.parent = t;
			
			uint64 parenthesisClose = FindMatchingToken(tokens, i, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

			if (parenthesisClose > info->end) {
				Log::CompilerError(t, "\"(\" needs a closing \")\"");
			}

			bool isFunction = i > 0 ? tokens[i-1].type == TokenType::Name : false;

			if (isFunction) {
				inf.start = i - 1;
				inf.end = ++parenthesisClose;

				e.result = ParseFunctionCall(tokens, &inf, localVariables);
			}else {
				inf.start = i + 1;
				inf.end = --parenthesisClose;

				e.result = ParseExpression(tokens, &inf, localVariables);
			}

			info->end -= parenthesisClose - inf.end;
		}

		if (e.type != ExpressionType::Undefined) expressions.Add(e);
	}

#pragma region precedence 1
	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		//post increment/decrement
		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement)) {
			if (i < 1) continue;
			Expression& left = expressions[i - 1];

			bool rightVar = false;

			if (i < expressions.GetCount()-1) {
				const Expression& right = expressions[i + 1];

				rightVar = right.type == ExpressionType::Variable;
			}


			if (left.type == ExpressionType::Variable) {
				if (left.variable->isConstant) {
					Log::CompilerError(left.parent, "Left hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(left.variable->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(left.parent, "Left hand operand of must be a interger/float scalar");
				}

				const Variable* var = left.variable;
				
				InstLoad* load = new InstLoad(var->type->typeId, var->variableId, 0);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : -1U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}

				InstStore* store = new InstStore(var->variableId, operation->id, 0);

				instructions.Add(load);
				instructions.Add(operation);

				postIncrements.Add(store);

				left.type = ExpressionType::Result;
				left.result.isVariable = false;
				left.result.type = var->type;
				left.result.id = load->id;

				expressions.RemoveAt(i--);
			} else if (rightVar) {
				Log::CompilerError(e.parent, "Left or right hand operand must be a lvalue");
			}
		}
	}

#pragma endregion

#pragma region precedence 2

	for (uint64 i = expressions.GetCount()-1; (int64)i >= 0; i--) {
		const Expression& e = expressions[i];

		if (e.type == ExpressionType::Type) { //Cast
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type);

			
			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer or float");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer or float");
			}

			if (*e.castType == type) {
				Log::CompilerWarning(e.parent, "Unnecessary cast");
				expressions.RemoveAt(i);
			} else {
				right.type = ExpressionType::Result;
				right.result = Cast(e.castType, type, operandId);

				if (right.result.id == nullptr) {
					Log::CompilerError(e.parent, "The only castable types are scalar integers or floats");
				}

				expressions.RemoveAt(i);
			}
			
		} else if (e.type != ExpressionType::Operator) {
			continue;
		}

		//pre increment/decrement
		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement)) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "Left or right hand operand must be a lvalue");
			}

			Expression& right = expressions[i + 1];

			if (right.type == ExpressionType::Variable) {
				if (right.variable->isConstant) {
					Log::CompilerError(right.parent, "Right hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(right.variable->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(right.parent, "Right hand operand of must be a interger/float scalar");
				}

				const Variable* var = right.variable;

				InstLoad* load = new InstLoad(var->type->typeId, var->variableId, 0);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : -1U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}
				
				InstStore* store = new InstStore(var->variableId, operation->id, 0);

				instructions.Add(load);
				instructions.Add(operation);
				instructions.Add(store);

				right.type = ExpressionType::Result;
				right.result.isVariable = false;
				right.result.type = var->type;
				right.result.id = operation->id;

				expressions.RemoveAt(i);
			} else {
				Log::CompilerError(e.parent, "Right hand operand must be lvalue");
			}

		} else if (e.operatorType == TokenType::OperatorNegate) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			InstBase* operation = nullptr;

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			if (type->componentType == Type::Int) {
				if (!type->sign) {
					if (type->type == Type::Vector) {
						type = CreateTypePrimitiveVector(Type::Int, type->bits, 1, type->rows);
					} else {
						type = CreateTypePrimitiveScalar(Type::Int, type->bits, 1);
					}
				}

				operation = new InstSNegate(type->typeId, operandId);
			} else if (type->componentType == Type::Float) {
				operation = new InstFNegate(type->typeId, operandId);
			} else {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.result.isVariable = false;
			right.result.type = type;
			right.result.id = operation->id;

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorLogicalNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a variable or value");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer, float or a bool result from an expression");
			}

			InstBase* operation = nullptr;

			ID* constantId = nullptr;

			if (type->type != Type::Bool) {
				type = CreateTypeBool();
			} else {
				float tmp = 0.0f;
				constantId = CreateConstant(type, type->type == Type::Float ? *(uint32*)&tmp : 0);
			}
			

			if (type->type == Type::Bool) {
				operation = new InstLogicalNot(type->typeId, operandId);
			} else if (type->type == Type::Int) {
				operation = new InstINotEqual(type->typeId, operandId, constantId);
			} else if (type->type == Type::Float) {
				operation = new InstFOrdNotEqual(type->typeId, operandId, constantId);
			} 

			instructions.Add(operation);

			right.result.isVariable = false;
			right.result.type = type;
			right.result.id = operation->id;

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorBitwiseNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Vector)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			InstNot* operation = new InstNot(type->typeId, operandId);
			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.result.isVariable = false;
			right.result.type = type;
			right.result.id = operation->id;

			expressions.RemoveAt(i);
		}
	}

#pragma endregion

#pragma region precedence 3

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorMul || e.operatorType == TokenType::OperatorDiv) {
			bool mul = e.operatorType == TokenType::OperatorMul;
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			InstBase* instruction = nullptr;
			ResultVariable ret = {0};

			if (*lType == rType) {
				if (Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
					if (mul) {
						ret = Multiply(lType, lOperandId, rOperandId);
					} else {
						ret = Divide(lType, lOperandId, rOperandId);
					}
				} else if (lType->type == Type::Matrix) {
					if (mul) {
						instruction = new InstMatrixTimesMatrix(lType->typeId, lOperandId, rOperandId);
						ret.type = lType;
						ret.id = instruction->id;
					} else {
						Log::CompilerError(e.parent, "Cannot divide 2 matrices");
					}
				} else {
					Log::CompilerError(e.parent, "Invalid types, cannot multiply or divide types");
				}
			} else {
				if (lType->type == Type::Vector && mul) {
					if (rType->type == Type::Int || rType->type == Type::Float) {
						if (lType->componentType != rType->type || lType->bits != rType->bits) {
							Log::CompilerError(e.parent, "Scalar must match component type");
						}

						instruction = new InstVectorTimesScalar(lType->typeId, lOperandId, rOperandId);
						ret.type = lType;
						ret.id = instruction->id;
					} else if (rType->type == Type::Matrix) {
						if (lType->componentType != rType->componentType || lType->bits != rType->bits || lType->rows != rType->columns) {
							Log::CompilerError(e.parent, "Component types must match");
						}

						instruction = new InstVectorTimesMatrix(lType->typeId, lOperandId, rOperandId);
						ret.type = lType;
						ret.id = instruction->id;
					} else {
						Log::CompilerError(e.parent, "Type missmatch");
					}
				} else if (lType->type == Type::Matrix && mul) {
					if (rType->type == Type::Int || rType->type == Type::Float) {
						if (lType->componentType != rType->type || lType->bits != rType->bits) {
							Log::CompilerError(e.parent, "Scalar must match component type");
						}

						instruction = new InstMatrixTimesScalar(lType->typeId, lOperandId, rOperandId);
						ret.type = lType;
						ret.id = instruction->id;
					} else if (rType->type == Type::Vector) {
						if (lType->componentType != rType->componentType || lType->bits != rType->bits || lType->columns != rType->rows) {
							Log::CompilerError(e.parent, "Component types must match");
						}

						instruction = new InstMatrixTimesVector(lType->typeId, lOperandId, rOperandId);
						ret.type = lType;
						ret.id = instruction->id;
					} else {
						Log::CompilerError(e.parent, "Type missmatch");
					}
				}
			}

			if (!instruction) instructions.Add(instruction);

			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;
			
			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 4

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorAdd || e.operatorType == TokenType::OperatorSub) {
			bool add = e.operatorType == TokenType::OperatorAdd;
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

			if (*lType == rType) {
				if (Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
					if (add) {
						ret = Add(lType, lOperandId, rOperandId);
					} else {
						ret = Subtract(lType, lOperandId, rOperandId);
					}
				}  else {
					Log::CompilerError(e.parent, "Invalid types, cannot add or subtract types");
				}
			}

			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 5

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorRightShift || e.operatorType == TokenType::OperatorLeftShift) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);


			if (*lType != rType) {
				Log::CompilerError(e.parent, "Type missmatch, types must be eaqual");
			}

			InstBase* instruction = nullptr;
			ResultVariable ret = { 0 };


			if (e.operatorType == TokenType::OperatorRightShift) {
				instruction = new InstShiftRightLogical(lType->typeId, lOperandId, rOperandId);
			} else {
				instruction = new InstShiftLeftLogical(lType->typeId, lOperandId, rOperandId);
			}

			instructions.Add(instruction);

			ret.type = lType;
			ret.id = instruction->id;
			
			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 6

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorLess, TokenType::OperatorLessEqual, TokenType::OperatorGreater, TokenType::OperatorGreaterEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;
			ResultVariable ret = { 0 };

			if (!Utils::CompareEnums(left.type, CompareOperation::Or, Type::Int, Type::Float) || !Utils::CompareEnums(right.type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			bool floatCmp = false;
			
			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						convInst = new InstFConvert(lType->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = new InstFConvert(rType->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, rType->typeString.str);
					}
				} else { // Int
					if (rType->sign) {
						convInst = new InstConvertSToF(lType->typeId, rOperandId);
						rId = convInst->id;
					} else {
						convInst = new InstConvertUToF(lType->typeId, rOperandId);
						rId = convInst->id;
					}

					Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
				}

				floatCmp = true;
			} else { //Int
				if (rType->type == Type::Int) {
					if (lType->sign != rType->sign) {
						Log::CompilerWarning(right.parent, "Sign missmatch");
					}

					TypePrimitive* tmp = nullptr;

					if (lType->bits > rType->bits) {
						convInst = rType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 1))->typeId, rOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 0))->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, tmp->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = lType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 1))->typeId, lOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 0))->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, tmp->typeString.str);
					}
				} else { // Float
					if (lType->sign) {
						convInst = new InstConvertSToF(rType->typeId, lOperandId);
						lId = convInst->id;
					} else {
						convInst = new InstConvertUToF(rType->typeId, lOperandId);
						lId = convInst->id;
					}

					Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, rType->typeString.str);

					floatCmp = true;
				}
			}

			ret.type = CreateTypeBool();
			ID* retTypeId = ret.type->typeId;

			if (floatCmp) {
				switch (e.operatorType) {
					case TokenType::OperatorLess:
						instruction = new InstFOrdLessThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorLessEqual:
						instruction = new InstFOrdLessThanEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreater:
						instruction = new InstFOrdGreaterThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreaterEqual:
						instruction = new InstFOrdGreaterThanEqual(retTypeId, lId, rId);
						break;
				}
			} else {
				switch (e.operatorType) {
					case TokenType::OperatorLess:
						instruction = lType->sign ? new InstSLessThan(retTypeId, lId, rId) : (InstBase*)new InstULessThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorLessEqual:
						instruction = lType->sign ? new InstSLessThanEqual(retTypeId, lId, rId) : (InstBase*)new InstULessThanEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreater:
						instruction = lType->sign ? new InstSGreaterThan(retTypeId, lId, rId) : (InstBase*)new InstUGreaterThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreaterEqual:
						instruction = lType->sign ? new InstSGreaterThanEqual(retTypeId, lId, rId) : (InstBase*)new InstUGreaterThanEqual(retTypeId, lId, rId);
						break;
				}
			}

			if (convInst) instructions.Add(convInst);
			instructions.Add(instruction);

			ret.id = instruction->id;

			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion
	
#pragma region precedence 7

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorEqual, TokenType::OperatorNotEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;
			ResultVariable ret = { 0 };

			if (!Utils::CompareEnums(left.type, CompareOperation::Or, Type::Int, Type::Float) || !Utils::CompareEnums(right.type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			bool floatCmp = false;

			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						convInst = new InstFConvert(lType->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = new InstFConvert(rType->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, rType->typeString.str);
					}
				} else { // Int
					if (rType->sign) {
						convInst = new InstConvertSToF(lType->typeId, rOperandId);
						rId = convInst->id;
					} else {
						convInst = new InstConvertUToF(lType->typeId, rOperandId);
						rId = convInst->id;
					}

					Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
				}

				floatCmp = true;
			} else { //Int
				if (rType->type == Type::Int) {
					TypePrimitive* tmp = nullptr;

					if (lType->bits > rType->bits) {
						convInst = rType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 1))->typeId, rOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 0))->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, tmp->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = lType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 1))->typeId, lOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 0))->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, tmp->typeString.str);
					}
				} else { // Float
					if (lType->sign) {
						convInst = new InstConvertSToF(rType->typeId, lOperandId);
						lId = convInst->id;
					} else {
						convInst = new InstConvertUToF(rType->typeId, lOperandId);
						lId = convInst->id;
					}

					Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, rType->typeString.str);

					floatCmp = true;
				}
			}

			ret.type = CreateTypeBool();
			ID* retTypeId = ret.type->typeId;

			if (floatCmp) {
				switch (e.operatorType) {
					case TokenType::OperatorEqual:
						instruction = new InstFOrdEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorNotEqual:
						instruction = new InstFOrdNotEqual(retTypeId, lId, rId);
						break;
				}
			} else {
				switch (e.operatorType) {
					case TokenType::OperatorEqual:
						instruction = new InstIEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorNotEqual:
						instruction = new InstINotEqual(retTypeId, lId, rId);
						break;
				}
			}

			if (convInst) instructions.Add(convInst);
			instructions.Add(instruction);

			ret.id = instruction->id;

			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 8

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorEqual, TokenType::OperatorNotEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

			if (lType->type != Type::Int || rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int");
			}

			//TODO: check matching bith width

			InstBase* inst = new InstBitwiseAnd(lType->typeId, lOperandId, rOperandId);

			instructions.Add(inst);

			ret.type = lType;
			ret.id = inst->id;

			left.type == ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

	instructions.Add(postIncrements);

	ResultVariable result = {0};

	if (expressions.GetCount() > 1) {
		const Expression& e = expressions[1];
		Log::CompilerError(e.parent, "Unexpected symbol \"%s\"", e.parent.string.str);
	}

	Expression e = expressions[0];

	switch (e.type) {
		case ExpressionType::Variable:
			result.isVariable = true;
			result.type = e.variable->type;
			result.id = e.variable->variableId;
			break;
		case ExpressionType::Constant:
			result.isConstant = true;
		case ExpressionType::Result:
			result = e.result;
			break;
	}

	return result;
}

Compiler::ResultVariable Compiler::ParseFunctionCall(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	Token functionName = tokens[info->start];
	TypeBase* type = nullptr;

	if (functionName.type == TokenType::OperatorGreater) {
		uint64 len = 0;
		info->start -= 3;

		functionName = tokens[info->start];
		type = CreateType(tokens, info->start, &len);
		functionName.string = type->typeString;

		info->end -= len;
	}

	List<ResultVariable> parameterResults;

	uint64 offset = 0;

	const Token& parenthesisOpen = tokens[info->start + offset++];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	uint64 parenthesisClose = FindMatchingToken(tokens, info->start + offset - 1, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (parenthesisClose == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}

	bool moreParams = true;

	offset += info->start;

	do {
		uint64 end = tokens.Find<TokenType>(TokenType::Comma, CmpFunc, offset);

		if (end-- > parenthesisClose) {
			end = parenthesisClose-1;
			moreParams = false;
		}

		ParseInfo inf;

		inf.start = offset;
		inf.end = end;

		ResultVariable res = ParseExpression(tokens, &inf, localVariables);

		offset = end + 2;
		
		parameterResults.Add(res);
	} while (moreParams);

	info->len = offset-info->start;

	ResultVariable r;

	if (type == nullptr) {
		uint64 fOffset = 0;
	
		List<FunctionDeclaration*> decls = GetFunctionDeclarations(functionName.string);

		if (!decls.GetCount()) {
			Log::CompilerError(functionName, "No function with name \"%s\"", functionName.string.str);
		}
	

		for (uint64 i = 0; i < decls.GetCount(); i++) {
			if (decls[i]->parameters.GetCount() != parameterResults.GetCount()) {
				decls.RemoveAt(i--);
			}
		}

		if (decls.GetCount() == 0) {
			Log::CompilerError(functionName, "No overloaded version of function \"%s()\" takes %llu arguments", functionName.string.str, parameterResults.GetCount());
		}

		for (uint64 i = 0; i < parameterResults.GetCount(); i++) {
			ResultVariable& res = parameterResults[i];

			for (uint64 j = 0; j < decls.GetCount(); j++) {
				FunctionDeclaration* d = decls[j];

				Variable* param = d->parameters[i];
				TypeBase* dt = param->type;

				if (param->type->type == Type::Pointer) {
					//pass by reference but argument is rvalue
					if (!res.isVariable /*&& !param->isConstant*/) {
						if (decls.GetCount() == 1) {
							Log::CompilerError(functionName, "argument %llu in \"%s\" must be a lvalue", i, functionName.string.str);
						} else {
							decls.RemoveAt(j--);
							continue;
						}
					}
				} else if (*dt != res.type) {
					if (decls.GetCount() == 1) {
						ID* operandId = res.id;

						if (res.isVariable) {
							InstLoad* load = new InstLoad(res.type->typeId, res.id, 0);
							instructions.Add(load);

							operandId = load->id;
						}

						TypeBase* tmp = res.type;
						res = Cast(dt, res.type, operandId);

						if (res.id == nullptr) {
							Log::CompilerError(functionName, "argument %llu in \"%s\" must be an \"%s\"", i, functionName.string.str, dt->typeString.str);
						} else {
							Log::CompilerWarning(functionName, "Implicit conversion from \"%s\" to \"%s\"", tmp->typeString.str, dt->typeString.str);
						}
					} else {
						decls.RemoveAt(j--);
						continue;
					}
				}
			}
		}

		FunctionDeclaration* decl = decls[0];

		ID** ids = new ID*[parameterResults.GetCount()];

		for (uint64 i = 0; i < parameterResults.GetCount(); i++) {
			ids[i] = parameterResults[i].id;
		}

		InstFunctionCall* call = new InstFunctionCall(decl->returnType->typeId, decl->id, (uint32)decl->parameters.GetCount(), ids);
		instructions.Add(call);

		delete[] ids;

		r.id = call->id;
		r.type = decl->returnType;
		r.isVariable = false;
	} else if (Utils::CompareEnums(functionName.type, CompareOperation::Or, TokenType::TypeVector, TokenType::TypeMatrix)) {

	} else {
		Log::CompilerError(functionName, "%s is not a function", functionName.string.str);
	}


	return r;
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