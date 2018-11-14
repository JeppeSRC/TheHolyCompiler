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

//

#pragma once

#include "instructions.h"
#include <core/thctypes.h>

namespace thc {
namespace core {
namespace instruction {

class InstExt : public InstBase {
public:
	uint32 resultTypeId;
	uint32 setId;
	uint32 opCode;

	uint32 operand0;
	uint32 operand1;
	uint32 operand2;


	InstExt(uint32 wordCount, const char* const literalName, uint32 resultTypeId, uint32 setId, uint32 opCode, uint32 operand0, uint32 operand1 = 0, uint32 operand2 = 0);

	virtual void GetInstWords(uint32* words) const override;
};



class InstExtRound: public InstExt {
public:
	InstExtRound(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtRoundEven : public InstExt {
public:
	InstExtRoundEven(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtTrunc : public InstExt {
public:
	InstExtTrunc(uint32 resultTypeId, uint32 setId, uint32 xId);
};


class InstExtFAbs : public InstExt {
public:
	InstExtFAbs(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtSAbs : public InstExt {
public:
	InstExtSAbs(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtFSign : public InstExt {
public:
	InstExtFSign(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtSSign : public InstExt {
public:
	InstExtSSign(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtFloor : public InstExt {
public:
	InstExtFloor(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtCeil : public InstExt {
public:
	InstExtCeil(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtFract : public InstExt {
public:
	InstExtFract(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtRadians : public InstExt {
public:
	InstExtRadians(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtDegrees : public InstExt {
public:
	InstExtDegrees(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtSin : public InstExt {
public:
	InstExtSin(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtCos : public InstExt {
public:
	InstExtCos(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtTan : public InstExt {
public:
	InstExtTan(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtASin : public InstExt {
public:
	InstExtASin(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtACos : public InstExt {
public:
	InstExtACos(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtATan : public InstExt {
public:
	InstExtATan(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtSinh : public InstExt {
public:
	InstExtSinh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtCosh : public InstExt {
public:
	InstExtCosh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtTanh : public InstExt {
public:
	InstExtTanh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtASinh : public InstExt {
public:
	InstExtASinh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtACosh : public InstExt {
public:
	InstExtACosh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtATanh : public InstExt {
public:
	InstExtATanh(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtATan2 : public InstExt {
public:
	InstExtATan2(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtPow : public InstExt {
public:
	InstExtPow(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtExp : public InstExt {
public:
	InstExtExp(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtLog : public InstExt {
public:
	InstExtLog(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtExp2 : public InstExt {
public:
	InstExtExp2(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtLog2 : public InstExt {
public:
	InstExtLog2(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtSqrt  : public InstExt {
public:
	InstExtSqrt (uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtInvSqrt : public InstExt {
public:
	InstExtInvSqrt(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtDeterminant : public InstExt {
public:
	InstExtDeterminant(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtMatInv : public InstExt {
public:
	InstExtMatInv(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtModf : public InstExt {
public:
	InstExtModf(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 iId);
};

class InstExtFMin : public InstExt {
public:
	InstExtFMin(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtUMin : public InstExt {
public:
	InstExtUMin(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtSMin : public InstExt {
public:
	InstExtSMin(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtFMax : public InstExt {
public:
	InstExtFMax(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtUMax : public InstExt {
public:
	InstExtUMax(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtSMax : public InstExt {
public:
	InstExtSMax(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtFClamp : public InstExt {
public:
	InstExtFClamp(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 minValId, uint32 maxValId);
};

class InstExtUClamp : public InstExt {
public:
	InstExtUClamp(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 minValId, uint32 maxValId);
};

class InstExtSClamp : public InstExt {
public:
	InstExtSClamp(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 minValId, uint32 maxValId);
};

class InstExtFMix : public InstExt {
public:
	InstExtFMix(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId, uint32 aId);
};

class InstExtFma : public InstExt {
public:
	InstExtFma(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId, uint32 aId);
};

class InstExtLength : public InstExt {
public:
	InstExtLength(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtDistance : public InstExt {
public:
	InstExtDistance(uint32 resultTypeId, uint32 setId, uint32 p0Id, uint32 p1Id);
};

class InstExtCross : public InstExt {
public:
	InstExtCross(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

class InstExtNormalize : public InstExt {
public:
	InstExtNormalize(uint32 resultTypeId, uint32 setId, uint32 xId);
};

class InstExtReflect : public InstExt {
public:
	InstExtReflect(uint32 resultTypeId, uint32 setId, uint32 xId, uint32 yId);
};

}
}
}

