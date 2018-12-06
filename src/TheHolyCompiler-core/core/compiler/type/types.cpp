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

#include "types.h"
#include <util/thc_assert.h>

namespace thc {
namespace core {
namespace type {

using namespace parsing;

Type ConvertToType(TokenType type) {
	switch (type) {
		case TokenType::TypeVoid:
			return Type::Void;
		case TokenType::TypeBool:
			return Type::Bool;
		case TokenType::TypeInt:
			return Type::Int;
		case TokenType::TypeFloat:
			return Type::Float;
		case TokenType::TypeVec:
			return Type::Vector;
		case TokenType::TypeMat:
			return Type::Matrix;
	}

	THC_ASSERT(type != type);

	return Type::Fail;
}


InstTypeBase::InstTypeBase(Type type, uint32 opCode, uint32 wordCount, const char* const literalName) : InstBase(opCode, wordCount, literalName, true, instruction::InstType::Type), type(type) { }

InstTypeBase::~InstTypeBase() { }

InstTypeVoid::InstTypeVoid() : InstTypeBase(Type::Void, THC_SPIRV_OPCODE_OpTypeVoid, 2, "OpTypeVoid") {  }

InstTypeInt::InstTypeInt(uint32 bits, uint32 sign) : InstTypeBase(Type::Int, THC_SPIRV_OPCODE_OpTypeInt, 4, "OpTypeInt"), bits(bits), sign(sign) { }

InstTypeFloat::InstTypeFloat(uint32 bits) : InstTypeBase(Type::Float, THC_SPIRV_OPCODE_OpTypeFloat, 3, "OpTypeFloat"), bits(bits) { }

InstTypeVector::InstTypeVector(uint32 compCount, uint32 compTypeId) : InstTypeBase(Type::Vector, THC_SPIRV_OPCODE_OpTypeVector, 4, "OpTypeVector"), componentCount(compCount), componentTypeId(compTypeId) {}

InstTypeMatrix::InstTypeMatrix(uint32 columnCount, uint32 columnTypeId) : InstTypeBase(Type::Matrix, THC_SPIRV_OPCODE_OpTypeMatrix, 4, "OpTypeMatrix"), columnCount(columnCount), columnTypeId(columnTypeId) {}

InstTypeArray::InstTypeArray(uint32 elementCount, uint32 elementTypeId) : InstTypeBase(Type::Array, THC_SPIRV_OPCODE_OpTypeArray, 4, "OpTypeArray"), elementCount(elementCount), elementTypeId(elementTypeId) {}

InstTypeStruct::InstTypeStruct(uint32 memberCount, uint32* memberTypeIds) : InstTypeBase(Type::Struct, THC_SPIRV_OPCODE_OpTypeStruct, 2, "OpTypeStruct"), memberCount(memberCount) { memcpy(memberTypeId, memberTypeIds, memberCount << 2); }

InstTypePointer::InstTypePointer(uint32 storageClass, uint32 typeId) : InstTypeBase(Type::Pointer, THC_SPIRV_OPCODE_OpTypePointer, 4, "OpTypePointer"), storageClass(storageClass), typeId(typeId) {}

InstTypeFunction::InstTypeFunction(uint32 returnTypeId, uint32 parameterCount, uint32* parameterIds) : InstTypeBase(Type::Function, THC_SPIRV_OPCODE_OpTypeFunction, 3, "OpTypeFunction"), returnTypeId(returnTypeId), parameterCount(parameterCount) { memcpy(parameterId, parameterIds, parameterCount << 2); }

bool InstTypeVoid::operator==(const InstTypeBase* type) const {
	return true;
}

bool InstTypeInt::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeInt* t = (InstTypeInt*)type;

	return bits == t->bits && sign == t->sign;
}

bool InstTypeFloat::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeFloat* t = (InstTypeFloat*)type;

	return bits == t->bits;
}

bool InstTypeVector::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeVector* t = (InstTypeVector*)type;

	return componentCount == t->componentCount && componentTypeId == t->componentTypeId;
}

bool InstTypeMatrix::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeMatrix* t = (InstTypeMatrix*)type;

	return columnCount == t->columnCount && columnTypeId == t->columnTypeId;
}

bool InstTypeArray::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeArray* t = (InstTypeArray*)type;

	return elementCount == t->elementCount && elementTypeId == t->elementTypeId;
}

bool InstTypeStruct::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeStruct* t = (InstTypeStruct*)type;

	if (memberCount == t->memberCount) {
		for (uint32 i = 0; i < memberCount; i++) {
			if (memberTypeId[i] != t->memberTypeId[i]) return false;
		}
		
		return true;
	}

	return false;
}

bool InstTypePointer::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypePointer* t = (InstTypePointer*)type;

	return typeId == t->typeId && storageClass == t->storageClass;
}

bool InstTypeFunction::operator==(const InstTypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	InstTypeFunction* t = (InstTypeFunction*)type;

	if (returnTypeId == t->returnTypeId && parameterCount == t->parameterCount) {
		for (uint32 i = 0; i < parameterCount; i++) {
			if (parameterId[i] != t->parameterId[i]) return false;
		}

		return true;
	}

	return false;
}


}
}
}