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
#include <core/thctypes.h>
#include <core/compiler/idmanager.h>


namespace thc {
namespace core {
namespace instruction {

enum class InstType {
	None,
	Instruction,
	Type
};

class InstBase {
public:
	InstType type;
	compiler::ID* id;
	uint32 opCode;
	mutable uint32 wordCount;
	char* literalName;

	InstBase(uint32 opCode, uint32 wordCount, const char* const literalName, bool resultId = false, InstType type = InstType::Instruction);
	virtual ~InstBase();

	virtual void GetInstWords(uint32* words) const;

	virtual bool operator==(const InstBase* const inst) const { return false; }
};

#pragma region misc

class InstNop : public InstBase {
public:

	InstNop();
};

class InstUndef : public InstBase {
public:
	compiler::ID* resultTypeId;

	InstUndef(compiler::ID* resultTypeId);

	void GetInstWords(uint32* words) const override;
};

class InstSizeOf : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* pointerId;

	InstSizeOf(compiler::ID* resultTypeId, compiler::ID* pointerId);

	void GetInstWords(uint32* words) const override;
};

#pragma endregion

#pragma region debug

class InstSourceContinued : public InstBase {
public:
	char* source;

	InstSourceContinued(const char* const source);

	void GetInstWords(uint32* words) const override;
};

class InstSource : public InstBase {
public:
	uint32 sourceLanguage;
	uint32 version;
	compiler::ID* fileNameId;
	char* source;

	InstSource(uint32 sourceLanguage, uint32 version, compiler::ID* fileNameId, const char* const source);

	void GetInstWords(uint32* words) const override;
};

class InstSourceExtension : public InstBase {
public:
	char* extension;

	InstSourceExtension(const char* const extension);

	void GetInstWords(uint32* words) const override;
};

class InstName : public InstBase {
public:
	compiler::ID* targetId;
	char* name;

	InstName(compiler::ID* targetId, const char* const name);

	void GetInstWords(uint32* words) const override;
};


class InstMemberName : public InstBase {
public:
	compiler::ID* typeId;
	uint32 member;
	char* name;

	InstMemberName(compiler::ID* typeId, uint32 member, const char* const name);

	void GetInstWords(uint32* words) const override;
};


class InstString : public InstBase {
public:
	char* string;

	InstString(const char* const string);

	void GetInstWords(uint32* words) const override;
};

class InstLine : public InstBase {
public:
	compiler::ID* fileNameId;
	uint32 line;
	uint32 column;

	InstLine(compiler::ID* fileNameId, uint32 line, uint32 column);

	void GetInstWords(uint32* words) const override;
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
	compiler::ID* targetId;
	uint32 decoration;
	uint32 numDecorationLiterals;
	uint32 literals[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstDecorate(compiler::ID* targetId, uint32 decoration, uint32* literals, uint32 numDecorationLiterals);

	void GetInstWords(uint32* words) const override;
};

class InstMemberDecorate : public InstBase {
public:
	compiler::ID* structId;
	uint32 member;
	uint32 decoration;
	uint32 numDecorationLiterals;
	uint32 literals[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstMemberDecorate(compiler::ID* structId, uint32 member, uint32 decoration, uint32* literals, uint32 numDecorationLiterals);

	void GetInstWords(uint32* words) const override;
};

class InstDecorationGroup : public InstBase {
public:
	InstDecorationGroup();

	void GetInstWords(uint32* words) const override;
};

class InstGroupDecorate : public InstBase {
public:
	compiler::ID* groupId;
	uint32 numTargets;
	compiler::ID* targetId[THC_LIMIT_DECORATIONS_PER_TARGET];

	InstGroupDecorate(compiler::ID* groupId, compiler::ID** targets, uint32 numTargets);

	void GetInstWords(uint32* words) const override;
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

	void GetInstWords(uint32* words) const;
};

class InstExtInstImport : public InstBase {
public:
	char* extensionSet;

	InstExtInstImport(const char* const extensionSet);

	void GetInstWords(uint32* words) const;
};

//TODO: OpExtInst will be 

#pragma endregion

#pragma region modes settings

class InstMemoryModel : public InstBase {
public:
	uint32 addressingModel;
	uint32 memoryModel;

	InstMemoryModel(uint32 addressingModel, uint32 memoryModel);

	void GetInstWords(uint32* words) const override;
};

class InstEntryPoint : public InstBase {
public:
	uint32 executionModel;
	compiler::ID* entryPointId;
	char* entryPointName;
	uint32 inoutVariableCount;
	compiler::ID* inoutVariableId[THC_LIMIT_GLOBAL_VARIABLES];

	InstEntryPoint(uint32 executionModel, compiler::ID* entryPointId, const char* const entryPointName, uint32 inoutVariableCount, compiler::ID** inoutVariableIds);

	void GetInstWords(uint32* words) const override;
};

class InstExecutionMode : public InstBase {
public:
	compiler::ID* entryPointId;
	uint32 mode;
	uint32 extraOperandCount;
	uint32 extraOperand[256];

	InstExecutionMode(compiler::ID* entryPointId, uint32 mode, uint32 extraOperandCount, uint32* extraOperands);

	void GetInstWords(uint32* words) const;
};

class InstCapability : public InstBase {
public:
	uint32 capability;

	InstCapability(uint32 capability);

	void GetInstWords(uint32* words) const override;
};

//TODO: OpExecutionModeId

#pragma endregion

#pragma region constant creation

class InstConstantTrue : public InstBase {
public:
	compiler::ID* resultTypeId;

	InstConstantTrue(compiler::ID* resultTypeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstBase* const inst) const override;
};

class InstConstantFalse : public InstBase {
public:
	compiler::ID* resultTypeId;

	InstConstantFalse(compiler::ID* resultTypeId);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstBase* const inst) const override;
};

class InstConstant : public InstBase {
public:
	compiler::ID* resultTypeId;
	uint32 valueCount;
	uint32* values;

	InstConstant(compiler::ID* resultTypeId, uint32 valueCount, void* values);
	InstConstant(compiler::ID* resultTypeId, uint32 value);
	InstConstant(compiler::ID* resultTypeId, float32 value);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstBase* const inst) const override;
};

class InstConstantComposite : public InstBase {
public:
	compiler::ID* resultTypeId;
	uint32 constituentCount;
	compiler::ID* constituentId[THC_LIMIT_OPTYPESTRUCT_MEMBERS];

	InstConstantComposite(compiler::ID* resultTypeId, uint32 constituentCount, compiler::ID** constituentIds);

	void GetInstWords(uint32* words) const override;

	bool operator==(const InstBase* const inst) const override;
};

//TOOD: OpConstantNull, OpSpec*

#pragma endregion

#pragma region memory instructions

class InstVariable : public InstBase {
public:
	compiler::ID* resultTypeId;
	uint32 storageClass;
	uint32 initializer;

	InstVariable(compiler::ID* resultTypeId, uint32 storageClass, uint32 initializer);

	void GetInstWords(uint32* words) const override;
};

class InstLoad : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* pointerId;
	uint32 memoryAccess;

	InstLoad(compiler::ID* resultTypeId, compiler::ID* pointerId, uint32 memoryAccess);

	void GetInstWords(uint32* words) const override;
};

class InstStore : public InstBase {
public:
	compiler::ID* pointerId;
	compiler::ID* objectId;
	uint32 memoryAccess;

	InstStore(compiler::ID* pointerId, compiler::ID* objectId, uint32 memoryAccess);

	void GetInstWords(uint32* words) const override;
};


class InstCopyMemory : public InstBase {
public:
	compiler::ID* targetId;
	compiler::ID* sourceId;
	uint32 memoryAccess;

	InstCopyMemory(compiler::ID* targetId, compiler::ID* sourceId, uint32 memoryAccess);

	void GetInstWords(uint32* words) const override;
};

class InstCopyMemorySized : public InstBase {
public:
	compiler::ID* targetId;
	compiler::ID* sourceId;
	compiler::ID* sizeId;
	uint32 memoryAccess;

	InstCopyMemorySized(compiler::ID* targetId, compiler::ID* sourceId, compiler::ID* sizeId, uint32 memoryAccess);

	void GetInstWords(uint32* words) const override;
};

class InstAccessChain : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* baseId;
	uint32 indexCount;
	compiler::ID* indexId[THC_LIMIT_INDEXES];

	InstAccessChain(compiler::ID* resultTypeId, compiler::ID* baseId, uint32 indexCount, compiler::ID** indexIds);

	void GetInstWords(uint32* words) const override;
};

class InstInBoundsAccessChain : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* baseId;
	uint32 indexCount;
	compiler::ID* indexId[THC_LIMIT_INDEXES];

	InstInBoundsAccessChain(compiler::ID* resultTypeId, compiler::ID* baseId, uint32 indexCount, compiler::ID** indexIds);

	void GetInstWords(uint32* words) const override;
};

//TODO: OpPtrAccessChain, OpInBoundsPtrAccessChain, OpArrayLength

#pragma endregion

#pragma region function instructions

class InstFunction : public InstBase {
public:
	compiler::ID* resultTypeId;
	uint32 functionControl;
	compiler::ID* functionTypeId;

	InstFunction(compiler::ID* resultTypeId, uint32 functionControl, compiler::ID* functionTypeId);

	void GetInstWords(uint32* words) const override;
}; 

class InstFunctionParameter : public InstBase {
public:
	compiler::ID* resultTypeId;

	InstFunctionParameter(compiler::ID* resultTypeId);

	void GetInstWords(uint32* words) const override;
};


class InstFunctionEnd : public InstBase {
public:

	InstFunctionEnd();
};

class InstFunctionCall : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* functionId;
	uint32 argumentCount;
	compiler::ID* argumentId[THC_LIMIT_OPFUNCTIONCALL_ARGUMENTS];

	InstFunctionCall(compiler::ID* resultTypeId, compiler::ID* functionId, uint32 argumentCount, compiler::ID** argumentIds);

	void GetInstWords(uint32* words) const override;
};

#pragma endregion

#pragma region image instructions

//TODO: image instructions

#pragma endregion

#pragma region conversion instructions

class InstConvertFToU : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertFToU(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstConvertFToS : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertFToS(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstConvertSToF : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertSToF(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstConvertUToF : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertUToF(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstUConvert : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstUConvert(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstSConvert : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstSConvert(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstFConvert : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstFConvert(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

//TODO: OpQuantizeToF16

class InstConvertPtrToU : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertPtrToU(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

class InstConvertUToPtr : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* valueId;

	InstConvertUToPtr(compiler::ID* resultTypeId, compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
};

//TODO: OpBitcast

#pragma endregion

#pragma region composite instructions

class InstVectorExtractDynamic : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;
	compiler::ID* indexId;

	InstVectorExtractDynamic(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* indexId);

	void GetInstWords(uint32* words) const override;
};

class InstVectorInsertDynamic : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;
	compiler::ID* componentId;
	compiler::ID* indexId;

	InstVectorInsertDynamic(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* componentId, compiler::ID* indexId);

	void GetInstWords(uint32* words) const override;
};

class InstVectorShuffle : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vector1Id;
	compiler::ID* vector2Id;
	uint32 componentCount;
	uint32 component[4];

	InstVectorShuffle(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id, uint32 componentCount, uint32* components);

	void GetInstWords(uint32* words) const override;
};

class InstCompositeConstruct: public InstBase {
public:
	compiler::ID* resultTypeId;
	uint32 constituentCount;
	compiler::ID* constituentId[512];

	InstCompositeConstruct(compiler::ID* resultTypeId, uint32 constituentCount, compiler::ID** constituentIds);

	void GetInstWords(uint32* words) const override;
};

class InstCompositeExtract : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* compositeId;
	uint32 indexCount;
	uint32 index[THC_LIMIT_STRUCT_NESTING_DEPTH];

	InstCompositeExtract(compiler::ID* resultTypeId, compiler::ID* compositeId, uint32 indexCount, uint32* indices);

	void GetInstWords(uint32* words) const override;
};

class InstCompositeInsert : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* objectId;
	compiler::ID* compositeId;
	uint32 indexCount;
	uint32 index[THC_LIMIT_STRUCT_NESTING_DEPTH];

	InstCompositeInsert(compiler::ID* resultTypeId, compiler::ID* objectId, compiler::ID* compositeId, uint32 indexCount, uint32* indices);

	void GetInstWords(uint32* words) const override;
};

class InstCopyObject : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstCopyObject(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstTranspose : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* matrixId;

	InstTranspose(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

#pragma endregion

#pragma region arithmetic instructions

class InstSNegate : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstSNegate(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstFNegate : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstFNegate(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstIAdd : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstIAdd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFAdd : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFAdd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstISub : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstISub(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFSub : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFSub(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstIMul : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstIMul(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFMul : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFMul(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstUDiv : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstUDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSDiv : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFDiv : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFDiv(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstUMod : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstUMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSRem : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSRem(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSMod : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFRem : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFRem(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFMod : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFMod(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstVectorTimesScalar : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;
	compiler::ID* scalarId;

	InstVectorTimesScalar(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* scalarId);

	void GetInstWords(uint32* words) const override;
};

class InstMatrixTimesScalar : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* matrixId;
	compiler::ID* scalarId;

	InstMatrixTimesScalar(compiler::ID* resultTypeId, compiler::ID* matrixId, compiler::ID* scalarId);

	void GetInstWords(uint32* words) const override;
};

class InstVectorTimesMatrix : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;
	compiler::ID* matrixId;

	InstVectorTimesMatrix(compiler::ID* resultTypeId, compiler::ID* vectorId, compiler::ID* matrixId);

	void GetInstWords(uint32* words) const override;
};

class InstMatrixTimesVector: public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* matrixId;
	compiler::ID* vectorId;

	InstMatrixTimesVector(compiler::ID* resultTypeId, compiler::ID* matrixId, compiler::ID* vectorId);

	void GetInstWords(uint32* words) const override;
};

class InstMatrixTimesMatrix : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* matrix1Id;
	compiler::ID* matrix2Id;

	InstMatrixTimesMatrix(compiler::ID* resultTypeId, compiler::ID* matrix1Id, compiler::ID* matrix2Id);

	void GetInstWords(uint32* words) const override;
};

class InstOuterProduct : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vector1Id;
	compiler::ID* vector2Id;

	InstOuterProduct(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id);

	void GetInstWords(uint32* words) const override;
};

class InstDot : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vector1Id;
	compiler::ID* vector2Id;

	InstDot(compiler::ID* resultTypeId, compiler::ID* vector1Id, compiler::ID* vector2Id);

	void GetInstWords(uint32* words) const override;
};

class InstIAddCarry: public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstIAddCarry(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstISubBorrow : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstISubBorrow(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstUMulExtended : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstUMulExtended(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSMulExtended : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSMulExtended(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

#pragma endregion

#pragma region bit instructions

class InstShiftRightLogical : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* baseId;
	compiler::ID* shiftId;

	InstShiftRightLogical(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId);

	void GetInstWords(uint32* words) const override;
};

class InstShiftRightArithmetic : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* baseId;
	compiler::ID* shiftId;

	InstShiftRightArithmetic(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId);

	void GetInstWords(uint32* words) const override;
};

class InstShiftLeftLogical : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* baseId;
	compiler::ID* shiftId;

	InstShiftLeftLogical(compiler::ID* resultTypeId, compiler::ID* baseId, compiler::ID* shiftId);

	void GetInstWords(uint32* words) const override;
};

class InstBitwiseOr: public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstBitwiseOr(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstBitwiseXor : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstBitwiseXor(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstBitwiseAnd : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstBitwiseAnd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstNot : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstNot(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstBitReverse : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;

	InstBitReverse(compiler::ID* resultTypeId, compiler::ID* operand1Id);

	void GetInstWords(uint32* words) const override;
};

//TODO: OpBitFieldInsert, OpBitFieldSExtract, OpBitFieldUExtract, OpBitCount

#pragma endregion

#pragma region relational and logical instructions

class InstAny : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;

	InstAny(compiler::ID* resultTypeId, compiler::ID* vectorId);

	void GetInstWords(uint32* words) const override;
};

class InstAll : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* vectorId;

	InstAll(compiler::ID* resultTypeId, compiler::ID* vectorId);

	void GetInstWords(uint32* words) const override;
};

class InstIsNan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstIsNan(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstIsInf : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstIsInf(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};


class InstLogicalEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstLogicalEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstLogicalNotEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstLogicalNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstLogicalOr: public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstLogicalOr(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstLogicalAnd : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstLogicalAnd(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstLogicalNot : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operandId;

	InstLogicalNot(compiler::ID* resultTypeId, compiler::ID* operandId);

	void GetInstWords(uint32* words) const override;
};

class InstSelect : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* conditionId;
	compiler::ID* object1Id;
	compiler::ID* object2Id;

	InstSelect(compiler::ID* resultTypeId, compiler::ID* conditionId, compiler::ID* object1Id, compiler::ID* object2Id);

	void GetInstWords(uint32* words) const override;
};

class InstIEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstIEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};


class InstINotEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstINotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstUGreaterThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstUGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};


class InstSGreaterThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstUGreaterThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstUGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSGreaterThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstULessThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstULessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};


class InstSLessThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstULessThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstULessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstSLessThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstSLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdNotEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordNotEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordNotEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdLessThan: public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordLessThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordLessThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdGreaterThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordGreaterThan : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordGreaterThan(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdLessThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordLessThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordLessThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFOrdGreaterThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFOrdGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
};

class InstFUnordGreaterThanEqual : public InstBase {
public:
	compiler::ID* resultTypeId;
	compiler::ID* operand1Id;
	compiler::ID* operand2Id;

	InstFUnordGreaterThanEqual(compiler::ID* resultTypeId, compiler::ID* operand1Id, compiler::ID* operand2Id);

	void GetInstWords(uint32* words) const override;
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
	compiler::ID* resultTypeId;
	uint32 pairCount;
	PhiPair pairs[32];

	InstPhi(compiler::ID* resultTypeId, uint32 pairCount, PhiPair* pairs);

	void GetInstWords(uint32* words) const override;
};

class InstLoopMerge : public InstBase {
public:
	compiler::ID* mergeBlockId;
	compiler::ID* continueTargetId;
	uint32 loopControl;
	//TODO: parameters when needed

	InstLoopMerge(compiler::ID* mergeBlockId, compiler::ID* continueTargetId, uint32 loopControl);

	void GetInstWords(uint32* words) const override;
};

class InstSelectionMerge: public InstBase {
public:
	compiler::ID* mergeBlockId;
	uint32 selectionControl;

	InstSelectionMerge(compiler::ID* mergeBlockId, uint32 selectionControl);

	void GetInstWords(uint32* words) const override;
};

class InstLabel: public InstBase {
public:

	InstLabel();

	void GetInstWords(uint32* words) const override;
};

class InstBranch : public InstBase {
public:
	compiler::ID* targetLabelId;

	InstBranch(compiler::ID* targetLabelId);

	void GetInstWords(uint32* words) const override;
};

class InstBranchConditional : public InstBase {
public:
	compiler::ID* conditionId;
	compiler::ID* trueLabelId;
	compiler::ID* falseLabelId;
	uint32 trueWeight;
	uint32 falseWeight;

	InstBranchConditional(compiler::ID* conditionId, compiler::ID* trueLabelId, compiler::ID* falseLabelId, uint32 trueWeight, uint32 falseWeight);

	void GetInstWords(uint32* words) const override;
};

struct SwitchPair {
	uint32 litteral;
	compiler::ID* labelId;
};

class InstSwitch : public InstBase {
public:
	compiler::ID* selectorId;
	compiler::ID* defaultId;
	uint32 pairCount;
	SwitchPair pair[THC_LIMIT_OPSWITCH_PAIRS];

	InstSwitch(compiler::ID* selectorId, compiler::ID* defaultId, SwitchPair* pairs);

	void GetInstWords(uint32* words) const override;
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
	compiler::ID* valueId;

	InstReturnValue(compiler::ID* valueId);

	void GetInstWords(uint32* words) const override;
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