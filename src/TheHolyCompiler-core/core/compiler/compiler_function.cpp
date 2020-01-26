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
#include <vector>
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

	const Token& bracket = tokens[start + offset++];

	uint64 index = ~0;

	String declSig = GetFunctionSignature(decl);

	for (uint64 i = 0; i < functionDeclarations.GetCount(); i++) {
		FunctionDeclaration* d = functionDeclarations[i];

		if (d->name == decl->name) {
			if (GetFunctionSignature(d) == declSig) {
				index = i;
				break;
			} else {
				index--;
				break;
			}
		}
	}

	if (index == ~0) {
		CreateFunctionDeclaration(decl);
	} else if (bracket.type == TokenType::SemiColon || index == ~0 - 1) {
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

		if (v->type->type == Type::Pointer) {
			v->type = ((TypePointer*)v->type)->baseType;
		}

		decl->declInstructions.Add(pa);

		debugInstructions.Add(new InstName(pa->id, (decl->name + "_" + v->parameter.name).str));
	}

	functionDeclarations.Add(decl);
}

Compiler::Symbol* Compiler::ParseFunctionCall(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	Token functionName = tokens[info->start];

	info->start++;

	List<Symbol*> arguments = ParseParameters(tokens, info, localVariables);

	Symbol* ret = ParseExtFunctionCall(functionName, arguments);

	if (ret) return ret;

	ret = ParseBuiltinFunctionCall(functionName, arguments);

	if (ret) return ret;

	uint64 fOffset = 0;

	FunctionDeclaration* decl = GetFunctionDeclaration(functionName.string);

	if (decl == nullptr) {
		Log::CompilerError(functionName, "No function called \"%s\" exists", functionName.string.str);
	}

	String declSig = GetFunctionSignature(decl);

	if (arguments.GetCount() != decl->parameters.GetCount()) {
		Log::CompilerError(functionName, "Fuction %s takes %llu arguments", declSig.str, decl->parameters.GetCount());
	}

	List<ID*> ids(arguments.GetCount());

	for (uint64 i = 0; i < arguments.GetCount(); i++) {
		Symbol* res = arguments[i];
		Symbol* param = decl->parameters[i];

		TypeBase* argType = res->type;
		TypeBase* paramType = param->type;

		TypeBase* tmp = GetSwizzledType((TypePrimitive*)argType, res->swizzleIndices);

		if (*paramType != tmp) {
			Log::CompilerError(functionName, "Missmatching type in argument %u (%s) in function %s", i, tmp->typeString.str, declSig.str);
		}

		if (param->parameter.isReference) {
			if (res->symbolType == SymbolType::Variable) {
				if (res->variable.isConst && !param->parameter.isConst) {
					Log::CompilerError(functionName, "Argument %u (%s) in function %s is const", i, argType->typeString.str, declSig.str);
				}

				if (res->swizzleIndices.GetCount() > 0) {
					Log::CompilerError(functionName, "Argument %u (%s) in function %s cannot be swizzled", i, tmp->typeString.str, declSig.str);
				}

				ids.Add(res->id);
			} else {
				Log::CompilerError(functionName, "Argument %u (%s) in function %s must be lvalue", i, argType->typeString.str, declSig.str);
			}
		} else {
			ID* id = res->id;

			if (res->symbolType == SymbolType::Variable) {
				id = LoadVariable(res, true);
			} 

			if (res->swizzleIndices.GetCount() > 0) {
				id = GetSwizzledVector((TypePrimitive**)&tmp, id, res->swizzleIndices);
			}

			ids.Add(id);
		}

		
		
	}

	InstFunctionCall* call = new InstFunctionCall(decl->returnType->typeId, decl->id, (uint32)ids.GetCount(), ids.GetData());
	instructions.Add(call);
	
	return new Symbol(SymbolType::Result, decl->returnType, call->id);
}

Compiler::Symbol* Compiler::ParseExtFunctionCall(const Token& functionName, List<Symbol*>& arguments) {
	static ExtFunctionDeclaration tmp[] { 
		{"round",		1, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"roundeven",	2, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"trunc",		3, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"fabs",		4, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"sabs",		5, 1,  {ExtParamType::IntegerVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"fsign",		6, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"ssign",		7, 1,  {ExtParamType::IntegerVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"floor",		8, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"ceil",		9, 1,  {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"fract",		10, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"radians",		11, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"degrees",		12, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"sin",			13, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"cos",			14, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"tan",			15, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"asin",		16, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"acos",		17, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"atan",		18, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"sinh",		19, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"cosh",		20, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"tanh",		21, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"asinh",		22, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"acosh",		23, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"atanh",		24, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"atan2",		25, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"pow",			26, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"exp",			27, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"log",			28, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"exp2",		29, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"log2",		30, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"sqrt",		31, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"invsqrt",		32, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"determinant",	33, 1, {ExtParamType::FloatMatrix,			ExtParamType::None,											ExtParamType::None} },
		{"inverse",		34, 1, {ExtParamType::FloatMatrix,			ExtParamType::None,											ExtParamType::None} },
		{"modf",		35, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar | ExtParamType::Reference,	ExtParamType::None} },
		{"fmin",		37, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"umin",		38, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"smin",		39, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"fmax",		40, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"umax",		41, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"smax",		42, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"fclamp",		43, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"uclamp",		44, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"sclamp",		45, 2, {ExtParamType::IntegerVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"fmix",		46, 3, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::FloatVectorScalar} },
		{"step",		48, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"sstep",		49, 3, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::FloatVectorScalar} },
		{"fma",			50, 3, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::FloatVectorScalar} },
		{"frexp",		51, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"ldexp",		53, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::IntegerVectorScalar,							ExtParamType::None} },
		{"length",		66, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"distance",	67, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"cross",		68, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"normalize",	69, 1, {ExtParamType::FloatVectorScalar,	ExtParamType::None,											ExtParamType::None} },
		{"fforward",	70, 3, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::FloatVectorScalar} },
		{"reflect",		71, 2, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::None} },
		{"refract",		72, 3, {ExtParamType::FloatVectorScalar,	ExtParamType::FloatVectorScalar,							ExtParamType::FloatScalar} },
		};

	List<ExtFunctionDeclaration> decls;

	decls.Add(tmp);

	uint64 index = decls.Find<String>(functionName.string, [](const ExtFunctionDeclaration& item, const String& name) {
		return item.name == name;
	});

	if (index == ~0) return nullptr;

	const ExtFunctionDeclaration& decl = decls[index];

	if (decl.params != arguments.GetCount()) {
		Log::CompilerError(functionName, "Function \"%s\" doesn't take %llu arguments", functionName.string.str, arguments.GetCount());
	}

	List<ID*> ids;

	for (uint64 i = 0; i < arguments.GetCount(); i++) {
		Symbol* arg = arguments[i];
		ExtParamType param = decl.param[i];

		if (Utils::CompareEnums(param, CompareOperation::Equal, ExtParamType::Integer, ExtParamType::Vector, ExtParamType::Scalar)) {
			if (arg->type->type == Type::Vector) {
				if (arg->pType->componentType != Type::Int) {
					Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be a vector or scalar of type int", i, functionName.string.str);
				}
			} else if (arg->type->type != Type::Int) {
				Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be a vector or scalar of type int", i, functionName.string.str);
			}
		} else if (Utils::CompareEnums(param, CompareOperation::Equal, ExtParamType::Float, ExtParamType::Vector, ExtParamType::Scalar)) {
			if (arg->type->type == Type::Vector) {
				if (arg->pType->componentType != Type::Float) {
					Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be a vector or scalar of type float", i, functionName.string.str);
				}
			} else if (arg->type->type != Type::Float) {
				Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be a vector or scalar of type float", i, functionName.string.str);
			}
		} else if (Utils::CompareEnums(param, CompareOperation::Equal, ExtParamType::Float, ExtParamType::Matrix)) {
			if (arg->type->type == Type::Matrix) {
				if (arg->pType->componentType != Type::Float || arg->pType->rows != arg->pType->columns) {
					Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be a square matrix of type float", i, functionName.string.str);
				}
			}
		}

		if (arg->symbolType == SymbolType::Variable || (arg->symbolType == SymbolType::Parameter && arg->parameter.isReference)) {
			if (param == ExtParamType::Reference) {
				ids.Add(arg->id);
			} else {
				ids.Add(LoadVariable(arg, true));
			}
		} else if (param == ExtParamType::Reference) {
			Log::CompilerError(functionName, "Argument %llu in function \"%s\" must be of pointer type", i, functionName.string.str);
		} else {
			ids.Add(arg->id);
		}

	}

	if (extendedInstructionSet == nullptr) extendedInstructionSet = new InstExtInstImport("GLSL.std.450");

	InstBase* call = new InstExtInst(arguments[0]->type->typeId, extendedInstructionSet->id, decl.opCode, decl.params, ids.GetData());
	instructions.Add(call);

	return new Symbol(SymbolType::Result, arguments[0]->type, call->id);
}

Compiler::Symbol* Compiler::ParseBuiltinFunctionCall(const Token& functionName, List<Symbol*>& arguments) {
	Symbol* res = nullptr;

	if (functionName.string == "texture") {
		if (arguments.GetCount() != 2) {
			Log::CompilerError(functionName, "Function \"texture\" does not take %llu arguments", arguments.GetCount());
		}

		Symbol* arg0 = arguments[0];
		Symbol* arg1 = arguments[1];

		if (arg0->symbolType != SymbolType::Variable || arg0->type->type != Type::Image) {
			Log::CompilerError(functionName, "Argument 0 in function \"texture\" needs to be a sampler2D");
		}

		uint8 rows = 0;

		switch (arg0->iType->imageType) {
			case ImageType::Image1D:
				rows = 1;
				break;
			case ImageType::Image2D:
				rows = 2;
				break;
			case ImageType::Image3D:
			case ImageType::ImageCube:
				rows = 3;
		}

		if (rows == 0) {
			Log::CompilerError(functionName, "Invalid sampler type");
		}

		List<ID*> ids;

		if (arg1->type->type == Type::Vector && arg1->pType->rows == rows) {
			ids.Add(LoadVariable(arg0, false));
			
			if (arg1->symbolType == SymbolType::Variable || arg1->symbolType == SymbolType::Parameter) {
				ids.Add(LoadVariable(arg1, true));
			} else {
				ids.Add(arg1->id);
			}
		} else {
			Log::CompilerError(functionName, "Argument 1 in funciton \"texture\" needs to ba a vec%u_float32", rows);
		}

		TypeBase* retType = CreateTypePrimitiveVector(Type::Float, 32, 0, 4);

		InstBase* func = new InstImageSampledImplicitLod(retType->typeId, ids[0], ids[1], 0, 0, nullptr);
		instructions.Add(func);

		res = new Symbol(SymbolType::Result, retType, func->id);
	}

	return res;
}

Compiler::Symbol* Compiler::ParseTypeConstructor(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	Token tmp = tokens[info->start];

	TypePrimitive* type = (TypePrimitive*)CreateType(tokens, info->start, &info->len);

	List<Symbol*> arguments = ParseParameters(tokens, info, localVariables);

	Symbol* res = new Symbol(SymbolType::Constant, type);

	InstBase* inst = nullptr;

	List<ID*> ids;

	if (type->type == Type::Vector) {
		uint8 numComponents = 0;

		for (uint64 i = 0; i < arguments.GetCount(); i++) {
			Symbol* arg = arguments[i];
			TypePrimitive* t = (TypePrimitive*)arg->type;

			if (arg->swizzleIndices.GetCount() != 0) arg->id = GetSwizzledVector(&t, LoadVariable(arg, true), arg->swizzleIndices);

			if (type->componentType != t->componentType || type->bits != t->bits) {
				Log::CompilerError(tmp, "Argument \"%s\"(%llu) is not compatible with \"%s\"", t->typeString.str, i, type->typeString.str);
			}

			numComponents += t->rows > 0 ? t->rows : 1;

			if (arg->symbolType != SymbolType::Constant) res->symbolType = SymbolType::Result;

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

	if (ids.GetCount() > 1) {
		if (res->symbolType == SymbolType::Constant) {
			inst = new InstConstantComposite(type->typeId, (uint32)ids.GetCount(), ids.GetData());
			CheckConstantExist(&inst);
		} else {
			inst = new InstCompositeConstruct(type->typeId, (uint32)ids.GetCount(), ids.GetData());
			instructions.Add(inst);
		}

		res->id = inst->id;
	} else {
		res->id = ids[0];
	}
	

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
		uint64 open = tokens.Find<TokenType>(TokenType::ParenthesisOpen, CmpFunc, offset);
		uint64 close = FindMatchingToken(tokens, open, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);
		parenthesisClose = FindMatchingToken(tokens, info->start, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

		if (end > open && end < close) {
			end = tokens.Find<TokenType>(TokenType::Comma, CmpFunc, close);
		}

		if (end-- > parenthesisClose) {
			end = parenthesisClose - 1;
			moreParams = false;
		}

		ParseInfo inf;

		inf.start = offset;
		inf.end = end;
		inf.len = 0;

		Symbol* res = ParseExpression(tokens, &inf, localVariables);

		offset = inf.end + 2;

		info->len += inf.len;

		parameterResults.Add(res);
	} while (moreParams);

	info->end = parenthesisClose;

	return std::move(parameterResults);
}

String Compiler::GetFunctionSignature(FunctionDeclaration* decl) {
	return GetFunctionSignature(decl->parameters, decl->name);
}

String Compiler::GetFunctionSignature(List<Symbol*> parameters, const String& functionName) {
	String result = functionName + "(";

	for (uint64 i = 0; i < parameters.GetCount(); i++) {
		THC_ASSERT(parameters[i]->symbolType == SymbolType::Parameter);

		Symbol* param = parameters[i];

		if (param->parameter.isConst) result += "const ";
		
		result += param->type->typeString;

		if (param->parameter.isReference) {
			result += "&, ";//* FUNCTION
			uint64 index = result.Find("*");
			if (index != ~0) result.Remove(index, index+9);
		} else {
			result += ", ";
		}
	}
	
	uint64 len = result.length-1;

	if (parameters.GetCount() > 0) result.Remove(len-1, len);

	result += ")";

	return result;
}

}
}
}