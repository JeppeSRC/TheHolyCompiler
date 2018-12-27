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
#include <util/utils.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace instruction;
using namespace type;

bool Compiler::findStructFunc(TypeBase* const& curr, const String& name) {
	if (curr->type == Type::Struct) {
		return curr->typeString == name;
	}

	return false;
};

InstBase* Compiler::GetInstFromID(uint32 id)  {
	auto cmp = [](InstBase* const& curr, const uint32& id) -> bool {
		return curr->id == id;
	};

	uint64 index = instructions.Find<uint32>(id, cmp);

	if (index != ~0) {
		return instructions[index];
	}
	
	return nullptr;
}

void Compiler::CheckTypeExist(InstTypeBase** type) {
	auto cmp = [](InstBase* const& curr, InstTypeBase* const& type) -> bool {
		if (curr->type != InstType::Type) return false;

		InstTypeBase* t = (InstTypeBase*)curr;

		if (t->type == type->type) {
			return *t == type;
		}
		
		return false;
	};

	uint64 index = types.Find<InstTypeBase*>(*type, cmp);

	if (index == ~0) {
		types.Add(*type);
	} else {
		delete *type;

		*type = (InstTypeBase*)types[index];
	}
}

void Compiler::CheckTypeExist(TypeBase** type) {
	auto cmp = [](TypeBase* const& curr, TypeBase* const& type) -> bool {
		return *curr == type;
	};

	uint64 index = typeDefinitions.Find<TypeBase*>(*type, cmp);

	if (index == ~0) {
		typeDefinitions.Add(*type);
	} else {
		delete *type;

		*type = (TypeBase*)typeDefinitions[index];
	}
}

void Compiler::CheckConstantExist(InstBase** constant) {
	uint64 index = types.Find<InstBase*>(*constant, [](InstBase* const& curr, InstBase* const& inst) -> bool {
		return *curr == inst;
	});

	if (index == ~0) {
		types.Add(*constant);
	} else {
		delete *constant;

		*constant = types[index];
	}
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitive(const Token& token) {
	THC_ASSERT(token.type == TokenType::Value);
	TypePrimitive* type = new TypePrimitive;

	type->type = ConvertToType(token.valueType);
	type->componentType = type->type;
	type->bits = 32;
	type->sign = token.sign;
	type->rows = 0;
	type->columns = 0;
	type->typeString = GetTypeString(type);
	type->typeId = ~0;

	CheckTypeExist((TypeBase**)&type);

	if (type->typeId != ~0) {
		return type;
	}

	InstTypeBase* t = nullptr;

	switch (type->type) {
		case Type::Int:
			t = new InstTypeInt(type->bits, type->sign);
			break;
		case Type::Float:
			t = new InstTypeFloat(type->bits);
			break;
	}

	CheckTypeExist((InstTypeBase**)&t);

	type->typeId = t->id;

	return type;
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitive(List<Token>& tokens, uint64 start) {

	uint64 offset = 0;

	const Token& token = tokens[start + offset++];

	if (!Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeVec, TokenType::TypeMat, TokenType::TypeVoid)) {
		Log::CompilerError(token, "Unexpecet symbol \"%s\" expected a valid type", token.string.str);
	} else if (token.type == TokenType::TypeVoid) {
		TypePrimitive* var = new TypePrimitive;
		var->type = Type::Void;
		var->typeString = "void";
		var->componentType = Type::Void;
		var->bits = 0;
		var->sign = 0;
		var->rows = 0;
		var->columns = 0;
		var->typeId = ~0;

		CheckTypeExist((TypeBase**)&var);

		tokens.RemoveAt(start);

		if (var->typeId != ~0) {
			return var;
		}

		InstTypeVoid* v = new InstTypeVoid();

		types.Add(v);

		var->typeId = v->id;
		typeDefinitions.Add(var);

		return var;
	}

	TypePrimitive tmpVar;

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeVec, TokenType::TypeMat)) {
		const Token& open = tokens[start + offset++];

		if (open.type == TokenType::OperatorLess) {
			const Token& type = tokens[start + offset++];

			if (!Utils::CompareEnums(type.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt)) {
				Log::CompilerError(type, "Unexpected symbol \"%s\" expected a valid type", type.string.str);
			}

			const Token& close = tokens[start + offset++];

			if (close.type != TokenType::OperatorGreater) {
				Log::CompilerError(close, "Unexpected symbol \"%s\" expected a \">\"", close.string.str);
			}

			tmpVar.componentType = ConvertToType(type.type);
			tmpVar.bits = type.bits;
			tmpVar.sign = type.sign;
		} else {
			tmpVar.componentType = Type::Float;;
			tmpVar.bits = CompilerOptions::FPPrecision32() ? 32 : 64;
			tmpVar.sign = 0;

			offset--;
		}
	} else {
		tmpVar.componentType = ConvertToType(token.type);
		tmpVar.bits = token.bits;
		tmpVar.sign = token.sign;
	}
	

	tmpVar.type = ConvertToType(token.type);
	tmpVar.rows = token.rows;
	tmpVar.columns = token.columns;
	
	uint64 index = typeDefinitions.Find<TypePrimitive*>(&tmpVar, [](TypeBase* const& curr, TypePrimitive* const& other) -> bool {
		return *curr == other;
	});

	TypePrimitive* var = nullptr;

	if (index != ~0) {
		var = (TypePrimitive*)typeDefinitions[index];
	} else {

		switch (tmpVar.type) {
			case Type::Int:
			case Type::Float:
				var = CreateTypePrimitiveScalar(tmpVar.type, tmpVar.bits, tmpVar.sign);
				break;
			case Type::Vector:
				var = CreateTypePrimitiveVector(tmpVar.componentType, tmpVar.bits, tmpVar.sign, tmpVar.rows);
				break;
			case type::Type::Matrix:
				var = CreateTypePrimtiveMatrix(tmpVar.componentType, tmpVar.bits, tmpVar.sign, tmpVar.rows, tmpVar.columns);
				break;
		}
	}
	
	tokens.Remove(start, start + offset-1);

	return var;
}

Compiler::TypePrimitive* Compiler::CreateTypeBool() {
	TypePrimitive* var = new TypePrimitive;

	var->type = Type::Bool;
	var->componentType = Type::Bool;
	var->bits = 0;
	var->sign = 0;
	var->rows = 0;
	var->columns = 0;
	var->typeId = ~0;

	CheckTypeExist((TypeBase**)&var);

	if (var->typeId != ~0) {
		return var;
	}

	InstTypeBool* b = new InstTypeBool;

	CheckTypeExist((InstTypeBase**)&b);

	var->typeId = b->id;
	var->typeString = "bool";

	typeDefinitions.Add(var);

	return var;
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitiveScalar(Type type, uint8 bits, uint8 sign) {
	THC_ASSERT(Utils::CompareEnums(type, CompareOperation::Or, Type::Int, Type::Float));

	TypePrimitive* var = new TypePrimitive;

	var->type = type;
	var->componentType = type;
	var->bits = bits;
	var->sign = sign;
	var->rows = 0;
	var->columns = 0;
	var->typeId = ~0;

	CheckTypeExist((TypeBase**)&var);

	if (var->typeId != ~0) {
		return var;
	}

	InstTypeBase* t = nullptr;

	switch (type) {
		case Type::Int:
			t = new InstTypeInt(bits, sign);
			break;
		case Type::Float:
			t = new InstTypeFloat(bits);
			break;
	}

	CheckTypeExist((InstTypeBase**)&t);

	var->typeId = t->id;
	var->typeString = GetTypeString(var);

	typeDefinitions.Add(var);

	return var;
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitiveVector(Type componentType, uint8 bits, uint8 sign, uint8 rows) {
	THC_ASSERT(Utils::CompareEnums(componentType, CompareOperation::Or, Type::Int, Type::Float));

	TypePrimitive* vec = new TypePrimitive;

	vec->type = Type::Vector;
	vec->componentType = componentType;
	vec->bits = bits;
	vec->sign = sign;
	vec->rows = rows;
	vec->columns = 0;
	vec->typeId = ~0;

	CheckTypeExist((TypeBase**)&vec);

	if (vec->typeId != ~0) {
		return vec;
	}
	
	InstTypeVector* t = new InstTypeVector(vec->rows, CreateTypePrimitiveScalar(componentType, bits, sign)->typeId);

	CheckTypeExist((InstTypeBase**)&t);

	vec->typeId = t->id;
	vec->typeString = GetTypeString(vec);

	typeDefinitions.Add(vec);

	return vec;
}

Compiler::TypePrimitive* Compiler::CreateTypePrimtiveMatrix(Type componentType, uint8 bits, uint8 sign, uint8 rows, uint8 columns) {
	THC_ASSERT(Utils::CompareEnums(componentType, CompareOperation::Or, Type::Int, Type::Float));

	TypePrimitive* mat = new TypePrimitive;

	mat->type = Type::Matrix;
	mat->componentType = componentType;
	mat->bits = bits;
	mat->sign = sign;
	mat->rows = rows;
	mat->columns = columns;
	mat->typeId = ~0;

	CheckTypeExist((TypeBase**)&mat);

	if (mat->typeId != ~0) {
		return mat;
	}

	TypeBase* vec = CreateTypePrimitiveVector(componentType, bits, sign, rows);

	InstTypeMatrix* m = new InstTypeMatrix(columns, vec->typeId);

	CheckTypeExist((InstTypeBase**)&m);

	mat->typeId = m->id;
	mat->typeString = GetTypeString(mat);

	return mat;
}

Compiler::TypeStruct* Compiler::CreateTypeStruct(List<Token>& tokens, uint64 start) {
	TypeStruct* var = new TypeStruct;

	uint64 offset = 0;

	const Token& name = tokens[start + offset++];

	if (name.type != TokenType::Name) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected valid name", name.string.str);
		return nullptr;
	}

	const Token& bracket = tokens[start + offset++];

	if (bracket.type != TokenType::CurlyBracketOpen) {
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \"{\"", bracket.string.str);
	}

	List<uint32> ids;

	while (true) {
		//const Token& type = tokens[start + offset];

		TypeBase* tmp = CreateType(tokens, start + offset);

		/*if (Utils::CompareEnums(type.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeVec, TokenType::TypeMat)) {
			uint64 typeLocation = start + offset;

			tmp = CreateType(tokens, typeLocation);
		} else {
			uint64 index = typeDefinitions.Find<String>(type.string, findStructFunc);

			tmp = typeDefinitions[index];

			offset++;
		}*/

		const Token& tokenName = tokens[start + offset++];

		if (tokenName.type != TokenType::Name) {
			Log::CompilerError(tokenName, "Unexpected symbol \"%s\" expected valid name", tokenName.string.str);
		}

		uint64 index = var->members.Find<String>(tokenName.string, [](const StructMember& curr, const String& name) -> bool {
			if (curr.name == name) {
				return true;
			}

			return false;
		});

		if (index != ~0) {
			Log::CompilerError(tokenName, "There is already a member callad \"%s\" in \"%s\"", tokenName.string.str, name.string.str);
		}

		const Token& semi = tokens[start + offset++];

		if (semi.type != TokenType::SemiColon) {
			Log::CompilerError(semi, "Unexpected symbol \"%s\" expected \";\"", semi.string.str);
		}

		StructMember m;

		m.name = tokenName.string;
		m.type = tmp;

		var->members.Emplace(m);

		ids.Add(tmp->typeId);

		if (tokens[start + offset].type == TokenType::CurlyBracketClose) {
			const Token& t = tokens[start + ++offset];

			if (t.type == TokenType::SemiColon) {
				break;
			} else {
				Log::CompilerError(t, "Unexpected symbol \"%s\" expected \";\"", t.string.str);
			}
		}
	}

	var->type = Type::Struct;
	var->typeString = name.string;

	if (typeDefinitions.Find<String>(var->typeString, findStructFunc) == ~0) {
		typeDefinitions.Add(var);
	} else {
		delete var;
		Log::CompilerError(name, "Struct redefinition");
	}

	InstTypeStruct* st = new InstTypeStruct((uint32)ids.GetCount(), ids.GetData());

	CheckTypeExist((InstTypeBase**)&st);

	tokens.Remove(start, start + offset);

	uint32 memberOffset = 0;

	for (uint64 i = 0; i < var->members.GetCount(); i++) {
		annotationIstructions.Add(new InstMemberDecorate(st->id, i, THC_SPIRV_DECORATION_OFFSET, &memberOffset, 1));

		memberOffset += var->members[i].type->GetSize();
	}

	var->typeId = st->id;

	return var;
}

Compiler::TypeArray* Compiler::CreateTypeArray(List<Token>& tokens, uint64 start) {
	TypeArray* var = new TypeArray;

	uint64 offset = 0;

	const Token& token = tokens[start + offset++];

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeInt, TokenType::TypeFloat, TokenType::TypeVec, TokenType::TypeMat)) {
		var->elementType = CreateTypePrimitive(tokens, start);
		offset--;
	} else if (token.type == TokenType::Name) {
		uint64 index = typeDefinitions.Find<String>(token.string, findStructFunc);

		if (index != ~0) {
			var->elementType = typeDefinitions[index];
		} else {
			Log::CompilerError(token, "Unexpected symbol \"%s\" expected valid type", token.string.str);
		}

	} else {
		Log::CompilerError(token, "Unexpected symbol \"%s\" expected valid type", token.string.str);
	}

	const Token& open = tokens[start + offset++];

	if (open.type != TokenType::BracketOpen) {
		Log::CompilerError(open, "Unexpected symbol \"%s\" expected \"[\"", open.string.str);
	}

	const Token& count = tokens[start + offset++];

	if (count.type != TokenType::Value) {
		Log::CompilerError(count, "Unexpected symbol \"%s\" expected a valid integer value", count.string.str);
	}

	const Token& close = tokens[start + offset++];

	if (close.type != TokenType::BracketClose) {
		Log::CompilerError(close, "Unexpected symbol \"%s\" expected \"]\"", close.string.str);
	}

	var->type = Type::Array;
	var->typeString = GetTypeString(var->elementType) + "[" + count.string + "]";
	var->elementCount = (uint32)count.value;
	var->typeId = ~0;

	CheckTypeExist((TypeBase**)&var);

	tokens.Remove(start, start + offset-1);

	if (var->typeId != ~0) {
		return var;
	}

	InstTypeArray* array = new InstTypeArray(CreateConstantS32(var->elementCount), var->elementType->typeId);

	var->typeId = array->id;

	types.Add(array);

	return var;
}

Compiler::TypeBase* Compiler::CreateType(List<Token>& tokens, uint64 start) {
	const Token& token = tokens[start];

	const Token& arr = tokens[start + 1];

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeMat, TokenType::TypeVec, TokenType::TypeVoid)) {
		if (arr.type == TokenType::BracketOpen) {
			return CreateTypeArray(tokens, start);
		} else {
			return CreateTypePrimitive(tokens, start);
		}
	}

	if (token.type == TokenType::Name) {
		uint64 index = typeDefinitions.Find<String>(token.string, findStructFunc);

		if (index != ~0) {
			if (arr.type == TokenType::BracketOpen) {
				return CreateTypeArray(tokens, start);
			} else {
				return typeDefinitions[index];
			}
		}
	}

	return nullptr;
}

String Compiler::GetTypeString(const TypeBase* const type) const {
	String name;

	const TypeArray* arr = (const TypeArray*)type;
	const TypePrimitive* prim = (const TypePrimitive*)type;
	const TypePointer* p = (const TypePointer*)type;

	switch (type->type) {
		case Type::Void:
			name = "void";
			break;
		case Type::Bool:
			name = "bool";
			break;
		case Type::Int:
			switch (prim->sign) {
				case 0:
					name = "uint";
					break;
				default:
					name = "int";
			}


			switch (prim->bits) {
				case 8:
					name.Append("8");
					break;
				case 16:
					name.Append("16");
					break;
				case 32:
					name.Append("32");
					break;
				case 64:
					name.Append("64");
					break;
			}

			break;
		case Type::Float:
			name = "float";
			switch (prim->bits) {
				case 32:
					name.Append("32");
					break;
				case 64:
					name.Append("64");
					break;
			}
			break;

		case Type::Vector:
			name = "vec";

			switch (prim->rows) {
				case 2:
					name.Append("2");
					break;
				case 3:
					name.Append("3");
					break;
				case 4:
					name.Append("4");
					break;
			}

			name.Append("_");

			switch (prim->componentType) {
				case Type::Int:
					switch (prim->sign) {
						case 0:
							name.Append("uint");
							break;
						default:
							name.Append("int");
					}


					switch (prim->bits) {
						case 8:
							name.Append("8");
							break;
						case 16:
							name.Append("16");
							break;
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
				case Type::Float:
					name.Append("float");
					switch (prim->bits) {
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
			}

			break;

		case Type::Matrix:
			name = "mat";

			switch (prim->columns) {
				case 3:
					name.Append("3");
					break;
				case 4:
					name.Append("4");
					break;
			}

			name.Append("_");

			switch (prim->componentType) {
				case Type::Int:
					switch (prim->sign) {
						case 0:
							name.Append("uint");
							break;
						default:
							name.Append("int");
					}


					switch (prim->bits) {
						case 8:
							name.Append("8");
							break;
						case 16:
							name.Append("16");
							break;
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
				case Type::Float:
					name.Append("float");
					switch (prim->bits) {
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
			}

			break;

		case Type::Pointer:
			name = GetTypeString(p->pointerType).Append("* ");

			switch (p->storageClass) {
				case THC_SPIRV_STORAGE_CLASS_INPUT:
					name.Append("INPUT");
					break;
				case THC_SPIRV_STORAGE_CLASS_OUTPUT:
					name.Append("OUTPUT");
					break;
				case THC_SPIRV_STORAGE_CLASS_PRIVATE:
					name.Append("PRIVATE");
					break;
				case THC_SPIRV_STORAGE_CLASS_UNIFORM:
					name.Append("UNIFORM");
					break;
				case THC_SPIRV_STORAGE_CLASS_FUNCTION:
					name.Append("FUNCTION");
					break;
			}

			break;

		default:
			name = type->typeString;
			break;

	}

	return name;
}

uint32 Compiler::ScopeToStorageClass(VariableScope scope) {
	switch (scope) {
		case VariableScope::In:
			return THC_SPIRV_STORAGE_CLASS_INPUT;
		case VariableScope::Out:
			return THC_SPIRV_STORAGE_CLASS_OUTPUT;
		case VariableScope::Private:
			return THC_SPIRV_STORAGE_CLASS_PRIVATE;
		case VariableScope::Uniform:
			return THC_SPIRV_STORAGE_CLASS_UNIFORM;
		case VariableScope::Function:
			return THC_SPIRV_STORAGE_CLASS_FUNCTION;
	}

	return ~0;
}

Compiler::Variable* Compiler::GetVariable(const String& name) const {
	for (uint64 i = 0; i < localVariables.GetCount(); i++) {
		Variable* v = localVariables[i];

		if (v->name == name) return v;
	}

	for (uint64 i = 0; i < globalVariables.GetCount(); i++) {
		Variable* v = globalVariables[i];

		if (v->name == name) return v;
	}

	return nullptr;
}

bool Compiler::CheckLocalName(const String& name) const {
	uint64 index = localVariables.Find<String>(name, [](Variable* const& curr, const String& name) -> bool {
		if (curr->name == name) return true;
	});

	return index == ~0;
}

bool Compiler::CheckGlobalName(const String& name) const {
	uint64 index = globalVariables.Find<String>(name, [](Variable* const& curr, const String& name) -> bool {
		if (curr->name == name) return true;

		const TypeStruct* type = (const TypeStruct*)curr->type;

		if (curr->name == "_uniform_buf") {
			for (uint64 i = 0; i < type->members.GetCount(); i++) {
				if (type->members[i].name == name) return true;
			}
		}

		return false;
	});

	return index == ~0;
}

Compiler::TypePointer* Compiler::CreateTypePointer(const TypeBase* const type, VariableScope scope) {
	TypePointer* p = new TypePointer;

	p->type = Type::Pointer;
	p->pointerType = (TypeBase*)type;
	p->storageClass = ScopeToStorageClass(scope);
	p->typeId = ~0;
	
	CheckTypeExist((TypeBase**)&p);

	if (p->typeId != ~0) {
		return p;
	}

	p->typeString = GetTypeString(p);

	InstTypePointer* pointer = new InstTypePointer(p->storageClass, type->typeId);

	CheckTypeExist((InstTypeBase**)&pointer);

	p->typeId = pointer->id;

	return p;
}

Compiler::Variable* Compiler::CreateGlobalVariable(const TypeBase* const type, VariableScope scope, const String& name) {
	Variable* var = new Variable;

	var->scope = scope;
	var->name = name;

	var->type = (TypeBase*)type;

	TypePointer* pointer = CreateTypePointer(type, scope);

	var->typePointerId = pointer->typeId;

	InstVariable* opVar = new InstVariable(type->typeId, pointer->storageClass, 0);

	var->variableId = opVar->id;

	types.Add(opVar);
	globalVariables.Add(var);

	return var;
}

Compiler::Variable* Compiler::CreateLocalVariable(const TypeBase* const type, const String& name) {
	Variable* var = new Variable;

	var->scope = VariableScope::None;
	var->name = name;
	var->type = (TypeBase*)type;

	TypePointer* pointer = CreateTypePointer(type, VariableScope::Function);

	var->typePointerId = pointer->typeId;

	InstVariable* opVar = new InstVariable(type->typeId, pointer->storageClass, 0);

	var->variableId = opVar->id;

	localVariables.Add(var);

	return var;
}

Compiler::Variable* Compiler::CreateParameterVariable(const FunctionParameter* const param, InstFunctionParameter** opParam) {
	Variable* var = new Variable;

	var->scope = VariableScope::None;
	var->name = param->name;
	var->type = param->type;

	TypePointer* pointer = CreateTypePointer(param->type, VariableScope::Function);

	var->typePointerId = pointer->typeId;

	*opParam = new InstFunctionParameter(var->typePointerId);

	var->variableId = (*opParam)->id;

	localVariables.Add(var);

	return var;
}

utils::List<Compiler::FunctionDeclaration*> Compiler::GetFunctionDeclarations(const String& name) {
	List<FunctionDeclaration*> decls;

	for (uint64 i = 0; i < functionDeclarations.GetCount(); i++) {
		FunctionDeclaration* d = functionDeclarations[i];

		if (d->name == name) {
			decls.Add(d);
		}
	}

	return decls;
}

bool Compiler::CheckParameterName(const List<FunctionParameter*>& params, const String& name) {
	auto cmp = [](FunctionParameter* const& curr, const String& name) -> bool {
		return curr->name == name;
	};

	return params.Find<String>(name, cmp) == ~0;
}

uint32 Compiler::CreateConstantS32(int32 value) {
	TypePrimitive p;

	p.type = Type::Int;
	p.componentType = Type::Int;
	p.bits = 32;
	p.sign = 1;
	p.rows = 0;
	p.columns = 0;
	p.typeId = ~0;

	return CreateConstant(&p, *(uint32*)&value);
}

uint32 Compiler::CreateConstant(const TypeBase* const type, uint32 value) {
	if (!IsTypeComposite(type)) {
		Log::Error("Can't create Constant from a composite \"%s\"", type->typeString);
		return ~0;
	}

	InstConstant* constant = new InstConstant(type->typeId, value);

	CheckConstantExist(&constant);

	return constant->id;
}

uint32 Compiler::CreateConstant(const TypeBase* const type, float32 value) {
	return CreateConstant(type, *(uint32*)&value);
}

uint32 Compiler::CreateConstantComposite(const TypeBase* const type, const List<uint32>& values) {
	const uint32* v = values.GetData();

	return CreateConstantComposite(type, &v);
}

uint32 Compiler::CreateConstantComposite(const TypeBase* const type, const uint32** values) {
	if (!IsTypeComposite(type)) {
		Log::Error("Can't create ConstantComposite from a non composite \"%s\"", type->typeString);
		return ~0;
	}

	uint32 id;

	switch (type->type) {
		case Type::Vector:
			id = CreateConstantCompositeVector(type, values);
			break;
		case Type::Matrix:
			id = CreateConstantCompositeMatrix(type, values);
			break;
		case Type::Array:
			id = CreateConstantCompositeArray(type, values);
			break;
		case Type::Struct:
			id = CreateConstantCompositeStruct(type, values);
			break;

	}

	return id;
}

uint32 Compiler::CreateConstantCompositeVector(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<uint32> ids;

	TypePrimitive* p = new TypePrimitive;
	
	p->type = prim->componentType;
	p->componentType = prim->componentType;
	p->bits = prim->bits;
	p->rows = prim->rows;
	p->columns = prim->columns;
	p->typeString = GetTypeString(p);
	p->typeId = ~0;

	CheckTypeExist((TypeBase**)&p);

	for (uint8 i = 0; i < prim->rows; i++) {
		ids.Add(CreateConstant(p, (*values)[i]));
	}

	*values += prim->columns;

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, prim->rows, ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeMatrix(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<uint32> ids;

	TypePrimitive* p = new TypePrimitive;

	p->type = Type::Vector;
	p->componentType = prim->componentType;
	p->bits = prim->bits;
	p->rows = prim->rows;
	p->columns = prim->columns;
	p->typeString = GetTypeString(p);
	p->typeId = ~0;

	CheckTypeExist((TypeBase**)&p);

	for (uint8 i = 0; i < p->rows; i++) {
		ids.Add(CreateConstantCompositeVector(p, values));
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, p->rows, ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeArray(const TypeBase* const type, const uint32** values) {
	const TypeArray* arr = (const TypeArray*)type;

	List<uint32> ids;

	if (IsTypeComposite(type)) {
		for (uint32 i = 0; i < arr->elementCount; i++) {
			ids.Add(CreateConstantComposite(arr->elementType, values));
		}
	} else {
		for (uint32 i = 0; i < arr->elementCount; i++) {
			ids.Add(CreateConstant(arr->elementType, (*values)[i]));
		}

		*values += arr->elementCount;
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, arr->elementCount, ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeStruct(const TypeBase* const type, const uint32** values) {
	const TypeStruct* str = (const TypeStruct*)type;

	List<uint32> ids;

	for (uint64 i = 0; i < str->members.GetCount(); i++) {
		const TypeBase* member = str->members[i].type;

		if (IsTypeComposite(member)) {
			ids.Add(CreateConstantComposite(member, values));
		} else {
			ids.Add(CreateConstant(member, **values));
			*values += 1;
		}
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, (uint32)ids.GetCount(), ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

bool Compiler::IsTypeComposite(const TypeBase* const type) const {
	return Utils::CompareEnums(type->type, CompareOperation::Or, Type::Vector, Type::Matrix, Type::Array, Type::Struct);
}

bool Compiler::IsCharAllowedInName(const char c, bool first) const {
	if (c >= 'A' && c <= 'Z') return true;
	else if (c >= 'a' && c <= 'z') return true;
	else if (c == '_' || (c >= '0' && c <= '9' && !first)) return true;

	return false;
}

bool Compiler::IsCharWhitespace(const char c) const {
	return (c == ' ' || c == '\n' || c == '\t');
}


void Compiler::ProcessName(Token& t) const {
	struct TokenProperties {
		String name;
		TokenType type;
		uint8 bits;
		uint8 sign;
		uint8 rows;
		uint8 columns;
	};

	static TokenProperties props[]{
		{"if",       TokenType::ControlFlowIf, 0, 0, 0, 0},
		{"switch",   TokenType::ControlFlowSwitch, 0, 0, 0, 0},
		{"else",     TokenType::ControlFlowElse, 0, 0, 0, 0},
		{"for",      TokenType::ControlFlowFor, 0, 0, 0, 0},
		{"while",    TokenType::ControlFlowWhile, 0, 0, 0, 0},
		{"break",    TokenType::ControlFlowBreak, 0, 0, 0, 0},
		{"continue", TokenType::ControlFlowContinue, 0, 0, 0, 0},
		{"return",   TokenType::ControlFlowReturn, 0, 0, 0, 0},

		{"struct",   TokenType::DataStruct, 0, 0, 0, 0},
		{"layout",   TokenType::DataLayout, 0, 0, 0, 0},
		{"in",       TokenType::DataIn, 0, 0, 0, 0},
		{"out",      TokenType::DataOut, 0, 0, 0, 0},
		{"uniform",  TokenType::DataUniform, 0, 0, 0, 0},

		{"void", TokenType::TypeVoid, 0, 0, 0, 0},
		{"bool", TokenType::TypeInt, 8, 0, 0, 0},
		{"byte", TokenType::TypeInt, 8, 0, 0, 0},

		{"uint8",  TokenType::TypeInt, 8,  0, 0, 0},
		{"uint16", TokenType::TypeInt, 16, 0, 0, 0},
		{"uint32", TokenType::TypeInt, 32, 0, 0, 0},
		{"uint64", TokenType::TypeInt, 64, 0, 0, 0},

		{"int8",  TokenType::TypeInt, 8,  1, 0, 0},
		{"int16", TokenType::TypeInt, 16, 1, 0, 0},
		{"int32", TokenType::TypeInt, 32, 1, 0, 0},
		{"int64", TokenType::TypeInt, 64, 1, 0, 0},

		{"float",  TokenType::TypeFloat, CompilerOptions::FPPrecision32() ? 32 : 64, 0, 0, 0},
		{"float32",  TokenType::TypeFloat, 32, 0, 0, 0},
		{"float64",  TokenType::TypeFloat, 64, 0, 0, 0},

		{"vec2", TokenType::TypeVec, 0, 0, 2, 0},
		{"vec3", TokenType::TypeVec, 0, 0, 3, 0},
		{"vec4", TokenType::TypeVec, 0, 0, 4, 0},

		{"mat3", TokenType::TypeMat, 0, 0, 3, 3},
		{"mat4", TokenType::TypeMat, 0, 0, 4, 4},
	};

	for (uint64 i = 0; i < sizeof(props) / sizeof(TokenProperties); i++) {
		const TokenProperties& tmp = props[i];
		if (t.string == tmp.name) {
			t.type = tmp.type;
			t.bits = tmp.bits;
			t.sign = tmp.sign;
			t.rows = tmp.rows;
			t.columns = tmp.columns;

			break;
		}
	}
}

uint64 Compiler::FindMatchingToken(const List<Token>& tokens, uint64 start, TokenType open, TokenType close) const {
	uint64 count = 0;

	for (uint64 i = start; i < tokens.GetCount(); i++) {
		const Token& t = tokens[i];

		if (t.type == open) {
			count++;
		} else if (t.type == close) {
			if (count == 1) return i;
			count--;
		}
	}

	return ~0;
}

}
}
}