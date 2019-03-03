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
	compiler::ID* resultTypeId;
	compiler::ID* setId;
	uint32 opCode;

	compiler::ID* operand0;
	compiler::ID* operand1;
	compiler::ID* operand2;


	InstExt(uint32 wordCount, const char* const literalName, compiler::ID* resultTypeId, compiler::ID* setId, uint32 opCode, compiler::ID* operand0, compiler::ID* operand1 = 0, compiler::ID* operand2 = 0);

	virtual void GetInstWords(uint32* words) const override;
};



class InstExtRound: public InstExt {
public:
	InstExtRound(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtRoundEven : public InstExt {
public:
	InstExtRoundEven(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtTrunc : public InstExt {
public:
	InstExtTrunc(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};


class InstExtFAbs : public InstExt {
public:
	InstExtFAbs(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtSAbs : public InstExt {
public:
	InstExtSAbs(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtFSign : public InstExt {
public:
	InstExtFSign(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtSSign : public InstExt {
public:
	InstExtSSign(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtFloor : public InstExt {
public:
	InstExtFloor(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtCeil : public InstExt {
public:
	InstExtCeil(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtFract : public InstExt {
public:
	InstExtFract(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtRadians : public InstExt {
public:
	InstExtRadians(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtDegrees : public InstExt {
public:
	InstExtDegrees(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtSin : public InstExt {
public:
	InstExtSin(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtCos : public InstExt {
public:
	InstExtCos(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtTan : public InstExt {
public:
	InstExtTan(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtASin : public InstExt {
public:
	InstExtASin(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtACos : public InstExt {
public:
	InstExtACos(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtATan : public InstExt {
public:
	InstExtATan(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtSinh : public InstExt {
public:
	InstExtSinh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtCosh : public InstExt {
public:
	InstExtCosh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtTanh : public InstExt {
public:
	InstExtTanh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtASinh : public InstExt {
public:
	InstExtASinh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtACosh : public InstExt {
public:
	InstExtACosh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtATanh : public InstExt {
public:
	InstExtATanh(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtATan2 : public InstExt {
public:
	InstExtATan2(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtPow : public InstExt {
public:
	InstExtPow(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtExp : public InstExt {
public:
	InstExtExp(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtLog : public InstExt {
public:
	InstExtLog(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtExp2 : public InstExt {
public:
	InstExtExp2(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtLog2 : public InstExt {
public:
	InstExtLog2(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtSqrt  : public InstExt {
public:
	InstExtSqrt (compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtInvSqrt : public InstExt {
public:
	InstExtInvSqrt(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtDeterminant : public InstExt {
public:
	InstExtDeterminant(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtMatInv : public InstExt {
public:
	InstExtMatInv(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtModf : public InstExt {
public:
	InstExtModf(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* iId);
};

class InstExtFMin : public InstExt {
public:
	InstExtFMin(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtUMin : public InstExt {
public:
	InstExtUMin(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtSMin : public InstExt {
public:
	InstExtSMin(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtFMax : public InstExt {
public:
	InstExtFMax(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtUMax : public InstExt {
public:
	InstExtUMax(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtSMax : public InstExt {
public:
	InstExtSMax(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtFClamp : public InstExt {
public:
	InstExtFClamp(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* minValId, compiler::ID* maxValId);
};

class InstExtUClamp : public InstExt {
public:
	InstExtUClamp(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* minValId, compiler::ID* maxValId);
};

class InstExtSClamp : public InstExt {
public:
	InstExtSClamp(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* minValId, compiler::ID* maxValId);
};

class InstExtFMix : public InstExt {
public:
	InstExtFMix(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId, compiler::ID* aId);
};

class InstExtFma : public InstExt {
public:
	InstExtFma(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId, compiler::ID* aId);
};

class InstExtLength : public InstExt {
public:
	InstExtLength(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtDistance : public InstExt {
public:
	InstExtDistance(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* p0Id, compiler::ID* p1Id);
};

class InstExtCross : public InstExt {
public:
	InstExtCross(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

class InstExtNormalize : public InstExt {
public:
	InstExtNormalize(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId);
};

class InstExtReflect : public InstExt {
public:
	InstExtReflect(compiler::ID* resultTypeId, compiler::ID* setId, compiler::ID* xId, compiler::ID* yId);
};

}
}
}

