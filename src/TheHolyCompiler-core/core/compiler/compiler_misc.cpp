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

void Compiler::CheckTypeExist(InstTypeBase** type) {
	auto cmp = [](InstBase* const& curr, InstTypeBase* const& type) -> bool {
		if (curr->type != InstType::Type) return false;

		InstTypeBase* t = (InstTypeBase*)curr;

		if (t->type == type->type) {
			return *t == type;
		}
		
		return false;
	};

	uint64 index = typeInstructions.Find<InstTypeBase*>(*type, cmp);

	if (index == ~0) {
		typeInstructions.Add(*type);
	} else {
		delete *type;

		*type = (InstTypeBase*)typeInstructions[index];
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
	uint64 index = typeInstructions.Find<InstBase*>(*constant, [](InstBase* const& curr, InstBase* const& inst) -> bool {
		return *curr == inst;
	});

	if (index == ~0) {
		typeInstructions.Add(*constant);
	} else {
		delete *constant;

		*constant = typeInstructions[index];
	}
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitive(List<Token>& tokens, uint64 start, uint64* len) {

	uint64 offset = 0;

	const Token& token = tokens[start + offset++];

	if (!Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeVector, TokenType::TypeMatrix, TokenType::TypeVoid)) {
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
		var->typeId = nullptr;

		CheckTypeExist((TypeBase * *)& var);

		tokens.RemoveAt(start);

		if (var->typeId != nullptr) {
			return var;
		}

		InstTypeVoid* v = new InstTypeVoid();
		typeInstructions.Add(v);

		var->typeId = v->id;

		return var;
	}

	TypePrimitive tmpVar;

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeVector, TokenType::TypeMatrix)) {
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
			tmpVar.bits = CompilerOptions::FPDefaultPrecision32() ? 32 : 64;
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
			case Type::Bool:
				var = CreateTypeBool();
				break;
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

	if (len) *len += offset;

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
	var->typeId = nullptr;

	CheckTypeExist((TypeBase**)&var);

	if (var->typeId != nullptr) {
		return var;
	}

	InstTypeBool* b = new InstTypeBool;

	CheckTypeExist((InstTypeBase**)&b);

	var->typeId = b->id;
	var->typeString = "bool";

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
	var->typeId = nullptr;

	CheckTypeExist((TypeBase**)&var);

	if (var->typeId != nullptr) {
		return var;
	}

	InstTypeBase* t = nullptr;

	switch (type) {
		case Type::Int:
			t = new InstTypeInt(bits, 0); //Remove signedness from all integers, signedness will be handled internally
			break;
		case Type::Float:
			t = new InstTypeFloat(bits);
			break;
	}

	CheckTypeExist((InstTypeBase**)&t);

	var->typeId = t->id;
	var->typeString = GetTypeString(var);

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
	vec->typeId = nullptr;

	CheckTypeExist((TypeBase**)&vec);

	if (vec->typeId != nullptr) {
		return vec;
	}
	
	InstTypeVector* t = new InstTypeVector(vec->rows, CreateTypePrimitiveScalar(componentType, bits, sign)->typeId);

	CheckTypeExist((InstTypeBase**)&t);

	vec->typeId = t->id;
	vec->typeString = GetTypeString(vec);

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
	mat->typeId = nullptr;

	CheckTypeExist((TypeBase**)&mat);

	if (mat->typeId != nullptr) {
		return mat;
	}

	TypeBase* vec = CreateTypePrimitiveVector(componentType, bits, sign, rows);

	InstTypeMatrix* m = new InstTypeMatrix(columns, vec->typeId);

	CheckTypeExist((InstTypeBase**)&m);

	mat->typeId = m->id;
	mat->typeString = GetTypeString(mat);

	return mat;
}

Compiler::TypePrimitive* Compiler::ModifyTypePrimitiveBitWidth(TypePrimitive* base, uint8 bits) {
	THC_ASSERT(Utils::CompareEnums(base->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector));

	switch (base->type) {
		case Type::Int:
		case Type::Float:
			return CreateTypePrimitiveScalar(base->type, bits, base->sign);
		case Type::Vector:
			return CreateTypePrimitiveVector(base->componentType, bits, base->sign, base->rows);
	}

	return nullptr;
}

Compiler::TypeStruct* Compiler::CreateTypeStruct(List<Token>& tokens, uint64 start, uint64* len) {
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

	List<ID*> ids;

	while (true) {
		TypeBase* tmp = CreateType(tokens, start + offset, len);

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

	debugInstructions.Add(new InstName(st->id, var->typeString.str));

	uint32 memberOffset = 0;

	for (uint64 i = 0; i < var->members.GetCount(); i++) {
		const StructMember& m = var->members[i];
		debugInstructions.Add(new InstMemberName(st->id, (uint32)i, m.name.str));
		annotationIstructions.Add(new InstMemberDecorate(st->id, (uint32)i, THC_SPIRV_DECORATION_OFFSET, &memberOffset, 1));

		if (m.type->type == Type::Matrix) {
			TypePrimitive* mat = (TypePrimitive*)m.type;
			uint32 stride = mat->rows * (mat->bits / 8);
			annotationIstructions.Add(new InstMemberDecorate(st->id, (uint32)i, THC_SPIRV_DECORATION_COL_MAJOR, nullptr, 0));
			annotationIstructions.Add(new InstMemberDecorate(st->id, (uint32)i, THC_SPIRV_DECORATION_MATRIX_STRIDE, &stride, 1));
		}
		
		memberOffset += m.type->GetSize();
	}

	var->typeId = st->id;

	annotationIstructions.Add(new InstDecorate(st->id, THC_SPIRV_DECORATION_BLOCK, nullptr, 0));

	if (len) *len += offset;

	return var;
}

Compiler::TypeArray* Compiler::CreateTypeArray(List<Token>& tokens, uint64 start, uint64* len) {
	TypeArray* var = new TypeArray;

	uint64 offset = 0;

	const Token& token = tokens[start + offset++];

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeInt, TokenType::TypeFloat, TokenType::TypeVector, TokenType::TypeMatrix)) {
		var->elementType = CreateTypePrimitive(tokens, start, len);
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
	var->typeId = nullptr;

	CheckTypeExist((TypeBase**)&var);

	tokens.Remove(start, start + offset-1);

	if (len) *len += offset;

	if (var->typeId != nullptr) {
		return var;
	}

	InstTypeArray* array = new InstTypeArray(CreateConstantS32(var->elementCount), var->elementType->typeId);

	CheckTypeExist((InstTypeBase**)&array);

	var->typeId = array->id;

	return var;
}

Compiler::TypeImage* Compiler::CreateTypeImage(List<Token>& tokens, uint64 start, uint64* len) {
	TypeImage* var = new TypeImage;
	
	Token& sampler = tokens[start];

	if (!Utils::CompareEnums(sampler.type, CompareOperation::Or, TokenType::TypeImage1D, TokenType::TypeImage2D, TokenType::TypeImage3D, TokenType::TypeImageCube)) {
		Log::CompilerError(sampler, "uniform must be sampler or buffer");
	}

	var->type = Type::SampledImage;
	var->imageType = (ImageType)((uint8)sampler.type - (uint8)TokenType::TypeImage1D);
	var->depth = 0;
	var->arrayed = 0;
	var->multiSampled = 0;
	var->sampled = 1;

	CheckTypeExist((TypeBase**)&var);

	tokens.RemoveAt(start);

	if (len) *len += 1;

	if (var->typeId != nullptr) {
		return var;
	}

	var->typeString = GetTypeString(var);

	InstTypeImage* image = new InstTypeImage(CreateTypePrimitiveScalar(Type::Float, 32, 0)->typeId, (uint32)var->imageType, var->depth, var->arrayed, var->multiSampled, var->sampled, THC_SPIRV_IMAGE_FORMAT_UNKNOWN);
	
	CheckTypeExist((InstTypeBase**)&image);

	InstTypeSampledImage* sampledImage = new InstTypeSampledImage(image->id);

	CheckTypeExist((InstTypeBase**)&sampledImage);

	var->imageId = image->id;
	var->typeId = sampledImage->id;

	return var;
}

Compiler::TypeBase* Compiler::CreateType(List<Token>& tokens, uint64 start, uint64* len) {
	const Token& token = tokens[start];

	const Token& arr = tokens[start + 1];

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeMatrix, TokenType::TypeVector, TokenType::TypeVoid)) {
		if (arr.type == TokenType::BracketOpen) {
			return CreateTypeArray(tokens, start, len);
		} else {
			return CreateTypePrimitive(tokens, start, len);
		}
	}

	if (token.type == TokenType::Name) {
		uint64 index = typeDefinitions.Find<String>(token.string, findStructFunc);

		if (index != ~0) {
			if (arr.type == TokenType::BracketOpen) {
				return CreateTypeArray(tokens, start, len);
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
			name = GetTypeString(p->baseType).Append("* ");

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
		case Type::SampledImage:
			switch (((TypeImage*)type)->imageType) {
				case ImageType::Image1D:
					name = "sampler1D";
					break;
				case ImageType::Image2D:
					name = "sampler2D";
					break;
				case ImageType::Image3D:
					name = "sampler3D";
					break;
				case ImageType::ImageCube:
					name = "samplerCube";
					break;
			}

			if (((TypeImage*)type)->multiSampled) {
				name += "MS";
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
		case VariableScope::UniformConstant:
			return THC_SPIRV_STORAGE_CLASS_UNIFORM_CONSTANT;
	}

	return ~0;
}

Compiler::Symbol* Compiler::GetVariable(const String& name, VariableStack* localVariables) const {
	Symbol* var = localVariables->GetVariable(name);

	if (var == nullptr) {
		for (uint64 i = 0; i < globalVariables.GetCount(); i++) {
			Symbol* v = globalVariables[i];

			if (v->variable.name == name) {
				var = v;
				break;
			}
		}
	}
	return var;
}

bool Compiler::CheckGlobalName(const String& name) const {
	uint64 index = globalVariables.Find<String>(name, [](Symbol* const& curr, const String& name) -> bool {
		return curr->variable.name == name;
	});

	return index == ~0;
}

Compiler::TypePointer* Compiler::CreateTypePointer(const TypeBase* const type, VariableScope scope) {
	TypePointer* p = new TypePointer;

	p->type = Type::Pointer;
	p->baseType = (TypeBase*)type;
	p->storageClass = ScopeToStorageClass(scope);
	p->typeId = nullptr;

	CheckTypeExist((TypeBase**)&p);

	if (p->typeId != nullptr) {
		return p;
	}

	p->typeString = GetTypeString(p);

	InstTypePointer* pointer = new InstTypePointer(p->storageClass, type->typeId);

	CheckTypeExist((InstTypeBase**)&pointer);

	p->typeId = pointer->id;

	return p;
}

Compiler::Symbol* Compiler::CreateGlobalVariable(const TypeBase* const type, VariableScope scope, const String& name) {
	TypePointer* pointer = CreateTypePointer(type, scope);
	InstVariable* opVar = new InstVariable(pointer->typeId, pointer->storageClass, 0);

	Symbol* var = new Symbol(SymbolType::Variable, const_cast<TypeBase*>(type), opVar->id);
	var->variable.scope = scope;
	var->variable.name = name;
	var->variable.isConst = false;
	var->variable.loadId = nullptr;

	debugInstructions.Add(new InstName(opVar->id, name.str));

	typeInstructions.Add(opVar);
	globalVariables.Add(var);

	return var;
}

Compiler::Symbol* Compiler::CreateLocalVariable(const TypeBase* const type, const String& name, VariableStack* localVariables) {

	

	TypePointer* pointer = CreateTypePointer(type, VariableScope::Function);
	InstVariable* opVar = new InstVariable(pointer->typeId, pointer->storageClass, 0);

	Symbol* var = new Symbol(SymbolType::Variable, const_cast<TypeBase*>(type), opVar->id);
	var->variable.scope = VariableScope::Function;
	var->variable.name = name;
	var->variable.isConst = false;
	var->variable.loadId = nullptr;

	debugInstructions.Add(new InstName(opVar->id, name.str));
	
	localVariables->AddVariable(var, opVar);

	return var;
}

#define CAST_ERROR if (t) Log::CompilerError(*t, "No conversion available from %s to %s", type->typeString.str, cType->typeString.str);

Compiler::Symbol* Compiler::Cast(TypeBase* castType, TypeBase* currType, ID* operandId, const Token* t) {
	TypePrimitive* cType = (TypePrimitive*)castType;
	TypePrimitive* type = (TypePrimitive*)currType;

	if (!Utils::CompareEnums(cType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector) && !Utils::CompareEnums(type->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector)) {
		CAST_ERROR;
	} else if (cType->type == Type::Vector || type->type == Type::Vector) {
		if (cType->rows != type->rows) {
			CAST_ERROR;
		}
	}

	InstBase* operation = nullptr;

	if (cType->componentType == Type::Int) {
		if (type->componentType == Type::Int) {
			if (cType->sign) {
				operation = new InstSConvert(cType->typeId, operandId);
			} else {
				operation = new InstUConvert(cType->typeId, operandId);
			}
		} else if (type->componentType == Type::Float) { //Float
			if (cType->sign) {
				operation = new InstConvertFToS(cType->typeId, operandId);
			} else {
				operation = new InstConvertFToU(cType->typeId, operandId);
			}
		} else { // Bool
			CAST_ERROR;
		}
	} else if (cType->componentType == Type::Float) { //Float
		if (type->componentType == Type::Float) {
			operation = new InstFConvert(cType->typeId, operandId);
		} else if (type->componentType == Type::Int) { //Int
			if (type->sign) {
				operation = new InstConvertSToF(cType->typeId, operandId);
			} else {
				operation = new InstConvertUToF(cType->typeId, operandId);
			}
		} else { //Bool
			CAST_ERROR;
		}
	} else { //Bool
		if (type->type == Type::Int) { //Int
			operation = new InstINotEqual(castType->typeId, operandId, CreateConstant(type, 0U));
		} else if (type->type == Type::Float) { //Float
			operation = new InstFOrdNotEqual(castType->typeId, operandId, CreateConstant(type, 0.0F));
		} else {
			CAST_ERROR;
		}
	}

	instructions.Add(operation);

	return new Symbol(SymbolType::Result, castType, operation->id);
}

Compiler::Symbol* Compiler::ImplicitCast(TypeBase* castType, TypeBase* currType, ID* operandId, const Token* t) {
	if (!CompilerOptions::ImplicitConversions()) {
		Log::CompilerError(*t, "Implicit conversion. %s -> %s", currType->typeString.str, castType->typeString.str);
	}

	Symbol* ret = Cast(castType, currType, operandId, t);

	Log::CompilerWarning(*t, "Implicit conversion %s -> %s", currType->typeString.str, castType->typeString.str);

	return ret;
}

ID* Compiler::ImplicitCastId(TypeBase* castType, TypeBase* currType, ID* operandId, const Token* t) {
	Symbol* tmp = ImplicitCast(castType, currType, operandId, t);
	ID* ret = tmp->id;
	delete tmp;
	return ret;
}

Compiler::Symbol* Compiler::Add(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const Token* t) {
	if (!Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector) || !Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
		Log::CompilerError(*t, "Invalid operands %s + %s", type1->typeString.str, type2->typeString.str);
	}

	TypePrimitive* lType = (TypePrimitive*)type1;
	TypePrimitive* rType = (TypePrimitive*)type2;

	if (lType->type == Type::Vector) {
		if (rType->type == Type::Vector && lType->rows != rType->rows) {
			Log::CompilerError(*t, "Invalid operands %s + %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (rType->type == Type::Vector) {
		Log::CompilerError(*t, "Invalid operands %s + %s", type1->typeString.str, type2->typeString.str);
	}

	InstBase* instruction = nullptr;

	if (lType->componentType == Type::Int) {
		if (rType->componentType == Type::Int) {
			if (lType->bits > rType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
				operand2 = ImplicitCastId(tmpType, rType, operand2, t);
				rType = tmpType;
			} else if (rType->bits > lType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
				operand1 = ImplicitCastId(tmpType, lType, operand1, t);
				lType = tmpType;
			}

			instruction = new InstIAdd(lType->typeId, operand1, operand2);
		} else if (rType->componentType == Type::Float) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFAdd(rType->typeId, tmp, operand2);
			lType = rType;
		}
	} else if (lType->componentType == Type::Float) {
		if (rType->componentType == Type::Float) {
			if (lType->bits > rType->bits) {
				operand2 = ImplicitCastId(lType, rType, operand2, t);
				rType = lType;
			} else if (rType->bits > lType->bits) {
				operand1 = ImplicitCastId(rType, lType, operand1, t);
				lType = rType;
			}

			instruction = new InstFAdd(lType->typeId, operand1, operand2);
		} else if (rType->componentType == Type::Int) {
			ID* tmp = ImplicitCastId(lType, rType, operand2, t);

			instruction = new InstFAdd(lType->typeId, operand1, tmp);
		}
	}

	instructions.Add(instruction);

	return new Symbol(SymbolType::Result, lType, instruction->id);
}

Compiler::Symbol* Compiler::Subtract(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const Token* t) {
	if (!Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector) || !Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
		Log::CompilerError(*t, "Invalid operands %s - %s", type1->typeString.str, type2->typeString.str);
	}

	TypePrimitive* lType = (TypePrimitive*)type1;
	TypePrimitive* rType = (TypePrimitive*)type2;

	if (lType->type == Type::Vector) {
		if (rType->type == Type::Vector && lType->rows != rType->rows) {
			Log::CompilerError(*t, "Invalid operands %s - %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (rType->type == Type::Vector) {
		Log::CompilerError(*t, "Invalid operands %s - %s", type1->typeString.str, type2->typeString.str);
	}

	InstBase* instruction = nullptr;

	if (lType->componentType == Type::Int) {
		if (rType->componentType == Type::Int) {
			if (lType->bits > rType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
				operand2 = ImplicitCastId(tmpType, rType, operand2, t);
				rType = tmpType;
			} else if (rType->bits > lType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
				operand1 = ImplicitCastId(tmpType, lType, operand1, t);
				lType = tmpType;
			}

			instruction = new InstISub(lType->typeId, operand1, operand2);
		} else if (rType->componentType == Type::Float) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFSub(rType->typeId, tmp, operand2); 
			lType = rType;
		}
	} else if (lType->componentType == Type::Float) {
		if (rType->componentType == Type::Float) {
			if (lType->bits > rType->bits) {
				operand2 = ImplicitCastId(lType, rType, operand2, t);
				rType = lType;
			} else if (rType->bits > lType->bits) {
				operand1 = ImplicitCastId(rType, lType, operand1, t);
				lType = rType;
			}

			instruction = new InstFSub(lType->typeId, operand1, operand2);
		} else if (rType->componentType == Type::Int) {
			ID* tmp = ImplicitCastId(lType, rType, operand2, t);

			instruction = new InstFSub(lType->typeId, operand1, tmp);
		}
	}

	instructions.Add(instruction);

	return new Symbol(SymbolType::Result, lType, instruction->id);
}

Compiler::Symbol* Compiler::Multiply(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const Token* t) {
	if (!Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector, Type::Matrix) || !Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector, Type::Matrix)) {
		Log::CompilerError(*t, "Invalid operands %s * %s", type1->typeString.str, type2->typeString.str);
	}

	TypePrimitive* lType = (TypePrimitive*)type1;
	TypePrimitive* rType = (TypePrimitive*)type2;

	InstBase* instruction = nullptr;

	if (lType->type == Type::Matrix) {
		if (*lType == rType) {
			instruction = new InstMatrixTimesMatrix(lType->typeId, operand1, operand2);
		} else if (rType->type == Type::Vector && lType->columns == rType->rows) {
			if (lType->componentType != rType->componentType || lType->bits != rType->bits) {
				TypePrimitive* tmpType = CreateTypePrimitiveVector(lType->componentType, lType->bits, lType->sign, lType->columns);
				operand2 = ImplicitCastId(tmpType, rType, operand2, t);
				rType = tmpType;
			}

			instruction = new InstMatrixTimesVector((lType = rType)->typeId, operand1, operand2);
		} else {
			Log::CompilerError(*t, "Invalid operands %s * %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (lType->type == Type::Vector) {
		if (rType->type == Type::Vector) {
			if (lType->componentType == Type::Int) {
				if (rType->componentType == Type::Int) {
					if (lType->bits > rType->bits) {
						TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
						operand2 = ImplicitCastId(tmpType, rType, operand2, t);
						rType = tmpType;
					} else if (rType->bits > lType->bits) {
						TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
						operand1 = ImplicitCastId(tmpType, lType, operand1, t);
						lType = tmpType;
					}

					instruction = new InstIMul(lType->typeId, operand1, operand2);
				} else if (rType->componentType == Type::Float) {
					ID* tmp = ImplicitCastId(rType, lType, operand1, t);

					instruction = new InstFMul((lType = rType)->typeId, tmp, operand2);
				}
			} else if (lType->componentType == Type::Float) {
				if (rType->componentType == Type::Float) {
					if (lType->bits > rType->bits) {
						operand2 = ImplicitCastId(lType, rType, operand2, t);
						rType = lType;
					} else if (rType->bits > lType->bits) {
						operand1 = ImplicitCastId(rType, lType, operand1, t);
						lType = rType;
					}

					instruction = new InstFMul(lType->typeId, operand1, operand2);
				} else if (rType->componentType == Type::Int) {
					ID* tmp = ImplicitCastId(lType, rType, operand2, t);

					instruction = new InstFMul(lType->typeId, operand1, tmp);
				}
			}
		} else if (rType->type == Type::Matrix && lType->rows == rType->columns) {
			if (lType->componentType != rType->componentType || lType->bits != rType->bits) {
				TypePrimitive* tmpType = CreateTypePrimitiveVector(rType->componentType, rType->bits, rType->sign, rType->columns);
				operand1 = ImplicitCastId(tmpType, lType, operand1, t);
				lType = tmpType;
			}

			instruction = new InstVectorTimesMatrix(lType->typeId, operand1, operand2);
		} else if (rType->type == Type::Int) {
			if (lType->componentType == Type::Int) {
				if (lType->bits > rType->bits) {
					TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
					operand2 = ImplicitCastId(tmpType, rType, operand2, t);
					rType = tmpType;
				} else if (rType->bits > lType->bits) {
					TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
					operand1 = ImplicitCastId(tmpType, lType, operand1, t);
					lType = tmpType;
				}

				instruction = new InstVectorTimesScalar(lType->typeId, operand1, operand2);
			} else if (lType->componentType == Type::Float) {
				ID* tmp = ImplicitCastId(CreateTypePrimitiveScalar(Type::Float, lType->bits, lType->sign), rType, operand2, t);

				instruction = new InstVectorTimesScalar(lType->typeId, operand1, tmp);
			}
		} else if (rType->type == Type::Float) {
			if (lType->componentType == Type::Int) {
				TypePrimitive* tmpType = CreateTypePrimitiveVector(Type::Float, rType->bits, rType->sign, lType->rows);
				ID* tmp = ImplicitCastId(tmpType, lType, operand1, t);

				instruction = new InstVectorTimesScalar(tmpType->typeId, tmp, operand2);
				lType = tmpType;
			} else if (lType->componentType == Type::Float) {
				if (lType->bits > rType->bits) {
					TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
					operand2 = ImplicitCastId(tmpType, rType, operand2, t);
					rType = tmpType;
				} else if (rType->bits > lType->bits) {
					TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
					operand1 = ImplicitCastId(tmpType, lType, operand1, t);
					lType = tmpType;
				}

				instruction = new InstVectorTimesScalar(lType->typeId, operand1, operand2);
			}
		}
	} else if (lType->type == Type::Int) {
		if (rType->type == Type::Int) {
			if (lType->bits > rType->bits) {
				operand2 = ImplicitCastId(lType, rType, operand2, t);
				rType = lType;
			} else if (rType->bits > lType->bits) {
				operand1 = ImplicitCastId(rType, lType, operand1, t);
				lType = rType;
			}

			instruction = new InstIMul(lType->typeId, operand1, operand2);
		} else if (rType->type == Type::Float) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFMul(rType->typeId, tmp, operand2);
			lType = rType;
		} else {
			Log::CompilerError(*t, "Invalid operands %s * %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (lType->type == Type::Float) {
		if (rType->type == Type::Float) {
			if (lType->bits > rType->bits) {
				operand2 = ImplicitCastId(lType, rType, operand2, t);
				rType = lType;
			} else if (rType->bits > lType->bits) {
				operand1 = ImplicitCastId(rType, lType, operand1, t);
				lType = rType;
			}

			instruction = new InstFMul(lType->typeId, operand1, operand2);
		} else if (rType->type == Type::Int) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFMul((lType = rType)->typeId, tmp, operand2);
		} else {
			Log::CompilerError(*t, "Invalid operands %s * %s", type1->typeString.str, type2->typeString.str);
		}
	}

	instructions.Add(instruction);

	return new Symbol(SymbolType::Result, lType, instruction->id);
}

Compiler::Symbol* Compiler::Divide(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const Token* t) {
	if (!Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector) || !Utils::CompareEnums(type1->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
		Log::CompilerError(*t, "Invalid operands %s / %s", type1->typeString.str, type2->typeString.str);
	}

	TypePrimitive* lType = (TypePrimitive*)type1;
	TypePrimitive* rType = (TypePrimitive*)type2;

	InstBase* instruction = nullptr;

	if (lType->type == Type::Vector) {
		if (rType->type == Type::Vector) {
			if (lType->componentType == Type::Int) {
				if (rType->componentType == Type::Int) {
					if (lType->bits > rType->bits) {
						TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
						operand2 = ImplicitCastId(tmpType, rType, operand2, t);
						rType = tmpType;
					} else if (rType->bits > lType->bits) {
						TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
						operand1 = ImplicitCastId(tmpType, lType, operand1, t);
						lType = tmpType;
					}

					if (lType->sign) {
						instruction = new InstUDiv(lType->typeId, operand1, operand2);
					} else {
						instruction = new InstSDiv(lType->typeId, operand1, operand2);
					}
					
				} else if (rType->componentType == Type::Float) {
					ID* tmp = ImplicitCastId(rType, lType, operand1, t);

					instruction = new InstFDiv(rType->typeId, tmp, operand2);
					lType = rType;
				}
			} else if (lType->componentType == Type::Float) {
				if (rType->componentType == Type::Float) {
					if (lType->bits > rType->bits) {
						operand2 = ImplicitCastId(lType, rType, operand2, t);
						rType = lType;
					} else if (rType->bits > lType->bits) {
						operand1 = ImplicitCastId(rType, lType, operand1, t);
						lType = rType;
					}

					instruction = new InstFDiv(lType->typeId, operand1, operand2);
				} else if (rType->componentType == Type::Int) {
					ID* tmp = ImplicitCastId(lType, rType, operand2, t);

					instruction = new InstFDiv(lType->typeId, operand1, tmp);
				}
			}
		} else {
			Log::CompilerError(*t, "Invalid operands %s / %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (lType->type == Type::Int) {
		if (rType->type == Type::Int) {
			if (lType->bits > rType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(rType, lType->bits);
				operand2 = ImplicitCastId(tmpType, rType, operand2, t);
				rType = tmpType;
			} else if (rType->bits > lType->bits) {
				TypePrimitive* tmpType = ModifyTypePrimitiveBitWidth(lType, rType->bits);
				operand1 = ImplicitCastId(tmpType, lType, operand1, t);
				lType = tmpType;
			}

			if (lType->sign) {
				instruction = new InstUDiv(lType->typeId, operand1, operand2);
			} else {
				instruction = new InstSDiv(lType->typeId, operand1, operand2);
			}

		} else if (rType->type == Type::Float) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFDiv(rType->typeId, tmp, operand2);
			lType = rType;
		} else {
			Log::CompilerError(*t, "Invalid operands %s * %s", type1->typeString.str, type2->typeString.str);
		}
	} else if (lType->type == Type::Float) {
		if (rType->type == Type::Float) {
			if (lType->bits > rType->bits) {
				operand2 = ImplicitCastId(lType, rType, operand2, t);
				rType = lType;
			} else if (rType->bits > lType->bits) {
				operand1 = ImplicitCastId(rType, lType, operand1, t);
				lType = rType;
			}

			instruction = new InstFDiv(lType->typeId, operand1, operand2);
		} else if (rType->type == Type::Int) {
			ID* tmp = ImplicitCastId(rType, lType, operand1, t);

			instruction = new InstFDiv(rType->typeId, tmp, operand2);
			lType = rType;
		} else {
			Log::CompilerError(*t, "Invalid operands %s / %s", type1->typeString.str, type2->typeString.str);
		}
	}

	instructions.Add(instruction);

	return new Symbol(SymbolType::Result, lType, instruction->id);
}

Compiler::FunctionDeclaration* Compiler::GetFunctionDeclaration(const String& name) {
	List<FunctionDeclaration*> decls;

	for (uint64 i = 0; i < functionDeclarations.GetCount(); i++) {
		FunctionDeclaration* d = functionDeclarations[i];

		if (d->name == name) {
			return d;
		}
	}

	return nullptr;
}

void Compiler::CreateFunctionType(FunctionDeclaration* decl) {
	List<ID*> ids;
	for (uint64 i = 0; i < decl->parameters.GetCount(); i++) {
		ids.Add(decl->parameters[i]->type->typeId);
	}

	InstTypeFunction* f = new InstTypeFunction(decl->returnType->typeId, (uint32)ids.GetCount(), ids.GetData());

	CheckTypeExist((InstTypeBase**)&f);

	decl->typeId = f->id;
}

bool Compiler::CheckParameterName(const List<Symbol*>& params, const String& name) {
	auto cmp = [](Symbol* const& curr, const String& name) -> bool {
		THC_ASSERT(curr->symbolType == SymbolType::Parameter);
		return curr->parameter.name == name;
	};

	return params.Find<String>(name, cmp) == ~0;
}

ID* Compiler::CreateConstantBool(bool value) {
	InstBase* base = nullptr;

	ID* type = CreateTypeBool()->typeId;

	if (value) {
		base = new InstConstantTrue(type);
	} else {
		base = new InstConstantFalse(type);
	}

	CheckConstantExist(&base);

	return base->id;
}

ID* Compiler::CreateConstantS32(int32 value) {
	TypeBase* type = CreateTypePrimitiveScalar(Type::Int, 32, 1);

	return CreateConstant(type, *(uint32*)&value);
}

ID* Compiler::CreateConstant(const TypeBase* const type, uint32 value) {
	if (IsTypeComposite(type)) {
		Log::Error("Can't create Constant from a composite \"%s\"", type->typeString);
		return nullptr;
	}

	InstConstant* constant = new InstConstant(type->typeId, value);

	CheckConstantExist(&constant);

	return constant->id;
}

ID* Compiler::CreateConstant(const TypeBase* const type, float32 value) {
	return CreateConstant(type, *(uint32*)&value);
}

ID* Compiler::CreateConstantComposite(const TypeBase* const type, const List<uint32>& values) {
	const uint32* v = values.GetData();

	return CreateConstantComposite(type, &v);
}

ID* Compiler::CreateConstantComposite(const TypeBase* const type, const uint32** values) {
	if (!IsTypeComposite(type)) {
		Log::Error("Can't create ConstantComposite from a non composite \"%s\"", type->typeString);
		return nullptr;
	}

	ID* id;

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

ID* Compiler::CreateConstantCompositeVector(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<ID*> ids;

	TypePrimitive* tmp = (TypePrimitive*)type;
	TypePrimitive* p = CreateTypePrimitiveScalar(tmp->componentType, tmp->bits, tmp->sign);

	for (uint8 i = 0; i < prim->rows; i++) {
		ids.Add(CreateConstant(p, (*values)[i]));
	}

	*values += prim->columns;

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, prim->rows, ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

ID* Compiler::CreateConstantCompositeMatrix(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<ID*> ids;

	TypePrimitive* tmp = (TypePrimitive*)type;
	TypePrimitive* p = CreateTypePrimitiveVector(tmp->componentType, tmp->bits, tmp->sign, tmp->rows);

	for (uint8 i = 0; i < tmp->columns; i++) {
		ids.Add(CreateConstantCompositeVector(p, values));
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, tmp->columns, ids.GetData());

	CheckConstantExist(&composite);

	return composite->id;
}

ID* Compiler::CreateConstantCompositeArray(const TypeBase* const type, const uint32** values) {
	const TypeArray* arr = (const TypeArray*)type;

	List<ID*> ids;

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

ID* Compiler::CreateConstantCompositeStruct(const TypeBase* const type, const uint32** values) {
	const TypeStruct* str = (const TypeStruct*)type;

	List<ID*> ids;

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

		{"const",    TokenType::ModifierConst, 0, 0, 0, 0},

		{"struct",   TokenType::DataStruct, 0, 0, 0, 0},
		{"layout",   TokenType::DataLayout, 0, 0, 0, 0},
		{"in",       TokenType::DataIn, 0, 0, 0, 0},
		{"out",      TokenType::DataOut, 0, 0, 0, 0},
		{"uniform",  TokenType::DataUniform, 0, 0, 0, 0},

		{"void", TokenType::TypeVoid, 0, 0, 0, 0},
		{"bool", TokenType::TypeBool, 0, 0, 0, 0},
		{"byte", TokenType::TypeInt, 8, 0, 0, 0},

		{"uint8",  TokenType::TypeInt, 8,  0, 0, 0},
		{"uint16", TokenType::TypeInt, 16, 0, 0, 0},
		{"uint32", TokenType::TypeInt, 32, 0, 0, 0},
		{"uint64", TokenType::TypeInt, 64, 0, 0, 0},

		{"int8",  TokenType::TypeInt, 8,  1, 0, 0},
		{"int16", TokenType::TypeInt, 16, 1, 0, 0},
		{"int32", TokenType::TypeInt, 32, 1, 0, 0},
		{"int64", TokenType::TypeInt, 64, 1, 0, 0},

		{"float",  TokenType::TypeFloat, CompilerOptions::FPDefaultPrecision32() ? (uint8)32 : (uint8)64, 0, 0, 0},
		{"float32",  TokenType::TypeFloat, 32, 0, 0, 0},
		{"float64",  TokenType::TypeFloat, 64, 0, 0, 0},

		{"vec2", TokenType::TypeVector, 0, 0, 2, 0},
		{"vec3", TokenType::TypeVector, 0, 0, 3, 0},
		{"vec4", TokenType::TypeVector, 0, 0, 4, 0},

		{"mat3", TokenType::TypeMatrix, 0, 0, 3, 3},
		{"mat4", TokenType::TypeMatrix, 0, 0, 4, 4},

		{"sampler1D",	TokenType::TypeImage1D, 0, 0, 0, 0},
		{"sampler2D",	TokenType::TypeImage2D, 0, 0, 0, 0},
		{"sampler3D",	TokenType::TypeImage3D, 0, 0, 0, 0},
		{"samplerCube", TokenType::TypeImageCube, 0, 0, 0, 0},
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

ID* Compiler::GetExpressionOperandId(const Expression* e, TypePrimitive** type, bool swizzle, ID** ogID) {
	ID* id = nullptr;

	if (e->type == ExpressionType::Variable) {
		Symbol* v = e->symbol;
		*type = (TypePrimitive*)v->type;
		id = LoadVariable(e->symbol, true);
	} else if (e->type == ExpressionType::Result || e->type == ExpressionType::Constant) {
		id = e->symbol->id;
		*type = (TypePrimitive*)e->symbol->type;
	}

	if (ogID) *ogID = id;

	if (swizzle) {
		id = GetSwizzledVector(type, id, e->symbol->swizzleIndices);
		const_cast<Expression*>(e)->symbol->swizzleIndices.Clear();
	}

	return id;
}

ID* Compiler::LoadVariable(Symbol* var, bool usePreviousLoad) {
	THC_ASSERT(var->symbolType == SymbolType::Variable || var->symbolType == SymbolType::Parameter);

	if (var->symbolType == SymbolType::Parameter && !var->parameter.isReference) {
		return var->id;
	}
	
	if (usePreviousLoad && var->variable.loadId != nullptr) {
		return var->variable.loadId;
	}

	InstLoad* load = new InstLoad(var->type->typeId, var->id, 0);
	instructions.Add(load);

	return var->variable.loadId = load->id;
}

void Compiler::StoreVariable(Symbol* var, ID* storeId, bool setAsLoadId) {
	if (var->symbolType == SymbolType::Parameter && !var->parameter.isReference) {
		var->id = storeId;
	} else if (setAsLoadId) {
		var->variable.loadId = storeId;
	} else {
		InstStore* store = new InstStore(var->id, storeId, 0);
		instructions.Add(store);

		var->variable.loadId = nullptr;
	}
}

List<ID*> Compiler::GetIDs(List<Symbol*>& things) {
	uint64 num = things.GetCount();
	List<ID*> ids(num);

	for (uint64 i = 0; i < num; i++) {
		ids.Add(things[i]->id);
	}

	return std::move(ids);
}

List<uint32> Compiler::GetVectorShuffleIndices(const Token& token, const TypePrimitive* type) {
	List<uint32> ret(4);

	uint64 count = token.string.length;

	if (count > 4) {
		Log::CompilerError(token, "Cannot make vector with %llu components", count);
	}

	for (uint64 i = 0; i < count; i++) {
		switch (token.string[i]) {
			case 'x':
				ret.Add(0);
				break;
			case 'y':
				ret.Add(1);
				break;
			case 'z':
				if (type->rows < 3) Log::CompilerError(token, "No z component in a \"%s\"", type->typeString.str);
				ret.Add(2);
				break;
			case 'w':
				if (type->rows < 4) Log::CompilerError(token, "No w component in a \"%s\"", type->typeString.str);
				ret.Add(3);
				break;
			default:
				Log::CompilerError(token, "\"%c\" is not a valid member of a \"%s\"", token.string[i], type->typeString.str);
		}
	}

	return std::move(ret);
}

Compiler::TypePrimitive* Compiler::GetSwizzledType(TypePrimitive* base, const List<uint32>& indices) {
	uint32 rows = (uint32)indices.GetCount();

	if (rows == 0) return base;

	if (rows == 1) {
		return CreateTypePrimitiveScalar(base->componentType, base->bits, base->sign);
	} else {
		return CreateTypePrimitiveVector(base->componentType, base->bits, base->sign, rows);
	}
}


ID* Compiler::GetSwizzledVector(TypePrimitive** type, ID* load, const List<uint32>& indices) {
	TypePrimitive* t = *type;
	ID* id = load;

	if (indices.GetCount() == 1) {
		t = CreateTypePrimitiveScalar(t->componentType, t->bits, t->sign);
		InstBase* inst = new InstCompositeExtract(t->typeId, load, 1, indices.GetData());
		instructions.Add(inst);
		id = inst->id;
	} else if (indices.GetCount() > 1) {
		uint32 rows = (uint32)indices.GetCount();
		t = CreateTypePrimitiveVector(t->componentType, t->bits, t->sign, rows);
		InstBase* inst = new InstVectorShuffle(t->typeId, load, load, rows, indices.GetData());
		instructions.Add(inst);
		id = inst->id;
	}

	*type = t;

	return id;
}

}
}
}