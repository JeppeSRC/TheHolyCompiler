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

#include "extendedinstructions.h"

#define THC_SPIRV_EXT_OPCODE_OpRound 1
#define THC_SPIRV_EXT_OPCODE_OpRoundEven 2
#define THC_SPIRV_EXT_OPCODE_OpTrunc 3
#define THC_SPIRV_EXT_OPCODE_OpFAbs 4
#define THC_SPIRV_EXT_OPCODE_OpSAbs 5
#define THC_SPIRV_EXT_OPCODE_OpFSign 6
#define THC_SPIRV_EXT_OPCODE_OpSSign 7
#define THC_SPIRV_EXT_OPCODE_OpFloor 8
#define THC_SPIRV_EXT_OPCODE_OpCeil 9
#define THC_SPIRV_EXT_OPCODE_OpFract 10
#define THC_SPIRV_EXT_OPCODE_OpRadians 11
#define THC_SPIRV_EXT_OPCODE_OpDegrees 12
#define THC_SPIRV_EXT_OPCODE_OpSin 13
#define THC_SPIRV_EXT_OPCODE_OpCos 14
#define THC_SPIRV_EXT_OPCODE_OpTan 15
#define THC_SPIRV_EXT_OPCODE_OpASin 16
#define THC_SPIRV_EXT_OPCODE_OpACos 17
#define THC_SPIRV_EXT_OPCODE_OpATan 18
#define THC_SPIRV_EXT_OPCODE_OpSinh 19
#define THC_SPIRV_EXT_OPCODE_OpCosh 20
#define THC_SPIRV_EXT_OPCODE_OpTanh 21
#define THC_SPIRV_EXT_OPCODE_OpASinh 22
#define THC_SPIRV_EXT_OPCODE_OpACosh 23
#define THC_SPIRV_EXT_OPCODE_OpATanh 24
#define THC_SPIRV_EXT_OPCODE_OpATan2 25
#define THC_SPIRV_EXT_OPCODE_OpPow 26
#define THC_SPIRV_EXT_OPCODE_OpExp 27
#define THC_SPIRV_EXT_OPCODE_OpLog 28
#define THC_SPIRV_EXT_OPCODE_OpExp2 29
#define THC_SPIRV_EXT_OPCODE_OpLog2 30
#define THC_SPIRV_EXT_OPCODE_OpSqrt 31
#define THC_SPIRV_EXT_OPCODE_OpInvSqrt 32
#define THC_SPIRV_EXT_OPCODE_OpDeterminant 33
#define THC_SPIRV_EXT_OPCODE_OpMatInv 34
#define THC_SPIRV_EXT_OPCODE_OpModf 35
#define THC_SPIRV_EXT_OPCODE_OpFMin 37
#define THC_SPIRV_EXT_OPCODE_OpUMin 38
#define THC_SPIRV_EXT_OPCODE_OpSMin 39
#define THC_SPIRV_EXT_OPCODE_OpFMax 40
#define THC_SPIRV_EXT_OPCODE_OpUMax 41
#define THC_SPIRV_EXT_OPCODE_OpSMax 42
#define THC_SPIRV_EXT_OPCODE_OpFClamp 43
#define THC_SPIRV_EXT_OPCODE_OpUClamp 44
#define THC_SPIRV_EXT_OPCODE_OpSClamp 45
#define THC_SPIRV_EXT_OPCODE_OpFMix 46
#define THC_SPIRV_EXT_OPCODE_OpFma 50
#define THC_SPIRV_EXT_OPCODE_OpLength 66
#define THC_SPIRV_EXT_OPCODE_OpDistance 67
#define THC_SPIRV_EXT_OPCODE_OpCross 68
#define THC_SPIRV_EXT_OPCODE_OpNormalize 69
#define THC_SPIRV_EXT_OPCODE_OpReflect 71

namespace thc {
namespace core {
namespace instruction {

void InstExt::GetInstWords(unsigned int* words) const {
	InstBase::GetInstWords(words);

	words[1] = resultTypeId;
	words[2] = id;
	words[3] = setId;
	words[4] = opCode;
	words[5] = operand0;
	words[6] = operand1;
	words[7] = operand2;
}

InstExt::InstExt(unsigned int wordCount, const char* const literalName, unsigned int resultTypeId, unsigned int setId, unsigned int opCode, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstBase(THC_SPIRV_OPCODE_OpExtInst, 5 + wordCount, literalName, true), resultTypeId(resultTypeId), setId(setId), opCode(opCode), operand0(operand0), operand1(operand1), operand2(operand2) {}

InstExtRound::InstExtRound(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtRound", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpRound, operand0) {}

InstExtRoundEven::InstExtRoundEven(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtRoundEven", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpRoundEven, operand0) {}

InstExtTrunc::InstExtTrunc(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtTrunc", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpTrunc, operand0) {}

InstExtFAbs::InstExtFAbs(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtFAbs", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFAbs, operand0) {}

InstExtSAbs::InstExtSAbs(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtSAbs", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSAbs, operand0) {}

InstExtFSign::InstExtFSign(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtFSign", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFSign, operand0) {}

InstExtSSign::InstExtSSign(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtSSign", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSSign, operand0) {}

InstExtFloor::InstExtFloor(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtFloor", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFloor, operand0) {}

InstExtCeil::InstExtCeil(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtCeil", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpCeil, operand0) {}

InstExtFract::InstExtFract(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtFract", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFract, operand0) {}

InstExtRadians::InstExtRadians(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtRadians", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpRadians, operand0) {}

InstExtDegrees::InstExtDegrees(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtDegrees", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpDegrees, operand0) {}

InstExtSin::InstExtSin(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtSin", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSin, operand0) {}

InstExtCos::InstExtCos(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtCos", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpCos, operand0) {}

InstExtTan::InstExtTan(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtTan", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpTan, operand0) {}

InstExtASin::InstExtASin(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtASin", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpASin, operand0) {}

InstExtACos::InstExtACos(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtACos", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpACos, operand0) {}

InstExtATan::InstExtATan(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtATan", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpATan, operand0) {}

InstExtSinh::InstExtSinh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtSinh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSinh, operand0) {}

InstExtCosh::InstExtCosh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtCosh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpCosh, operand0) {}

InstExtTanh::InstExtTanh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtTanh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpTanh, operand0) {}

InstExtASinh::InstExtASinh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtASinh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpASinh, operand0) {}

InstExtACosh::InstExtACosh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtACosh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpACosh, operand0) {}

InstExtATanh::InstExtATanh(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtATanh", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpATanh, operand0) {}

InstExtATan2::InstExtATan2(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int yId) : InstExt(1, "OpExtATan2", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpATan2, operand0, yId) {}

InstExtPow::InstExtPow(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int yId) : InstExt(1, "OpExtPow", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpPow, operand0, yId) {}

InstExtExp::InstExtExp(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtExp", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpExp, operand0) {}

InstExtLog::InstExtLog(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtLog", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpLog, operand0) {}

InstExtExp2::InstExtExp2(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtExp2", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpExp2, operand0) {}

InstExtLog2::InstExtLog2(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtLog2", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpLog2, operand0) {}

InstExtSqrt::InstExtSqrt(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtSqrt", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSqrt, operand0) {}

InstExtInvSqrt::InstExtInvSqrt(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtInvSqrt", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpInvSqrt, operand0) {}

InstExtDeterminant::InstExtDeterminant(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtDeterminant", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpDeterminant, operand0) {}

InstExtMatInv::InstExtMatInv(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtMatInv", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpMatInv, operand0) {}

InstExtModf::InstExtModf(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtModf", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpModf, operand0, operand1) {}

InstExtFMin::InstExtFMin(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtFMin", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFMin, operand0, operand1) {}

InstExtUMin::InstExtUMin(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtUMin", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpUMin, operand0, operand1) {}

InstExtSMin::InstExtSMin(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtSMin", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSMin, operand0, operand1) {}

InstExtFMax::InstExtFMax(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtFMax", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFMax, operand0, operand1) {}

InstExtUMax::InstExtUMax(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtUMax", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpUMax, operand0, operand1) {}

InstExtSMax::InstExtSMax(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtSMax", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSMax, operand0, operand1) {}

InstExtFClamp::InstExtFClamp(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstExt(3, "OpExtFClamp", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFClamp, operand0, operand1, operand2) {}

InstExtUClamp::InstExtUClamp(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstExt(3, "OpExtUClamp", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpUClamp, operand0, operand1, operand2) {}

InstExtSClamp::InstExtSClamp(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstExt(3, "OpExtSClamp", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpSClamp, operand0, operand1, operand2) {}

InstExtFMix::InstExtFMix(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstExt(3, "OpExtFMix", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFMix, operand0, operand1, operand2) {}

InstExtFma::InstExtFma(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1, unsigned int operand2) : InstExt(3, "OpExtFma", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpFma, operand0, operand1, operand2) {}

InstExtLength::InstExtLength(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtLength", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpLength, operand0) {}

InstExtDistance::InstExtDistance(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtDistance", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpDistance, operand0, operand1) {}

InstExtCross::InstExtCross(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtCross", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpCross, operand0, operand1) {}

InstExtNormalize::InstExtNormalize(unsigned int resultTypeId, unsigned int setId, unsigned int operand0) : InstExt(1, "OpExtNormalize", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpNormalize, operand0) {}

InstExtReflect::InstExtReflect(unsigned int resultTypeId, unsigned int setId, unsigned int operand0, unsigned int operand1) : InstExt(2, "OpExtReflect", resultTypeId, setId, THC_SPIRV_EXT_OPCODE_OpReflect, operand0, operand1) {}

}
}
}