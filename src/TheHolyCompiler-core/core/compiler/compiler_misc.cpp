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

#include "compiler.h"

namespace thc {
namespace core {
namespace compiler {

using namespace utils;

bool Compiler::IsCharAllowedInName(const char c, bool first) const {
	if (c >= 'A' && c <= 'Z') return true;
	else if (c >= 'a' && c <= 'z') return true;
	else if (c == '_' || (c >= '0' && c <= '9' && !first)) return true;

	return false;
}

bool Compiler::IsCharWhitespace(const char c) const {
	return (c == ' ' || c == '\n' || c == '\t');
}


void Compiler::ProcessName(Compiler::Token& t) const {
	struct TokenProperties {
		String name;
		TokenType type;
		uint8 bits;
		uint8 rows;
		uint8 columns;
	};

	static TokenProperties props[]{
		{"if",       TokenType::ControlFlowIf, 0, 0, 0},
		{"switch",   TokenType::ControlFlowSwitch, 0, 0, 0},
		{"else",     TokenType::ControlFlowElse, 0, 0, 0},
		{"for",      TokenType::ControlFlowFor, 0, 0, 0},
		{"while",    TokenType::ControlFlowWhile, 0, 0, 0},
		{"break",    TokenType::ControlFlowBreak, 0, 0, 0},
		{"continue", TokenType::ControlFlowContinue, 0, 0, 0},
		{"return",   TokenType::ControlFlowReturn, 0, 0, 0},

		{"struct",   TokenType::DataStruct, 0, 0, 0},
		{"layout",   TokenType::DataLayout, 0, 0, 0},
		{"in",       TokenType::DataIn, 0, 0, 0},
		{"out",      TokenType::DataOut, 0, 0, 0},
		{"uniform",  TokenType::DataUniform, 0, 0, 0},

		{"void", TokenType::TypeVoid, 0, 0, 0},
		{"bool", TokenType::TypeBool, 8, 0, 0},
		{"byte", TokenType::TypeByte, 8, 0, 0},

		{"uint8",  TokenType::TypeUint, 8,  0, 0},
		{"uint16", TokenType::TypeUint, 16, 0, 0},
		{"uint32", TokenType::TypeUint, 32, 0, 0},
		{"uint64", TokenType::TypeUint, 64, 0, 0},

		{"int8",  TokenType::TypeInt, 8,  0, 0},
		{"int16", TokenType::TypeInt, 16, 0, 0},
		{"int32", TokenType::TypeInt, 32, 0, 0},
		{"int64", TokenType::TypeInt, 64, 0, 0},

		{"float32",  TokenType::TypeFloat, 32, 0, 0},
		{"float64",  TokenType::TypeFloat, 64, 0, 0},

		{"vec2", TokenType::TypeVec, 0, 0, 2},
		{"vec3", TokenType::TypeVec, 0, 0, 3},
		{"vec4", TokenType::TypeVec, 0, 0, 4},

		{"mat3", TokenType::TypeMat, 0, 3, 3},
		{"mat4", TokenType::TypeMat, 0, 4, 4},
	};

	for (uint64 i = 0; i < sizeof(props) / sizeof(TokenProperties); i++) {
		const TokenProperties& tmp = props[i];
		if (t.string == tmp.name) {
			t.type = tmp.type;
			t.bits = tmp.bits;
			t.rows = tmp.rows;
			t.column = tmp.columns;

			break;
		}
	}

}

}
}
}