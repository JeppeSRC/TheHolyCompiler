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
#include <util/utils.h>
#include <util/list.h>
#include <core/compiler/parsing/line.h>
#include "options.h"
#include "idmanager.h"

namespace thc {
namespace core {
namespace compiler {

class Compiler {
private:
	enum class TokenType: uint64 {
		None,

		ParenthesisOpen,
		ParenthesisClose,
		BracketOpen,
		BracketClose,
		CurlyBracketOpen,
		CurlyBracketClose,

		SemiColon,
		Name,
		Value,

		TypeVoid,
		TypeBool,
		TypeByte,
		TypeUint,
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
		OperatorCompoundAdd,
		OperatorCompoundSub,
		OperatorCompoundMul,
		OperatorCompoundDiv,

		ModifierConst,
		ModifierReference,

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
				
	};

	struct Token {
		TokenType type;

		union {
			float64 fValue;
			uint64 value;

			struct {
				uint8 bits;
				uint8 rows;
				uint8 columns;
				TokenType compType : 40;
			};
		};

		utils::String string;
		parsing::Line line;
		uint64 column;

		
		Token(TokenType type, const utils::String& string, const parsing::Line& line, uint64 column); Token() {}
		Token(TokenType type, uint64 value, const utils::String& string, const parsing::Line& line, uint64 column);
		Token(const Token& other);
		Token(const Token* other);
		Token(Token&& other);

		Token& operator=(const Token& other);
		Token& operator=(Token&& other);

	};

private:
	bool IsCharAllowedInName(const char c, bool first = true) const;
	bool IsCharWhitespace(const char c) const;
	void ProcessName(Token& t) const;

private:
	utils::String code;
	utils::String filename;
	utils::List<parsing::Line> lines;
	utils::List<utils::String> defines;
	utils::List<utils::String> includes;

	utils::List<Token> Tokenize();

	bool Process();

	Compiler(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
public:
	static bool Run(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
	static bool Run(const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);

};

}
}
}