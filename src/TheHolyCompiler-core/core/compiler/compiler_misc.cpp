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
void Compiler::CheckTypeExist(InstTypeBase** type) {
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

void Compiler::CheckTypeExist(TypeBase** type) {
	auto cmp = [](TypeBase* const& curr, TypeBase* const& type) -> bool {
		return *curr == type;
	};

	uint64 index = typeDefinitions.Find<TypeBase*>(*type, cmp);

	if (index == ~0) {
		typeDefinitions.Add(*type);
	} else {
		delete *type;

		*type = (TypeBase*)typeDefinitions[index];
	}
}

void Compiler::CheckConstantExist(InstBase** constant) {
	uint64 index = types.Find<InstBase*>(*constant, [](InstBase* const& curr, InstBase* const& inst) -> bool {
		return *curr == inst;
	});

	if (index == ~0) {
		types.Add(*constant);
	} else {
		delete *constant;

		*constant = types[index];
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

	if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeVec, TokenType::TypeMat)) {
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
			var->componentType = Type::Float;;
			var->bits = 32;
			var->sign = 0;
		}
	} else {
		var->componentType = ConvertToType(token.type);
		var->bits = token.bits;
		var->sign = token.sign;
	}
	

	var->type = ConvertToType(token.type);
	var->rows = token.rows;
	var->columns = token.columns;
	var->typeString = GetTypeString(var);
	
	uint64 index = typeDefinitions.Find<TypePrimitive*>(var, [](TypeBase* const& curr, TypePrimitive* const& other) -> bool {
		return *curr == other;
	});


	if (index != ~0) {
		delete var;
		var = (TypePrimitive*)typeDefinitions[index];
	} else {
		InstTypeBase* t = nullptr;

		switch (var->type) {
			case Type::Int:
				t = new InstTypeInt(var->bits, var->sign);
				break;
			case Type::Float:
				t = new InstTypeFloat(var->bits);
				break;
			case Type::Vector:
			{
				InstTypeBase* tt = nullptr;

				switch (var->componentType) {
					case Type::Float:
						tt = new InstTypeFloat(var->bits);
						break;
					case Type::Int:
						tt = new InstTypeInt(var->bits, var->sign);
						break;
				}

				CheckTypeExist(&tt);

				t = new InstTypeVector(var->rows, tt->id);

			}
			break;
			case type::Type::Matrix:
			{
				InstTypeBase* tt = nullptr;

				switch (var->componentType) {
					case Type::Float:
						tt = new InstTypeFloat(var->bits);
						break;
					case Type::Int:
						tt = new InstTypeInt(var->bits, var->sign);
						break;
				}

				CheckTypeExist(&tt);

				uint32 compId = tt->id;

				tt = new InstTypeVector(var->rows, compId);

				CheckTypeExist(&tt);

				compId = tt->id;

				t = new InstTypeMatrix(var->columns, compId);
			}

		}

		CheckTypeExist(&t);

		var->typeId = t->id;

		typeDefinitions.Add(var);
	}

	return var;
}

Compiler::TypeStruct* Compiler::CreateTypeStruct(const List<Token>& tokens, uint64 start) {
	auto findStructFunc = [](TypeBase* const& curr, const String& name) -> bool {
		if (curr->type == Type::Struct) {
			return curr->typeString == name;
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

		TypeBase* tmp = nullptr;

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

		uint64 index = var->members.Find<String>(tokenName.string, [](TypeBase* const& curr, const String& name) -> bool {
			if (curr->name == name) {
				return true;
			}

			return false;
		});

		if (index != ~0) {
			Log::CompilerError(tokenName, "There is already a member callad \"%s\" in \"%s\"", tokenName.string.str, name.string.str);
			return nullptr;
		}

		const Token& semi = tokens[start + offset++];

		if (semi.type != TokenType::SemiColon) {
			Log::CompilerError(semi, "Unexpected symbol \"%s\" expected \";\"", semi.string.str);
			return nullptr;
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

	InstTypeStruct* st = new InstTypeStruct((uint32)ids.GetCount(), ids.GetData());

	var->type = Type::Struct;
	var->typeString = name.string;
	
	if (typeDefinitions.Find<String>(var->name, findStructFunc) == ~0) {
		typeDefinitions.Add(var);
	} else {
		delete var;
		Log::CompilerError(name, "Struct redefinition");
		return nullptr;
	}

	return var;
}

Compiler::TypeArray* Compiler::CreateTypeArray(const List<Token>& tokens, uint64 start, uint64 size) {

}

String Compiler::GetTypeString(const TypeBase* const type) const {
	String name;

	const TypeArray* arr = (const TypeArray*)type;
	const TypePrimitive* prim = (const TypePrimitive*)type;

	switch (type->type) {
		case Type::Void:
			name = "void";
			break;
		case Type::Bool:
			name = "bool";
			break;
		case Type::Int:
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
		case Type::Float:
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

		case Type::Vector:
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

			switch (prim->componentType) {
				case Type::Int:
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
				case Type::Float:
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

		case Type::Matrix:
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

			switch (prim->componentType) {
				case Type::Int:
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
				case Type::Float:
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

uint32 Compiler::CreateConstant(const TypeBase* const type, uint32 value) {
	if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
		Log::Error("Can't create Constant from a composite \"%s\"", type->typeString);
		return ~0;
	}

	InstConstant* constant = new InstConstant(type->typeId, value);

	CheckConstantExist((InstBase**)&constant);

	return constant->id;
}

uint32 Compiler::CreateConstant(const TypeBase* const type, float32 value) {
	return CreateConstant(type, *(uint32*)&value);
}

uint32 Compiler::CreateConstantComposite(const TypeBase* const type, const List<uint32>& values) {
	const uint32* v = values.GetData();

	return CreateConstantComposite(type, &v);
}

uint32 Compiler::CreateConstantComposite(const TypeBase* const type, const uint32** values) {
	if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Vector, Type::Matrix, Type::Array, Type::Struct)) {
		Log::Error("Can't create ConstantComposite from a non composite \"%s\"", type->typeString);
		return ~0;
	}

	uint32 id;

	switch (type->type) {
		case Type::Vector:
			id = CreateConstantCompositeVector(type, values);
			break;
		case Type::Matrix:
			id = CreateConstantCompositeMatrix(type, values);
			break;
		case Type::Array:
			id = CreateConstantCompositeArray(type, values);
			break;
		case Type::Struct:
			id = CreateConstantCompositeStruct(type, values);
			break;

	}

	return id;
}

uint32 Compiler::CreateConstantCompositeVector(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<uint32> ids;

	TypePrimitive* p = new TypePrimitive;
	
	p->type = prim->componentType;
	p->componentType = prim->componentType;
	p->bits = prim->bits;
	p->rows = prim->rows;
	p->columns = prim->columns;
	p->typeString = GetTypeString(p);
	p->typeId = ~0;

	CheckTypeExist((TypeBase**)&p);

	for (uint8 i = 0; i < prim->rows; i++) {
		ids.Add(CreateConstant(p, (*values)[i]));
	}

	*values += prim->columns;

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, prim->rows, ids.GetData());

	CheckConstantExist((InstBase**)&composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeMatrix(const TypeBase* const type, const uint32** values) {
	const TypePrimitive* prim = (const TypePrimitive*)type;

	List<uint32> ids;

	TypePrimitive* p = new TypePrimitive;

	p->type = Type::Vector;
	p->componentType = prim->componentType;
	p->bits = prim->bits;
	p->rows = prim->rows;
	p->columns = prim->columns;
	p->typeString = GetTypeString(p);
	p->typeId = ~0;

	CheckTypeExist((TypeBase**)&p);

	for (uint8 i = 0; i < p->rows; i++) {
		ids.Add(CreateConstantCompositeVector(p, values));
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, p->rows, ids.GetData());

	CheckConstantExist((InstBase**)&composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeArray(const TypeBase* const type, const uint32** values) {
	const TypeArray* arr = (const TypeArray*)type;

	List<uint32> ids;

	if (Utils::CompareEnums(arr->elementType->type, CompareOperation::Or, Type::Vector, Type::Matrix, Type::Struct)) {
		for (uint32 i = 0; i < arr->elementCount; i++) {
			ids.Add(CreateConstantComposite(arr->elementType, values));
		}
	} else {
		for (uint32 i = 0; i < arr->elementCount; i++) {
			ids.Add(CreateConstant(arr->elementType, (*values)[i]));
		}

		*values += arr->elementCount;
	}

	InstConstantComposite* composite = new InstConstantComposite(type->typeId, arr->elementCount, ids.GetData());

	CheckConstantExist((InstBase**)composite);

	return composite->id;
}

uint32 Compiler::CreateConstantCompositeStruct(const TypeBase* const type, const uint32** values) {

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

		{"vec2", TokenType::TypeVec, 0, 0, 2, 0},
		{"vec3", TokenType::TypeVec, 0, 0, 3, 0},
		{"vec4", TokenType::TypeVec, 0, 0, 4, 0},

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