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
#include <core/spirvdefines.h>
#include <core/spirvlimits.h>


namespace thc {
namespace core {
namespace instruction {

class InstBase {
public:
	unsigned int id;
	unsigned int opCode;
	mutable unsigned int wordCount;
	char* literalName;

	InstBase(unsigned int opCode, unsigned int wordCount, const char* const literalName, bool resultId = false);
	virtual ~InstBase();

	virtual void GetInstWords(unsigned int* words) const;
};

#pragma region misc

class InstNop : public InstBase {
public:

	InstNop();
};

class InstUndef : public InstBase {
public:
	unsigned int resultTypeId;

	InstUndef(unsigned int resultTypeId);

	void GetInstWords(unsigned int* words) const override;
};

class InstSizeOf : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int pointerId;

	InstSizeOf(unsigned int resultTypeId, unsigned int pointerId);

	void GetInstWords(unsigned int* words) const override;
};

#pragma endregion

#pragma region debug

class InstSourceContinued : public InstBase {
public:
	char* source;

	InstSourceContinued(const char* const source);

	void GetInstWords(unsigned int* words) const override;
};

class InstSource : public InstBase {
public:
	unsigned int sourceLanguage;
	unsigned int version;
	unsigned int fileNameId;
	char* source;

	InstSource(unsigned int sourceLanguage, unsigned int version, unsigned int fileNameId, const char* const source);

	void GetInstWords(unsigned int* words) const override;
};

class InstSourceExtension : public InstBase {
public:
	char* extension;

	InstSourceExtension(const char* const extension);

	void GetInstWords(unsigned int* words) const override;
};

class InstName : public InstBase {
public:
	unsigned int targetId;
	char* name;

	InstName(unsigned int targetId, const char* const name);

	void GetInstWords(unsigned int* words) const override;
};


class InstMemberName : public InstBase {
public:
	unsigned int typeId;
	unsigned int member;
	char* name;

	InstMemberName(unsigned int typeId, unsigned int member, const char* const name);

	void GetInstWords(unsigned int* words) const override;
};


class InstString : public InstBase {
public:
	char* string;

	InstString(const char* const string);

	void GetInstWords(unsigned int* words) const override;
};

class InstLine : public InstBase {
public:
	unsigned int fileNameId;
	unsigned int line;
	unsigned int column;

	InstLine(unsigned int fileNameId, unsigned int line, unsigned int column);

	void GetInstWords(unsigned int* words) const override;
};

class InstNoLine : public InstBase {
public:

	InstNoLine();
};

//Todo: OpModuleProcessed

#pragma endregion

#pragma region annotations

class InstDecorate : public InstBase {
public:
	unsigned int targetId;
	unsigned int decoration;
	unsigned int numDecorationLiterals;
	unsigned int literals[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstDecorate(unsigned int targetId, unsigned int decoration, unsigned int* literals, unsigned int numDecorationLiterals);

	void GetInstWords(unsigned int* words) const override;
};

class InstMemberDecorate : public InstBase {
public:
	unsigned int structId;
	unsigned int decoration;
	unsigned int numDecorationLiterals;
	unsigned int literals[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstMemberDecorate(unsigned int structId, unsigned int decoration, unsigned int* literals, unsigned int numDecorationLiterals);

	void GetInstWords(unsigned int* words) const override;
};

class InstDecorationGroup : public InstBase {
public:
	InstDecorationGroup();

	void GetInstWords(unsigned int* words) const override;
};

class InstGroupDecorate : public InstBase {
public:
	unsigned int groupId;
	unsigned int numTargets;
	unsigned int targets[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstGroupDecorate(unsigned int groupId, unsigned int* targets, unsigned int numTargets);

	void GetInstWords(unsigned int* words) const override;
};

class InstGroupMemberDecorate : public InstBase {

};

class InstDecorateId : public InstBase {

};

#pragma endregion

#pragma region extension instructions

class InstExtension : public InstBase {
public:
	char* extension;

	InstExtension(const char* const extension);

	void GetInstWords(unsigned int* words) const;
};

class InstExtInstImport : public InstBase {
public:
	char* extensionSet;

	InstExtInstImport(const char* const extensionSet);

	void GetInstWords(unsigned int* words) const;
};

//TODO: OpExtInst will be 

#pragma endregion

#pragma region modes settings

class InstMemoryModel : public InstBase {
public:
	unsigned int addressingModel;
	unsigned int memoryModel;

	InstMemoryModel(unsigned int addressingModel, unsigned int memoryModel);

	void GetInstWords(unsigned int* words) const override;
};

class InstEntryPoint : public InstBase {
public:
	unsigned int executionModel;
	unsigned int entryPointId;
	char* entryPointName;
	unsigned int inoutVariableCount;
	unsigned int inoutVariables[THC_LIMIT_GLOBAL_VARIABLES];

	InstEntryPoint(unsigned int executionModel, unsigned int entryPointId, const char* const entryPointName, unsigned int inoutVariableCount, unsigned int* inoutVariables);

	void GetInstWords(unsigned int* words) const override;
};

class InstExecutionMode : public InstBase {
public:
	unsigned int entryPointId;
	unsigned int mode;
	unsigned int extraOperandCount;
	unsigned int extraOperand[256];

	InstExecutionMode(unsigned int entryPointId, unsigned int mode, unsigned int extraOperandCount, unsigned int* extraOperands);

	void GetInstWords(unsigned int* words) const;
};

class InstCapability : public InstBase {
public:
	unsigned int capability;

	InstCapability(unsigned int capability);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpExecutionModeId

#pragma endregion

#pragma region constant creation

//TODO: Boolean constants, OpConstatnTrue and OpConstantFalse


class InstConstant : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueCount;
	unsigned int* values;

	union {
		unsigned int u32;
		float f32;
	};

	InstConstant(unsigned int resultTypeId, unsigned int valueCount, void* values);
	InstConstant(unsigned int resultTypeId, unsigned int value);
	InstConstant(unsigned int resultTypeId, float value);

	void GetInstWords(unsigned int* words) const override;
};

class InstConstantComposite : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int constituentCount;
	unsigned int constituent[128];

	InstConstantComposite(unsigned int resultTypeId, unsigned int constituentCount, unsigned int* constituents);

	void GetInstWords(unsigned int* words) const override;
};

//TOOD: OpConstantNull, OpSpec*

#pragma endregion

#pragma region memory instructions

class InstVariable : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int storageClass;
	unsigned int initializer;

	InstVariable(unsigned int resultTypeId, unsigned int storageClass, unsigned int initializer);

	void GetInstWords(unsigned int* words) const override;
};

class InstLoad : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int pointerId;
	unsigned int memoryAccess;

	InstLoad(unsigned int resultTypeId, unsigned int pointerId, unsigned int memoryAccess);

	void GetInstWords(unsigned int* words) const override;
};

class InstStore : public InstBase {
public:
	unsigned int pointerId;
	unsigned int objectId;
	unsigned int memoryAccess;

	InstStore(unsigned int pointerId, unsigned int objectId, unsigned int memoryAccess);

	void GetInstWords(unsigned int* words) const override;
};


class InstCopyMemory : public InstBase {
public:
	unsigned int targetId;
	unsigned int sourceId;
	unsigned int memoryAccess;

	InstCopyMemory(unsigned int targetId, unsigned int sourceId, unsigned int memoryAccess);

	void GetInstWords(unsigned int* words) const override;
};

class InstCopyMemorySized : public InstBase {
public:
	unsigned int targetId;
	unsigned int sourceId;
	unsigned int sizeId;
	unsigned int memoryAccess;

	InstCopyMemorySized(unsigned int targetId, unsigned int sourceId, unsigned int sizeId, unsigned int memoryAccess);

	void GetInstWords(unsigned int* words) const override;
};

class InstAccessChain : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int baseId;
	unsigned int indexCount;
	unsigned int index[THC_LIMIT_INDEXES];

	InstAccessChain(unsigned int resultTypeId, unsigned int baseId, unsigned int indexCount, unsigned int* indices);

	void GetInstWords(unsigned int* words) const override;
};

class InstInBoundsAccessChain : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int baseId;
	unsigned int indexCount;
	unsigned int index[THC_LIMIT_INDEXES];

	InstInBoundsAccessChain(unsigned int resultTypeId, unsigned int baseId, unsigned int indexCount, unsigned int* indices);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpPtrAccessChain, OpInBoundsPtrAccessChain, OpArrayLength

#pragma endregion

#pragma region function instructions

class InstFunction : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int functionControl;
	unsigned int functionTypeId;

	InstFunction(unsigned int resultTypeId, unsigned int functionControl, unsigned int functionTypeId);

	void GetInstWords(unsigned int* words) const override;
}; 

class InstFunctionParameter : public InstBase {
public:
	unsigned int resultTypeId;

	InstFunctionParameter(unsigned int resultTypeId);

	void GetInstWords(unsigned int* words) const override;
};


class InstFunctionEnd : public InstBase {
public:

	InstFunctionEnd();
};

class InstFunctionCall : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int functionId;
	unsigned int argumentCount;
	unsigned int argument[THC_LIMIT_OPFUNCTIONCALL_ARGUMENTS];

	InstFunctionCall(unsigned int resultTypeId, unsigned int functionId, unsigned int argumentCount, unsigned int* arguments);

	void GetInstWords(unsigned int* words) const override;
};

#pragma endregion

#pragma region image instructions

//TODO: image instructions

#pragma endregion

#pragma region conversion instructions

class InstConvertFToU : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertFToU(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstConvertFToS : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertFToS(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstConvertSToF : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertSToF(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstConvertUToF : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertUToF(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstUConvert : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstUConvert(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstSConvert : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstSConvert(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstFConvert : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstFConvert(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpQuantizeToF16

class InstConvertPtrToU : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertPtrToU(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

class InstConvertUToPtr : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int valueId;

	InstConvertUToPtr(unsigned int resultTypeId, unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpBitcast

#pragma endregion

#pragma region composite instructions

class InstVectorExtractDynamic : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;
	unsigned int indexId;

	InstVectorExtractDynamic(unsigned int resultTypeId, unsigned int vectorId, unsigned int indexId);

	void GetInstWords(unsigned int* words) const override;
};

class InstVectorInsertDynamic : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;
	unsigned int componentId;
	unsigned int indexId;

	InstVectorInsertDynamic(unsigned int resultTypeId, unsigned int vectorId, unsigned int componentId, unsigned int indexId);

	void GetInstWords(unsigned int* words) const override;
};

class InstVectorShuffle : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vector1Id;
	unsigned int vector2Id;
	unsigned int componentCount;
	unsigned int component[4];

	InstVectorShuffle(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id, unsigned int componentCount, unsigned int* components);

	void GetInstWords(unsigned int* words) const override;
};

class InstCompositeConstruct: public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int constituentCount;
	unsigned int constituent[512];

	InstCompositeConstruct(unsigned int resultTypeId, unsigned int constituentCount, unsigned int* constituents);

	void GetInstWords(unsigned int* words) const override;
};

class InstCompositeExtract : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int compositeId;
	unsigned int indexCount;
	unsigned int index[THC_LIMIT_STRUCT_NESTING_DEPTH];

	InstCompositeExtract(unsigned int resultTypeId, unsigned int compositeId, unsigned int indexCount, unsigned int* indices);

	void GetInstWords(unsigned int* words) const override;
};

class InstCompositeInsert : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int objectId;
	unsigned int compositeId;
	unsigned int indexCount;
	unsigned int index[THC_LIMIT_STRUCT_NESTING_DEPTH];

	InstCompositeInsert(unsigned int resultTypeId, unsigned int objectId, unsigned int compositeId, unsigned int indexCount, unsigned int* indices);

	void GetInstWords(unsigned int* words) const override;
};

class InstCopyObject : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstCopyObject(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstTranspose : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int matrixId;

	InstTranspose(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

#pragma endregion

#pragma region arithmetic instructions

class InstSNegate : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstSNegate(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstFNegate : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstFNegate(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstIAdd: InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstIAdd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFAdd : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFAdd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstISub : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstISub(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFSub : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFSub(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstIMul : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstIMul(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFMul : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFMul(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstUDiv : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstUDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSDiv : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFDiv : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFDiv(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstUMod : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstUMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSRem : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSRem(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSMod : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFRem : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFRem(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFMod : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFMod(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstVectorTimesScalar : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;
	unsigned int scalarId;

	InstVectorTimesScalar(unsigned int resultTypeId, unsigned int vectorId, unsigned int scalarId);

	void GetInstWords(unsigned int* words) const override;
};

class InstMatrixTimesScalar : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int matrixId;
	unsigned int scalarId;

	InstMatrixTimesScalar(unsigned int resultTypeId, unsigned int matrixId, unsigned int scalarId);

	void GetInstWords(unsigned int* words) const override;
};

class InstVectorTimesMatrix : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;
	unsigned int matrixId;

	InstVectorTimesMatrix(unsigned int resultTypeId, unsigned int vectorId, unsigned int matrixId);

	void GetInstWords(unsigned int* words) const override;
};

class InstMatrixTimesVector: InstBase {
public:
	unsigned int resultTypeId;
	unsigned int matrixId;
	unsigned int vectorId;

	InstMatrixTimesVector(unsigned int resultTypeId, unsigned int matrixId, unsigned int vectorId);

	void GetInstWords(unsigned int* words) const override;
};

class InstMatrixTimesMatrix : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int matrix1Id;
	unsigned int matrix2Id;

	InstMatrixTimesMatrix(unsigned int resultTypeId, unsigned int matrix1Id, unsigned int matrix2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstOuterProduct : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vector1Id;
	unsigned int vector2Id;

	InstOuterProduct(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstDot : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vector1Id;
	unsigned int vector2Id;

	InstDot(unsigned int resultTypeId, unsigned int vector1Id, unsigned int vector2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstIAddCarry: InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstIAddCarry(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstISubBorrow : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstISubBorrow(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstUMulExtended : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstUMulExtended(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSMulExtended : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSMulExtended(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

#pragma endregion

#pragma region bit instructions

class InstShiftRightLogical : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int baseId;
	unsigned int shiftId;

	InstShiftRightLogical(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId);

	void GetInstWords(unsigned int* words) const override;
};

class InstShiftRightArithmetic : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int baseId;
	unsigned int shiftId;

	InstShiftRightArithmetic(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId);

	void GetInstWords(unsigned int* words) const override;
};

class InstShiftLeftLogical : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int baseId;
	unsigned int shiftId;

	InstShiftLeftLogical(unsigned int resultTypeId, unsigned int baseId, unsigned int shiftId);

	void GetInstWords(unsigned int* words) const override;
};

class InstBitwiseOr: InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstBitwiseOr(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstBitwiseXor : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstBitwiseXor(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstBitwiseAnd : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstBitwiseAnd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstNot : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstNot(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstBitReverse : InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;

	InstBitReverse(unsigned int resultTypeId, unsigned int operand1Id);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpBitFieldInsert, OpBitFieldSExtract, OpBitFieldUExtract, OpBitCount

#pragma endregion

#pragma region relational and logical instructions

class InstAny : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;

	InstAny(unsigned int resultTypeId, unsigned int vectorId);

	void GetInstWords(unsigned int* words) const override;
};

class InstAll : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int vectorId;

	InstAll(unsigned int resultTypeId, unsigned int vectorId);

	void GetInstWords(unsigned int* words) const override;
};

class InstIsNan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstIsNan(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstIsInf : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstIsInf(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};


class InstLogicalEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstLogicalEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstLogicalNotEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstLogicalNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstLogicalOr: public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstLogicalOr(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstLogicalAnd : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstLogicalAnd(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstLogicalNot : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operandId;

	InstLogicalNot(unsigned int resultTypeId, unsigned int operandId);

	void GetInstWords(unsigned int* words) const override;
};

class InstSelect : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int conditionId;
	unsigned int object1Id;
	unsigned int object2Id;

	InstSelect(unsigned int resultTypeId, unsigned int conditionId, unsigned int object1Id, unsigned int object2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstIEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstIEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstIEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstIEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstINotEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstINotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstUGreaterThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstUGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};


class InstSGreaterThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstUGreaterThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstUGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSGreaterThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstULessThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstULessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};


class InstSLessThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstULessThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstULessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstSLessThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstSLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdNotEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordNotEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordNotEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdLessThan: public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordLessThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordLessThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdGreaterThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordGreaterThan : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordGreaterThan(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdLessThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordLessThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordLessThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFOrdGreaterThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFOrdGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

class InstFUnordGreaterThanEqual : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int operand1Id;
	unsigned int operand2Id;

	InstFUnordGreaterThanEqual(unsigned int resultTypeId, unsigned int operand1Id, unsigned int operand2Id);

	void GetInstWords(unsigned int* words) const override;
};

#pragma endregion

#pragma region derivative instructions

//TODO: implement this shit, sometime

#pragma endregion

#pragma region control-flow instructions

#define MAKE_PHI_PAIR(variable, block) ((PhiPair)((variable & 0xFFFFFFFF) | ((((PhiPair)block & 0xFFFFFFFF)) << 32)))

typedef unsigned long long PhiPair;

class InstPhi : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int pairCount;
	PhiPair pairs[32];

	InstPhi(unsigned int resultTypeId, unsigned int pairCount, PhiPair* pairs);

	void GetInstWords(unsigned int* words) const override;
};

class InstLoopMerge : public InstBase {
public:
	unsigned int mergeBlockId;
	unsigned int continueTargetId;
	unsigned int loopControl;
	//TODO: parameters when needed

	InstLoopMerge(unsigned int mergeBlockId, unsigned int continueTargetId, unsigned int loopControl);

	void GetInstWords(unsigned int* words) const override;
};

class InstSelectionMerge: public InstBase {
public:
	unsigned int mergeBlockId;
	unsigned int selectionControl;

	InstSelectionMerge(unsigned int mergeBlockId, unsigned int selectionControl);

	void GetInstWords(unsigned int* words) const override;
};

class InstLabel: public InstBase {
public:

	InstLabel();

	void GetInstWords(unsigned int* words) const override;
};

class InstBranch : public InstBase {
public:
	unsigned int targetLabelId;

	InstBranch(unsigned int targetLabelId);

	void GetInstWords(unsigned int* words) const override;
};

class InstBranchConditional : public InstBase {
public:
	unsigned int conditionId;
	unsigned int trueLabelId;
	unsigned int falseLabelId;
	unsigned int trueWeight;
	unsigned int falseWeight;

	InstBranchConditional(unsigned int conditionId, unsigned int trueLabelId, unsigned int falseLabelId, unsigned int trueWeight, unsigned int falseWeight);

	void GetInstWords(unsigned int* words) const override;
};

typedef unsigned long long SwitchPair;

class InstSwitch : public InstBase {
public:
	unsigned int selectorId;
	unsigned int defaultId;
	unsigned int pairCount;
	SwitchPair pairs[THC_LIMIT_OPSWITCH_PAIRS];

	InstSwitch(unsigned int selectorId, unsigned int defaultId);

	void GetInstWords(unsigned int* words) const override;
};

class InstKill : public InstBase {
public:

	InstKill();
};

class InstReturn : public InstBase {
public:

	InstReturn();
};

class InstReturnValue : public InstBase {
public:
	unsigned int valueId;

	InstReturnValue(unsigned int valueId);

	void GetInstWords(unsigned int* words) const override;
};

//TODO: OpLifetimeStart, OpLifetimeStop

#pragma endregion

#pragma region atomic instructions

//TODO: this shit


#pragma endregion

#pragma region primitive instructions

//TODO: geometry stuff

#pragma endregion

#pragma region barrier instructions

//TODO:

#pragma endregion

#pragma region group instructions

//TODO:

#pragma endregion

#pragma region device-side enqueue instructions

//TODO:

#pragma endregion

#pragma region pipe instructions

//TODO:


#pragma endregion

#pragma region non-uniform instructions

//TODO:

#pragma endregion

}
}
}