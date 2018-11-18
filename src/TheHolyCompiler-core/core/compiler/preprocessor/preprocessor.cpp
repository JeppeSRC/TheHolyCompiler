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

#include <core/compiler/preprocessor/preprocessor.h>
#include <core/compiler/parsing/line.h>
#include <util/utils.h>
#include <util/log.h>
#include <chrono>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;
using namespace parsing;

void PreProcessor::RemoveComments(String& code) {
	uint64 index = 0;

	while ((index = code.Find("/*")) != ~0) {
		uint64 next = code.Find("*/", index);

		if (next != ~0) {
			uint64 numLines = code.Count("\n", index, next);

			if (numLines) {
				char* tmp = new char[numLines+1];
				tmp[numLines] = 0;

				for (uint64 i = 0; i < numLines; i++) {
					tmp[i] = '\n';
				}

				code.Insert(index, next+1, tmp);
			} else {
				code.Remove(index, next+1);
			}
		} else {
			uint64 col = 0;
			uint64 line = Utils::FindLine(code, index, &col);
			Log::CompilerError(fileName.str, line, col, "Multiline comment is missing end");
			break;
		}
	}

	index = 0;

	while ((index = code.Find("//", index+1)) != ~0) {
		uint64 end = code.Find("\n", index+2);

		if (end == ~0) {
			end = code.length-1;
		}

		code.Remove(index, end-1);
	}
}

void PreProcessor::ProcessInclude(uint64& index) {
	const Line& l = lines[index];
	const String& line = l.string;

	uint64 firstBracket = line.Find("<");
	uint64 secondBracket = line.Find(">", firstBracket+1);

	String file = line.SubString(firstBracket+1, secondBracket-1);

	String fullPath = FindFile(file, Utils::GetPathFromFile(fileName));

	if (fullPath == "AlreadyIncluded") {
		Log::CompilerDebug(l, -1, "File \"%s\" has already been included", fullPath.str);
		return;
	} else if (fullPath == "NotFound") {
		Log::CompilerError(l, firstBracket+1, "File \"%s\" not found", file.str);
		return;
	}

	lines.RemoveAt(index);
	lines.InsertList(index, Line::GetLinesFromFile(fullPath));

	index--;
}

void PreProcessor::ProcessDefine(uint64& index) {
	const Line& l = lines[index];
	String& line = lines[index].string;

	line.Append(" ");

	uint64 nameStart = line.Find("#define")+7;
	uint64 nameEnd = line.Find(" ", nameStart+1);

	String name = line.SubString(nameStart, nameEnd);
	Utils::RemoveWhiteSpace(name);

	String value = line.SubString(nameEnd, line.length-1);
	Utils::RemoveWhiteSpace(value);

	uint64 defIndex = IsDefined(name);

	if (defIndex != ~0) {
		Log::CompilerWarning(l, nameStart, "Macro redefinition \"%s\"", name.str);

		defines[defIndex].value = value;
	} else {
		defines.Emplace(name, value);
	}

	lines.RemoveAt(index--);
}

void PreProcessor::ProcessUndef(uint64& index) {
	const Line& l = lines[index];
	String& line = lines[index].string;

	uint64 nameStart = line.Find("#undef")+6;

	String name = line.SubString(nameStart, line.length-1);
	Utils::RemoveWhiteSpace(name);

	uint64 defIndex = IsDefined(name);

	if (defIndex != ~0) {
		defines.RemoveAt(defIndex);
	} else {
		Log::CompilerWarning(l, nameStart, "No macro \"%s\" is not defined");
	}

	lines.RemoveAt(index--);
}

void PreProcessor::ProcessIf(uint64& index, bool ifdef) {
	const Line& l = lines[index];
	String& line = lines[index].string;

	uint64 endifIndex = FindLineWith("#endif", index+1);

	if (endifIndex == ~0) {
		Log::CompilerError(l, 1, "Missing #endif directive");
		return;
	}

	uint64 elifIndex = FindLineWith("#elif", index+1);
	uint64 elseIndex = FindLineWith("#else", index+1);

	if (elifIndex > endifIndex) elifIndex = ~0;
	if (elseIndex > endifIndex) elseIndex = ~0;

	bool res = false;

	if (ifdef) {
		uint64 nameStart = line.Find("#ifdef ")+7;
		
		String name = line.SubString(nameStart, line.length-1);
		Utils::RemoveWhiteSpace(name);

		res = IsDefined(name) != ~0 ? true : false;
	} else {
		res = ProcessStatement(0, ~0, TokenizeStatement(line, l), l);
	}
	
	if (res) {
		if (elifIndex != ~0) {
			for (uint64 i = endifIndex; i >= elifIndex; i--) {
				lines.RemoveAt(i);
			}
		} else if (elseIndex != ~0) {
			for (uint64 i = endifIndex; i >= elseIndex; i--) {
				lines.RemoveAt(i);
			}
		} else {
			lines.RemoveAt(endifIndex);
		}

		lines.RemoveAt(index--);
		return;
	} else if (elifIndex != ~0) {
		do {
			const Line& elifLine = lines[elifIndex];

			if (ProcessStatement(0, ~0, TokenizeStatement(elifLine.string, elifLine), elifLine)) {
				uint64 remIndex = FindLineWith("#elif ", elifIndex+1);

				if (remIndex == ~0 || remIndex > elseIndex || remIndex > endifIndex) {
					remIndex = elseIndex != ~0 ? elseIndex : endifIndex;
				}

				for (uint64 i = endifIndex; i >= remIndex; i--) {
					lines.RemoveAt(i);
				}

				for (uint64 i = elifIndex; i >= index; i--) {
					lines.RemoveAt(i);
				}

				index--;
				return;
			} 
		} while ((elifIndex = FindLineWith("#elif ", elifIndex+1)) != ~0);
	} 

	if (elseIndex != ~0) {
		lines.RemoveAt(endifIndex);
		
		for (uint64 i = elseIndex; i >= index; i--) {
			lines.RemoveAt(i);
		}
	} else {
		for (uint64 i = endifIndex; i >= index; i--) {
			lines.RemoveAt(i);
		}
	}

	index--;
}

void PreProcessor::ProcessMessage(uint64& index, bool error) {
	const Line& l = lines[index];
	String& line = lines[index].string;

	uint64 messageStart = line.Find("\"")+1;

	if (messageStart == ~0) {
		Log::CompilerWarning(l, 1, "Invalid syntax, proper syntax: '#message \"some message\"");
		lines.RemoveAt(index--);
		return;
	}

	uint64 messageEnd = line.Find("\"", messageStart)-1;

	if (messageEnd == ~0) {
		Log::CompilerWarning(l, messageStart-1, "Invalid syntax, message has no end '\"'");
		lines.RemoveAt(index--);
		return;
	}

	String message = line.SubString(messageStart, messageEnd);

	if (error) {
		Log::CompilerError(l, 1, message.str);
	} else {
		Log::CompilerInfo(l, 1, message.str);
	}
}

void PreProcessor::Process() {

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		const Line& l = lines[i];
		const String& line = l.string;
		
		if (line.Find("#include ") != ~0) {
			ProcessInclude(i);
		} else if (line.Find("#define ") != ~0) {
			ProcessDefine(i);
		} else if (line.Find("#undef ") != ~0) {
			ProcessUndef(i);
		} else if (line.Find("#if ") != ~0) {
			ProcessIf(i, false);
		} else if (line.Find("#ifdef ") != ~0) {
			ProcessIf(i, true);
		} else if (line.Find("#message ") != ~0) {
			ProcessMessage(i, false);
		} else if (line.Find("#error ") != ~0) {
			ProcessMessage(i, true);
		}
	}

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		ReplaceMacrosWithValue(lines[i].string);
	}
	
}

bool PreProcessor::ProcessStatement(uint64 start, uint64 end, List<PreProcessor::Token>& tokens, const Line& line) {
	auto CmpFunc = [](const PreProcessor::Token& curr, const char& type) -> bool {
		return curr.string[0] == type;
	};

	if (end == ~0) end = tokens.GetCount()-1;

	uint64 parenthesisOpen = 0;
	uint64 offset = start;

	while ((parenthesisOpen = tokens.Find<char>('(', CmpFunc, offset)) != ~0) {
		if (parenthesisOpen <= end && parenthesisOpen >= start) {
			uint64 parenthesisClose = FindMatchingParenthesis(tokens, parenthesisOpen, line);

			if (parenthesisClose == ~0 || parenthesisClose > end) {
				Log::CompilerError(line, tokens[parenthesisOpen].column, "Missing closing parenthesis");
			}

			offset = parenthesisOpen + 1;

			ProcessStatement(offset, parenthesisClose-1, tokens, line);

			tokens.RemoveAt(offset+1);
			tokens.RemoveAt(offset-1);

			end -= parenthesisClose - parenthesisOpen;
		} else {
			break;
		}
	}


#pragma region precedence 2
	for (int64 i = end; i >= (int64)start; i--) {
		PreProcessor::Token& token = tokens[i];
		
		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorBitwiseNot || token.valueType == TokenValue::OperatorLogicalNot)) {
			if (i == end) {
				Log::CompilerError(line, token.column, "Operator \%s\" requires a right-hand operand", token.string.str);
				break;
			} 

			PreProcessor::Token& rightOperand = tokens[i+1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorBitwiseNot:
					rightOperand.value = ~rightOperand.value;
					break;
				case TokenValue::OperatorLogicalNot:
					rightOperand.value = rightOperand.value ? 0 : 1;
					break;
			}

			tokens.RemoveAt(i);
			end--;
		}
	}

#pragma endregion

#pragma region precedence 3

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorMul|| token.valueType == TokenValue::OperatorDiv)) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorMul:
					leftOperand.value *= rightOperand.value;
					break;
				case TokenValue::OperatorDiv:
					leftOperand.value /= rightOperand.value;
					break;
			}

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 4

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorAdd || token.valueType == TokenValue::OperatorSub)) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorAdd:
					leftOperand.value += rightOperand.value;
					break;
				case TokenValue::OperatorSub:
					leftOperand.value -= rightOperand.value;
					break;
			}

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 5

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorBitwiseShitLeft || token.valueType == TokenValue::OperatorBitwiseShitLeft)) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorBitwiseShitLeft:
					leftOperand.value <<= rightOperand.value;
					break;
				case TokenValue::OperatorBitwiseShiftRight:
					leftOperand.value >>= rightOperand.value;
					break;
			}

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 6

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorLess|| token.valueType == TokenValue::OperatorLessEqual|| token.valueType == TokenValue::OperatorGreater|| token.valueType == TokenValue::OperatorGreaterEqual)) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorLess:
					leftOperand.value = leftOperand.value < rightOperand.value ? 1 : 0;
					break;
				case TokenValue::OperatorLessEqual:
					leftOperand.value = leftOperand.value <= rightOperand.value ? 1 : 0;
					break;
				case TokenValue::OperatorGreater:
					leftOperand.value = leftOperand.value > rightOperand.value ? 1 : 0;
					break;
				case TokenValue::OperatorGreaterEqual:
					leftOperand.value = leftOperand.value >= rightOperand.value ? 1 : 0;
					break;
			}

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 7

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && (token.valueType == TokenValue::OperatorEqual|| token.valueType == TokenValue::OperatorNotEqual)) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			switch (token.valueType) {
				case TokenValue::OperatorEqual:
					leftOperand.value = leftOperand.value == rightOperand.value ? 1 : 0;
					break;
				case TokenValue::OperatorNotEqual:
					leftOperand.value = leftOperand.value != rightOperand.value ? 1 : 0;
					break;
			}

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 8

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && token.valueType == TokenValue::OperatorBitwiseAnd) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			leftOperand.value &= rightOperand.value;

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 9

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && token.valueType == TokenValue::OperatorBitwiseXor) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			leftOperand.value ^= rightOperand.value;

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 10

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && token.valueType == TokenValue::OperatorBitwiseOr) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			leftOperand.value |= rightOperand.value;

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 11

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && token.valueType == TokenValue::OperatorLogicalAnd) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			leftOperand.value = ((leftOperand.value > 0 ? true : false) && (rightOperand.value > 0 ? true : false)) ? 1 : 0;

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}

#pragma endregion

#pragma region precedence 12

	for (uint64 i = start; i <= end; i++) {
		PreProcessor::Token& token = tokens[i];

		if (token.type == TokenType::Operator && token.valueType == TokenValue::OperatorLogicalOr) {
			if (i == end || i == start) {
				Log::CompilerError(line, token.column, "Operator \"%s\" requires both a left-hand and a right-hand operand", token.string.str);
				break;
			}

			PreProcessor::Token& rightOperand = tokens[i+1];
			PreProcessor::Token& leftOperand = tokens[i-1];

			if (rightOperand.type != TokenType::Value) {
				Log::CompilerError(line, rightOperand.column, "Operator \"%s\": Right-hand operand must be a value", token.string.str);
				break;
			}

			if (leftOperand.type != TokenType::Value) {
				Log::CompilerError(line, leftOperand.column, "Operator \"%s\": Left-hand operand must be a value", token.string.str);
				break;
			}

			leftOperand.value = ((leftOperand.value > 0 ? true : false) || (rightOperand.value > 0 ? true : false)) ? 1 : 0;

			tokens.RemoveAt(i+1);
			tokens.RemoveAt(i);
			end -= 2;
		}
	}


#pragma endregion

	return tokens[start].value > 0 ? true : false;
}

List<PreProcessor::Token> PreProcessor::TokenizeStatement(const String& code, const Line& line) {
	List<PreProcessor::Token> tokens(code.length);

	uint64 i = code.Find("#if ")+4;

	if (i == (~0)+4) i = code.Find("#elif ")+6;

	for (; i < code.length; i++) {
		if (code[i] == ' ') {
			continue;
		} else if (code[i] == '&' && code[i+1] == '&') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorLogicalAnd, "&&", ++i);
		} else if (code[i] == '|' && code[i+1] == '|') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorLogicalOr, "||", ++i);
		} else if (code[i] == '=' && code[i+1] == '=') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorEqual, "==", ++i);
		} else if (code[i] == '!' && code[i+1] == '=') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorNotEqual, "!=", ++i);
		} else if (code[i] == '>' && code[i+1] == '=') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorGreaterEqual, ">=", ++i);
		} else if (code[i] == '<' && code[i+1] == '=') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorLessEqual, "<=", ++i);
		} else if (code[i] == '&') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorBitwiseAnd, "&", i+1);
		} else if (code[i] == '|') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorBitwiseOr, "|", i+1);
		} else if (code[i] == '~') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorBitwiseNot, "~", i+1);
		} else if (code[i] == '^') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorBitwiseXor, "^", i+1);
		} else if (code[i] == '(') {
			tokens.Emplace(TokenType::Parenthesis, TokenValue::ParenthesisOpen, "(", i+1);
		} else if (code[i] == ')') {
			tokens.Emplace(TokenType::Parenthesis, TokenValue::ParenthesisClose, ")", i+1);
		} else if (code[i] == '+') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorAdd, "+", i+1);
		} else if (code[i] == '-') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorSub, "-", i+1);
		} else if (code[i] == '*') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorMul, "*", i+1);
		} else if (code[i] == '/') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorDiv, "/", i+1);
		} else if (code[i] == '>') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorGreater, ">", i+1);
		} else if (code[i] == '<') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorLess, "<", i+1);
		} else if (code[i] == '!') {
			tokens.Emplace(TokenType::Operator, TokenValue::OperatorLogicalNot, "!", i+1);
		} else if (code[i] >= '0' && code[i] <= '9') {
			uint64 len = 0;
			uint64 value = 0;

			if (code[i-1] == '-') {
				tokens.RemoveAt(tokens.GetCount()-1);
										
				value = Utils::StringToUint64(code.str+i-1, &len);

				tokens.Emplace(TokenType::Value, value, code.SubString(i-1, i+len-1), i+1);
				continue;
			}

			value = Utils::StringToUint64(code.str+i, &len);

			tokens.Emplace(TokenType::Value, value, code.SubString(i, i+len-1), i+1);
		} else if (code.Find("defined(", i) == i) {
			uint64 end = code.Find(')', i);

			String macro = code.SubString(i+8, end-1);

			tokens.Emplace(TokenType::Value, IsDefined(macro) != ~0 ? 1 : 0, code.SubString(i, end), i+1);

			i = end;
		} else {
			bool res = false;
			for (uint64 j = 0; j < defines.GetCount(); j++) {
				const String& macroName = defines[j].name;

				if (code.Find(macroName, i) == i) {
					const String& svalue = defines[j].value;
					uint64 value = Utils::StringToUint64(svalue.str);

					if (value == ~0) {
						Log::CompilerError(line, i, "Macro \"%s\"(%s) doesn't define a numeric value", macroName.str, svalue.str);
						break;
					}

					tokens.Emplace(TokenType::Value, value, svalue, i+1);

					i += macroName.length-1;

					res = true;
					break;
				}
			}

			if (res) continue;

			Log::CompilerError(line, i, "Unknown symbol: %c", code[i]);
		}
	}

	return tokens;
}

void PreProcessor::ReplaceMacrosWithValue(String& code) {
	for (uint64 i = 0; i < defines.GetCount(); i++) {
		const String& name = defines[i].name;

		uint64 num = code.Count(name);

		for (uint64 i = 0; i < num; i++) {
			uint64 index = code.Find(name);

			code.Insert(index, index+name.length-1, defines[i].value);

			ReplaceMacrosWithValue(code);
		}
	}
}

uint64 PreProcessor::FindMatchingParenthesis(const List<PreProcessor::Token>& tokens, uint64 start, const Line& line) {
	uint64 count = 0;

	for (uint64 i = 0; i < tokens.GetCount(); i++) {
		const PreProcessor::Token& t = tokens[i];

		if (t.type == TokenType::Parenthesis) {
			if (t.valueType == TokenValue::ParenthesisOpen) {
				count++;
			} else if (t.valueType == TokenValue::ParenthesisClose) {
				if (count == 1) return i;
				else if (count == 0) {
					Log::CompilerError(line, t.column, "One lonley ')' found without a matching '('");
					break;
				}
				count--;
			}
		}
	}

	return ~0;
}

uint64 PreProcessor::FindLineWith(const String& string, uint64 offset) {
	for (uint64 i = offset; i < lines.GetCount(); i++) {
		if (lines[i].string.Find(string) != ~0) {
			return i;
		}
	}

	return ~0;
}

PreProcessor::PreProcessor(String code, const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) : fileName(fileName), includeDirectories(includeDirs) {
	this->defines.Reserve(defines.GetCount());

	for (uint64 i = 0; i < defines.GetCount(); i++) {
		this->defines.Emplace(defines[i], "");
	}

	RemoveComments(code);

	lines = Line::GetLinesFromString(code, fileName);
}

String PreProcessor::Run(const String& code, const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) {
	auto start = std::chrono::high_resolution_clock::now();
	PreProcessor pp(code, fileName, defines, includeDirs);
	
	pp.Process();

	String res = Line::ToString(pp.lines);

	auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start).count();

	Log::Debug("PreProcessing took %lld microseconds", time);

	return res;
}

String PreProcessor::Run(const String& fileName, const utils::List<utils::String>& defines, const utils::List<utils::String>& includeDirs) {
	return Run(Utils::ReadFile(fileName), fileName, defines, includeDirs);
}


}
}
}