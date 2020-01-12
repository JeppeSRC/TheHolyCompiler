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
#include <core/preprocessor/preprocessor.h>
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;
using namespace instruction;

auto  CmpFunc = [](const Token& curr, const TokenType& c) -> bool {
	return curr.type == c;
};

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
		Symbol* param = new Symbol(SymbolType::Parameter);

		param->parameter.scope = VariableScope::Function;

		const Token& token = tokens[start + offset++];

		param->parameter.isConst = token.type == TokenType::ModifierConst;

		if (!param->parameter.isConst) {
			offset--;
		}

		TypeBase* type = CreateType(tokens, start + offset, nullptr);

		if (type == nullptr) {
			const Token& tmp = tokens[start + offset];
			Log::CompilerError(tmp, "Unexpected symbol \"%s\" expected valid type", tmp.string.str);
		}

		const Token& ref = tokens[start + offset++];

		if (ref.type == TokenType::ModifierReference) {
			param->type = CreateTypePointer(type, VariableScope::Function);
			param->parameter.isReference = true;
		} else {
			offset--;
			param->type = type;
			param->parameter.isReference = false;
		}

		const Token& name = tokens[start + offset++];

		if (name.type == TokenType::Name) {
			param->parameter.name = name.string;
			if (!CheckGlobalName(name.string)) {
				Log::CompilerWarning(name, "Local parameter \"%s\" overriding global variable", name.string.str);
			}
		} else {
			offset--;
			param->parameter.name = "";
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

	if (index == ~0) {
		CreateFunctionDeclaration(decl);
	} else if (bracket.type == TokenType::SemiColon) {
		Log::CompilerError(name, "Redeclaration of function \"%s\"", name.string.str);
	}

	if (bracket.type == TokenType::CurlyBracketOpen) {
		if (index != ~0) {
			FunctionDeclaration* old = decl;
			decl = functionDeclarations[index];

			for (uint64 i = 0; i < decl->parameters.GetCount(); i++) {
				if ((decl->parameters[i]->parameter.name = old->parameters[i]->parameter.name) == "") {
					Log::CompilerError(name, "Parameter %u needs a name!", i);
				}
			}
		}

		instructions.Add(decl->declInstructions);

		if (decl->defined) {
			Log::CompilerError(tokens[start], "Redefinition");
		}

		VariableStack localVariables(this, decl->parameters);

		instructions.Add(new InstLabel);

		uint64 index = instructions.GetCount();

		ParseBody(decl, tokens, start + offset, &localVariables);

		instructions.InsertList(index, localVariables.variableInstructions); //Add all OpVariable instructions at the beginning of the first block

		InstBase* last = instructions[instructions.GetCount() - 1];

		if (last->opCode != THC_SPIRV_OPCODE_OpReturn && last->opCode != THC_SPIRV_OPCODE_OpReturnValue) {
			instructions.Add(new InstReturn());
		}

		instructions.Add(new InstFunctionEnd);

		decl->defined = true;
	} else if (bracket.type != TokenType::SemiColon) {
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \";\" or \"{\"", bracket.string.str);
	}

	tokens.Remove(start, start + offset - 1);
}

void Compiler::CreateFunctionDeclaration(FunctionDeclaration* decl) {
	if (decl->declInstructions.GetCount() != 0) {
		return;
	}

	decl->defined = false;

	CreateFunctionType(decl);

	InstFunction* func = new InstFunction(decl->returnType->typeId, THC_SPIRV_FUNCTION_CONTROL_NONE, decl->typeId);
	decl->declInstructions.Add(func);

	debugInstructions.Add(new InstName(func->id, decl->name.str));

	decl->id = func->id;

	for (uint64 i = 0; i < decl->parameters.GetCount(); i++) {
		Symbol* v = decl->parameters[i];

		InstFunctionParameter* pa = new InstFunctionParameter(v->type->typeId);

		v->id= pa->id;

		decl->declInstructions.Add(pa);

		debugInstructions.Add(new InstName(pa->id, (decl->name + "_" + v->parameter.name).str));
	}

	functionDeclarations.Add(decl);
}

Compiler::Symbol* Compiler::ParseFunctionCall(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	Token functionName = tokens[info->start];

	ParseInfo inf;
	inf.start = info->start + 1;

	List<Symbol*> parameterResults = ParseParameters(tokens, &inf, localVariables);

	info->len = inf.len;

	uint64 fOffset = 0;

	FunctionDeclaration* decl = GetFunctionDeclaration(functionName.string);

	if (decl == nullptr) {
		Log::CompilerError(functionName, "No function called \"%s\" exists", functionName.string.str);
	}

	for (uint64 i = 0; i < parameterResults.GetCount(); i++) {
		Symbol* res = parameterResults[i];


		Symbol* param = decl->parameters[i];
		TypeBase* dt = param->type;

		if (dt->type == Type::Pointer) {
			//pass by reference but argument is rvalue
			if (res->symbolType != SymbolType::Variable) {
				Log::CompilerError(functionName, "argument %llu in \"%s\" must be a lvalue", i, functionName.string.str);
			}
		} else if (*dt != res->type) {
			continue;
		}
	}
	List<ID*> ids = Compiler::GetIDs(parameterResults);

	InstFunctionCall* call = new InstFunctionCall(decl->returnType->typeId, decl->id, (uint32)ids.GetCount(), ids.GetData());
	instructions.Add(call);
	
	return new Symbol(SymbolType::Result, decl->returnType, call->id);
}

Compiler::Symbol* Compiler::ParseTypeConstructor(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	info->end = 0;

	Token tmp = tokens[info->start];

	TypePrimitive* type = (TypePrimitive*)CreateType(tokens, info->start, &info->end);

	List<Symbol*> arguments = ParseParameters(tokens, info, localVariables);

	Symbol* res = new Symbol(SymbolType::Constant, type);

	InstBase* inst = nullptr;

	List<ID*> ids;

	if (type->type == Type::Vector) {
		uint8 numComponents = 0;

		for (uint64 i = 0; i < arguments.GetCount(); i++) {
			TypePrimitive* t = (TypePrimitive*)arguments[i]->type;

			if (type->componentType != t->componentType || type->bits != t->bits) {
				Log::CompilerError(tmp, "Argument \"%s\"(%llu) is not compatible with \"%s\"", t->typeString.str, i, type->typeString.str);
			}

			numComponents += t->rows > 0 ? t->rows : 1;

			if (arguments[i]->symbolType != SymbolType::Constant) res->symbolType = SymbolType::Result;

		}

		if (numComponents != type->rows) {
			Log::CompilerError(tmp, "Total component count must be %llu is %u", type->rows, numComponents);
		}

		ids = Compiler::GetIDs(arguments);
	} else if (type->type == Type::Matrix) {
		if (arguments.GetCount() != type->columns) {
			Log::CompilerError(tmp, "Argument count must be %llu is %u", arguments.GetCount(), type->columns);
		}

		for (uint64 i = 0; i < arguments.GetCount(); i++) {
			TypePrimitive* t = (TypePrimitive*)arguments[i]->type;

			if (t->componentType != type->componentType || t->bits != type->bits || t->rows != type->rows || t->type != Type::Vector) {
				Log::CompilerError(tmp, "Argument \"%s\"(%llu) is not compatible with \"%s\"", t->typeString.str, i, type->typeString.str);
			}

			if (arguments[i]->symbolType != SymbolType::Constant) res->symbolType = SymbolType::Result;
		}

		ids = Compiler::GetIDs(arguments);
	} else {
		Log::CompilerError(tmp, "\"%s\" doesn't have a constructor", type->typeString.str);
	}

	if (res->symbolType == SymbolType::Constant) {
		inst = new InstConstantComposite(type->typeId, (uint32)ids.GetCount(), ids.GetData());
		CheckConstantExist(&inst);
	} else {
		inst = new InstCompositeConstruct(type->typeId, (uint32)ids.GetCount(), ids.GetData());
		instructions.Add(inst);
	}
	
	res->id = inst->id;

	return res;
}

List<Compiler::Symbol*> Compiler::ParseParameters(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	uint64 offset = info->start;

	const Token& parenthesisOpen = tokens[offset];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	uint64 parenthesisClose = FindMatchingToken(tokens, offset++, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (parenthesisClose == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}


	List<Symbol*> parameterResults;

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

		Symbol* res = ParseExpression(tokens, &inf, localVariables);

		offset = inf.end + 2;

		parameterResults.Add(res);
	} while (moreParams);

	info->len = offset - info->start;

	return std::move(parameterResults);
}

}
}
}