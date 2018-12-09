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

	words[1] = resultTypeId;
	words[2] = id;
}

void InstSizeOf::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = pointerId;
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
	words[3] = fileNameId;
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

	words[1] = targetId;

	InstBase::GetInstWords(words);
	memcpy(words+2, name, len);
}

void InstMemberName::GetInstWords(uint32* words) const {
	uint64 len = strlen(name)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = typeId;
	words[2] = member;

	InstBase::GetInstWords(words);
	memcpy(words+3, name, len);
}

void InstString::GetInstWords(uint32* words) const {
	uint64 len = strlen(string)+1;
	wordCount += (uint32)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = id;

	InstBase::GetInstWords(words);
	memcpy(words+2, string, len);
}

void InstLine::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = fileNameId;
	words[2] = line;
	words[3] = column;
}

void InstDecorate::GetInstWords(uint32* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = decoration;
	memcpy(words+3, literals, numDecorationLiterals << 2);
}

void InstMemberDecorate::GetInstWords(uint32* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = structId;
	words[2] = decoration;
	memcpy(words+3, literals, numDecorationLiterals << 2);
}

void InstDecorationGroup::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
}

void InstGroupDecorate::GetInstWords(uint32* words) const {
	wordCount += numTargets;
	InstBase::GetInstWords(words);

	words[1] = groupId;
	memcpy(words+2, targets, numTargets << 2);
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

	words[1] = id;

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
	words[2] = entryPointId;

	memcpy(words+3, entryPointName, len);
	
	uint32 offset = ((uint32)len >> 4) + ((uint32)len % 4 ? 1 : 0) + 3;

	memcpy(words+offset, inoutVariables, inoutVariableCount << 2);
}

void InstExecutionMode::GetInstWords(uint32* words) const {
	wordCount += extraOperandCount;
	InstBase::GetInstWords(words);

	words[1] = entryPointId;
	words[2] = mode;
	
	memcpy(words+3, extraOperand, extraOperandCount << 2);
}

void InstCapability::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = capability;
}

void InstConstant::GetInstWords(uint32* words) const {
	wordCount += valueCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	if (valueCount > 0) {
		memcpy(words+3, values, valueCount << 2);
	} else {
		words[3] = u32;
	}
}

void InstConstantComposite::GetInstWords(uint32* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, constituents, constituentCount << 2);
}

void InstVariable::GetInstWords(uint32* words) const {
	wordCount += initializer ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = storageClass;
	words[4] = initializer;
}

void InstLoad::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = pointerId;
	words[4] = memoryAccess;
}

void InstStore::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = pointerId;
	words[2] = objectId;
	words[3] = memoryAccess;
}

void InstCopyMemory::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = sourceId;
	words[3] = memoryAccess;
}

void InstCopyMemorySized::GetInstWords(uint32* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = sourceId;
	words[3] = sizeId;
	words[4] = memoryAccess;
}

void InstAccessChain::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;

	memcpy(words+4, index, indexCount << 2);
}

void InstInBoundsAccessChain::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;

	memcpy(words+4, index, indexCount << 2);
}

void InstFunction::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = functionControl;
	words[4] = functionTypeId;
}

void InstFunctionParameter::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
}

void InstFunctionCall::GetInstWords(uint32* words) const {
	wordCount += argumentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = functionId;

	memcpy(words+4, argument, argumentCount << 2);
}

void InstConvertFToU::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertFToS::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertSToF::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertUToF::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstUConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstSConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstFConvert::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertPtrToU::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertUToPtr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstVectorExtractDynamic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = indexId;
}

void InstVectorInsertDynamic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = componentId;
	words[5] = indexId;
}

void InstVectorShuffle::GetInstWords(uint32* words) const {
	wordCount += componentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
	
	memcpy(words+5, component, componentCount << 2);
}

void InstCompositeConstruct::GetInstWords(uint32* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, constituent, constituentCount << 2);
}

void InstCompositeExtract::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = compositeId;

	memcpy(words+4, index, indexCount << 2);
}

void InstCompositeInsert::GetInstWords(uint32* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = objectId;
	words[4] = compositeId;

	memcpy(words+5, index, indexCount << 2);
}

void InstCopyObject::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstTranspose::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
}

void InstSNegate::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstFNegate::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstIAdd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFAdd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstISub::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFSub::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstIMul::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFMul::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}


void InstSDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFDiv::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSRem::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFRem::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFMod::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstVectorTimesScalar::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = scalarId;
}

void InstMatrixTimesScalar::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
	words[4] = scalarId;
}

void InstVectorTimesMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = matrixId;
}

void InstMatrixTimesVector::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
	words[4] = vectorId;
}

void InstMatrixTimesMatrix::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrix1Id;
	words[4] = matrix2Id;
}

void InstOuterProduct::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
}

void InstDot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
}


void InstIAddCarry::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstISubBorrow::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUMulExtended::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSMulExtended::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstShiftRightLogical::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstShiftRightArithmetic::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstShiftLeftLogical::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstBitwiseOr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstBitwiseXor::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstBitwiseAnd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstNot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstBitReverse::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
}

void InstAny::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
}

void InstAll::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
}

void InstIsNan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstIsInf::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstLogicalEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalOr::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalAnd::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalNot::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstSelect::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = conditionId;
	words[4] = object1Id;
	words[5] = object2Id;
}

void InstIEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstINotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstULessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstULessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordNotEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordLessThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordGreaterThan::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordLessThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordGreaterThanEqual::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstPhi::GetInstWords(uint32* words) const {
	wordCount += pairCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, pairs, pairCount << 3);
}

void InstLoopMerge::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId;
	words[2] = continueTargetId;
	words[3] = loopControl;
}

void InstSelectionMerge::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId;
	words[2] = selectionControl;
}

void InstLabel::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
}

void InstBranch::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = targetLabelId;
}

void InstBranchConditional::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = conditionId;
	words[2] = trueLabelId;
	words[3] = falseLabelId;
	words[4] = trueWeight;
	words[5] = falseWeight;
}

void InstSwitch::GetInstWords(uint32* words) const {
	wordCount += pairCount;
	InstBase::GetInstWords(words);

	words[1] = selectorId;
	words[2] = defaultId;

	memcpy(words+3, pairs, pairCount << 3);
}

void InstReturnValue::GetInstWords(uint32* words) const {
	InstBase::GetInstWords(words);

	words[1] = valueId;
}


}
}
}

