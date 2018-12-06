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

InstBase::InstBase(uint32 opCode, uint32 wordCount, const char* const literalName, bool resultId, InstType type) : opCode(opCode), wordCount(wordCount), type(type) {
	uint64 len = strlen(literalName) + 1;
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

InstUndef::InstUndef(uint32 resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpUndef, 3, "OpUndef", true), resultTypeId(resultTypeId) { }

InstSizeOf::InstSizeOf(uint32 resultTypeId, uint32 pointerId) : InstBase(THC_SPIRV_OPCODE_OpSizeOf, 4, "OpSizeOf", true), resultTypeId(resultTypeId), pointerId(pointerId) { }

InstSourceContinued::InstSourceContinued(const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSourceContinued, 1, "OpSourceContinued") { Utils::CopyString(this->source, source); }

InstSource::InstSource(uint32 sourceLanguage, uint32 version, uint32 fileNameId, const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSource, 4, "OpSource"), sourceLanguage(sourceLanguage), version(version), fileNameId(fileNameId) { Utils::CopyString(this->source, source); }

InstSourceExtension::InstSourceExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpSourceExtension, 1, "OpSourceExtension") { Utils::CopyString(this->extension, extension); }

InstName::InstName(uint32 targetId, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpName, 2, "OpName"), targetId(targetId) { Utils::CopyString(this->name, name); }

InstMemberName::InstMemberName(uint32 typeId, uint32 member, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpMemberName, 3, "OpMemberName"), typeId(typeId), member(member) { Utils::CopyString(this->name, name); }

InstString::InstString(const char* const string) : InstBase(THC_SPIRV_OPCODE_OpString, 2, "OpString", true) { Utils::CopyString(this->string, string); }

InstLine::InstLine(uint32 fileNameId, uint32 line, uint32 column) : InstBase(THC_SPIRV_OPCODE_OpLine, 4, "OpLine"), fileNameId(fileNameId), line(line), column(column) {}

InstNoLine::InstNoLine() : InstBase(THC_SPIRV_OPCODE_OpNoLine, 1, "OpNoLine") { }

InstDecorate::InstDecorate(uint32 targetId, uint32 decoration, uint32* literals, uint32 numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpDecorate, 3, "OpDecorate"), targetId(targetId), decoration(decoration), numDecorationLiterals() { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstMemberDecorate::InstMemberDecorate(uint32 structId, uint32 decoration, uint32* literals, uint32 numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpMemberDecorate, 4, "OpMemberDecorate"), structId(structId), decoration(decoration), numDecorationLiterals(numDecorationLiterals) { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstDecorationGroup::InstDecorationGroup() : InstBase(THC_SPIRV_OPCODE_OpDecorateGroup, 2, "OpDecorationGroup", true) { }

InstGroupDecorate::InstGroupDecorate(uint32 groupId, uint32* targets, uint32 numTargets) : InstBase(THC_SPIRV_OPCODE_OpGroupDecorate, 1, "OpGroupDecorate"), groupId(groupId), numTargets(numTargets) { memcpy(this->targets, targets, numTargets << 2); }

InstExtension::InstExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpExtension, 1, "OpExtension") { Utils::CopyString(this->extension, extension); }

InstExtInstImport::InstExtInstImport(const char* const extensionSet) : InstBase(THC_SPIRV_OPCODE_OpExtInstImport, 2, "OpExtInstImport", true) { Utils::CopyString(this->extensionSet, extensionSet); }

InstMemoryModel::InstMemoryModel(uint32 addressingModel, uint32 memoryModel) : InstBase(THC_SPIRV_OPCODE_OpMemoryModel, 3, "OpMemoryModel") {}

InstEntryPoint::InstEntryPoint(uint32 executionModel, uint32 entryPointId, const char* const entryPointName, uint32 inoutVariableCount, uint32* inoutVariables) : InstBase(THC_SPIRV_OPCODE_OpEntryPoint, 3, "OpEntryPoint"), executionModel(executionModel), entryPointId(entryPointId), inoutVariableCount(inoutVariableCount){ Utils::CopyString(this->entryPointName, entryPointName); memcpy(this->inoutVariables, inoutVariables, inoutVariableCount << 2); }

InstExecutionMode::InstExecutionMode(uint32 entryPointId, uint32 mode, uint32 extraOperandCount, uint32* extraOperands) : InstBase(THC_SPIRV_OPCODE_OpExecutionMode, 3, "OpExecutionMode"), entryPointId(entryPointId), mode(mode), extraOperandCount(extraOperandCount) { memcpy(this->extraOperand, extraOperand, extraOperandCount << 2); }

InstCapability::InstCapability(uint32 capability) : InstBase(THC_SPIRV_OPCODE_OpCapability, 2, "OpCapability"), capability(capability) { }

InstConstant::InstConstant(uint32 resultTypeId, uint32 valueCount, void* values) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(valueCount) { memcpy(this->values, values, valueCount << 2); }

InstConstant::InstConstant(uint32 resultTypeId, uint32 value) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(0), u32(value) { }

InstConstant::InstConstant(uint32 resultTypeId, float32 value) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(0), f32(value) {}

InstConstantComposite::InstConstantComposite(uint32 resultTypeId, uint32 constituentCount, uint32* constituents) : InstBase(THC_SPIRV_OPCODE_OpConstantComposite, 3, "OpConstantComposite", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(this->constituent, constituents, constituentCount << 2); }

InstVariable::InstVariable(uint32 resultTypeId, uint32 storageClass, uint32 initializer) : InstBase(THC_SPIRV_OPCODE_OpVariable, 4, "OpVariable", true), resultTypeId(resultTypeId), storageClass(storageClass), initializer(initializer) { }

InstLoad::InstLoad(uint32 resultTypeId, uint32 pointerId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpLoad, 4, "OpLoad", true), resultTypeId(resultTypeId), pointerId(pointerId), memoryAccess(memoryAccess) { }

InstStore::InstStore(uint32 pointerId, uint32 objectId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpStore, 3, "OpStore"), pointerId(pointerId), objectId(objectId), memoryAccess(memoryAccess) { }

InstCopyMemory::InstCopyMemory(uint32 targetId, uint32 sourceId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemory, 3, "OpCopyMemory"), targetId(targetId), sourceId(sourceId), memoryAccess(memoryAccess) { }

InstCopyMemorySized::InstCopyMemorySized(uint32 targetId, uint32 sourceId, uint32 sizeId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemorySized, 4, "OpCopyMemorySized"), targetId(targetId), sourceId(sourceId), sizeId(sizeId), memoryAccess(memoryAccess) { }

InstAccessChain::InstAccessChain(uint32 resultTypeId, uint32 baseId, uint32 indexCount, uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpAccessChain, 4, "OpAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstInBoundsAccessChain::InstInBoundsAccessChain(uint32 resultTypeId, uint32 baseId, uint32 indexCount, uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpInBoundsAccessChain, 4, "OpInBoundsAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstFunction::InstFunction(uint32 resultTypeId, uint32 functionControl, uint32 functionTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunction, 5, "OpFunction", true), resultTypeId(resultTypeId), functionControl(functionControl), functionTypeId(functionTypeId) { }

InstFunctionParameter::InstFunctionParameter(uint32 resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunctionParameter, 3, "OpFunctionParameter", true), resultTypeId(resultTypeId) { }

InstFunctionEnd::InstFunctionEnd() : InstBase(THC_SPIRV_OPCODE_OpFunctionEnd, 1, "OpFunctionEnd") { }

InstFunctionCall::InstFunctionCall(uint32 resultTypeId, uint32 functionId, uint32 argumentCount, uint32* arguments) : InstBase(THC_SPIRV_OPCODE_OpFunctionCall, 4, "OpFunctionCall", true), resultTypeId(resultTypeId), functionId(functionId), argumentCount(argumentCount) { memcpy(this->argument, arguments, argumentCount << 2); }

InstConvertFToU::InstConvertFToU(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToU, 4, "OpConvertFToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertFToS::InstConvertFToS(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToS, 4, "OpConvertFToS", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertSToF::InstConvertSToF(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertSToF, 4, "OpConvertSToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToF::InstConvertUToF(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToF, 4, "OpConvertUToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstUConvert::InstUConvert(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpUConvert, 4, "OpUConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstSConvert::InstSConvert(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpSConvert, 4, "OpSConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstFConvert::InstFConvert(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpFConvert, 4, "OpFConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertPtrToU::InstConvertPtrToU(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertPtrToU, 4, "OpConvertPtrToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToPtr::InstConvertUToPtr(uint32 resultTypeId, uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToPtr, 4, "OpConvertUToPtr", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstVectorExtractDynamic::InstVectorExtractDynamic(uint32 resultTypeId, uint32 vectorId, uint32 indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorExtractDynamic, 5, "OpVectorExtractDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), indexId(indexId) { }

InstVectorInsertDynamic::InstVectorInsertDynamic(uint32 resultTypeId, uint32 vectorId, uint32 componentId, uint32 indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorInsertDynamic, 6, "OpVectorInsertDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), componentId(componentId), indexId(indexId) { }

InstVectorShuffle::InstVectorShuffle(uint32 resultTypeId, uint32 vector1Id, uint32 vector2Id, uint32 componentCount, uint32* components) : InstBase(THC_SPIRV_OPCODE_OpVectorShuffle, 5, "OpVectorShuffle", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id), componentCount(componentCount) { memcpy(this->component, components, componentCount << 2); }

InstCompositeConstruct::InstCompositeConstruct(uint32 resultTypeId, uint32 constituentCount, uint32* constituents) : InstBase(THC_SPIRV_OPCODE_OpCompositeConstruct, 3, "OpCompositeConstruct", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(this->constituent, constituents, constituentCount << 2); }

InstCompositeExtract::InstCompositeExtract(uint32 resultTypeId, uint32 compositeId, uint32 indexCount, uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeExtract, 4, "OpCompositeExtract", true), resultTypeId(resultTypeId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCompositeInsert::InstCompositeInsert(uint32 resultTypeId, uint32 objectId, uint32 compositeId, uint32 indexCount, uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeInsert, 5, "OpCompositeInsert", true), resultTypeId(resultTypeId), objectId(objectId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCopyObject::InstCopyObject(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpCopyObject, 4, "OpCopyObject", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstTranspose::InstTranspose(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpTranspose, 4, "OpTranspose", true), resultTypeId(resultTypeId), matrixId(matrixId) { }

InstSNegate::InstSNegate(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpSNegate, 4, "OpSNegate", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstFNegate::InstFNegate(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpFNegate, 4, "OpFNegate", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIAdd::InstIAdd(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAdd, 5, "OpIAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFAdd::InstFAdd(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFAdd, 5, "OpFAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISub::InstISub(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISub, 5, "OpISub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFSub::InstFSub(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFSub, 5, "OpFSub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstIMul::InstIMul(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIMul, 5, "OpIMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMul::InstFMul(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMul, 5, "OpFMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUDiv::InstUDiv(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUDiv, 5, "OpUDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSDiv::InstSDiv(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSDiv, 5, "OpSDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFDiv::InstFDiv(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFDiv, 5, "OpFDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMod::InstUMod(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMod, 5, "OpUMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSRem::InstSRem(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSRem, 5, "OpSRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMod::InstSMod(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMod, 5, "OpSMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFRem::InstFRem(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFRem, 5, "OpFRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMod::InstFMod(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMod, 5, "OpFMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstVectorTimesScalar::InstVectorTimesScalar(uint32 resultTypeId, uint32 vectorId, uint32 scalarId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesScalar, 5, "OpVectorTimesScalar", true), resultTypeId(resultTypeId), vectorId(vectorId), scalarId(scalarId) {}

InstMatrixTimesScalar::InstMatrixTimesScalar(uint32 resultTypeId, uint32 matrixUd, uint32 scalarId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesScalar, 5, "OpMatrixTimesScalar", true), resultTypeId(resultTypeId), matrixId(matrixId), scalarId(scalarId) {}

InstVectorTimesMatrix::InstVectorTimesMatrix(uint32 resultTypeId, uint32 vectorId, uint32 matrixId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesMatrix, 5, "OpVectorTimesMatrix", true), resultTypeId(resultTypeId), vectorId(vectorId), matrixId(matrixId) {}

InstMatrixTimesVector::InstMatrixTimesVector(uint32 resultTypeId, uint32 matrixId, uint32 vectorId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesVector, 5, "OpMatrixTimesVector", true), resultTypeId(resultTypeId), matrixId(matrixId), vectorId(vectorId) {}

InstMatrixTimesMatrix::InstMatrixTimesMatrix(uint32 resultTypeId, uint32 matrix1Id, uint32 matrix2Id) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesMatrix, 5, "OpMatrixTimesMatrix", true), resultTypeId(resultTypeId), matrix1Id(matrix1Id), matrix2Id(matrix2Id) {}

InstOuterProduct::InstOuterProduct(uint32 resultTypeId, uint32 vector1Id, uint32 vector2Id) : InstBase(THC_SPIRV_OPCODE_OpOuterProduct, 5, "OpOuterProduct", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstDot::InstDot(uint32 resultTypeId, uint32 vector1Id, uint32 vector2Id) : InstBase(THC_SPIRV_OPCODE_OpDot, 5, "OpDot", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstIAddCarry::InstIAddCarry(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAddCarry, 5, "OpIAddCarry", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISubBorrow::InstISubBorrow(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISubBorrow, 5, "OpISubBorrow", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMulExtended::InstUMulExtended(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMulExtended, 5, "OpUMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMulExtended::InstSMulExtended(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMulExtended, 5, "OpSMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstShiftRightLogical::InstShiftRightLogical(uint32 resultTypeId, uint32 baseId, uint32 shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightLogical, 5, "OpShiftRightLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftRightArithmetic::InstShiftRightArithmetic(uint32 resultTypeId, uint32 baseId, uint32 shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightArithmetic, 5, "OpShiftRightArithmetic", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftLeftLogical::InstShiftLeftLogical(uint32 resultTypeId, uint32 baseId, uint32 shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftLeftLogical, 5, "OpShiftLeftLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstBitwiseOr::InstBitwiseOr(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseOr, 5, "OpBitwiseOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseXor::InstBitwiseXor(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseXor, 5, "OpBitwiseXor", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseAnd::InstBitwiseAnd(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseAnd, 5, "OpBitwiseAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstNot::InstNot(uint32 resultTypeId, uint32 operand1Id) : InstBase(THC_SPIRV_OPCODE_OpNot, 4, "OpNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstBitReverse::InstBitReverse(uint32 resultTypeId, uint32 operand1Id) : InstBase(THC_SPIRV_OPCODE_OpBitReverse, 5, "OpBitReverse", true), resultTypeId(resultTypeId), operand1Id(operand1Id) {}

InstAny::InstAny(uint32 resultTypeId, uint32 vectorId) : InstBase(THC_SPIRV_OPCODE_OpAny, 4, "OpAny", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstAll::InstAll(uint32 resultTypeId, uint32 vectorId) : InstBase(THC_SPIRV_OPCODE_OpAll, 4, "OpAll", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstIsNan::InstIsNan(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpIsNan, 4, "OpIsNan", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIsInf::InstIsInf(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpIsInf, 4, "OpIsInf", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstLogicalEqual::InstLogicalEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalEqual, 5, "OpLogicalEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNotEqual::InstLogicalNotEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalNotEqual, 5, "OpLogicalNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalOr::InstLogicalOr(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalOr, 5, "OpLogicalOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalAnd::InstLogicalAnd(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalAnd, 5, "OpLogicalAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNot::InstLogicalNot(uint32 resultTypeId, uint32 operandId) : InstBase(THC_SPIRV_OPCODE_OpLogicalNot, 4, "OpLogicalNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstSelect::InstSelect(uint32 resultTypeId, uint32 conditionId, uint32 object1Id, uint32 object2Id) : InstBase(THC_SPIRV_OPCODE_OpSelect, 6, "OpSelect", true), resultTypeId(resultTypeId), conditionId(conditionId), object1Id(object1Id), object2Id(object2Id) {}

InstIEqual::InstIEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIEqual, 5, "OpIEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstINotEqual::InstINotEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpINotEqual, 5, "OpINotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThan::InstUGreaterThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThan, 5, "OpUGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThan::InstSGreaterThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThan, 5, "OpSGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThanEqual::InstUGreaterThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThanEqual, 5, "OpUGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThanEqual::InstSGreaterThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThanEqual, 5, "OpSGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThan::InstULessThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThan, 5, "OpULessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThan::InstSLessThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThan, 5, "OpSLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThanEqual::InstULessThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThanEqual, 5, "OpULessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThanEqual::InstSLessThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThanEqual, 5, "OpSLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdEqual::InstFOrdEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdEqual, 5, "OpFOrdEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordEqual::InstFUnordEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordEqual, 5, "OpFUnordEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdNotEqual::InstFOrdNotEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdNotEqual, 5, "OpFOrdNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordNotEqual::InstFUnordNotEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordNotEqual, 5, "OpFUnordNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThan::InstFOrdLessThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThan, 5, "OpFOrdLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThan::InstFUnordLessThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThan, 5, "OpFUnordLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThan::InstFOrdGreaterThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThan, 5, "OpFOrdGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThan::InstFUnordGreaterThan(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThan, 5, "OpFUnordGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThanEqual::InstFOrdLessThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThanEqual, 5, "OpFOrdLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThanEqual::InstFUnordLessThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThanEqual, 5, "OpFUnordLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThanEqual::InstFOrdGreaterThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThanEqual, 5, "OpFOrdGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThanEqual::InstFUnordGreaterThanEqual(uint32 resultTypeId, uint32 operand1Id, uint32 operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThanEqual, 5, "OpFUnordGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstPhi::InstPhi(uint32 resultTypeId, uint32 pairCount, PhiPair* pairs) : InstBase(THC_SPIRV_OPCODE_OpPhi, 3, "OpPhi", true), resultTypeId(resultTypeId), pairCount(pairCount) { memcpy(this->pairs, pairs, pairCount << 3); }

InstLoopMerge::InstLoopMerge(uint32 mergeBlockId, uint32 continueTargetId, uint32 loopControl) : InstBase(THC_SPIRV_OPCODE_OpLoopMerge, 4, "OpLoopMerge"), mergeBlockId(mergeBlockId), continueTargetId(continueTargetId), loopControl(loopControl) {}

InstSelectionMerge::InstSelectionMerge(uint32 mergeBlockId, uint32 selectionControl) : InstBase(THC_SPIRV_OPCODE_OpSelectionMerge, 3, "OpSelectionMerge"), mergeBlockId(mergeBlockId), selectionControl(selectionControl) {}

InstLabel::InstLabel() : InstBase(THC_SPIRV_OPCODE_OpLabel, 2, "OpLabel", true) {}

InstBranch::InstBranch(uint32 targetLabelId) : InstBase(THC_SPIRV_OPCODE_OpBranch, 2, "OpBranch"), targetLabelId(targetLabelId) {}

InstBranchConditional::InstBranchConditional(uint32 conditionId, uint32 trueLabelId, uint32 falseLabelId, uint32 trueWeight, uint32 falseWeight) : InstBase(THC_SPIRV_OPCODE_OpBranchConditional, 6, "OpBranchConditional"), conditionId(conditionId), trueLabelId(trueLabelId), falseLabelId(falseLabelId), trueWeight(trueWeight), falseWeight(falseWeight) {}

InstSwitch::InstSwitch(uint32 selectorId, uint32 defaultId) : InstBase(THC_SPIRV_OPCODE_OpSwitch, 3, "OpSwitch"), selectorId(selectorId), defaultId(defaultId), pairCount(pairCount) { memcpy(this->pairs, pairs, pairCount << 3); }

InstKill::InstKill() : InstBase(THC_SPIRV_OPCODE_OpKill, 1, "OpKill") {}

InstReturn::InstReturn() : InstBase(THC_SPIRV_OPCODE_OpReturn, 1, "OpReturn") {}

InstReturnValue::InstReturnValue(uint32 valueId) : InstBase(THC_SPIRV_OPCODE_OpReturnValue, 2, "OpReturnValue"), valueId(valueId) {}

}
}
}