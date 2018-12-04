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


TypeBase::TypeBase(Type type, uint32 opCode, uint32 wordCount, const char* const literalName) : InstBase(opCode, wordCount, literalName, true, instruction::InstType::Type), type(type) { }

TypeBase::~TypeBase() { }

TypeVoid::TypeVoid() : TypeBase(Type::Void, THC_SPIRV_OPCODE_OpTypeVoid, 2, "OpTypeVoid") {  }

TypeInt::TypeInt(uint32 bits, uint32 sign) : TypeBase(Type::Int, THC_SPIRV_OPCODE_OpTypeInt, 4, "OpTypeInt"), bits(bits), sign(sign) { }

TypeFloat::TypeFloat(uint32 bits) : TypeBase(Type::Float, THC_SPIRV_OPCODE_OpTypeFloat, 3, "OpTypeFloat"), bits(bits) { }

TypeVector::TypeVector(uint32 compCount, uint32 compTypeId) : TypeBase(Type::Vector, THC_SPIRV_OPCODE_OpTypeVector, 4, "OpTypeVector"), componentCount(compCount), componentTypeId(compTypeId) {}

TypeMatrix::TypeMatrix(uint32 columnCount, uint32 columnTypeId) : TypeBase(Type::Matrix, THC_SPIRV_OPCODE_OpTypeMatrix, 4, "OpTypeMatrix"), columnCount(columnCount), columnTypeId(columnTypeId) {}

TypeArray::TypeArray(uint32 elementCount, uint32 elementTypeId) : TypeBase(Type::Array, THC_SPIRV_OPCODE_OpTypeArray, 4, "OpTypeArray"), elementCount(elementCount), elementTypeId(elementTypeId) {}

TypeStruct::TypeStruct(uint32 memberCount, uint32* memberTypeIds) : TypeBase(Type::Struct, THC_SPIRV_OPCODE_OpTypeStruct, 2, "OpTypeStruct"), memberCount(memberCount) { memcpy(memberTypeId, memberTypeIds, memberCount << 2); }

TypePointer::TypePointer(uint32 storageClass, uint32 typeId) : TypeBase(Type::Pointer, THC_SPIRV_OPCODE_OpTypePointer, 4, "OpTypePointer"), storageClass(storageClass), typeId(typeId) {}

TypeFunction::TypeFunction(uint32 returnTypeId, uint32 parameterCount, uint32* parameterIds) : TypeBase(Type::Function, THC_SPIRV_OPCODE_OpTypeFunction, 3, "OpTypeFunction"), returnTypeId(returnTypeId), parameterCount(parameterCount) { memcpy(parameterId, parameterIds, parameterCount << 2); }

bool TypeVoid::operator==(const TypeBase* type) const {
	return true;
}

bool TypeInt::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeInt* t = (TypeInt*)type;

	return bits == t->bits && sign == t->sign;
}

bool TypeFloat::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeFloat* t = (TypeFloat*)type;

	return bits == t->bits;
}

bool TypeVector::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeVector* t = (TypeVector*)type;

	return componentCount == t->componentCount && componentTypeId == t->componentTypeId;
}

bool TypeMatrix::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeMatrix* t = (TypeMatrix*)type;

	return columnCount == t->columnCount && columnTypeId == t->columnTypeId;
}

bool TypeArray::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeArray* t = (TypeArray*)type;

	return elementCount == t->elementCount && elementTypeId == t->elementTypeId;
}

bool TypeStruct::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeStruct* t = (TypeStruct*)type;

	if (memberCount == t->memberCount) {
		for (uint32 i = 0; i < memberCount; i++) {
			if (memberTypeId[i] != t->memberTypeId[i]) return false;
		}
		
		return true;
	}

	return false;
}

bool TypePointer::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypePointer* t = (TypePointer*)type;

	return typeId == t->typeId && storageClass == t->storageClass;
}

bool TypeFunction::operator==(const TypeBase* type) const {
	THC_ASSERT(this->type != type->type);
	TypeFunction* t = (TypeFunction*)type;

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