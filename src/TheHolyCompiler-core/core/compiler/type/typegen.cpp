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

void TypeVoid::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
}

void TypeInt::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = bits;
	words[3] = sign;
}

void TypeFloat::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = bits;
}

void TypeVector::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = componentTypeId;
	words[3] = componentCount;
}

void TypeMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = columnTypeId;
	words[3] = columnCount;
}

void TypeArray::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = elementTypeId;
	words[3] = elementCount;
}

void TypeStruct::GetInstWords(uint32* words) const {
	wordCount += memberCount;
	InstBase::GetInstWords(words);

	words[1] = id;
	memcpy(words+2, memberTypeId, memberCount << 2);
}

void TypePointer::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = storageClass;
	words[3] = typeId;
}

void TypeFunction::GetInstWords(uint32* words) const {
	wordCount += parameterCount;
	InstBase::GetInstWords(words);

	words[1] = id;
	words[2] = returnTypeId;
	memcpy(words+3, parameterId, parameterCount << 2);
}

}
}
}