/*
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

InstUndef::InstUndef(compiler::ID* resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpUndef, 3, "OpUndef", true), resultTypeId(resultTypeId) { }

InstSizeOf::InstSizeOf(compiler::ID* resultTypeId, compiler::ID* pointerId) : InstBase(THC_SPIRV_OPCODE_OpSizeOf, 4, "OpSizeOf", true), resultTypeId(resultTypeId), pointerId(pointerId) { }

InstSourceContinued::InstSourceContinued(const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSourceContinued, 1, "OpSourceContinued") { Utils::CopyString(this->source, source); }

InstSource::InstSource(uint32 sourceLanguage, uint32 version, compiler::ID* fileNameId, const char* const source) : InstBase(THC_SPIRV_OPCODE_OpSource, 4, "OpSource"), sourceLanguage(sourceLanguage), version(version), fileNameId(fileNameId) { Utils::CopyString(this->source, source); }

InstSourceExtension::InstSourceExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpSourceExtension, 1, "OpSourceExtension") { Utils::CopyString(this->extension, extension); }

InstName::InstName(compiler::ID* targetId, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpName, 2, "OpName"), targetId(targetId) { Utils::CopyString(this->name, name); }

InstMemberName::InstMemberName(compiler::ID* typeId, uint32 member, const char* const name) : InstBase(THC_SPIRV_OPCODE_OpMemberName, 3, "OpMemberName"), typeId(typeId), member(member) { Utils::CopyString(this->name, name); }

InstString::InstString(const char* const string) : InstBase(THC_SPIRV_OPCODE_OpString, 2, "OpString", true) { Utils::CopyString(this->string, string); }

InstLine::InstLine(compiler::ID* fileNameId, uint32 line, uint32 column) : InstBase(THC_SPIRV_OPCODE_OpLine, 4, "OpLine"), fileNameId(fileNameId), line(line), column(column) {}

InstNoLine::InstNoLine() : InstBase(THC_SPIRV_OPCODE_OpNoLine, 1, "OpNoLine") { }

InstDecorate::InstDecorate(compiler::ID* targetId, uint32 decoration, const uint32* literals, uint32 numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpDecorate, 3, "OpDecorate"), targetId(targetId), decoration(decoration), numDecorationLiterals(numDecorationLiterals) { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstMemberDecorate::InstMemberDecorate(compiler::ID* structId, uint32 member, uint32 decoration, const uint32* literals, uint32 numDecorationLiterals) : InstBase(THC_SPIRV_OPCODE_OpMemberDecorate, 4, "OpMemberDecorate"), structId(structId), member(member), decoration(decoration), numDecorationLiterals(numDecorationLiterals) { memcpy(this->literals, literals, numDecorationLiterals << 2); }

InstDecorationGroup::InstDecorationGroup() : InstBase(THC_SPIRV_OPCODE_OpDecorateGroup, 2, "OpDecorationGroup", true) { }

InstGroupDecorate::InstGroupDecorate(compiler::ID* groupId, compiler::ID** targetIds, uint32 numTargets) : InstBase(THC_SPIRV_OPCODE_OpGroupDecorate, 1, "OpGroupDecorate"), groupId(groupId), numTargets(numTargets) { memcpy(targetId, targetIds, numTargets * sizeof(void*)); }

InstExtension::InstExtension(const char* const extension) : InstBase(THC_SPIRV_OPCODE_OpExtension, 1, "OpExtension") { Utils::CopyString(this->extension, extension); }

InstExtInstImport::InstExtInstImport(const char* const extensionSet) : InstBase(THC_SPIRV_OPCODE_OpExtInstImport, 2, "OpExtInstImport", true) { Utils::CopyString(this->extensionSet, extensionSet); }

InstExtInst::InstExtInst(ID* resultType, ID* set, uint32 opCode, uint32 numOperands, ID** operands) : InstBase(THC_SPIRV_OPCODE_OpExtInst, 5 + numOperands, "OpExtInst", true), resultType(resultType), set(set), opCode(opCode), numOperands(numOperands) { memcpy(this->operands, operands, sizeof(void*) * numOperands); }

InstMemoryModel::InstMemoryModel(uint32 addressingModel, uint32 memoryModel) : InstBase(THC_SPIRV_OPCODE_OpMemoryModel, 3, "OpMemoryModel"), addressingModel(addressingModel), memoryModel(memoryModel) {}

InstEntryPoint::InstEntryPoint(uint32 executionModel, compiler::ID* entryPointId, const char* const entryPointName, uint32 inoutVariableCount, compiler::ID** inoutVariableIds) : InstBase(THC_SPIRV_OPCODE_OpEntryPoint, 3, "OpEntryPoint"), executionModel(executionModel), entryPointId(entryPointId), inoutVariableCount(inoutVariableCount){ Utils::CopyString(this->entryPointName, entryPointName); memcpy(inoutVariableId, inoutVariableIds, inoutVariableCount * sizeof(void*)); }

InstExecutionMode::InstExecutionMode(compiler::ID* entryPointId, uint32 mode, uint32 extraOperandCount, const uint32* extraOperands) : InstBase(THC_SPIRV_OPCODE_OpExecutionMode, 3, "OpExecutionMode"), entryPointId(entryPointId), mode(mode), extraOperandCount(extraOperandCount) { memcpy(this->extraOperand, extraOperand, extraOperandCount << 2); }

InstCapability::InstCapability(uint32 capability) : InstBase(THC_SPIRV_OPCODE_OpCapability, 2, "OpCapability"), capability(capability) { }

InstConstantTrue::InstConstantTrue(compiler::ID* resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpConstantTrue, 3, "OpConstantTrue", true), resultTypeId(resultTypeId) {}

InstConstantFalse::InstConstantFalse(compiler::ID* resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpConstantFalse, 3, "OpConstantFalse", true), resultTypeId(resultTypeId) {}

InstConstant::InstConstant(compiler::ID* resultTypeId, uint32 valueCount, void* values) : InstBase(THC_SPIRV_OPCODE_OpConstant, 3, "OpConstant", true), resultTypeId(resultTypeId), valueCount(valueCount), values(new uint32[valueCount]) { memcpy(this->values, values, valueCount << 2); }

InstConstant::InstConstant(compiler::ID* resultTypeId, uint32 value) : InstConstant(resultTypeId, 1, &value) {}

InstConstant::InstConstant(compiler::ID* resultTypeId, float32 value) : InstConstant(resultTypeId, 1, &value) {}

InstConstantComposite::InstConstantComposite(compiler::ID* resultTypeId, uint32 constituentCount, compiler::ID** constituentIds) : InstBase(THC_SPIRV_OPCODE_OpConstantComposite, 3, "OpConstantComposite", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(constituentId, constituentIds, constituentCount * sizeof(void*)); }

InstVariable::InstVariable(compiler::ID* resultTypeId, uint32 storageClass, uint32 initializer) : InstBase(THC_SPIRV_OPCODE_OpVariable, 4, "OpVariable", true), resultTypeId(resultTypeId), storageClass(storageClass), initializer(initializer) { }

InstLoad::InstLoad(compiler::ID* resultTypeId, compiler::ID* pointerId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpLoad, 4, "OpLoad", true), resultTypeId(resultTypeId), pointerId(pointerId), memoryAccess(memoryAccess) { }

InstStore::InstStore(compiler::ID* pointerId, compiler::ID* objectId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpStore, 3, "OpStore"), pointerId(pointerId), objectId(objectId), memoryAccess(memoryAccess) { }

InstCopyMemory::InstCopyMemory(compiler::ID* targetId, compiler::ID* sourceId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemory, 3, "OpCopyMemory"), targetId(targetId), sourceId(sourceId), memoryAccess(memoryAccess) { }

InstCopyMemorySized::InstCopyMemorySized(compiler::ID* targetId, compiler::ID* sourceId, compiler::ID* sizeId, uint32 memoryAccess) : InstBase(THC_SPIRV_OPCODE_OpCopyMemorySized, 4, "OpCopyMemorySized"), targetId(targetId), sourceId(sourceId), sizeId(sizeId), memoryAccess(memoryAccess) { }

InstAccessChain::InstAccessChain(compiler::ID* resultTypeId, compiler::ID* baseId, uint32 indexCount, compiler::ID** indexIds) : InstBase(THC_SPIRV_OPCODE_OpAccessChain, 4, "OpAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(indexId, indexIds, indexCount * sizeof(void*)); }

InstInBoundsAccessChain::InstInBoundsAccessChain(compiler::ID* resultTypeId, compiler::ID* baseId, uint32 indexCount, compiler::ID** indexIds) : InstBase(THC_SPIRV_OPCODE_OpInBoundsAccessChain, 4, "OpInBoundsAccessChain", true), resultTypeId(resultTypeId), baseId(baseId), indexCount(indexCount) { memcpy(indexId, indexIds, indexCount * sizeof(void*)); }

InstFunction::InstFunction(compiler::ID* resultTypeId, uint32 functionControl, compiler::ID* functionTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunction, 5, "OpFunction", true), resultTypeId(resultTypeId), functionControl(functionControl), functionTypeId(functionTypeId) { }

InstFunctionParameter::InstFunctionParameter(compiler::ID* resultTypeId) : InstBase(THC_SPIRV_OPCODE_OpFunctionParameter, 3, "OpFunctionParameter", true), resultTypeId(resultTypeId) { }

InstFunctionEnd::InstFunctionEnd() : InstBase(THC_SPIRV_OPCODE_OpFunctionEnd, 1, "OpFunctionEnd") { }

InstFunctionCall::InstFunctionCall(compiler::ID* resultTypeId, compiler::ID* functionId, uint32 argumentCount, compiler::ID** argumentIds) : InstBase(THC_SPIRV_OPCODE_OpFunctionCall, 4, "OpFunctionCall", true), resultTypeId(resultTypeId), functionId(functionId), argumentCount(argumentCount) { memcpy(argumentId, argumentIds, argumentCount * sizeof(void*)); }

InstImageSampledImplicitLod::InstImageSampledImplicitLod(ID* resultType, ID* image, ID* coordinate, uint32 imageOperand, uint32 numOperands, ID** operands) : InstBase(THC_SPIRV_OPCODE_OpImageSampleImplicitLod, 5 + (imageOperand ? 1 + numOperands : 0), "OpImageSampleImplicitLod", true), resultType(resultType), image(image), coordinate(coordinate), imageOperand(imageOperand), numOperands(numOperands) { this->operands = new ID * [numOperands]; memcpy(this->operands, operands, numOperands * sizeof(void*)); }

InstConvertFToU::InstConvertFToU(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToU, 4, "OpConvertFToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertFToS::InstConvertFToS(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertFToS, 4, "OpConvertFToS", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertSToF::InstConvertSToF(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertSToF, 4, "OpConvertSToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToF::InstConvertUToF(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToF, 4, "OpConvertUToF", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstUConvert::InstUConvert(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpUConvert, 4, "OpUConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstSConvert::InstSConvert(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpSConvert, 4, "OpSConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstFConvert::InstFConvert(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpFConvert, 4, "OpFConvert", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertPtrToU::InstConvertPtrToU(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertPtrToU, 4, "OpConvertPtrToU", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstConvertUToPtr::InstConvertUToPtr(compiler::ID* resultTypeId, compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpConvertUToPtr, 4, "OpConvertUToPtr", true), resultTypeId(resultTypeId), valueId(valueId) { }

InstVectorExtractDynamic::InstVectorExtractDynamic(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorExtractDynamic, 5, "OpVectorExtractDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), indexId(indexId) { }

InstVectorInsertDynamic::InstVectorInsertDynamic(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* componentId, compiler::ID* indexId) : InstBase(THC_SPIRV_OPCODE_OpVectorInsertDynamic, 6, "OpVectorInsertDynamic", true), resultTypeId(resultTypeId), vectorId(vectorId), componentId(componentId), indexId(indexId) { }

InstVectorShuffle::InstVectorShuffle(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id, uint32 componentCount, const uint32* components) : InstBase(THC_SPIRV_OPCODE_OpVectorShuffle, 5, "OpVectorShuffle", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id), componentCount(componentCount) { memcpy(this->component, components, componentCount << 2); }

InstCompositeConstruct::InstCompositeConstruct(compiler::ID* resultTypeId, uint32 constituentCount, compiler::ID** constituentIds) : InstBase(THC_SPIRV_OPCODE_OpCompositeConstruct, 3, "OpCompositeConstruct", true), resultTypeId(resultTypeId), constituentCount(constituentCount) { memcpy(constituentId, constituentIds, constituentCount * sizeof(void*)); }

InstCompositeExtract::InstCompositeExtract(compiler::ID* resultTypeId, compiler::ID* compositeId, uint32 indexCount, const uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeExtract, 4, "OpCompositeExtract", true), resultTypeId(resultTypeId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCompositeInsert::InstCompositeInsert(compiler::ID* resultTypeId, compiler::ID* objectId, compiler::ID* compositeId, uint32 indexCount, const uint32* indices) : InstBase(THC_SPIRV_OPCODE_OpCompositeInsert, 5, "OpCompositeInsert", true), resultTypeId(resultTypeId), objectId(objectId), compositeId(compositeId), indexCount(indexCount) { memcpy(this->index, indices, indexCount << 2); }

InstCopyObject::InstCopyObject(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpCopyObject, 4, "OpCopyObject", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstTranspose::InstTranspose(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpTranspose, 4, "OpTranspose", true), resultTypeId(resultTypeId), matrixId(matrixId) { }

InstSNegate::InstSNegate(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpSNegate, 4, "OpSNegate", true), resultTypeId(resultTypeId), operandId(operandId) { }

InstFNegate::InstFNegate(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpFNegate, 4, "OpFNegate", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIAdd::InstIAdd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAdd, 5, "OpIAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFAdd::InstFAdd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFAdd, 5, "OpFAdd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISub::InstISub(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISub, 5, "OpISub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFSub::InstFSub(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFSub, 5, "OpFSub", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstIMul::InstIMul(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIMul, 5, "OpIMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMul::InstFMul(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMul, 5, "OpFMul", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUDiv::InstUDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUDiv, 5, "OpUDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSDiv::InstSDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSDiv, 5, "OpSDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFDiv::InstFDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFDiv, 5, "OpFDiv", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMod::InstUMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMod, 5, "OpUMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSRem::InstSRem(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSRem, 5, "OpSRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMod::InstSMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMod, 5, "OpSMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFRem::InstFRem(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFRem, 5, "OpFRem", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFMod::InstFMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFMod, 5, "OpFMod", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstVectorTimesScalar::InstVectorTimesScalar(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* scalarId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesScalar, 5, "OpVectorTimesScalar", true), resultTypeId(resultTypeId), vectorId(vectorId), scalarId(scalarId) {}

InstMatrixTimesScalar::InstMatrixTimesScalar(compiler::ID* resultTypeId, compiler::ID* matrixId, compiler::ID* scalarId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesScalar, 5, "OpMatrixTimesScalar", true), resultTypeId(resultTypeId), matrixId(matrixId), scalarId(scalarId) {}

InstVectorTimesMatrix::InstVectorTimesMatrix(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* matrixId) : InstBase(THC_SPIRV_OPCODE_OpVectorTimesMatrix, 5, "OpVectorTimesMatrix", true), resultTypeId(resultTypeId), vectorId(vectorId), matrixId(matrixId) {}

InstMatrixTimesVector::InstMatrixTimesVector(compiler::ID* resultTypeId, compiler::ID* matrixId, compiler::ID* vectorId) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesVector, 5, "OpMatrixTimesVector", true), resultTypeId(resultTypeId), matrixId(matrixId), vectorId(vectorId) {}

InstMatrixTimesMatrix::InstMatrixTimesMatrix(compiler::ID* resultTypeId, compiler::ID* matrix1Id, compiler::ID* matrix2Id) : InstBase(THC_SPIRV_OPCODE_OpMatrixTimesMatrix, 5, "OpMatrixTimesMatrix", true), resultTypeId(resultTypeId), matrix1Id(matrix1Id), matrix2Id(matrix2Id) {}

InstOuterProduct::InstOuterProduct(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id) : InstBase(THC_SPIRV_OPCODE_OpOuterProduct, 5, "OpOuterProduct", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstDot::InstDot(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id) : InstBase(THC_SPIRV_OPCODE_OpDot, 5, "OpDot", true), resultTypeId(resultTypeId), vector1Id(vector1Id), vector2Id(vector2Id) {}

InstIAddCarry::InstIAddCarry(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIAddCarry, 5, "OpIAddCarry", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstISubBorrow::InstISubBorrow(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpISubBorrow, 5, "OpISubBorrow", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUMulExtended::InstUMulExtended(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUMulExtended, 5, "OpUMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSMulExtended::InstSMulExtended(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSMulExtended, 5, "OpSMulExtended", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstShiftRightLogical::InstShiftRightLogical(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightLogical, 5, "OpShiftRightLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftRightArithmetic::InstShiftRightArithmetic(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftRightArithmetic, 5, "OpShiftRightArithmetic", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstShiftLeftLogical::InstShiftLeftLogical(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId) : InstBase(THC_SPIRV_OPCODE_OpShiftLeftLogical, 5, "OpShiftLeftLogical", true), resultTypeId(resultTypeId), baseId(baseId), shiftId(shiftId) {}

InstBitwiseOr::InstBitwiseOr(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseOr, 5, "OpBitwiseOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseXor::InstBitwiseXor(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseXor, 5, "OpBitwiseXor", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstBitwiseAnd::InstBitwiseAnd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpBitwiseAnd, 5, "OpBitwiseAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstNot::InstNot(compiler::ID* resultTypeId, compiler::ID* operand1Id) : InstBase(THC_SPIRV_OPCODE_OpNot, 4, "OpNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstBitReverse::InstBitReverse(compiler::ID* resultTypeId, compiler::ID* operand1Id) : InstBase(THC_SPIRV_OPCODE_OpBitReverse, 5, "OpBitReverse", true), resultTypeId(resultTypeId), operand1Id(operand1Id) {}

InstAny::InstAny(compiler::ID* resultTypeId, compiler::ID* vectorId) : InstBase(THC_SPIRV_OPCODE_OpAny, 4, "OpAny", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstAll::InstAll(compiler::ID* resultTypeId, compiler::ID* vectorId) : InstBase(THC_SPIRV_OPCODE_OpAll, 4, "OpAll", true), resultTypeId(resultTypeId), vectorId(vectorId) {}

InstIsNan::InstIsNan(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpIsNan, 4, "OpIsNan", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstIsInf::InstIsInf(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpIsInf, 4, "OpIsInf", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstLogicalEqual::InstLogicalEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalEqual, 5, "OpLogicalEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNotEqual::InstLogicalNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalNotEqual, 5, "OpLogicalNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalOr::InstLogicalOr(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalOr, 5, "OpLogicalOr", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalAnd::InstLogicalAnd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpLogicalAnd, 5, "OpLogicalAnd", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstLogicalNot::InstLogicalNot(compiler::ID* resultTypeId, compiler::ID* operandId) : InstBase(THC_SPIRV_OPCODE_OpLogicalNot, 4, "OpLogicalNot", true), resultTypeId(resultTypeId), operandId(operandId) {}

InstSelect::InstSelect(compiler::ID* resultTypeId, compiler::ID* conditionId, compiler::ID* object1Id, compiler::ID* object2Id) : InstBase(THC_SPIRV_OPCODE_OpSelect, 6, "OpSelect", true), resultTypeId(resultTypeId), conditionId(conditionId), object1Id(object1Id), object2Id(object2Id) {}

InstIEqual::InstIEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpIEqual, 5, "OpIEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstINotEqual::InstINotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpINotEqual, 5, "OpINotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThan::InstUGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThan, 5, "OpUGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThan::InstSGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThan, 5, "OpSGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstUGreaterThanEqual::InstUGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpUGreaterThanEqual, 5, "OpUGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSGreaterThanEqual::InstSGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSGreaterThanEqual, 5, "OpSGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThan::InstULessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThan, 5, "OpULessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThan::InstSLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThan, 5, "OpSLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstULessThanEqual::InstULessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpULessThanEqual, 5, "OpULessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstSLessThanEqual::InstSLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpSLessThanEqual, 5, "OpSLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdEqual::InstFOrdEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdEqual, 5, "OpFOrdEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordEqual::InstFUnordEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordEqual, 5, "OpFUnordEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdNotEqual::InstFOrdNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdNotEqual, 5, "OpFOrdNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordNotEqual::InstFUnordNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordNotEqual, 5, "OpFUnordNotEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThan::InstFOrdLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThan, 5, "OpFOrdLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThan::InstFUnordLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThan, 5, "OpFUnordLessThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThan::InstFOrdGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThan, 5, "OpFOrdGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThan::InstFUnordGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThan, 5, "OpFUnordGreaterThan", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdLessThanEqual::InstFOrdLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdLessThanEqual, 5, "OpFOrdLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordLessThanEqual::InstFUnordLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordLessThanEqual, 5, "OpFUnordLessThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFOrdGreaterThanEqual::InstFOrdGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFOrdGreaterThanEqual, 5, "OpFOrdGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstFUnordGreaterThanEqual::InstFUnordGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id) : InstBase(THC_SPIRV_OPCODE_OpFUnordGreaterThanEqual, 5, "OpFUnordGreaterThanEqual", true), resultTypeId(resultTypeId), operand1Id(operand1Id), operand2Id(operand2Id) {}

InstPhi::InstPhi(compiler::ID* resultTypeId, uint32 pairCount, PhiPair* pairs) : InstBase(THC_SPIRV_OPCODE_OpPhi, 3, "OpPhi", true), resultTypeId(resultTypeId), pairCount(pairCount) { memcpy(this->pairs, pairs, pairCount << 3); }

InstLoopMerge::InstLoopMerge(compiler::ID* mergeBlockId, compiler::ID* continueTargetId, uint32 loopControl) : InstBase(THC_SPIRV_OPCODE_OpLoopMerge, 4, "OpLoopMerge"), mergeBlockId(mergeBlockId), continueTargetId(continueTargetId), loopControl(loopControl) {}

InstSelectionMerge::InstSelectionMerge(compiler::ID* mergeBlockId, uint32 selectionControl) : InstBase(THC_SPIRV_OPCODE_OpSelectionMerge, 3, "OpSelectionMerge"), mergeBlockId(mergeBlockId), selectionControl(selectionControl) {}

InstLabel::InstLabel() : InstBase(THC_SPIRV_OPCODE_OpLabel, 2, "OpLabel", true) {}

InstBranch::InstBranch(ID* targetLabelId) : InstBase(THC_SPIRV_OPCODE_OpBranch, 2, "OpBranch"), targetLabelId(targetLabelId) {}

InstBranchConditional::InstBranchConditional(compiler::ID* conditionId, compiler::ID* trueLabelId, compiler::ID* falseLabelId, uint32 trueWeight, uint32 falseWeight) : InstBase(THC_SPIRV_OPCODE_OpBranchConditional, 6, "OpBranchConditional"), conditionId(conditionId), trueLabelId(trueLabelId), falseLabelId(falseLabelId), trueWeight(trueWeight), falseWeight(falseWeight) {}

InstSwitch::InstSwitch(compiler::ID* selectorId, compiler::ID* defaultId, SwitchPair* pairs) : InstBase(THC_SPIRV_OPCODE_OpSwitch, 3, "OpSwitch"), selectorId(selectorId), defaultId(defaultId), pairCount(pairCount) { memcpy(pair, pairs, pairCount * sizeof(SwitchPair)); }

InstKill::InstKill() : InstBase(THC_SPIRV_OPCODE_OpKill, 1, "OpKill") {}

InstReturn::InstReturn() : InstBase(THC_SPIRV_OPCODE_OpReturn, 1, "OpReturn") {}

InstReturnValue::InstReturnValue(compiler::ID* valueId) : InstBase(THC_SPIRV_OPCODE_OpReturnValue, 2, "OpReturnValue"), valueId(valueId) {}

bool InstConstantTrue::operator==(const InstBase* const inst) const {
	return inst->opCode == THC_SPIRV_OPCODE_OpConstantTrue;
}

bool InstConstantFalse::operator==(const InstBase* const inst) const {
	return inst->opCode == THC_SPIRV_OPCODE_OpConstantFalse;
}

bool InstConstant::operator==(const InstBase* const inst) const {
	if (inst->opCode != this->opCode) {
		return false;
	}

	const InstConstant* c = (const InstConstant*)inst;

	if (resultTypeId == c->resultTypeId && valueCount == c->valueCount) {
		for (uint32 i = 0; i < valueCount; i++) {
			if (values[i] != c->values[i]) return false;
		}

		return true;
	}

	return false;
}

bool InstConstantComposite::operator==(const InstBase* const inst) const {
	if (inst->opCode != this->opCode) {
		return false;
	}

	const InstConstantComposite* c = (const InstConstantComposite*)inst;

	if (resultTypeId == c->resultTypeId && constituentCount == c->constituentCount) {
		for (uint32 i = 0; i < constituentCount; i++) {
			if (constituentId[i] != c->constituentId[i]) return false;
		}

		return true;
	}

	return false;
}

}
}
}