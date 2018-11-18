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
#include <core/compiler/parsing/line.h>
#include <core/thctypes.h>

namespace thc {
namespace core {
namespace preprocessor {

class PreProcessor {
private:
	struct Define {
		utils::String name;
		utils::String value;

		Define() {}
		Define(const utils::String& name, const utils::String& value);
		Define(const Define& other);
		Define(const Define* other);
		Define(Define&& other);

		Define& operator=(const Define& other);
		Define& operator=(Define&& other);

	};

	enum class TokenType {
		Value,
		Parenthesis,
		Operator,
	};

	enum class TokenValue : uint64 {
		None,
		OperatorLogicalAnd,
		OperatorLogicalOr,
		OperatorLogicalNot,
		OperatorEqual,
		OperatorNotEqual,
		OperatorGreater,
		OperatorLess,
		OperatorGreaterEqual,
		OperatorLessEqual,
		OperatorAdd,
		OperatorSub,
		OperatorMul,
		OperatorDiv,
		OperatorBitwiseAnd,
		OperatorBitwiseOr,
		OperatorBitwiseNot,
		OperatorBitwiseXor,
		OperatorBitwiseShitLeft,
		OperatorBitwiseShiftRight,

		ParenthesisOpen,
		ParenthesisClose,
	};

	struct Token {
		TokenType type;

		union {
			uint64 value;
			TokenValue valueType;
		};

		uint64 column;
		utils::String string;

		Token() {}
		Token(TokenType type, uint64 value, const utils::String& string, uint64 column);
		Token(TokenType type, TokenValue value, const utils::String& string, uint64 column);
		Token(const Token& other);
		Token(const Token* other);
		Token(Token&& other);

		Token& operator=(const Token& other);
		Token& operator=(Token&& other);

	};

private:
	utils::String fileName;
	utils::List<parsing::Line> lines;

	utils::List<Define> defines;
	utils::List<utils::String> includedFiles;
	utils::List<utils::String> includeDirectories;

	uint64 IsDefined(const utils::String& name);
	utils::String FindFile(const utils::String& fileName, utils::String parentDir);

	void RemoveComments(utils::String& code);
	void ProcessInclude(uint64& index);
	void ProcessDefine(uint64& index);
	void ProcessUndef(uint64& index);
	void ProcessIf(uint64& index);
	void ProcessIfdef(uint64& index);
	void ProcessMessage(uint64& index);
	void ProcessError(uint64& index);
	void Process();

private:
	bool ProcessStatement(uint64 start, uint64 end, utils::List<Token>& tokens, const parsing::Line& line);

private:
	utils::List<Token> TokenizeStatement(const utils::String& code, const parsing::Line& line);
	void ReplaceMacrosWithValue(utils::String& code);
	uint64 FindMatchingParenthesis(const utils::List<Token>& tokens, uint64 start, const parsing::Line& line);
	uint64 FindLineWith(const utils::String& string, uint64 offset);

private:
	PreProcessor(utils::String code, const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);

public:
	static utils::String Run(const utils::String& code, const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);
	static utils::String Run(const utils::String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs);
};

}
}
}
