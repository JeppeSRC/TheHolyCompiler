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

namespace thc {
namespace core {
namespace instruction {

class InstExt : public InstBase {
public:
	unsigned int resultTypeId;
	unsigned int setId;
	unsigned int opCode;

	unsigned int operand0;
	unsigned int operand1;
	unsigned int operand2;


	InstExt(unsigned int wordCount, const char* const literalName, unsigned int resultTypeId, unsigned int setId, unsigned int opCode, unsigned int operand0, unsigned int operand1 = 0, unsigned int operand2 = 0);

	virtual void GetInstWords(unsigned int* words) const override;
};



class InstExtRound: public InstExt {
public:
	InstExtRound(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtRoundEven : public InstExt {
public:
	InstExtRoundEven(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtTrunc : public InstExt {
public:
	InstExtTrunc(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};


class InstExtFAbs : public InstExt {
public:
	InstExtFAbs(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtSAbs : public InstExt {
public:
	InstExtSAbs(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtFSign : public InstExt {
public:
	InstExtFSign(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtSSign : public InstExt {
public:
	InstExtSSign(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtFloor : public InstExt {
public:
	InstExtFloor(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtCeil : public InstExt {
public:
	InstExtCeil(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtFract : public InstExt {
public:
	InstExtFract(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtRadians : public InstExt {
public:
	InstExtRadians(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtDegrees : public InstExt {
public:
	InstExtDegrees(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtSin : public InstExt {
public:
	InstExtSin(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtCos : public InstExt {
public:
	InstExtCos(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtTan : public InstExt {
public:
	InstExtTan(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtASin : public InstExt {
public:
	InstExtASin(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtACos : public InstExt {
public:
	InstExtACos(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtATan : public InstExt {
public:
	InstExtATan(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtSinh : public InstExt {
public:
	InstExtSinh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtCosh : public InstExt {
public:
	InstExtCosh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtTanh : public InstExt {
public:
	InstExtTanh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtASinh : public InstExt {
public:
	InstExtASinh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtACosh : public InstExt {
public:
	InstExtACosh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtATanh : public InstExt {
public:
	InstExtATanh(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtATan2 : public InstExt {
public:
	InstExtATan2(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtPow : public InstExt {
public:
	InstExtPow(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtExp : public InstExt {
public:
	InstExtExp(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtLog : public InstExt {
public:
	InstExtLog(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtExp2 : public InstExt {
public:
	InstExtExp2(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtLog2 : public InstExt {
public:
	InstExtLog2(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtSqrt  : public InstExt {
public:
	InstExtSqrt (unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtInvSqrt : public InstExt {
public:
	InstExtInvSqrt(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtDeterminant : public InstExt {
public:
	InstExtDeterminant(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtMatInv : public InstExt {
public:
	InstExtMatInv(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtModf : public InstExt {
public:
	InstExtModf(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int iId);
};

class InstExtFMin : public InstExt {
public:
	InstExtFMin(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtUMin : public InstExt {
public:
	InstExtUMin(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtSMin : public InstExt {
public:
	InstExtSMin(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtFMax : public InstExt {
public:
	InstExtFMax(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtUMax : public InstExt {
public:
	InstExtUMax(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtSMax : public InstExt {
public:
	InstExtSMax(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtFClamp : public InstExt {
public:
	InstExtFClamp(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int minValId, unsigned int maxValId);
};

class InstExtUClamp : public InstExt {
public:
	InstExtUClamp(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int minValId, unsigned int maxValId);
};

class InstExtSClamp : public InstExt {
public:
	InstExtSClamp(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int minValId, unsigned int maxValId);
};

class InstExtFMix : public InstExt {
public:
	InstExtFMix(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId, unsigned int aId);
};

class InstExtFma : public InstExt {
public:
	InstExtFma(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId, unsigned int aId);
};

class InstExtLength : public InstExt {
public:
	InstExtLength(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtDistance : public InstExt {
public:
	InstExtDistance(unsigned int resultTypeId, unsigned int setId, unsigned int p0Id, unsigned int p1Id);
};

class InstExtCross : public InstExt {
public:
	InstExtCross(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

class InstExtNormalize : public InstExt {
public:
	InstExtNormalize(unsigned int resultTypeId, unsigned int setId, unsigned int xId);
};

class InstExtReflect : public InstExt {
public:
	InstExtReflect(unsigned int resultTypeId, unsigned int setId, unsigned int xId, unsigned int yId);
};

}
}
}

