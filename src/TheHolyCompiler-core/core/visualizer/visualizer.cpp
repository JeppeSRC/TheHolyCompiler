#include "visulizer.h"
#include <util/log.h>
#include <core/spirvdefines.h>

using namespace thc;
using namespace core;
using namespace visualizer;
using namespace utils;

struct Thing {
	void Init(bool ret, char* string) { retId = ret; opCodeString = string; }
	char* opCodeString;
	bool retId;
};

static Thing thinge[1024];

#define PAIR(ret, op) thinge[op].Init(ret, ThingaMaBob(#op)) 

static char* ThingaMaBob(const char* string) {
	uint64 len = strlen(string) - 16;

	char* str = new char[len];
	memcpy(str, string + 17, len);

	return str;
}

static void InitializeOpCodes() {
	for (uint32 i = 0; i < 1024; i++) {
		thinge[i].opCodeString = "Unknown";
	}

	PAIR(false,	THC_SPIRV_OPCODE_OpNop);
	PAIR(true,	THC_SPIRV_OPCODE_OpUndef);
	PAIR(true,	THC_SPIRV_OPCODE_OpSizeOf);
	PAIR(false,	THC_SPIRV_OPCODE_OpSourceContinued);
	PAIR(false,	THC_SPIRV_OPCODE_OpSource);
	PAIR(false,	THC_SPIRV_OPCODE_OpSourceExtension);
	PAIR(false,	THC_SPIRV_OPCODE_OpName);
	PAIR(false,	THC_SPIRV_OPCODE_OpMemberName);
	PAIR(true,	THC_SPIRV_OPCODE_OpString);
	PAIR(false,	THC_SPIRV_OPCODE_OpLine);
	PAIR(false,	THC_SPIRV_OPCODE_OpNoLine);
	PAIR(false,	THC_SPIRV_OPCODE_OpDecorate);
	PAIR(false,	THC_SPIRV_OPCODE_OpMemberDecorate);
	PAIR(true,	THC_SPIRV_OPCODE_OpDecorateGroup);
	PAIR(false,	THC_SPIRV_OPCODE_OpGroupDecorate);
	PAIR(false,	THC_SPIRV_OPCODE_OpGroupMemberDecorate);
	PAIR(false,	THC_SPIRV_OPCODE_OpDecorateId);
	PAIR(false,	THC_SPIRV_OPCODE_OpExtension);
	PAIR(true,	THC_SPIRV_OPCODE_OpExtInstImport);
	PAIR(true,	THC_SPIRV_OPCODE_OpExtInst);
	PAIR(false,	THC_SPIRV_OPCODE_OpMemoryModel);
	PAIR(false,	THC_SPIRV_OPCODE_OpEntryPoint);
	PAIR(false,	THC_SPIRV_OPCODE_OpExecutionMode);
	PAIR(false,	THC_SPIRV_OPCODE_OpCapability);
	PAIR(false,	THC_SPIRV_OPCODE_OpExecutionModeId);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeVoid);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeBool);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeInt);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeFloat);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeVector);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeMatrix);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeImage);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeSampler);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeSampledImage);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeArray);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeRuntimeArray);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeStruct);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeOpaque);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypePointer);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeFunction);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeEvent);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeDeviceEvent);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeReserveId);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeQueue);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypePipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeForwardPointer);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypePipeStorage);
	PAIR(true,	THC_SPIRV_OPCODE_OpTypeNamedBarrier);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantTrue);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantFalse);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstant);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantComposite);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantSampler);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantNull);
	PAIR(true,	THC_SPIRV_OPCODE_OpSpecConstantTrue);
	PAIR(true,	THC_SPIRV_OPCODE_OpSpecConstantFalse);
	PAIR(true,	THC_SPIRV_OPCODE_OpSpecConstant);
	PAIR(true,	THC_SPIRV_OPCODE_OpSpecConstantComposite);
	PAIR(true,	THC_SPIRV_OPCODE_OpSpecConstantOp);
	PAIR(true,	THC_SPIRV_OPCODE_OpVariable);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageTexelPointer);
	PAIR(true,	THC_SPIRV_OPCODE_OpLoad);
	PAIR(true,	THC_SPIRV_OPCODE_OpStore);
	PAIR(true,	THC_SPIRV_OPCODE_OpCopyMemory);
	PAIR(true,	THC_SPIRV_OPCODE_OpCopyMemorySized);
	PAIR(true,	THC_SPIRV_OPCODE_OpAccessChain);
	PAIR(true,	THC_SPIRV_OPCODE_OpInBoundsAccessChain);
	PAIR(true,	THC_SPIRV_OPCODE_OpPtrAccessChain);
	PAIR(true,	THC_SPIRV_OPCODE_OpArrayLength);
	PAIR(true,	THC_SPIRV_OPCODE_OpGenericPtrMemSemantics);
	PAIR(true,	THC_SPIRV_OPCODE_OpInBoundsPtrAccessChain);
	PAIR(true,	THC_SPIRV_OPCODE_OpFunction);
	PAIR(true,	THC_SPIRV_OPCODE_OpFunctionParameter);
	PAIR(false,	THC_SPIRV_OPCODE_OpFunctionEnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpFunctionCall);
	PAIR(true,	THC_SPIRV_OPCODE_OpSampledImage);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleDrefImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleDrefExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleProjImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleProjExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleProjDrefImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSampleProjDrefExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageFetch);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageGather);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageDrefGather);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageRead);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageWrite);
	PAIR(true,	THC_SPIRV_OPCODE_OpImage);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQueryFormat);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQueryOrder);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQuerySizeLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQuerySize);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQueryLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQueryLevels);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageQuerySamples);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleDrefImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleDrefExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleProjImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleProjExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleProjDrefImplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseSampleProjDrefExplicitLod);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseFetch);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseGather);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseDrefGather);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseTexelsResident);
	PAIR(true,	THC_SPIRV_OPCODE_OpImageSparseRead);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertFToU);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertFToS);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertSToF);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertUToF);
	PAIR(true,	THC_SPIRV_OPCODE_OpUConvert);
	PAIR(true,	THC_SPIRV_OPCODE_OpSConvert);
	PAIR(true,	THC_SPIRV_OPCODE_OpFConvert);
	PAIR(true,	THC_SPIRV_OPCODE_OpQuantizeToF16);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertPtrToU);
	PAIR(true,	THC_SPIRV_OPCODE_OpSatConvertSToU);
	PAIR(true,	THC_SPIRV_OPCODE_OpSatConvertUToS);
	PAIR(true,	THC_SPIRV_OPCODE_OpConvertUToPtr);
	PAIR(true,	THC_SPIRV_OPCODE_OpPtrCastToGeneric);
	PAIR(true,	THC_SPIRV_OPCODE_OpGenericCastToPtr);
	PAIR(true,	THC_SPIRV_OPCODE_OpGenericCastToPtrExplicit);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitcast);
	PAIR(true,	THC_SPIRV_OPCODE_OpVectorExtractDynamic);
	PAIR(true,	THC_SPIRV_OPCODE_OpVectorInsertDynamic);
	PAIR(true,	THC_SPIRV_OPCODE_OpVectorShuffle);
	PAIR(true,	THC_SPIRV_OPCODE_OpCompositeConstruct);
	PAIR(true,	THC_SPIRV_OPCODE_OpCompositeExtract);
	PAIR(true,	THC_SPIRV_OPCODE_OpCompositeInsert);
	PAIR(true,	THC_SPIRV_OPCODE_OpCopyObject);
	PAIR(true,	THC_SPIRV_OPCODE_OpTranspose);
	PAIR(true,	THC_SPIRV_OPCODE_OpSNegate);
	PAIR(true,	THC_SPIRV_OPCODE_OpFNegate);
	PAIR(true,	THC_SPIRV_OPCODE_OpIAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpFAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpISub);
	PAIR(true,	THC_SPIRV_OPCODE_OpFSub);
	PAIR(true,	THC_SPIRV_OPCODE_OpIMul);
	PAIR(true,	THC_SPIRV_OPCODE_OpFMul);
	PAIR(true,	THC_SPIRV_OPCODE_OpUDiv);
	PAIR(true,	THC_SPIRV_OPCODE_OpSDiv);
	PAIR(true,	THC_SPIRV_OPCODE_OpFDiv);
	PAIR(true,	THC_SPIRV_OPCODE_OpUMod);
	PAIR(true,	THC_SPIRV_OPCODE_OpSRem);
	PAIR(true,	THC_SPIRV_OPCODE_OpSMod);
	PAIR(true,	THC_SPIRV_OPCODE_OpFRem);
	PAIR(true,	THC_SPIRV_OPCODE_OpFMod);
	PAIR(true,	THC_SPIRV_OPCODE_OpVectorTimesScalar);
	PAIR(true,	THC_SPIRV_OPCODE_OpMatrixTimesScalar);
	PAIR(true,	THC_SPIRV_OPCODE_OpVectorTimesMatrix);
	PAIR(true,	THC_SPIRV_OPCODE_OpMatrixTimesVector);
	PAIR(true,	THC_SPIRV_OPCODE_OpMatrixTimesMatrix);
	PAIR(true,	THC_SPIRV_OPCODE_OpOuterProduct);
	PAIR(true,	THC_SPIRV_OPCODE_OpDot);
	PAIR(true,	THC_SPIRV_OPCODE_OpIAddCarry);
	PAIR(true,	THC_SPIRV_OPCODE_OpISubBorrow);
	PAIR(true,	THC_SPIRV_OPCODE_OpUMulExtended);
	PAIR(true,	THC_SPIRV_OPCODE_OpSMulExtended);
	PAIR(true,	THC_SPIRV_OPCODE_OpShiftRightLogical);
	PAIR(true,	THC_SPIRV_OPCODE_OpShiftRightArithmetic);
	PAIR(true,	THC_SPIRV_OPCODE_OpShiftLeftLogical);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitwiseOr);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitwiseXor);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitwiseAnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpNot);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitFieldInsert);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitFieldSExtract);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitFieldUExtract);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitReverse);
	PAIR(true,	THC_SPIRV_OPCODE_OpBitCount);
	PAIR(true,	THC_SPIRV_OPCODE_OpAny);
	PAIR(true,	THC_SPIRV_OPCODE_OpAll);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsNan);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsInf);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsFinite);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsNormal);
	PAIR(true,	THC_SPIRV_OPCODE_OpSignBitSet);
	PAIR(true,	THC_SPIRV_OPCODE_OpLessOrGreater);
	PAIR(true,	THC_SPIRV_OPCODE_OpOrdered);
	PAIR(true,	THC_SPIRV_OPCODE_OpUnordered);
	PAIR(true,	THC_SPIRV_OPCODE_OpLogicalEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpLogicalNotEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpLogicalOr);
	PAIR(true,	THC_SPIRV_OPCODE_OpLogicalAnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpLogicalNot);
	PAIR(true,	THC_SPIRV_OPCODE_OpSelect);
	PAIR(true,	THC_SPIRV_OPCODE_OpIEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpINotEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpUGreaterThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpSGreaterThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpUGreaterThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpSGreaterThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpULessThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpSLessThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpULessThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpSLessThanEqual); 
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdEqual);	   
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdNotEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordNotEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdLessThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordLessThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdGreaterThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordGreaterThan);
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdLessThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordLessThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFOrdGreaterThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpFUnordGreaterThanEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdx);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdy);
	PAIR(true,	THC_SPIRV_OPCODE_OpFwidth);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdxFine);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdyFine);
	PAIR(true,	THC_SPIRV_OPCODE_OpFwidthFine);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdxCoarse);
	PAIR(true,	THC_SPIRV_OPCODE_OpDPdyCoarse);
	PAIR(true,	THC_SPIRV_OPCODE_OpFwidthCoarse);
	PAIR(true,	THC_SPIRV_OPCODE_OpPhi);
	PAIR(false,	THC_SPIRV_OPCODE_OpLoopMerge);
	PAIR(false,	THC_SPIRV_OPCODE_OpSelectionMerge);
	PAIR(true,	THC_SPIRV_OPCODE_OpLabel);
	PAIR(false,	THC_SPIRV_OPCODE_OpBranch);
	PAIR(false,	THC_SPIRV_OPCODE_OpBranchConditional);
	PAIR(false,	THC_SPIRV_OPCODE_OpSwitch);
	PAIR(false,	THC_SPIRV_OPCODE_OpKill);
	PAIR(false,	THC_SPIRV_OPCODE_OpReturn);
	PAIR(false,	THC_SPIRV_OPCODE_OpReturnValue);
	PAIR(false,	THC_SPIRV_OPCODE_OpUnreachable);
	PAIR(false,	THC_SPIRV_OPCODE_OpLifetimeStart);
	PAIR(false,	THC_SPIRV_OPCODE_OpLifetimeStop);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicLoad);
	PAIR(false,	THC_SPIRV_OPCODE_OpAtomicStore);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicExchange);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicCompareExchange);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicCompareExchangeWeak);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicIIncrement);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicIDecrement);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicIAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicISub);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicSMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicUMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicSMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicUMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicAnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicOr);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicXor);
	PAIR(true,	THC_SPIRV_OPCODE_OpAtomicFlagTestAndSet);
	PAIR(false,	THC_SPIRV_OPCODE_OpAtomicFlagClear);
	PAIR(false,	THC_SPIRV_OPCODE_OpEmitVertex);
	PAIR(false,	THC_SPIRV_OPCODE_OpEndPrimitive);
	PAIR(false,	THC_SPIRV_OPCODE_OpEmitStreamVertex);
	PAIR(false,	THC_SPIRV_OPCODE_OpEndStreamPrimitive);
	PAIR(false,	THC_SPIRV_OPCODE_OpControlBarrier);
	PAIR(false,	THC_SPIRV_OPCODE_OpMemoryBarrier);
	PAIR(true,	THC_SPIRV_OPCODE_OpNamedBarrierInitialize);
	PAIR(false,	THC_SPIRV_OPCODE_OpMemoryNamedBarrier);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupAsyncCopy);
	PAIR(false,	THC_SPIRV_OPCODE_OpGroupWaitEvents);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupAll);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupAny);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupBroadcast);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupIAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupFAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupFMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupUMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupSMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupFMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupUMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupSMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpEnqueueMarker);
	PAIR(true,	THC_SPIRV_OPCODE_OpEnqueueKernel);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetKernelNDrangeSubGroupCount);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetKernelNDrangeMaxSubGroupSize);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetKernelPreferredWorkGroupSizeMultiple);
	PAIR(false,	THC_SPIRV_OPCODE_OpRetainEvent);
	PAIR(false,	THC_SPIRV_OPCODE_OpReleaseEvent);
	PAIR(true,	THC_SPIRV_OPCODE_OpCreateUserEvent);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsValidEvent);
	PAIR(false,	THC_SPIRV_OPCODE_OpSetUserEventStatus);
	PAIR(false,	THC_SPIRV_OPCODE_OpCaptureEventProfilingInfo);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetDefaultQueue);
	PAIR(true,	THC_SPIRV_OPCODE_OpBuildNDRange);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetKernelLocalSizeForSubgroupCount);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetKernelMaxNumSubgroups);
	PAIR(true,	THC_SPIRV_OPCODE_OpReadPipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpWritePipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpReservedReadPipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpReservedWritePipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpReserveReadPipePackets);
	PAIR(true,	THC_SPIRV_OPCODE_OpReserveWritePipePackets);
	PAIR(false,	THC_SPIRV_OPCODE_OpCommitReadPipe);
	PAIR(false,	THC_SPIRV_OPCODE_OpCommitWritePipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpIsValidReserveId);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetNumPipePackets);
	PAIR(true,	THC_SPIRV_OPCODE_OpGetMaxPipePackets);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupReserveReadPipePackets);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupReserveWritePipePackets);
	PAIR(false,	THC_SPIRV_OPCODE_OpGroupCommitReadPipe);
	PAIR(false,	THC_SPIRV_OPCODE_OpGroupCommitWritePipe);
	PAIR(true,	THC_SPIRV_OPCODE_OpConstantPipeStorage);
	PAIR(true,	THC_SPIRV_OPCODE_OpCreatePipeFromPipeStorage);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformElect);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformAll);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformAny);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformAllEqual);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBroadcast);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBroadcastFirst);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBallot);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformInverseBallot);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBallotBitExtract);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBallotBitCount);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBallotFindLSB);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBallotFindMSB);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformShuffle);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformShuffleXor);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformShuffleUp);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformShuffleDown);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformIAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformFAdd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformIMul);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformFMul);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformSMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformUMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformFMin);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformSMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformUMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformFMax);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBitwiseAnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBitwiseOr);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformBitwiseXor);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformLogicalAnd);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformLogicalOr);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformLogicalXor);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformQuadBroadcast);
	PAIR(true,	THC_SPIRV_OPCODE_OpGroupNonUniformQuadSwap);

}

bool Visualizer::VisualizeSPIRV(const utils::String& filename) {
	FILE* file = fopen(filename.str, "rb");

	_fseeki64(file, 0, SEEK_END);
	uint64 length = _ftelli64(file);
	_fseeki64(file, 0, SEEK_SET);

	if (length & 0x3) {
		Log::Error("\"%s\" is not 4 byte aligned", filename.str);
		return false;
	}

	List<uint32> code;
	code.Resize(length >> 2);

	fread(code.GetData(), code.GetSize(), 1, file);
	fclose(file);

	return VisualizeSPIRV(code);
}

void print(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	printf("\n");
	va_end(list);
}

bool Visualizer::VisualizeSPIRV(List<uint32> code) {
	InitializeOpCodes();

	if (code[0] != THC_SPIRV_MAGIC_NUMBER) {
		Log::Error("Invalid file, file is not a spirv file");
		return false;
	}

	print("Header");
	print("Version: 0x%08X", code[1]);
	print("Generator: 0x%08X", code[2]);
	print("Bound: %u", code[3]);
	print("Schema: %u", code[4]);
	print("");
	print("Code");

	uint64 offset = 5;

	while (offset < code.GetCount()) {
		uint16 opCode = code[offset] & 0xFFFF;
		uint16 wordCount = (code[offset] >> 16) & 0xFFFF;

		const Thing& op = thinge[opCode];

		uint64 i = 1;

		if (op.retId) {
			printf(" %u\t%s", code[offset + 1], op.opCodeString);
			i++;
		} else {
			printf(" \t%s", op.opCodeString);
		}

		for (; i < wordCount; i++) {
			printf(" %u", code[offset + i]);
		}
		printf("\n");
		offset += wordCount;

	}

	return false;
}