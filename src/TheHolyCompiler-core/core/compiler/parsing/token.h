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

#include <util/string.h>
#include <util/list.h>
#include "line.h"

namespace thc {
namespace core {
namespace parsing {



enum class TokenType {
	None,

	ParenthesisOpen,
	ParenthesisClose,
	BracketOpen,
	BracketClose,
	CurlyBracketOpen,
	CurlyBracketClose,

	SemiColon,
	Comma,
	Name,
	Value,

	TypeVoid,
	TypeBool,
	TypeInt,
	TypeFloat,
	TypeVec,
	TypeMat,

	OperatorIncrement,
	OperatorDecrement,
	OperatorNegate,
	OperatorSelector,
	OperatorTernary1,
	OperatorTernary2,
	OperatorBitwiseAnd,
	OperatorBitwiseOr,
	OperatorBitwiseXor,
	OperatorBitwiseNot,
	OperatorLeftShift,
	OperatorRightShift,
	OperatorLogicalAnd,
	OperatorLogicalOr,
	OperatorLogicalNot,
	OperatorLogicalEqual,
	OperatorLogicalNotEqual,
	OperatorGreater,
	OperatorLess,
	OperatorGreaterEqual,
	OperatorLessEqual,
	OperatorAdd,
	OperatorSub,
	OperatorMul,
	OperatorDiv,
	OperatorAssign,
	OperatorCompoundAdd,
	OperatorCompoundSub,
	OperatorCompoundMul,
	OperatorCompoundDiv,

	ControlFlowIf,
	ControlFlowSwitch,
	ControlFlowElse,
	ControlFlowFor,
	ControlFlowWhile,
	ControlFlowBreak,
	ControlFlowContinue,
	ControlFlowReturn,

	DataStruct,
	DataLayout,
	DataIn,
	DataOut,
	DataUniform,

	ModifierConst,
	ModifierReference = OperatorBitwiseAnd,
};


bool operator>(TokenType left, TokenType right);
bool operator<(TokenType left, TokenType right);
bool operator>=(TokenType left, TokenType right);
bool operator<=(TokenType left, TokenType right);

class Token {
public:
	TokenType type;

	union {
		float64 fValue;
		uint64 value;
	};

	uint8 bits;
	uint8 sign;
	uint8 rows;
	uint8 columns;


	utils::String string;
	parsing::Line line;
	uint64 column;
	
	//Preprocesor
	Token(TokenType type, const utils::String& string, uint64 column);
	Token(TokenType type, uint64 value, const utils::String& string, uint64 column);

	//Compiler
	Token(TokenType type, const utils::String& string, const parsing::Line& line, uint64 column);
	Token(TokenType type, uint64 value, const utils::String& string, const parsing::Line& line, uint64 column);

	//Def
	Token() { }
	Token(const Token& other);
	Token(const Token* other);
	Token(Token&& other);

	Token& operator=(const Token& other);
	Token& operator=(Token&& other);

public:
	static bool ValidReturnType(TokenType type);
	static bool ValidInOutType(TokenType type);
};

}
}
}