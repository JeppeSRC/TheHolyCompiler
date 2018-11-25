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

#include "compiler.h"

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;

Compiler::Token::Token(Compiler::TokenType type, const String& string, const Line& line, uint64 column) : type(type), string(string), line(line), column(column) { }
Compiler::Token::Token(Compiler::TokenType type, uint64 value, const String& string, const Line& line, uint64 column) : type(type), value(value), string(string), line(line), column(column) { }
Compiler::Token::Token(const Token& other) : type(other.type), value(other.value), string(other.string), line(other.line), column(other.column) { }
Compiler::Token::Token(const Token* other) : type(other->type), value(other->value), string(other->string), line(other->line), column(other->column) { }
Compiler::Token::Token(Token&& other) {
	type = other.type;
	value = other.value;
	column = other.column;
	string = std::move(other.string);
	line = std::move(other.line);
}

Compiler::Token& Compiler::Token::operator=(const Compiler::Token& other) {
	if (this != &other) {
		type = other.type;
		value = other.value;
		column = other.column;
		string = other.string;
		line = other.line;
	}

	return *this;
}

Compiler::Token& Compiler::Token::operator=(Compiler::Token&& other) {
	if (this != &other) {
		type = other.type;
		value = other.value;
		column = other.column;
		string = std::move(other.string);
		line = std::move(other.line);
	}

	return *this;
}

}
}
}