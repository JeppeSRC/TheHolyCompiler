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

void InstTypeVoid::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
}

void InstTypeBool::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
}

void InstTypeInt::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = bits;
	words[3] = sign;
}

void InstTypeFloat::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = bits;
}

void InstTypeVector::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = componentTypeId->id;
	words[3] = componentCount;
}

void InstTypeMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = columnTypeId->id;
	words[3] = columnCount;
}

void InstTypeArray::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = elementTypeId->id;
	words[3] = elementCountId->id;
}

void InstTypeStruct::GetInstWords(uint32* words) const {
	wordCount += memberCount;
	InstBase::GetInstWords(words);

	words[1] = id->id;

	for (uint32 i = 0; i < memberCount; i++) {
		words[i + 2] = memberTypeId[i]->id;
	}
}

void InstTypePointer::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = storageClass;
	words[3] = typeId->id;
}

void InstTypeFunction::GetInstWords(uint32* words) const {
	wordCount += parameterCount;
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = returnTypeId->id;
	
	for (uint32 i = 0; i < parameterCount; i++) {
		words[i + 3] = parameterId[i]->id;
	}
}

void InstTypeImage::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = sampledType->id;
	words[3] = dim;
	words[4] = depth;
	words[5] = arrayed;
	words[6] = multiSampled;
	words[7] = sampled;
	words[8] = imageFormat;
}

void InstTypeSampledImage::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
	words[2] = imageType->id;

}

}
}
}