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
#include <core/compiler/parsing/token.h>

namespace thc {
namespace core {
namespace type {

enum class Type {
	Fail,
	Void,
	Bool,
	Int,
	Float,
	Vector,
	Matrix,
	Array,
	Struct,
	Function,
	Pointer
};

Type ConvertToType(parsing::TokenType t);

class InstTypeBase : public instruction::InstBase {
public:
	Type type;

	InstTypeBase(Type type, uint32 opCode, uint32 wordCount, const char* const literalName);
	virtual ~InstTypeBase();

	virtual void GetInstWords(uint32* words) const = 0;

	virtual bool operator==(const InstTypeBase* other) const = 0;
};

class InstTypeVoid : public InstTypeBase {
public:
	InstTypeVoid();

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeInt : public InstTypeBase {
public:
	uint32 bits;
	uint32 sign;

	InstTypeInt(uint32 bits, uint32 sign);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeFloat : public InstTypeBase {
public:
	uint32 bits;

	InstTypeFloat(uint32 bits);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeVector : public InstTypeBase {
public:
	uint32 componentCount;
	uint32 componentTypeId;

	InstTypeVector(uint32 compCount, uint32 compTypeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeMatrix : public InstTypeBase {
public:
	uint32 columnCount;
	uint32 columnTypeId;

	InstTypeMatrix(uint32 columnCount, uint32 columnTypeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeArray : public InstTypeBase {
public:
	uint32 elementCount;
	uint32 elementTypeId;

	InstTypeArray(uint32 elementCount, uint32 elementTypeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeStruct : public InstTypeBase {
public:
	uint32 memberCount;
	uint32 memberTypeId[THC_LIMIT_OPTYPESTRUCT_MEMBERS];

	InstTypeStruct(uint32 memberCount, uint32* memberTypeIds);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypePointer : public InstTypeBase {
public:
	uint32 storageClass;
	uint32 typeId;

	InstTypePointer(uint32 storageClass, uint32 typeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

class InstTypeFunction : public InstTypeBase {
public:
	uint32 returnTypeId;
	uint32 parameterCount;
	uint32 parameterId[THC_LIMIT_FUNCTION_PARAMETERS];

	InstTypeFunction(uint32 returnTypeId, uint32 parameterCount, uint32* parameterIds);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstTypeBase* type) const override;
};

}
}
}