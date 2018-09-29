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

#pragma once

#include <memory>

#include <core/spirvlimits.h>
#include <core/instructions/instructions.h>

namespace thc {
namespace core {
namespace type {

enum Type {
	THC_TYPE_VOID,
	THC_TYPE_BOOL,
	THC_TYPE_INT,
	THC_TYPE_FLOAT,
	THC_TYPE_VECTOR,
	THC_TYPE_MATRIX,
	THC_TYPE_ARRAY,
	THC_TYPE_STRUCT,
	THC_TYPE_FUNCTION,
	THC_TYPE_POINTER
};


class TypeBase : public instruction::InstBase {
public:
	Type type;

	TypeBase(Type type, unsigned int opCode, unsigned int wordCount, const char* const literalName);
	virtual ~TypeBase();

	virtual void GetInstWords(unsigned int* words) const = 0;
};

class TypeVoid : public TypeBase {
public:
	TypeVoid();

	void GetInstWords(unsigned int* words) const override;
};

class TypeInt : public TypeBase {
public:
	unsigned int bits;
	unsigned int sign;

	TypeInt(unsigned int bits, unsigned int sign);

	void GetInstWords(unsigned int* words) const override;
};

class TypeFloat : public TypeBase {
public:
	unsigned int bits;

	TypeFloat(unsigned int bits);

	void GetInstWords(unsigned int* words) const override;
};

class TypeVector : public TypeBase {
public:
	unsigned int componentCount;
	unsigned int componentTypeId;

	TypeVector(unsigned int compCount, unsigned int compTypeId);

	void GetInstWords(unsigned int* words) const override;
};

class TypeMatrix : public TypeBase {
public:
	unsigned int columnCount;
	unsigned int columnTypeId;

	TypeMatrix(unsigned int columnCount, unsigned int columnTypeId);

	void GetInstWords(unsigned int* words) const override;
};

class TypeArray : public TypeBase {
public:
	unsigned int elementCount;
	unsigned int elementTypeId;

	TypeArray(unsigned int elementCount, unsigned int elementTypeId);

	void GetInstWords(unsigned int* words) const override;
};

class TypeStruct : public TypeBase {
public:
	unsigned int memberCount;
	unsigned int memberTypeId[THC_LIMIT_OPTYPESTRUCT_MEMBERS];

	TypeStruct(unsigned int memberCount, unsigned int* memberTypeIds);

	void GetInstWords(unsigned int* words) const override;
};

class TypePointer : public TypeBase {
public:
	unsigned int storageClass;
	unsigned int typeId;

	TypePointer(unsigned int storageClass, unsigned int typeId);

	void GetInstWords(unsigned int* words) const override;
};

class TypeFunction : public TypeBase {
public:
	unsigned int returnTypeId;
	unsigned int parameterCount;
	unsigned int parameterId[THC_LIMIT_FUNCTION_PARAMETERS];

	TypeFunction(unsigned int returnTypeId, unsigned int parameterCount, unsigned int* parameterIds);

	void GetInstWords(unsigned int* words) const override;
};

}
}
}