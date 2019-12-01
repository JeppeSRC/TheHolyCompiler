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
			CreateTypeStruct(tokens, i + 1, nullptr);
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

		instructions.Add(new InstFunctionEnd);
		
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
					ResultVariable tmp = ImplicitCast(retType, type, operandId, &next);

					operandId = res.id;
				}
				

				operation = new InstReturnValue(operandId);
			}

			instructions.Add(operation);

		} else if (token.type == TokenType::ControlFlowIf) {
			ParseIf(declaration, tokens, i, localVariables);

		}
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
		res = ImplicitCast(CreateTypeBool(), res.type, res.id, &tokens[start + 2]);
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

Compiler::ResultVariable Compiler::ParseFunctionCall(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	Token functionName = tokens[info->start];

	ParseInfo inf;
	inf.start = info->start + 1;

	List<ResultVariable> parameterResults = ParseParameters(tokens, &inf, localVariables);

	info->len = inf.len;

	ResultVariable r;

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

			if (dt->type == Type::Pointer) {
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
					res = ImplicitCast(dt, res.type, operandId, &functionName);

				} else {
					decls.RemoveAt(j--);
					continue;
				}
			}
		}
	}

	FunctionDeclaration* decl = decls[0];

	List<ID*> ids = Compiler::GetIDs(parameterResults);

	InstFunctionCall* call = new InstFunctionCall(decl->returnType->typeId, decl->id, (uint32)ids.GetCount(), ids.GetData());
	instructions.Add(call);

	r.id = call->id;
	r.type = decl->returnType;
	r.isVariable = false;

	return r;
}

Compiler::ResultVariable Compiler::ParseTypeConstructor(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	info->end = 0;

	Token tmp = tokens[info->start];

	TypePrimitive* type = (TypePrimitive*)CreateType(tokens, info->start, &info->end);

	List<ResultVariable> parameters = ParseParameters(tokens, info, localVariables);

	ResultVariable res;
	res.type = type;
	res.isConstant = false;
	res.isVariable = false;

	InstBase* inst = nullptr;

	if (type->type == Type::Vector) {
		uint8 numComponents = 0;

		for (uint64 i = 0; i < parameters.GetCount(); i++) {
			TypePrimitive* t = (TypePrimitive*)parameters[i].type;

			if (type->componentType != t->componentType || type->bits != t->bits) {
				Log::CompilerError(tmp, "Argument \"%s\"(%llu) is not compatible with \"%s\"", t->typeString.str, i, type->typeString.str);
			}

			numComponents += t->rows > 0 ? t->rows : 1;
		}

		if (numComponents != type->rows) {
			Log::CompilerError(tmp, "Total component count must be %llu is %u", type->rows, numComponents);
		}

		List<ID*> ids = Compiler::GetIDs(parameters);

		inst = new InstCompositeConstruct(type->typeId, (uint32)ids.GetCount(), ids.GetData());
	} else if (type->type == Type::Matrix) {
		if (parameters.GetCount() != type->columns) {
			Log::CompilerError(tmp, "Argument count must be %llu is %u", parameters.GetCount(), type->columns);
		} 

		for (uint64 i = 0; i < parameters.GetCount(); i++) {
			TypePrimitive* t = (TypePrimitive*)parameters[i].type;

			if (t->componentType != type->componentType || t->bits != type->bits || t->rows != type->rows || t->type != Type::Vector) {
				Log::CompilerError(tmp, "Argument \"%s\"(%llu) is not compatible with \"%s\"", t->typeString.str, i, type->typeString.str);
			}
		}

		List<ID*> ids = Compiler::GetIDs(parameters);

		inst = new InstCompositeConstruct(type->typeId, (uint32)ids.GetCount(), ids.GetData());
	} else {
		Log::CompilerError(tmp, "\"%s\" doesn't have a constructor", type->typeString.str);
	}

	instructions.Add(inst);
	res.id = inst->id;

	return res;
}

List<Compiler::ResultVariable> Compiler::ParseParameters(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	uint64 offset = info->start;

	const Token& parenthesisOpen = tokens[offset];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	uint64 parenthesisClose = FindMatchingToken(tokens, offset++, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (parenthesisClose == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}

	
	List<ResultVariable> parameterResults;
	
	bool moreParams = true;

	do {
		uint64 end = tokens.Find<TokenType>(TokenType::Comma, CmpFunc, offset);

		if (end-- > parenthesisClose) {
			end = parenthesisClose - 1;
			moreParams = false;
		}

		ParseInfo inf;

		inf.start = offset;
		inf.end = end;

		ResultVariable res = ParseExpression(tokens, &inf, localVariables);

		offset = inf.end + 2;

		parameterResults.Add(res);
	} while (moreParams);

	info->len = offset - info->start;

	return std::move(parameterResults);
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