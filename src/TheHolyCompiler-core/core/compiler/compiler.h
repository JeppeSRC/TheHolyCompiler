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
#include <core/compiler/parsing/token.h>
#include <core/compiler/type/types.h>
#include "options.h"
#include "idmanager.h"

namespace thc {
namespace core {
namespace compiler {

class Compiler {
private:

	enum class VariableScope {
		Unknown,
		Uniform, 
		In,
		Out,
		Global
	};

	enum class VariableType {
		Unknown,
		Primitive,
		Struct,
		Array
	};

	struct Variable {
		VariableScope scope;
		VariableType type;
		utils::String name;

		uint32 id; // in VariableStruct id = the location in the struct. In all other cases it's the id of the OpVariable
	};

	struct VariablePrimitive : public Variable {
		parsing::TokenType dataType;
		parsing::TokenType componentType;

		uint32 bits;
		uint32 rows;
		uint32 columns;
	};

	struct VariableStruct : public Variable {
		utils::List<Variable*> members;
	};

	struct VariableArray : public Variable {
		uint32 elementCount;
		Variable elementType;
	};

	utils::List<Variable*> variables;

	
	utils::List<instruction::InstBase*> debugInstructions;
	utils::List<instruction::InstBase*> annotationIstructions;
	utils::List<instruction::InstBase*> types;
	utils::List<instruction::InstBase*> instructions;

	instruction::InstBase* GetInstFromID(uint32 id);
	void CheckTypeExists(type::TypeBase** type); //Returns true if it existed

	VariablePrimitive* CreateVariablePrimitive(const utils::String& name, const utils::List<parsing::Token>& tokens, uint64 start, VariableScope scope);

private:
	bool IsCharAllowedInName(const char c, bool first = true) const;
	bool IsCharWhitespace(const char c) const;
	void ProcessName(parsing::Token& t) const;

private:
	utils::String code;
	utils::String filename;
	utils::List<parsing::Line> lines;
	utils::List<utils::String> defines;
	utils::List<utils::String> includes;

	utils::List<parsing::Token> Tokenize();
	void ParseTokens(utils::List<parsing::Token>& tokens);
	void ParseFunction(utils::List<parsing::Token>& tokens, uint64 start);

	bool Process();

	Compiler(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
public:
	static bool Run(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
	static bool Run(const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);

};

}
}
}