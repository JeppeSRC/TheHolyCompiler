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

namespace thc {
namespace core {
namespace type {


TypeBase::TypeBase(Type type) : type(type) { }

TypeInt::TypeInt(unsigned int bits, unsigned int sign) : TypeBase(THC_TYPE_INT), bits(bits), sign(sign) { }

TypeFloat::TypeFloat(unsigned int bits) : TypeBase(THC_TYPE_FLOAT), bits(bits) { }

TypeVector::TypeVector(unsigned int compCount, unsigned int compTypeId) : TypeBase(THC_TYPE_VECTOR), componentCount(compCount), componentTypeId(compTypeId) {}

TypeMatrix::TypeMatrix(unsigned int columnCount, unsigned int columnTypeId) : TypeBase(THC_TYPE_MATRIX), columnCount(columnCount), columnTypeId(columnTypeId) {}

TypeArray::TypeArray(unsigned int elementCount, unsigned int elementTypeId) : TypeBase(THC_TYPE_ARRAY), elementCount(elementCount), elementTypeId(elementTypeId) {}

TypeStruct::TypeStruct(unsigned int memberCount, unsigned int* memberTypeIds) : TypeBase(THC_TYPE_STRUCT), memberCount(memberCount) { memcpy(memberTypeId, memberTypeIds, memberCount * sizeof(unsigned int)); }

TypePointer::TypePointer(unsigned int storageClass, unsigned int typeId) : TypeBase(THC_TYPE_POINTER), storageClass(storageClass), typeId(typeId) {}

TypeFunction::TypeFunction(unsigned int returnTypeId, unsigned int parameterCount, unsigned int* parameterIds) : TypeBase(THC_TYPE_FUNCTION), returnTypeId(returnTypeId), parameterCount(parameterCount) { memcpy(parameterId, parameterIds, parameterCount * sizeof(unsigned int)); }

}
}
}