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
#include <util/utils.h>
#include <core/compiler/compiler.h>


namespace thc {
namespace core {
namespace instruction {

using namespace utils;
using namespace compiler;

InstBase::InstBase(unsigned int opCode, unsigned int wordCount, const char* const literalName, bool resultId) : opCode(opCode), wordCount(wordCount) {
	size_t len = strlen(literalName) + 1;
	this->literalName = new char[len];
	memcpy(this->literalName, literalName, len);

	if (resultId) {
		id = IDManager::GetNewId();
	}
}

InstBase::~InstBase() {
	delete[] literalName;
}

InstNop::InstNop() : InstBase(THC_SPIRV_OPCODE_OpNop, 1,  "OpNop") { }

InstUndef::InstUndef(unsigned int resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpUndef, 3, "OpUndef", true), resultTypeId(resultTypeId) { }

InstSizeOf::InstSizeOf(unsigned int resultTypeId, unsigned int pointerId) : InstBase(THC_SPIRV_OPCODE_OpSizeOf, 4, "OpSizeOf", true), resultTypeId(resultTypeId), pointerId(pointerId) { }

InstSourceContinued::InstSourceContinued(const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSourceContinued, 1, "OpSourceContinued") { Utils::CopyString(this->source, source); }

InstSource::InstSource(unsigned int sourceLanguage, unsigned int version, unsigned int fileNameId, const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSource, 4, "OpSource"), sourceLanguage(sourceLanguage), version(version), fileNameId(fileNameId) { Utils::CopyString(this->source, source); }

InstSourceExtension::InstSourceExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpSourceExtension, 1, "OpSourceExtension") { Utils::CopyString(this->extension, extension); }

InstName::InstName(unsigned int targetId, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpName, 2, "OpName"), targetId(targetId) { Utils::CopyString(this->name, name); }

InstMemberName::InstMemberName(unsigned int typeId, unsigned int member, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpMemberName, 3, "OpMemberName"), typeId(typeId), member(member) { Utils::CopyString(this->name, name); }

InstString::InstString(const char* const string) : InstBase(THC_SPIRV_OPCODE_OpString, 2, "OpString", true) { Utils::CopyString(this->string, string); }

InstLine::InstLine(unsigned int fileNameId, unsigned int line, unsigned int column) : InstBase(THC_SPIRV_OPCODE_OpLine, 4, "OpLine"), fileNameId(fileNameId), line(line), column(column) {}

InstNoLine::InstNoLine() : InstBase(THC_SPIRV_OPCODE_OpNoLine, 1, "OpNoLine") { }

InstDecorate::InstDecorate(unsigned int targetId, unsigned int decoration, unsigned int* literals, unsigned int numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpDecorate, 3, "OpDecorate"), targetId(targetId), decoration(decoration), numDecorationLiterals() { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstMemberDecorate::InstMemberDecorate(unsigned int structId, unsigned int decoration, unsigned int* literals, unsigned int numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpMemberDecorate, 4, "OpMemberDecorate"), structId(structId), decoration(decoration), numDecorationLiterals(numDecorationLiterals) { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstDecorationGroup::InstDecorationGroup() : InstBase(THC_SPIRV_OPCODE_OpDecorateGroup, 2, "OpDecorationGroup", true) { }

InstGroupDecorate::InstGroupDecorate(unsigned int groupId, unsigned int* targets, unsigned int numTargets) : InstBase(THC_SPIRV_OPCODE_OpGroupDecorate, 1, "OpGroupDecorate"), groupId(groupId), numTargets(numTargets) { memcpy(this->targets, targets, numTargets << 2); }

InstExtension::InstExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpExtension, 1, "OpExtension") { Utils::CopyString(this->extension, extension); }

InstExtInstImport::InstExtInstImport(const char* const extensionSet) : InstBase(THC_SPIRV_OPCODE_OpExtInstImport, 2, "OpExtInstImport", true) { Utils::CopyString(this->extensionSet, extensionSet); }

InstMemoryModel::InstMemoryModel(unsigned int addressingModel, unsigned int memoryModel) : InstBase(THC_SPIRV_OPCODE_OpMemoryModel, 3, "OpMemoryModel") {}

InstEntryPoint::InstEntryPoint(unsigned int executionModel, unsigned int entryPointId, const char* const entryPointName, unsigned int inoutVariableCount, unsigned int* inoutVariables) : InstBase(THC_SPIRV_OPCODE_OpEntryPoint, 3, "OpEntryPoint"), executionModel(executionModel), entryPointId(entryPointId), inoutVariableCount(inoutVariableCount){ Utils::CopyString(this->entryPointName, entryPointName); memcpy(this->inoutVariables, inoutVariables, inoutVariableCount << 2); }

InstExecutionMode::InstExecutionMode(unsigned int entryPointId, unsigned int mode, unsigned int extraOperandCount, unsigned int* extraOperands) : InstBase(THC_SPIRV_OPCODE_OpExecutionMode, 3, "OpExecutionMode"), entryPointId(entryPointId), mode(mode), extraOperandCount(extraOperandCount) { memcpy(this->extraOperand, extraOperand, extraOperandCount << 2); }

InstCapability::InstCapability(unsigned int capability) : InstBase(THC_SPIRV_OPCODE_OpCapability, 2, "OpCapability"), capability(capability) { }

InstConstant::InstConstant(unsigned int resultTypeId, unsigned int valueCount, void* values) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(valueCount) { memcpy(this->values, values, valueCount << 2); }

InstConstant::InstConstant(unsigned int resultTypeId, unsigned int value) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(0), u32(value) { }

InstConstant::InstConstant(unsigned int resultTypeId, float value) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(0), f32(value) {}

InstConstantComposite::InstConstantComposite(unsigned int resultTypeId, unsigned int constituentCount, unsigned int* constituents) : InstBase(THC_SPIRV_OPCODE_OpConstantComposite, 3, "OpConstantComposite", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(this->constituent, constituents, constituentCount << 2); }

InstVariable::InstVariable(unsigned int resultTypeId, unsigned int storageClass, unsigned int initializer) : InstBase(THC_SPIRV_OPCODE_OpVariable, 4, "OpVariable", true), resultTypeId(resultTypeId), storageClass(storageClass), initializer(initializer) { }

InstLoad::InstLoad(unsigned int resultTypeId, unsigned int pointerId, unsigned int memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpLoad, 4, "OpLoad", true), resultTypeId(resultTypeId), pointerId(pointerId), memoryAccess(memoryAccess) { }

InstStore::InstStore(unsigned int pointerId, unsigned int objectId, unsigned int memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpStore, 3, "OpStore"), pointerId(pointerId), objectId(objectId), memoryAccess(memoryAccess) { }

InstCopyMemory::InstCopyMemory(unsigned int targetId, unsigned int sourceId, unsigned int memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemory, 3, "OpCopyMemory"), targetId(targetId), sourceId(sourceId), memoryAccess(memoryAccess) { }

InstCopyMemorySized::InstCopyMemorySized(unsigned int targetId, unsigned int sourceId, unsigned int sizeId, unsigned int memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemorySized, 4, "OpCopyMemorySized"), targetId(targetId), sourceId(sourceId), sizeId(sizeId), memoryAccess(memoryAccess) { }

InstAccessChain::InstAccessChain(unsigned int resultTypeId, unsigned int baseId, unsigned int indexCount, unsigned int* indices) : InstBase(THC_SPIRV_OPCODE_OpAccessChain, 4, "OpAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstInBoundsAccessChain::InstInBoundsAccessChain(unsigned int resultTypeId, unsigned int baseId, unsigned int indexCount, unsigned int* indices) : InstBase(THC_SPIRV_OPCODE_OpInBoundsAccessChain, 4, "OpInBoundsAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstFunction::InstFunction(unsigned int resultTypeId, unsigned int functionControl, unsigned int functionTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunction, 5, "OpFunction", true), resultTypeId(resultTypeId), functionControl(functionControl), functionTypeId(functionTypeId) { }

InstFunctionParameter::InstFunctionParameter(unsigned int resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunctionParameter, 3, "OpFunctionParameter", true), resultTypeId(resultTypeId) { }

InstFunctionEnd::InstFunctionEnd() : InstBase(THC_SPIRV_OPCODE_OpFunctionEnd, 1, "OpFunctionEnd") { }

InstFunctionCall::InstFunctionCall(unsigned int resultTypeId, unsigned int functionId, unsigned int argumentCount, unsigned int* arguments) : InstBase(THC_SPIRV_OPCODE_OpFunctionCall, 4, "OpFunctionCall", true), resultTypeId(resultTypeId), functionId(functionId), argumentCount(argumentCount) { memcpy(this->argument, arguments, argumentCount << 2); }

InstConvertFToU::InstConvertFToU(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToU, 4, "OpConvertFToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertFToS::InstConvertFToS(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToS, 4, "OpConvertFToS", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertSToF::InstConvertSToF(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertSToF, 4, "OpConvertSToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToF::InstConvertUToF(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToF, 4, "OpConvertUToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstUConvert::InstUConvert(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpUConvert, 4, "OpUConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstSConvert::InstSConvert(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpSConvert, 4, "OpSConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstFConvert::InstFConvert(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpFConvert, 4, "OpFConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertPtrToU::InstConvertPtrToU(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertPtrToU, 4, "OpConvertPtrToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToPtr::InstConvertUToPtr(unsigned int resultTypeId, unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToPtr, 4, "OpConvertUToPtr", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstVectorExtractDynamic::InstVectorExtractDynamic(unsigned int resultTypeId, unsigned int vectorId, unsigned int indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorExtractDynamic, 5, "OpVectorExtractDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), indexId(indexId) { }

InstVectorInsertDynamic::InstVectorInsertDynamic(unsigned int resultTypeId, unsigned int vectorId, unsigned int componentId, unsigned int indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorInsertDynamic, 6, "OpVectorInsertDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), componentId(componentId), indexId(indexId) { }

InstVectorShuffle::InstVectorShuffle(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id, unsigned int componentCount, unsigned int* components) : InstBase(THC_SPIRV_OPCODE_OpVectorShuffle, 5, "OpVectorShuffle", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id), componentCount(componentCount) { memcpy(this->component, components, componentCount << 2); }

InstCompositeConstruct::InstCompositeConstruct(unsigned int resultTypeId, unsigned int constituentCount, unsigned int* constituents) : InstBase(THC_SPIRV_OPCODE_OpCompositeConstruct, 3, "OpCompositeConstruct", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(this->constituent, constituents, constituentCount << 2); }

InstCompositeExtract::InstCompositeExtract(unsigned int resultTypeId, unsigned int compositeId, unsigned int indexCount, unsigned int* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeExtract, 4, "OpCompositeExtract", true), resultTypeId(resultTypeId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCompositeInsert::InstCompositeInsert(unsigned int resultTypeId, unsigned int objectId, unsigned int compositeId, unsigned int indexCount, unsigned int* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeInsert, 5, "OpCompositeInsert", true), resultTypeId(resultTypeId), objectId(objectId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCopyObject::InstCopyObject(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpCopyObject, 4, "OpCopyObject", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstTranspose::InstTranspose(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpTranspose, 4, "OpTranspose", true), resultTypeId(resultTypeId), matrixId(matrixId) { }

InstSNegate::InstSNegate(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpSNegate, 4, "OpSNegate", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstFNegate::InstFNegate(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpFNegate, 4, "OpFNegate", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIAdd::InstIAdd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAdd, 5, "OpIAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFAdd::InstFAdd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFAdd, 5, "OpFAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISub::InstISub(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISub, 5, "OpISub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFSub::InstFSub(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFSub, 5, "OpFSub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstIMul::InstIMul(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIMul, 5, "OpIMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMul::InstFMul(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMul, 5, "OpFMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUDiv::InstUDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUDiv, 5, "OpUDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSDiv::InstSDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSDiv, 5, "OpSDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFDiv::InstFDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFDiv, 5, "OpFDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMod::InstUMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMod, 5, "OpUMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSRem::InstSRem(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSRem, 5, "OpSRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMod::InstSMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMod, 5, "OpSMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFRem::InstFRem(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFRem, 5, "OpFRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMod::InstFMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMod, 5, "OpFMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstVectorTimesScalar::InstVectorTimesScalar(unsigned int resultTypeId, unsigned int vectorId, unsigned int scalarId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesScalar, 5, "OpVectorTimesScalar", true), resultTypeId(resultTypeId), vectorId(vectorId), scalarId(scalarId) {}

InstMatrixTimesScalar::InstMatrixTimesScalar(unsigned int resultTypeId, unsigned int matrixUd, unsigned int scalarId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesScalar, 5, "OpMatrixTimesScalar", true), resultTypeId(resultTypeId), matrixId(matrixId), scalarId(scalarId) {}

InstVectorTimesMatrix::InstVectorTimesMatrix(unsigned int resultTypeId, unsigned int vectorId, unsigned int matrixId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesMatrix, 5, "OpVectorTimesMatrix", true), resultTypeId(resultTypeId), vectorId(vectorId), matrixId(matrixId) {}

InstMatrixTimesVector::InstMatrixTimesVector(unsigned int resultTypeId, unsigned int matrixId, unsigned int vectorId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesVector, 5, "OpMatrixTimesVector", true), resultTypeId(resultTypeId), matrixId(matrixId), vectorId(vectorId) {}

InstMatrixTimesMatrix::InstMatrixTimesMatrix(unsigned int resultTypeId, unsigned int matrix1Id, unsigned int matrix2Id) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesMatrix, 5, "OpMatrixTimesMatrix", true), resultTypeId(resultTypeId), matrix1Id(matrix1Id), matrix2Id(matrix2Id) {}

InstOuterProduct::InstOuterProduct(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id) : InstBase(THC_SPIRV_OPCODE_OpOuterProduct, 5, "OpOuterProduct", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstDot::InstDot(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id) : InstBase(THC_SPIRV_OPCODE_OpDot, 5, "OpDot", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstIAddCarry::InstIAddCarry(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAddCarry, 5, "OpIAddCarry", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISubBorrow::InstISubBorrow(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISubBorrow, 5, "OpISubBorrow", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMulExtended::InstUMulExtended(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMulExtended, 5, "OpUMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMulExtended::InstSMulExtended(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMulExtended, 5, "OpSMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstShiftRightLogical::InstShiftRightLogical(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightLogical, 5, "OpShiftRightLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftRightArithmetic::InstShiftRightArithmetic(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightArithmetic, 5, "OpShiftRightArithmetic", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftLeftLogical::InstShiftLeftLogical(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftLeftLogical, 5, "OpShiftLeftLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstBitwiseOr::InstBitwiseOr(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseOr, 5, "OpBitwiseOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseXor::InstBitwiseXor(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseXor, 5, "OpBitwiseXor", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseAnd::InstBitwiseAnd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseAnd, 5, "OpBitwiseAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstNot::InstNot(unsigned int resultTypeId, unsigned int operand1Id) : InstBase(THC_SPIRV_OPCODE_OpNot, 4, "OpNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstBitReverse::InstBitReverse(unsigned int resultTypeId, unsigned int operand1Id) : InstBase(THC_SPIRV_OPCODE_OpBitReverse, 5, "OpBitReverse", true), resultTypeId(resultTypeId), operand1Id(operand1Id) {}

InstAny::InstAny(unsigned int resultTypeId, unsigned int vectorId) : InstBase(THC_SPIRV_OPCODE_OpAny, 4, "OpAny", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstAll::InstAll(unsigned int resultTypeId, unsigned int vectorId) : InstBase(THC_SPIRV_OPCODE_OpAll, 4, "OpAll", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstIsNan::InstIsNan(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpIsNan, 4, "OpIsNan", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIsInf::InstIsInf(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpIsInf, 4, "OpIsInf", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstLogicalEqual::InstLogicalEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalEqual, 5, "OpLogicalEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNotEqual::InstLogicalNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalNotEqual, 5, "OpLogicalNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalOr::InstLogicalOr(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalOr, 5, "OpLogicalOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalAnd::InstLogicalAnd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalAnd, 5, "OpLogicalAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNot::InstLogicalNot(unsigned int resultTypeId, unsigned int operandId) : InstBase(THC_SPIRV_OPCODE_OpLogicalNot, 4, "OpLogicalNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstSelect::InstSelect(unsigned int resultTypeId, unsigned int conditionId, unsigned int object1Id, unsigned int object2Id) : InstBase(THC_SPIRV_OPCODE_OpSelect, 6, "OpSelect", true), resultTypeId(resultTypeId), conditionId(conditionId), object1Id(object1Id), object2Id(object2Id) {}

InstIEqual::InstIEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIEqual, 5, "OpIEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstINotEqual::InstINotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpINotEqual, 5, "OpINotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThan::InstUGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThan, 5, "OpUGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThan::InstSGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThan, 5, "OpSGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThanEqual::InstUGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThanEqual, 5, "OpUGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThanEqual::InstSGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThanEqual, 5, "OpSGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThan::InstULessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThan, 5, "OpULessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThan::InstSLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThan, 5, "OpSLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThanEqual::InstULessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThanEqual, 5, "OpULessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThanEqual::InstSLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThanEqual, 5, "OpSLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdEqual::InstFOrdEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdEqual, 5, "OpFOrdEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordEqual::InstFUnordEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordEqual, 5, "OpFUnordEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdNotEqual::InstFOrdNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdNotEqual, 5, "OpFOrdNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordNotEqual::InstFUnordNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordNotEqual, 5, "OpFUnordNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThan::InstFOrdLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThan, 5, "OpFOrdLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThan::InstFUnordLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThan, 5, "OpFUnordLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThan::InstFOrdGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThan, 5, "OpFOrdGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThan::InstFUnordGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThan, 5, "OpFUnordGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThanEqual::InstFOrdLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThanEqual, 5, "OpFOrdLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThanEqual::InstFUnordLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThanEqual, 5, "OpFUnordLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThanEqual::InstFOrdGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThanEqual, 5, "OpFOrdGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThanEqual::InstFUnordGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThanEqual, 5, "OpFUnordGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstPhi::InstPhi(unsigned int resultTypeId, unsigned int pairCount, PhiPair* pairs) : InstBase(THC_SPIRV_OPCODE_OpPhi, 3, "OpPhi", true), resultTypeId(resultTypeId), pairCount(pairCount) { memcpy(this->pairs, pairs, pairCount << 3); }

InstLoopMerge::InstLoopMerge(unsigned int mergeBlockId, unsigned int continueTargetId, unsigned int loopControl) : InstBase(THC_SPIRV_OPCODE_OpLoopMerge, 4, "OpLoopMerge"), mergeBlockId(mergeBlockId), continueTargetId(continueTargetId), loopControl(loopControl) {}

InstSelectionMerge::InstSelectionMerge(unsigned int mergeBlockId, unsigned int selectionControl) : InstBase(THC_SPIRV_OPCODE_OpSelectionMerge, 3, "OpSelectionMerge"), mergeBlockId(mergeBlockId), selectionControl(selectionControl) {}

InstLabel::InstLabel() : InstBase(THC_SPIRV_OPCODE_OpLabel, 2, "OpLabel", true) {}

InstBranch::InstBranch(unsigned int targetLabelId) : InstBase(THC_SPIRV_OPCODE_OpBranch, 2, "OpBranch"), targetLabelId(targetLabelId) {}

InstBranchConditional::InstBranchConditional(unsigned int conditionId, unsigned int trueLabelId, unsigned int falseLabelId, unsigned int trueWeight, unsigned int falseWeight) : InstBase(THC_SPIRV_OPCODE_OpBranchConditional, 6, "OpBranchConditional"), conditionId(conditionId), trueLabelId(trueLabelId), falseLabelId(falseLabelId), trueWeight(trueWeight), falseWeight(falseWeight) {}

InstSwitch::InstSwitch(unsigned int selectorId, unsigned int defaultId) : InstBase(THC_SPIRV_OPCODE_OpSwitch, 3, "OpSwitch"), selectorId(selectorId), defaultId(defaultId), pairCount(pairCount) { memcpy(this->pairs, pairs, pairCount << 3); }

InstKill::InstKill() : InstBase(THC_SPIRV_OPCODE_OpKill, 1, "OpKill") {}

InstReturn::InstReturn() : InstBase(THC_SPIRV_OPCODE_OpReturn, 1, "OpReturn") {}

InstReturnValue::InstReturnValue(unsigned int valueId) : InstBase(THC_SPIRV_OPCODE_OpReturnValue, 2, "OpReturnValue"), valueId(valueId) {}

}
}
}