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

#include "compiler.h"

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;

bool Compiler::TypeBase::operator==(const TypeBase* const other) const {
	return type == other->type;
}

bool Compiler::TypeBase::operator!=(const TypeBase* const other) const {
	return !operator==(other);
}

bool Compiler::TypePrimitive::operator==(const Compiler::TypeBase* const other) const {
	if (other->type == type) {
		const TypePrimitive* t = (const TypePrimitive*)other;

		return componentType == t->componentType && bits == t->bits && sign == t->sign && rows == t->rows && columns == t->columns;
	}

	return false;
}

bool Compiler::TypePrimitive::operator!=(const Compiler::TypeBase* const other) const {
	return !operator==(other);
}

bool Compiler::StructMember::operator==(const StructMember& other) const {
	return *type == other.type;
}

bool Compiler::StructMember::operator!=(const StructMember& other) const {
	return !operator==(other);
}

bool Compiler::TypeStruct::operator==(const TypeBase* const other) const {
	if (other->type == type) {
		const TypeStruct* t = (const TypeStruct*)other;

		for (uint64 i = 0; i < members.GetCount(); i++) {
			if (!(members[i] == t->members[i])) return false;
		}

		return true;
	}

	return false;
}

bool Compiler::TypeStruct::operator!=(const Compiler::TypeBase* const other) const {
	return !operator==(other);
}

bool Compiler::TypeArray::operator==(const TypeBase* const other) const {
	if (other->type == type) {
		const TypeArray* t = (const TypeArray*)other;

		return elementCount == t->elementCount && *elementType == t->elementType;
	}
	
	return false;
}

bool Compiler::TypeArray::operator!=(const Compiler::TypeBase* const other) const {
	return !operator==(other);
}

bool Compiler::TypePointer::operator==(const TypeBase* const other) const {
	if (other->type == type) {
		const TypePointer* t = (const TypePointer*)other;

		return *baseType == t->baseType && storageClass == t->storageClass;
	}

	return false;
}

bool Compiler::TypePointer::operator!=(const Compiler::TypeBase* const other) const {
	return !operator==(other);
}
/*bool Compiler::TypeFunction::operator==(const TypeBase* const other) const {
	if (other->type == type) {
		const TypeFunction* t = (const TypeFunction*)other;

		if (!(parameters.GetCount() == t->parameters.GetCount() && *returnType == t->returnType)) return false;

		for (uint64 i = 0; i < parameters.GetCount(); i++) {
			if (!(*parameters[i] == t->parameters[i])) return false;
		}

		return true;
	}

	return false;
}*/


bool Compiler::FunctionDeclaration::operator==(const FunctionDeclaration* const other) const {
	if (name == other->name && *returnType == other->returnType && parameters.GetCount() == other->parameters.GetCount()) {
		for (uint64 i = 0; i < parameters.GetCount(); i++) {
			Variable* param = parameters[i];
			Variable* paramOther = other->parameters[i];

			if (*param->type != paramOther->type || param->isConstant != paramOther->isConstant) return false;
		}

		return true;
	}

	return false;
}

uint32 Compiler::TypePrimitive::GetSize() const {
	switch (type) {
		case Type::Int:
		case Type::Float:
			return bits >> 3;
		case Type::Vector:
			return rows * (bits >> 3);
		case Type::Matrix:
			return rows * columns * (bits >> 3);
	}

	return ~0;
}

uint32 Compiler::TypeStruct::GetSize() const {
	uint32 total = 0;
	for (uint64 i = 0; i < members.GetCount(); i++) {
		total += members[i].type->GetSize();
	}

	return total;
}

uint32 Compiler::TypeStruct::GetMemberIndex(const String& name) {
	for (uint64 i = 0; i < members.GetCount(); i++) {
		if (members[i].name == name) return (uint32)i;
	}

	return ~0;
}

uint32 Compiler::TypeArray::GetSize() const {
	return elementCount * elementType->GetSize();
}

}
}
}