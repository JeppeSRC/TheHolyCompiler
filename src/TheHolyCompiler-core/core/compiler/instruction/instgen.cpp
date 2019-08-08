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

#include "instructions.h"


namespace thc {
namespace core {
namespace instruction {

void InstBase::GetInstWords(uint32* words) const {
	words[0] = opCode | (wordCount << 16);
}

void InstUndef::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
}

void InstSizeOf::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = pointerId->id;
}

void InstSourceContinued::GetInstWords(uint32* words) const {
	uint64 len = strlen(source)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	memcpy(words+1, source, len);
}

void InstSource::GetInstWords(uint32* words) const {
	uint64 len = strlen(source)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	
	words[1] = sourceLanguage;
	words[2] = version;
	words[3] = fileNameId->id;
	memcpy(words+4, source, len);
}

void InstSourceExtension::GetInstWords(uint32* words) const {
	uint64 len = strlen(extension)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	memcpy(words+1, extension, len);
}

void InstName::GetInstWords(uint32* words) const {
	uint64 len = strlen(name)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = targetId->id;

	InstBase::GetInstWords(words);
	memcpy(words+2, name, len);
}

void InstMemberName::GetInstWords(uint32* words) const {
	uint64 len = strlen(name)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = typeId->id;
	words[2] = member;

	InstBase::GetInstWords(words);
	memcpy(words+3, name, len);
}

void InstString::GetInstWords(uint32* words) const {
	uint64 len = strlen(string)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = id->id;

	InstBase::GetInstWords(words);
	memcpy(words+2, string, len);
}

void InstLine::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = fileNameId->id;
	words[2] = line;
	words[3] = column;
}

void InstDecorate::GetInstWords(uint32* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = targetId->id;
	words[2] = decoration;
	memcpy(words+3, literals, numDecorationLiterals << 2);
}

void InstMemberDecorate::GetInstWords(uint32* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = structId->id;
	words[2] = member;
	words[3] = decoration;
	memcpy(words+4, literals, numDecorationLiterals << 2);
}

void InstDecorationGroup::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
}

void InstGroupDecorate::GetInstWords(uint32* words) const {
	wordCount += numTargets;
	InstBase::GetInstWords(words);

	words[1] = groupId->id;

	for (uint32 i = 0; i < numTargets; i++) {
		words[i+2] = targetId[i]->id;
	}
}

void InstExtension::GetInstWords(uint32* words) const {
	uint64 len = strlen(extension) + 1;
	wordCount += (uint32)len;
	InstBase::GetInstWords(words);

	memcpy(words+1, extension, len);
}

void InstExtInstImport::GetInstWords(uint32* words) const {
	uint64 len = strlen(extensionSet) + 1;
	wordCount += (uint32)len;
	InstBase::GetInstWords(words);

	words[1] = id->id;

	memcpy(words+2, extensionSet, len);
}

void InstMemoryModel::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = addressingModel;
	words[2] = memoryModel;
}

void InstEntryPoint::GetInstWords(uint32* words) const {
	uint64 len = strlen(entryPointName) + 1;
	wordCount += (uint32)len + inoutVariableCount;
	InstBase::GetInstWords(words);

	words[1] = executionModel;
	words[2] = entryPointId->id;

	memcpy(words+3, entryPointName, len);
	
	uint32 offset = ((uint32)len >> 4) + ((uint32)len % 4 ? 1 : 0) + 3;

	for (uint32 i = 0; i < inoutVariableCount; i++) {
		words[i+offset] = inoutVariableId[i]->id;
	}
}

void InstExecutionMode::GetInstWords(uint32* words) const {
	wordCount += extraOperandCount;
	InstBase::GetInstWords(words);

	words[1] = entryPointId->id;
	words[2] = mode;
	
	memcpy(words+3, extraOperand, extraOperandCount << 2);
}

void InstCapability::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = capability;
}

void InstConstantTrue::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
}

void InstConstantFalse::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
}

void InstConstant::GetInstWords(uint32* words) const {
	wordCount += valueCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;

	if (valueCount > 0) {
		memcpy(words+3, values, valueCount << 2);
	} else {
		words[3] = u32;
	}
}

void InstConstantComposite::GetInstWords(uint32* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;

	for (uint32 i = 0; i < constituentCount; i++) {
		words[i + 3] = constituentId[i]->id;
	}
}

void InstVariable::GetInstWords(uint32* words) const {
	wordCount += initializer ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = storageClass;
	words[4] = initializer;
}

void InstLoad::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = pointerId->id;
	words[4] = memoryAccess;
}

void InstStore::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = pointerId->id;
	words[2] = objectId->id;
	words[3] = memoryAccess;
}

void InstCopyMemory::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId->id;
	words[2] = sourceId->id;
	words[3] = memoryAccess;
}

void InstCopyMemorySized::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId->id;
	words[2] = sourceId->id;
	words[3] = sizeId->id;
	words[4] = memoryAccess;
}

void InstAccessChain::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = baseId->id;

	for (uint32 i = 0; i < indexCount; i++) {
		words[i + 4] = indexId[i]->id;
	}
}

void InstInBoundsAccessChain::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = baseId->id;

	for (uint32 i = 0; i < indexCount; i++) {
		words[i + 4] = indexId[i]->id;
	}
}

void InstFunction::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = functionControl;
	words[4] = functionTypeId->id;
}

void InstFunctionParameter::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
}

void InstFunctionCall::GetInstWords(uint32* words) const {
	wordCount += argumentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = functionId->id;

	for (uint32 i = 0; i < argumentCount; i++) {
		words[i + 4] = argumentId[i]->id;
	}
}

void InstConvertFToU::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstConvertFToS::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstConvertSToF::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstConvertUToF::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstUConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstSConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstFConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstConvertPtrToU::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstConvertUToPtr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = valueId->id;
}

void InstVectorExtractDynamic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
	words[4] = indexId->id;
}

void InstVectorInsertDynamic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
	words[4] = componentId->id;
	words[5] = indexId->id;
}

void InstVectorShuffle::GetInstWords(uint32* words) const {
	wordCount += componentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vector1Id->id;
	words[4] = vector2Id->id;
	
	memcpy(words+5, component, componentCount << 2);
}

void InstCompositeConstruct::GetInstWords(uint32* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;

	for (uint32 i = 0; i < constituentCount; i++) {
		words[i + 3] = constituentId[i]->id;
	}
}

void InstCompositeExtract::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = compositeId->id;

	memcpy(words+4, index, indexCount << 2);
}

void InstCompositeInsert::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = objectId->id;
	words[4] = compositeId->id;

	memcpy(words+5, index, indexCount << 2);
}

void InstCopyObject::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstTranspose::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = matrixId->id;
}

void InstSNegate::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstFNegate::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstIAdd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFAdd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstISub::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFSub::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstIMul::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFMul::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstUDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}


void InstSDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstUMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSRem::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFRem::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstVectorTimesScalar::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
	words[4] = scalarId->id;
}

void InstMatrixTimesScalar::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = matrixId->id;
	words[4] = scalarId->id;
}

void InstVectorTimesMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
	words[4] = matrixId->id;
}

void InstMatrixTimesVector::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = matrixId->id;
	words[4] = vectorId->id;
}

void InstMatrixTimesMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = matrix1Id->id;
	words[4] = matrix2Id->id;
}

void InstOuterProduct::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vector1Id->id;
	words[4] = vector2Id->id;
}

void InstDot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vector1Id->id;
	words[4] = vector2Id->id;
}


void InstIAddCarry::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstISubBorrow::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstUMulExtended::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSMulExtended::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstShiftRightLogical::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = baseId->id;
	words[4] = shiftId->id;
}

void InstShiftRightArithmetic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = baseId->id;
	words[4] = shiftId->id;
}

void InstShiftLeftLogical::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = baseId->id;
	words[4] = shiftId->id;
}

void InstBitwiseOr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstBitwiseXor::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstBitwiseAnd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstNot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstBitReverse::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
}

void InstAny::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
}

void InstAll::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = vectorId->id;
}

void InstIsNan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstIsInf::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstLogicalEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstLogicalNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstLogicalOr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstLogicalAnd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstLogicalNot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operandId->id;
}

void InstSelect::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = conditionId->id;
	words[4] = object1Id->id;
	words[5] = object2Id->id;
}

void InstIEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstINotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstUGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstUGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstULessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstULessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstSLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFOrdGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstFUnordGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;
	words[3] = operand1Id->id;
	words[4] = operand2Id->id;
}

void InstPhi::GetInstWords(uint32* words) const {
	wordCount += pairCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId->id;
	words[2] = id->id;

	memcpy(words+3, pairs, pairCount << 3);
}

void InstLoopMerge::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId->id;
	words[2] = continueTargetId->id;
	words[3] = loopControl;
}

void InstSelectionMerge::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId->id;
	words[2] = selectionControl;
}

void InstLabel::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id->id;
}

void InstBranch::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = targetLabelId;
}

void InstBranchConditional::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = conditionId->id;
	words[2] = trueLabelId->id;
	words[3] = falseLabelId->id;
	words[4] = trueWeight;
	words[5] = falseWeight;
}

void InstSwitch::GetInstWords(uint32* words) const {
	wordCount += pairCount * 2;
	InstBase::GetInstWords(words);

	words[1] = selectorId->id;
	words[2] = defaultId->id;

	for (uint32 i = 0; i < pairCount; i++) {
		words[i * 2 + 3] = pair[i].litteral;
		words[i * 2 + 4] = pair[i].labelId->id;
	}
}

void InstReturnValue::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = valueId->id;
}


}
}
}

