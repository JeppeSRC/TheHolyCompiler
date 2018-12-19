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

				j += len;

			} else if (c0 == '-') {
				tokens.Emplace(TokenType::OperatorSub, "-", l, j);
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
		const Token& token = tokens[i];

		if (token.type == TokenType::DataLayout) {
			ParseLayout(tokens, i);
		} else if (token.type == TokenType::DataIn) {
			ParseInOut(tokens, i, VariableScope::In);
		} else if (token.type == TokenType::DataOut) {
			ParseInOut(tokens, i, VariableScope::Out);
		} else if (token.type == TokenType::DataStruct) {
			CreateTypeStruct(tokens, i + 1);
			tokens.RemoveAt(i);
		} else if (token.type == TokenType::Name) {
			const Token& t2 = tokens[i + 1];
			if (t2.type == TokenType::ParenthesisOpen) {
				ParseFunction(tokens, i - 1);
			} else {
				//TODO: implement global variables
			} 
		}

		i--;
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


		for (uint64 start = 0; start < str->members.GetCount(); start++) {
			if (!CheckGlobalName(str->members[start]->name)) {
				const Token& n = tokens[start + offset];
				Log::CompilerError(n, "Redefinition of global variable \"%s\" in \"%s\"", str->members[start]->name, n.string.str);
			}
		}

		var = CreateGlobalVariable(str, tmp.scope, "_uniform_buf");

		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_BINDING, &binding, 1));
		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_DESCRIPTORSET, &set, 1));
	} else {
		uint64 typeLocation = start + offset++;

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

		TypePrimitive* type = CreateTypePrimitive(tokens, typeLocation);

		var = CreateGlobalVariable(type, tmp.scope, name.string);

		annotationIstructions.Add(new InstDecorate(var->variableId, THC_SPIRV_DECORATION_LOCATION, &location, 1));
	}
	
	start--;

	tokens.Remove(start, start + offset-1);
}

void Compiler::ParseInOut(List<Token>& tokens, uint64 start, VariableScope scope) {
	uint64 offset = 0;

	TypePrimitive* type = CreateTypePrimitive(tokens, ++start + offset);

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

	TypeBase* retType = CreateType(tokens, start);

	const Token& returnType = tokens[start + offset++ ];

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


	while (true) {
		FunctionParameter* param = new FunctionParameter;
		const Token& token = tokens[start + offset++];

		param->constant = token.type == TokenType::ModifierConst;

		if (!param->constant) {
			offset--;
		}

		TypeBase* type = CreateType(tokens, start + offset++);

		if (type == nullptr) {
			const Token& tmp = tokens[start + offset];
			Log::CompilerError(tmp, "Unexpected symbol \"%s\" expected ", tmp.string.str);
		}

		const Token& name = tokens[start + offset++];

		if (name.type == TokenType::Name) {
			param->name = name.string;
			
			if (!CheckGlobalName(name.string)) {
				Log::CompilerWarning(name, "Local parameter \"%s\" overriding global variable", name.string.str);
			}
			//Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
		} else {
			offset--;
		}

		const Token& ref = tokens[start + offset++];

		param->reference = ref.type == TokenType::ModifierReference;

		if (!param->reference) {
			offset--;
			param->type = type;
		} else {
			param->type = CreateTypePointer(type, VariableScope::Function);
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

	auto cmp = [](FunctionDeclaration* const& curr, FunctionDeclaration* const& other) {
		return *curr == other;
	};

	auto CreateDeclarationType = [this](FunctionDeclaration* d) {
		List<uint32> ids;
		for (uint64 i = 0; i < d->parameters.GetCount(); i++) {
			ids.Add(d->parameters[i]->type->typeId);
		}

		InstTypeFunction* f = new InstTypeFunction(d->returnType->typeId, (uint32)ids.GetCount(), ids.GetData());

		CheckTypeExist((InstTypeBase**)&f);

		d->typeId = f->id;
	};

	const Token& bracket = tokens[start + offset++];

	uint64 index = functionDeclarations.Find<FunctionDeclaration*>(decl, cmp);

	if (bracket.type == TokenType::SemiColon) {
		decl->defined = false;
		decl->id = ~0;
		
		if (index == ~0) {
			CreateDeclarationType(decl);
			functionDeclarations.Add(decl);
		} else {
			Log::CompilerError(name, "Redeclaration of function \"%s\"", name.string.str);
		}

	} else if (bracket.type == TokenType::CurlyBracketOpen) {

		if (index == ~0) {
			CreateDeclarationType(decl);
			functionDeclarations.Add(decl);
		} else {
			FunctionDeclaration* old = decl;
			decl = functionDeclarations[index];

			for (uint64 i = 0; i < decl->parameters.GetCount(); i++) {
				decl->parameters[i]->name = old->parameters[i]->name;
			}

			CreateDeclarationType(decl);
		}

		ParseFunctionBody(decl, tokens, start + offset);
	} else {
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \";\" or \"{\"", bracket.string.str);
	}

	tokens.Remove(start, start + offset - 1);
}

void Compiler::ParseFunctionBody(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start) {
	InstFunction* func = new InstFunction(declaration->returnType->typeId, THC_SPIRV_FUNCTION_CONTROL_NONE, declaration->typeId);
	instructions.Add(func);

	declaration->id = func->id;

	for (uint64 i = 0; i < declaration->parameters.GetCount(); i++) {
		const FunctionParameter* p = declaration->parameters[i];

		InstFunctionParameter* pa = nullptr;

		Variable* var = CreateParameterVariable(p, &pa);

		instructions.Add(pa);
	}

	InstLabel* firstBlock = new InstLabel();
	instructions.Add(firstBlock);


	for (uint64 i = start; i < tokens.GetCount(); i++) {
		const Token& token = tokens[i];

		if (token.type == TokenType::CurlyBracketClose) {
			//end of function
			instructions.Add(new InstFunctionEnd);
			return;
		} else if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeMat, TokenType::TypeVec)) {
			//variable declaration
			TypeBase* t = CreateType(tokens, i);
			tokens.RemoveAt(i);

			const Token& name = tokens[i];

			if (name.type != TokenType::Name) {
				Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
			}

			Variable* var = CreateLocalVariable(t, name.string);
			tokens.RemoveAt(i);

			const Token& op = tokens[i];

			if (op.type == TokenType::SemiColon) {
				tokens.RemoveAt(i);
				i--;
				continue;
			}

			if (op.type == TokenType::OperatorAssign) {
				ParseAssignment(var, tokens, i);
				i--;
				continue;
			}
		} 
		else {
			Log::CompilerError(token, "Unexpected symbol \"%s\" expected \"}\"", token.string.str);
		}
	}
}

auto  CmpFunc = [](const Token& curr, const TokenType& c) -> bool {
	return curr.type == c;
};

//start = '='
void Compiler::ParseAssignment(Variable* variable, List<Token>& tokens, uint64 start) {
	uint64 end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, start);
	ResultVariable res = ParseExpression(tokens, start+1, end);

	if (*variable->type != res.type) {
		const Token& tmp = tokens[start];
		Log::CompilerError(tmp, "Right hand type \"%s\" cannot be assigned to left hand type \"%s\"", res.type->typeString.str, variable->type->typeString.str);
	}

	InstStore* store = new InstStore(variable->typePointerId, res.id, 0);
	instructions.Add(store);
	tokens.Remove(start, end);
}

Compiler::ResultVariable Compiler::ParseExpression(List<Token>& tokens, uint64 start, uint64 end) {
	List<Expression> expressions;

	for (uint64 i = start; i <= end; i++) {
		const Token& t = tokens[i];
		Expression e;

		if (t.type == TokenType::Name) { 
			const Token& next = tokens[i + 1];
			if (next.type == TokenType::OperatorSelector) { //Member selection in a struct
				Variable* str = GetVariable(t.string);
				
				if (str == nullptr) {
					Log::CompilerError(t, "Unexpected symbol \"%s\" expected a variable or constat", t.string.str);
				} else if (str->type->type != Type::Struct) {
					Log::CompilerError(next, "Left of operator \".\" must be a struct");
				}

				List<uint32> accessIds;
				uint64 offset = 2;

				TypeStruct* currStr = (TypeStruct*)str->type;

				e.type = ExpressionType::Variable;
				e.variable = new Variable;

				e.variable->scope = str->scope;
				e.variable->name = str->name;

				while (true) {
					const Token& member = tokens[i + offset++];

					if (member.type == TokenType::Name) {
						Log::CompilerError(member, "Right hand of operator \".\" must be a valid name");
					}

					e.variable->name.Append(".").Append(member.string);

					uint32 memberId = currStr->GetMemberIndex(member.string);

					if (memberId == ~0) {
						Log::CompilerError(member, "\"%s\" has no member \"%s\"", currStr->typeString.str, member.string.str);
					}

					accessIds.Add(memberId);

					const Token& selector = tokens[i + offset++];

					if (selector.type == TokenType::OperatorSelector) {
						currStr = (TypeStruct*)currStr->members[i];
						if (currStr->type == Type::Struct) {
							continue;
						} else {
							Log::CompilerError(selector, "Left of operator \".\" must be a struct");
						}
					}

					e.variable->type = currStr->members[memberId];

					break;
				}

				
				e.variable->typePointerId = CreateTypePointer(e.variable->type, e.variable->scope)->typeId;

				InstAccessChain* access = new InstAccessChain(e.variable->typePointerId, str->variableId, (uint32)accessIds.GetCount(), accessIds.GetData());
				e.variable->variableId = access->id;

				instructions.Add(access);
				i += offset-1;
			} else if (next.type == TokenType::ParenthesisOpen) { //FunctionCall
				uint64 removed = 0;
				e.type = ExpressionType::Result;
				e.result = ParseFunctionCall(tokens, i, &removed);

				i += removed;
			} else { //Normal variable
				e.type = ExpressionType::Variable;
				e.variable = GetVariable(t.string);

				if (e.variable == nullptr) {
					Log::CompilerError(t, "Unexpected symbol \"%s\" expected a variable or constat", t.string.str);
				}
			}
		} else if (t.type == TokenType::Value) {
			e.constant.type = CreateTypePrimitive(t);
			e.constant.id = CreateConstant(e.constant.type, (uint32)t.value);
		} else if (t.type >= TokenType::OperatorIncrement && t.type <= TokenType::OperatorDiv) {
			e.type = ExpressionType::Operator;
			e.operatorType = t.type;
		} else if (t.type == TokenType::ParenthesisOpen) {
			uint64 parenthesisClose = FindMatchingToken(tokens, i, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

			if (parenthesisClose > end) {
				Log::CompilerError(t, "\"(\" needs a closing \")\"");
			}

			e.type = ExpressionType::Result;
			e.result = ParseExpression(tokens, i + 1, parenthesisClose - 1);
		}

		expressions.Add(e);
	}

	ResultVariable r;

	return r;
}

Compiler::ResultVariable Compiler::ParseFunctionCall(List<Token>& tokens, uint64 start, uint64* len) {
	const Token& functionName = tokens[start];

	List<ResultVariable> parameterResults;

	uint64 offset = 1;

	const Token& parenthesisOpen = tokens[start + offset++];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	uint64 parenthesisClose = FindMatchingToken(tokens, start + offset - 1, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (parenthesisClose == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}

	bool moreParams = true;

	do {
		uint64 end = tokens.Find<TokenType>(TokenType::Comma, CmpFunc, start + offset);

		if (end > parenthesisClose) {
			end = parenthesisClose-1;
			moreParams = false;
		}

		offset = end + 1;

		ResultVariable res = ParseExpression(tokens, start + offset, end);
		
		parameterResults.Add(res);
	} while (moreParams);

	*len = offset;

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

			FunctionParameter* param = d->parameters[i];
			TypeBase* dt = param->type;

			if (param->reference) {
				//pass by reference but argument is rvalue
				if (!res.isVariable && !param->constant) {
					if (decls.GetCount() == 1) {
						Log::CompilerError(functionName, "argument %llu in \"%s\" must be a lvalue", i, functionName.string.str);
					} else {
						decls.RemoveAt(j--);
						continue;
					}
				}
			} else if (*dt != res.type) {
				if (decls.GetCount() == 1) {
					Log::CompilerError(functionName, "argument %llu in \"%s\" must be an \"%s\"", i, functionName.string.str, dt->typeString.str);
				} else {
					decls.RemoveAt(j--);
					continue;
				}
			}
		}
	}

	FunctionDeclaration* decl = decls[0];

	ResultVariable r;

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