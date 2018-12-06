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
#include <util/log.h>
#include <util/utils.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace instruction;
using namespace type;

InstBase* Compiler::GetInstFromID(uint32 id)  {
	auto cmp = [](InstBase* const& curr, const uint32& id) -> bool {
		return curr->id == id;
	};

	uint64 index = instructions.Find<uint32>(id, cmp);

	if (index != ~0) {
		return instructions[index];
	}
	
	return nullptr;
}
void Compiler::CheckTypeExists(InstTypeBase** type) {
	auto cmp = [](InstBase* const& curr, InstTypeBase* const& type) -> bool {
		if (curr->type != InstType::Type) return false;

		InstTypeBase* t = (InstTypeBase*)curr;

		if (t->type == type->type) {
			return *t == type;
		}
		
		return false;
	};

	uint64 index = types.Find<InstTypeBase*>(*type, cmp);

	if (index == ~0) {
		types.Add(*type);
	} else {
		delete *type;

		*type = (InstTypeBase*)types[index];
	}
}

Compiler::TypePrimitive* Compiler::CreateTypePrimitive(const List<Token>& tokens, uint64 start) {
	TypePrimitive* var = new TypePrimitive;

	const Token& token = tokens[start];

	if (!Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeVec, TokenType::TypeMat)) {
		Log::CompilerError(token, "Unexpecet symbol \"%s\" expected a valid type", token.string.str);
		return nullptr;
	}

	const Token& open = tokens[start + 1];

	if (open.type == TokenType::OperatorLess) {
		const Token& type = tokens[start + 2];

		if (!Utils::CompareEnums(type.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt)) {
			Log::CompilerError(type, "Unexpected symbol \"%s\" expected a valid type", type.string.str);
			return nullptr;
		}

		var->componentType = ConvertToType(type.type);
		var->bits = type.bits;
		var->sign = type.sign;
	} else {
		var->componentType = type::Type::Float;;
		var->bits = 32;
		var->sign = 0;
	}

	var->type = ConvertToType(token.type);
	var->dataType = ConvertToType(token.type);
	var->rows = token.rows;
	var->columns = token.columns;
	var->name = GetTypeString(var);
	

	InstTypeBase* t = nullptr;

	switch (var->type) {
		case type::Type::Int:
			t = new InstTypeInt(var->bits, var->sign);
			break;
		case type::Type::Float:
			t = new InstTypeFloat(var->bits);
			break;
		case type::Type::Vector:
		{
			InstTypeBase* tt = nullptr;

			switch (var->componentType) {
				case type::Type::Float:
					tt = new InstTypeFloat(var->bits);
					break;
				case type::Type::Int:
					tt = new InstTypeInt(var->bits, var->sign);
					break;
			}

			CheckTypeExists(&tt);

			t = new InstTypeVector(var->columns, tt->id);

		}
			break;
		case type::Type::Matrix:
		{
			InstTypeBase* tt = nullptr;

			switch (var->componentType) {
				case type::Type::Float:
					tt = new InstTypeFloat(var->bits);
					break;
				case type::Type::Int:
					tt = new InstTypeInt(var->bits, var->sign);
					break;
			}

			CheckTypeExists(&tt);

			uint32 compId = tt->id;

			tt = new InstTypeVector(var->rows, compId);

			CheckTypeExists(&tt);

			compId = tt->id;

			t = new InstTypeMatrix(var->columns, compId);
		}

	}

	CheckTypeExists(&t);

	return var;
}

Compiler::TypeStruct* Compiler::CreateTypeStruct(const List<Token>& tokens, uint64 start) {
	auto findStructFunc = [](Type* const& curr, const String& name) -> bool {
		if (curr->type == type::Type::Struct) {
			return curr->name == name;
		}

		return false;
	};

	TypeStruct* var = new TypeStruct;

	uint64 offset = 0;

	const Token& name = tokens[start + offset++];

	if (name.type != TokenType::Name) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected valid name", name.string.str);
		return nullptr;
	}

	const Token& bracket = tokens[start + offset++];

	if (bracket.type != TokenType::CurlyBracketOpen) {
		Log::CompilerError(bracket, "Unexpected symbol \"%s\" expected \"{\"", bracket.string.str);
		return nullptr;
	}

	List<uint32> ids;

	while (true) {
		const Token& type = tokens[start + offset++];

		Type* tmp = nullptr;

		if (Utils::CompareEnums(type.type, CompareOperation::Or, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeVec, TokenType::TypeMat)) {
			uint64 typeLocation = start + offset - 1;

			if (tokens[start + offset].type == TokenType::OperatorLess) {
				offset += 3;
			}

			tmp = CreateTypePrimitive(tokens, typeLocation);
		} else {
			uint64 index = typeDefinitions.Find<String>(type.string, findStructFunc);

			if (index == ~0) {
				Log::CompilerError(type, "Unexpected symbol \"%s\" expected valid type", type.string.str);
				return nullptr;
			}

			tmp = typeDefinitions[index];
		}

		const Token& tokenName = tokens[start + offset++];

		if (tokenName.type != TokenType::Name) {
			Log::CompilerError(tokenName, "Unexpected symbol \"%s\" expected valid name", tokenName.string.str);
			return nullptr;
		}

		const Token& semi = tokens[start + offset++];

		if (semi.type != TokenType::SemiColon) {
			Log::CompilerError(semi, "Unexpected symbol \"%s\" expected \";\"", semi.string.str);
			return;
		}

		tmp->name = tokenName.string;

		var->members.Add(tmp);

		ids.Add(tmp->typeId);

		if (tokens[start + offset].type == TokenType::CurlyBracketClose) {
			const Token& t = tokens[start + ++offset];

			if (t.type == TokenType::SemiColon) {
				break;
			} else {
				Log::CompilerError(t, "Unexpected symbol \"%s\" expected \";\"", t.string.str);
			}
		}
	}

	InstTypeStruct* st = new InstTypeStruct(ids.GetCount(), ids.GetData());

	var->type = type::Type::Struct;
	var->name = name.string;
	
	if (typeDefinitions.Find<String>(var->name, findStructFunc) == ~0) {
		typeDefinitions.Add(var);
	} else {
		delete var;
		Log::CompilerError(name, "Struct redefinition");
		return nullptr;
	}

	return var;
}

String Compiler::GetTypeString(const Type* type) const {
	String name;

	const TypeArray* arr = (const TypeArray*)type;
	const TypePrimitive* prim = (const TypePrimitive*)type;

	switch (type->type) {
		case type::Type::Void:
			name = "void";
			break;
		case type::Type::Bool:
			name = "bool";
			break;
		case type::Type::Int:
			switch (prim->sign) {
				case 0:
					name = "uint";
					break;
				default:
					name = "int";
			}


			switch (prim->bits) {
				case 8:
					name.Append("8");
					break;
				case 16:
					name.Append("16");
					break;
				case 32:
					name.Append("32");
					break;
				case 64:
					name.Append("64");
					break;
			}
			break;
		case type::Type::Float:
			name = "float";
			switch (prim->bits) {
				case 32:
					name.Append("32");
					break;
				case 64:
					name.Append("64");
					break;
			}
			break;
		case type::Type::Vector:
			name = "vec";

			switch (prim->columns) {
				case 2:
					name.Append("2");
					break;
				case 3:
					name.Append("3");
					break;
				case 4:
					name.Append("4");
					break;
			}

			name.Append("_");

			switch (prim->dataType) {
				case type::Type::Int:
					switch (prim->sign) {
						case 0:
							name.Append("uint");
							break;
						default:
							name.Append("int");
					}


					switch (prim->bits) {
						case 8:
							name.Append("8");
							break;
						case 16:
							name.Append("16");
							break;
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
				case type::Type::Float:
					name.Append("float");
					switch (prim->bits) {
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
			}

			break;

		case type::Type::Matrix:
			name = "mat";

			switch (prim->columns) {
				case 3:
					name.Append("3");
					break;
				case 4:
					name.Append("4");
					break;
			}

			name.Append("_");

			switch (prim->dataType) {
				case type::Type::Int:
					switch (prim->sign) {
						case 0:
							name.Append("uint");
							break;
						default:
							name.Append("int");
					}


					switch (prim->bits) {
						case 8:
							name.Append("8");
							break;
						case 16:
							name.Append("16");
							break;
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
				case type::Type::Float:
					name.Append("float");
					switch (prim->bits) {
						case 32:
							name.Append("32");
							break;
						case 64:
							name.Append("64");
							break;
					}
					break;
			}

			break;
	}

	return name;
}

bool Compiler::IsCharAllowedInName(const char c, bool first) const {
	if (c >= 'A' && c <= 'Z') return true;
	else if (c >= 'a' && c <= 'z') return true;
	else if (c == '_' || (c >= '0' && c <= '9' && !first)) return true;

	return false;
}

bool Compiler::IsCharWhitespace(const char c) const {
	return (c == ' ' || c == '\n' || c == '\t');
}


void Compiler::ProcessName(Token& t) const {
	struct TokenProperties {
		String name;
		TokenType type;
		uint8 bits;
		uint8 sign;
		uint8 rows;
		uint8 columns;
	};

	static TokenProperties props[]{
		{"if",       TokenType::ControlFlowIf, 0, 0, 0, 0},
		{"switch",   TokenType::ControlFlowSwitch, 0, 0, 0, 0},
		{"else",     TokenType::ControlFlowElse, 0, 0, 0, 0},
		{"for",      TokenType::ControlFlowFor, 0, 0, 0, 0},
		{"while",    TokenType::ControlFlowWhile, 0, 0, 0, 0},
		{"break",    TokenType::ControlFlowBreak, 0, 0, 0, 0},
		{"continue", TokenType::ControlFlowContinue, 0, 0, 0, 0},
		{"return",   TokenType::ControlFlowReturn, 0, 0, 0, 0},

		{"struct",   TokenType::DataStruct, 0, 0, 0, 0},
		{"layout",   TokenType::DataLayout, 0, 0, 0, 0},
		{"in",       TokenType::DataIn, 0, 0, 0, 0},
		{"out",      TokenType::DataOut, 0, 0, 0, 0},
		{"uniform",  TokenType::DataUniform, 0, 0, 0, 0},

		{"void", TokenType::TypeVoid, 0, 0, 0, 0},
		{"bool", TokenType::TypeBool, 8, 0, 0, 0},
		{"byte", TokenType::TypeInt, 8, 0, 0, 0},

		{"uint8",  TokenType::TypeInt, 8,  0, 0, 0},
		{"uint16", TokenType::TypeInt, 16, 0, 0, 0},
		{"uint32", TokenType::TypeInt, 32, 0, 0, 0},
		{"uint64", TokenType::TypeInt, 64, 0, 0, 0},

		{"int8",  TokenType::TypeInt, 8,  1, 0, 0},
		{"int16", TokenType::TypeInt, 16, 1, 0, 0},
		{"int32", TokenType::TypeInt, 32, 1, 0, 0},
		{"int64", TokenType::TypeInt, 64, 1, 0, 0},

		{"float32",  TokenType::TypeFloat, 32, 0, 0, 0},
		{"float64",  TokenType::TypeFloat, 64, 0, 0, 0},

		{"vec2", TokenType::TypeVec, 0, 0, 0, 2},
		{"vec3", TokenType::TypeVec, 0, 0, 0, 3},
		{"vec4", TokenType::TypeVec, 0, 0, 0, 4},

		{"mat3", TokenType::TypeMat, 0, 0, 3, 3},
		{"mat4", TokenType::TypeMat, 0, 0, 4, 4},
	};

	for (uint64 i = 0; i < sizeof(props) / sizeof(TokenProperties); i++) {
		const TokenProperties& tmp = props[i];
		if (t.string == tmp.name) {
			t.type = tmp.type;
			t.bits = tmp.bits;
			t.sign = tmp.sign;
			t.rows = tmp.rows;
			t.columns = tmp.columns;

			break;
		}
	}

}

}
}
}