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

void InstBase::GetInstWords(unsigned int* words) const {
	words[0] = opCode | (wordCount << 16);
}

void InstUndef::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
}

void InstSizeOf::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = pointerId;
}

void InstSourceContinued::GetInstWords(unsigned int* words) const {
	size_t len = strlen(source)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	memcpy(words+1, source, len);
}

void InstSource::GetInstWords(unsigned int* words) const {
	size_t len = strlen(source)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	
	words[1] = sourceLanguage;
	words[2] = version;
	words[3] = fileNameId;
	memcpy(words+4, source, len);
}

void InstSourceExtension::GetInstWords(unsigned int* words) const {
	size_t len = strlen(extension)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	InstBase::GetInstWords(words);
	memcpy(words+1, extension, len);
}

void InstName::GetInstWords(unsigned int* words) const {
	size_t len = strlen(name)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = targetId;

	InstBase::GetInstWords(words);
	memcpy(words+2, name, len);
}

void InstMemberName::GetInstWords(unsigned int* words) const {
	size_t len = strlen(name)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = typeId;
	words[2] = member;

	InstBase::GetInstWords(words);
	memcpy(words+3, name, len);
}

void InstString::GetInstWords(unsigned int* words) const {
	size_t len = strlen(string)+1;
	wordCount += (unsigned int)((len >> 2) + (len % 4 ? 1 : 0));

	words[1] = id;

	InstBase::GetInstWords(words);
	memcpy(words+2, string, len);
}

void InstLine::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = fileNameId;
	words[2] = line;
	words[3] = column;
}

void InstDecorate::GetInstWords(unsigned int* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = decoration;
	memcpy(words+3, literals, numDecorationLiterals << 2);
}

void InstMemberDecorate::GetInstWords(unsigned int* words) const {
	wordCount += numDecorationLiterals;
	InstBase::GetInstWords(words);

	words[1] = structId;
	words[2] = decoration;
	memcpy(words+3, literals, numDecorationLiterals << 2);
}

void InstDecorationGroup::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
}

void InstGroupDecorate::GetInstWords(unsigned int* words) const {
	wordCount += numTargets;
	InstBase::GetInstWords(words);

	words[1] = groupId;
	memcpy(words+2, targets, numTargets << 2);
}

void InstExtension::GetInstWords(unsigned int* words) const {
	size_t len = strlen(extension) + 1;
	wordCount += (unsigned int)len;
	InstBase::GetInstWords(words);

	memcpy(words+1, extension, len);
}

void InstExtInstImport::GetInstWords(unsigned int* words) const {
	size_t len = strlen(extensionSet) + 1;
	wordCount += (unsigned int)len;
	InstBase::GetInstWords(words);

	words[1] = id;

	memcpy(words+2, extensionSet, len);
}

void InstMemoryModel::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = addressingModel;
	words[2] = memoryModel;
}

void InstEntryPoint::GetInstWords(unsigned int* words) const {
	size_t len = strlen(entryPointName) + 1;
	wordCount += (unsigned int)len + inoutVariableCount;
	InstBase::GetInstWords(words);

	words[1] = executionModel;
	words[2] = entryPointId;

	memcpy(words+3, entryPointName, len);
	
	unsigned int offset = ((unsigned int)len >> 4) + ((unsigned int)len % 4 ? 1 : 0) + 3;

	memcpy(words+offset, inoutVariables, inoutVariableCount << 2);
}

void InstExecutionMode::GetInstWords(unsigned int* words) const {
	wordCount += extraOperandCount;
	InstBase::GetInstWords(words);

	words[1] = entryPointId;
	words[2] = mode;
	
	memcpy(words+3, extraOperand, extraOperandCount << 2);
}

void InstCapability::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = capability;
}

void InstConstant::GetInstWords(unsigned int* words) const {
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

void InstConstantComposite::GetInstWords(unsigned int* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, constituent, constituentCount << 2);
}

void InstVariable::GetInstWords(unsigned int* words) const {
	wordCount += initializer ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = storageClass;
	words[4] = initializer;
}

void InstLoad::GetInstWords(unsigned int* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = pointerId;
	words[4] = memoryAccess;
}

void InstStore::GetInstWords(unsigned int* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = pointerId;
	words[2] = objectId;
	words[3] = memoryAccess;
}

void InstCopyMemory::GetInstWords(unsigned int* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = sourceId;
	words[3] = memoryAccess;
}

void InstCopyMemorySized::GetInstWords(unsigned int* words) const {
	wordCount += memoryAccess ? 1 : 0;
	InstBase::GetInstWords(words);

	words[1] = targetId;
	words[2] = sourceId;
	words[3] = sizeId;
	words[4] = memoryAccess;
}

void InstAccessChain::GetInstWords(unsigned int* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;

	memcpy(words+4, index, indexCount << 2);
}

void InstInBoundsAccessChain::GetInstWords(unsigned int* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;

	memcpy(words+4, index, indexCount << 2);
}

void InstFunction::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = functionControl;
	words[4] = functionTypeId;
}

void InstFunctionParameter::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
}

void InstFunctionCall::GetInstWords(unsigned int* words) const {
	wordCount += argumentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = functionId;

	memcpy(words+4, argument, argumentCount << 2);
}

void InstConvertFToU::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertFToS::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertSToF::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertUToF::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstUConvert::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstSConvert::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstFConvert::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertPtrToU::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstConvertUToPtr::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = valueId;
}

void InstVectorExtractDynamic::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = indexId;
}

void InstVectorInsertDynamic::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = componentId;
	words[5] = indexId;
}

void InstVectorShuffle::GetInstWords(unsigned int* words) const {
	wordCount += componentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
	
	memcpy(words+5, component, componentCount << 2);
}

void InstCompositeConstruct::GetInstWords(unsigned int* words) const {
	wordCount += constituentCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, constituent, constituentCount << 2);
}

void InstCompositeExtract::GetInstWords(unsigned int* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = compositeId;

	memcpy(words+4, index, indexCount << 2);
}

void InstCompositeInsert::GetInstWords(unsigned int* words) const {
	wordCount += indexCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = objectId;
	words[4] = compositeId;

	memcpy(words+5, index, indexCount << 2);
}

void InstCopyObject::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstTranspose::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
}

void InstSNegate::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstFNegate::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstIAdd::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFAdd::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstISub::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFSub::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstIMul::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFMul::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUDiv::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}


void InstSDiv::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFDiv::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUMod::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSRem::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSMod::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFRem::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFMod::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstVectorTimesScalar::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = scalarId;
}

void InstMatrixTimesScalar::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
	words[4] = scalarId;
}

void InstVectorTimesMatrix::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
	words[4] = matrixId;
}

void InstMatrixTimesVector::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrixId;
	words[4] = vectorId;
}

void InstMatrixTimesMatrix::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = matrix1Id;
	words[4] = matrix2Id;
}

void InstOuterProduct::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
}

void InstDot::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vector1Id;
	words[4] = vector2Id;
}


void InstIAddCarry::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstISubBorrow::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUMulExtended::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSMulExtended::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstShiftRightLogical::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstShiftRightArithmetic::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstShiftLeftLogical::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = baseId;
	words[4] = shiftId;
}

void InstBitwiseOr::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstBitwiseXor::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstBitwiseAnd::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstNot::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstBitReverse::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
}

void InstAny::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
}

void InstAll::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = vectorId;
}

void InstIsNan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstIsInf::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstLogicalEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalNotEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalOr::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalAnd::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstLogicalNot::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operandId;
}

void InstSelect::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = conditionId;
	words[4] = object1Id;
	words[5] = object2Id;
}

void InstIEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstINotEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUGreaterThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSGreaterThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstUGreaterThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSGreaterThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstULessThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSLessThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstULessThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstSLessThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdNotEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordNotEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdLessThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordLessThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdGreaterThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordGreaterThan::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdLessThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordLessThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFOrdGreaterThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstFUnordGreaterThanEqual::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = operand1Id;
	words[4] = operand2Id;
}

void InstPhi::GetInstWords(unsigned int* words) const {
	wordCount += pairCount;
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;

	memcpy(words+3, pairs, pairCount << 3);
}

void InstLoopMerge::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId;
	words[2] = continueTargetId;
	words[3] = loopControl;
}

void InstSelectionMerge::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = mergeBlockId;
	words[2] = selectionControl;
}

void InstLabel::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = id;
}

void InstBranch::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = targetLabelId;
}

void InstBranchConditional::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = conditionId;
	words[2] = trueLabelId;
	words[3] = falseLabelId;
	words[4] = trueWeight;
	words[5] = falseWeight;
}

void InstSwitch::GetInstWords(unsigned int* words) const {
	wordCount += pairCount;
	InstBase::GetInstWords(words);

	words[1] = selectorId;
	words[2] = defaultId;

	memcpy(words+3, pairs, pairCount << 3);
}

void InstReturnValue::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = valueId;
}


}
}
}

