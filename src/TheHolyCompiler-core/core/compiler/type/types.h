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
#include <core/compiler/instruction/instructions.h>

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

	TypeBase(Type type, uint32 opCode, uint32 wordCount, const char* const literalName);
	virtual ~TypeBase();

	virtual void GetInstWords(uint32* words) const = 0;
};

class TypeVoid : public TypeBase {
public:
	TypeVoid();

	void GetInstWords(uint32* words) const override;
};

class TypeInt : public TypeBase {
public:
	uint32 bits;
	uint32 sign;

	TypeInt(uint32 bits, uint32 sign);

	void GetInstWords(uint32* words) const override;
};

class TypeFloat : public TypeBase {
public:
	uint32 bits;

	TypeFloat(uint32 bits);

	void GetInstWords(uint32* words) const override;
};

class TypeVector : public TypeBase {
public:
	uint32 componentCount;
	uint32 componentTypeId;

	TypeVector(uint32 compCount, uint32 compTypeId);

	void GetInstWords(uint32* words) const override;
};

class TypeMatrix : public TypeBase {
public:
	uint32 columnCount;
	uint32 columnTypeId;

	TypeMatrix(uint32 columnCount, uint32 columnTypeId);

	void GetInstWords(uint32* words) const override;
};

class TypeArray : public TypeBase {
public:
	uint32 elementCount;
	uint32 elementTypeId;

	TypeArray(uint32 elementCount, uint32 elementTypeId);

	void GetInstWords(uint32* words) const override;
};

class TypeStruct : public TypeBase {
public:
	uint32 memberCount;
	uint32 memberTypeId[THC_LIMIT_OPTYPESTRUCT_MEMBERS];

	TypeStruct(uint32 memberCount, uint32* memberTypeIds);

	void GetInstWords(uint32* words) const override;
};

class TypePointer : public TypeBase {
public:
	uint32 storageClass;
	uint32 typeId;

	TypePointer(uint32 storageClass, uint32 typeId);

	void GetInstWords(uint32* words) const override;
};

class TypeFunction : public TypeBase {
public:
	uint32 returnTypeId;
	uint32 parameterCount;
	uint32 parameterId[THC_LIMIT_FUNCTION_PARAMETERS];

	TypeFunction(uint32 returnTypeId, uint32 parameterCount, uint32* parameterIds);

	void GetInstWords(uint32* words) const override;
};

}
}
}