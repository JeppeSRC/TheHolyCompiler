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


#include "token.h"

namespace thc {
namespace core {
namespace parsing {

using namespace utils;

bool operator>(TokenType left, TokenType right) {
	return int(left) > int(right);
}

bool operator<(TokenType left, TokenType right) {
	return int(left) < int(right);
}

bool operator>=(TokenType left, TokenType right) {
	return int(left) >= int(right);
}

bool operator<=(TokenType left, TokenType right) {
	return int(left) <= int(right);
}


Token::Token(TokenType type, const String& string, uint64 column) : type(type), string(string), column(column) { }
Token::Token(TokenType type, uint64 value, const String& string, uint64 column) : type(type), value(value), string(string), column(column) { }
Token::Token(TokenType type, const String& string, const Line& line, uint64 column) : type(type), string(string), line(line), column(column) { }
Token::Token(TokenType type, uint64 value, const String& string, const Line& line, uint64 column) : type(type), value(value), string(string), line(line), column(column) { }
Token::Token(const Token& other) : type(other.type), value(other.value), string(other.string), line(other.line), column(other.column) { }
Token::Token(const Token* other) : type(other->type), value(other->value), string(other->string), line(other->line), column(other->column) { }
Token::Token(Token&& other) {
	type = other.type;
	value = other.value;
	column = other.column;
	string = std::move(other.string);
	line = std::move(other.line);
}

Token& Token::operator=(const Token& other) {
	if (this != &other) {
		type = other.type;
		value = other.value;
		column = other.column;
		string = other.string;
		line = other.line;
	}

	return *this;
}

Token& Token::operator=(Token&& other) {
	if (this != &other) {
		type = other.type;
		value = other.value;
		column = other.column;
		string = std::move(other.string);
		line = std::move(other.line);
	}

	return *this;
}

bool Token::ValidReturnType(TokenType type) {
	switch (type) {
		case TokenType::TypeVoid:
		case TokenType::TypeBool:
		case TokenType::TypeInt:
		case TokenType::TypeFloat:
		case TokenType::TypeVec:
		case TokenType::TypeMat:
			return true;
	}

	return false;
}

bool Token::ValidInOutType(TokenType type) {
	switch (type) {
		case TokenType::TypeInt:
		case TokenType::TypeFloat:
		case TokenType::TypeVec:
		case TokenType::TypeMat:
			return true;
	}

	return false;
}

}
}
}