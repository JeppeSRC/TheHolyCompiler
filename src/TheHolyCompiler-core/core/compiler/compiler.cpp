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
#include <util/utils.h>
#include <core/preprocessor/preprocessor.h>
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;
using namespace instruction;

bool TokenTypeCmpFunc(const Token& token, const TokenType& type) {
	return token.type == type;
}

auto  CmpFunc = [](const Token& curr, const TokenType& c) -> bool {
	return curr.type == c;
};

List<Token> Compiler::Tokenize() {
	List<Token> tokens;

	for (uint64 i = 0; i < lines.GetCount(); i++) {
		const Line& l = lines[i];
		const String& line = l.string;

		for (uint64 j = 0; j < line.length; j++) {
			const char c0 = line[j];
			const char c1 = j < line.length-1 ? line[j+1] : 0;

			if (IsCharWhitespace(c0)) {
				continue;
			} else if (c0 == '(') {
				tokens.Emplace(TokenType::ParenthesisOpen, "(", l, j+1);
			} else if (c0 == ')') {
				tokens.Emplace(TokenType::ParenthesisClose, ")", l, j+1);
			} else if (c0 == '{') {
				tokens.Emplace(TokenType::CurlyBracketOpen, "{", l, j+1);
			} else if (c0 == '}') {
				tokens.Emplace(TokenType::CurlyBracketClose, "}", l, j+1);
			} else if (c0 == '[') {
				tokens.Emplace(TokenType::BracketOpen, "[", l, j+1);
			} else if (c0 == ']') {
				tokens.Emplace(TokenType::BracketClose, "]", l, j+1);
			} else if (c0 == ';') {
				tokens.Emplace(TokenType::SemiColon, 0, ";", l, j+1);
			} else if (c0 == '+' && c1 == '+') {
				tokens.Emplace(TokenType::OperatorIncrement, "++", l, ++j);
			} else if (c0 == '-' && c1 == '-') {
				tokens.Emplace(TokenType::OperatorDecrement, "--", l, ++j);
			} else if (c0 == '+' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundAdd, "+=", l, ++j);
			} else if (c0 == '-' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundSub, "-=", l, ++j);
			} else if (c0 == '*' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundMul, "*=", l, ++j);
			} else if (c0 == '/' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorCompoundDiv, "/=", l, ++j);
			} else if (c0 == '<' && c1 == '<') {
				tokens.Emplace(TokenType::OperatorLeftShift, "<<", l, ++j);
			} else if (c0 == '>' && c1 == '>') {
				tokens.Emplace(TokenType::OperatorRightShift, ">>", l, ++j);
			} else if (c0 == '<' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorLessEqual, "<=", l, ++j);
			} else if (c0 == '>' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorGreaterEqual, ">=", l, ++j);
			} else if (c0 == '&' && c1 == '&') {
				tokens.Emplace(TokenType::OperatorLogicalAnd, "&&", l, ++j);
			} else if (c0 == '|' && c1 == '|') {
				tokens.Emplace(TokenType::OperatorLogicalOr, "||", l, ++j);
			} else if (c0 == '=' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorEqual, "==", l, ++j);
			} else if (c0 == '!' && c1 == '=') {
				tokens.Emplace(TokenType::OperatorNotEqual, "!=", l, ++j);
			} else if (c0 == '+') {
				tokens.Emplace(TokenType::OperatorAdd, "+", l, j+1);
			} else if (c0 == '*') {
				tokens.Emplace(TokenType::OperatorMul, "*", l, j+1);
			} else if (c0 == '/') {
				tokens.Emplace(TokenType::OperatorDiv, "/", l, j+1);
			} else if (c0 == '<') {
				tokens.Emplace(TokenType::OperatorLess, "<", l, j+1);
			} else if (c0 == '>') {
				tokens.Emplace(TokenType::OperatorGreater, ">", l, j+1);
			} else if (c0 == '!') {
				tokens.Emplace(TokenType::OperatorLogicalNot, "!", l, j+1);
			} else if (c0 == '&') {
				tokens.Emplace(TokenType::OperatorBitwiseAnd, "&", l, j+1);
			} else if (c0 == '|') {
				tokens.Emplace(TokenType::OperatorBitwiseOr, "|", l, j+1);
			} else if (c0 == '~') {
				tokens.Emplace(TokenType::OperatorBitwiseNot, "~", l, j+1);
			} else if (c0 == '^') {
				tokens.Emplace(TokenType::OperatorBitwiseXor, "^", l, j+1);
			} else if (c0 == '?') {
				tokens.Emplace(TokenType::OperatorTernary1, "?", l, j+1);
			} else if (c0 == ':') {
				tokens.Emplace(TokenType::OperatorTernary2, ":", l, j+1);
			} else if (c0 == '.') {
				tokens.Emplace(TokenType::OperatorSelector, ".", l, j+1);
			} else if (c0 == ',') {
				tokens.Emplace(TokenType::Comma, ",", l, j+1);
			} else if (c0 == '=') {
				tokens.Emplace(TokenType::OperatorAssign, "=", l, j+1);
			} else if (c0 >= '0' && c0 <= '9') {
				uint64 len = 0;
				
				ValueResult res = Utils::StringToValue(line.str+j-1, &len, l, j);

				Token tmp(TokenType::Value, res.value, line.SubString(j - (line[j - 1] == '-' ? 1 : 0), j + len - 1), l, j);

				switch (res.type) {
					case ValueResultType::Float:
						tmp.valueType = TokenType::TypeFloat;
						break;
					case ValueResultType::Int:
						tmp.valueType = TokenType::TypeInt;
						tmp.sign = res.sign;
				}

				tokens.Emplace(tmp);

				j += len-1;

			} else if (c0 == '-') {
				tokens.Emplace(TokenType::OperatorSub, "-", l, j+1);
			} else {
				uint64 end = ~0;

				for (uint64 c = j; c < line.length; c++) {
					if (!IsCharAllowedInName(line[c], c == j ? true : false)) {
						end = c;
						break;
					}
				}

				if (end == j) {
					Log::CompilerError(l, j, "Unexpect symbol \"%c\"", c0);
				}

				tokens.Emplace(TokenType::Name, 0, line.SubString(j, end-1), l, j+1);

				j = end-1;
			}
		}
	}

	for (uint64 i = 0; i < tokens.GetCount(); i++) {
		Token& t = tokens[i];

		if (t.type == TokenType::Name) {
			ProcessName(t);
		} else if (t.type == TokenType::OperatorSub) {
			if (i == 0) {
				t.type = TokenType::OperatorNegate;
			} else {
				const Token& left = tokens[i-1];
				const Token& right = tokens[i+1];

				if ((right.type == TokenType::Value || right.type == TokenType::Name) && (left.type != TokenType::Name || left.type != TokenType::Value)) {
					t.type = TokenType::OperatorNegate;
				}
			}
		}
	}

	return tokens;
}

void Compiler::ParseTokens(List<Token>& tokens) {
	for (uint64 i = 0; i < tokens.GetCount(); i++) {
		const Token& token = tokens[i];

		if (token.type == TokenType::DataLayout) {
			ParseLayout(tokens, i--);
		} else if (token.type == TokenType::DataIn) {
			ParseInOut(tokens, i--, VariableScope::In);
		} else if (token.type == TokenType::DataOut) {
			ParseInOut(tokens, i--, VariableScope::Out);
		} else if (token.type == TokenType::DataStruct) {
			CreateTypeStruct(tokens, i + 1, nullptr);
			tokens.RemoveAt(i--);
		} else if (token.type == TokenType::Name) {
			const Token& t2 = tokens[i + 1];
			if (t2.type == TokenType::ParenthesisOpen) {
				ParseFunction(tokens, --i);
				i--;
			} else {
				TypeBase* type = CreateType(tokens, i - 1, nullptr);

				Symbol* var = CreateGlobalVariable(type, VariableScope::Private, token.string);
				var->variable.isConst = tokens[i - 1].type == TokenType::ModifierConst;

				if (t2.type == TokenType::SemiColon) {
					continue;
				} else if (t2.type == TokenType::OperatorAssign) {
					 //TODO:
				}
			} 
		}
	}
}

void Compiler::ParseBody(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start, VariableStack* localVariables) {
	uint64 closeBracket = ~0;

	localVariables->PushStack(); //New stack frame

	for (uint64 i = start; i < tokens.GetCount(); i++) {
		const Token& token = tokens[i];

		if (token.type == TokenType::CurlyBracketClose) {
			//end of function
			closeBracket = i;
			break;
		} else if (Utils::CompareEnums(token.type, CompareOperation::Or, TokenType::TypeBool, TokenType::TypeFloat, TokenType::TypeInt, TokenType::TypeMatrix, TokenType::TypeVector)) {
			//variable declaration
			TypeBase* t = CreateType(tokens, i, nullptr);

			const Token& name = tokens[i];

			if (name.type != TokenType::Name) {
				Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
			}

			if (!localVariables->CheckName(name)) { }

			CreateLocalVariable(t, name.string, localVariables);

			const Token& next = tokens[i + 1];

			if (next.type == TokenType::SemiColon) {
				tokens.Remove(i, i + 1);
			}

			i--;
			
		} else if (token.type == TokenType::Name) {
			const Token& next = tokens[i + 1];

			uint64 index = typeDefinitions.Find<String>(token.string, findStructFunc);

			if (next.type == TokenType::ParenthesisOpen) {
				ParseInfo inf;
				inf.start = i;

				ParseFunctionCall(tokens, &inf, localVariables);

				tokens.Remove(i, i + inf.len - 1);
			} else if (index != ~0) {
				TypeStruct* str = (TypeStruct*)typeDefinitions[index];
				tokens.RemoveAt(i);

				const Token& name = tokens[i];

				if (name.type != TokenType::Name) {
					Log::CompilerError(name, "Unexpected symbol \"%s\" expected a valid name", name.string.str);
				}

				if (!localVariables->CheckName(name)) { }

				CreateLocalVariable(str, name.string, localVariables);

				const Token& assign = tokens[i + 1];

				if (assign.type == TokenType::SemiColon) {
					tokens.Remove(i, i-- + 1);
				}  else {
					Log::CompilerError(assign, "Unexpected symbol \"%s\" expected \";\"", assign.string.str);
				}
			} else {
				ParseInfo inf;
				inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, i);

				if (inf.end-- == ~0) {
					Log::CompilerError(token, "Expression is missing \";\"");
				}

				inf.start = Utils::CompareEnums(tokens[i - 1].type, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement) ? i - 1 : i;

				ParseExpression(tokens, &inf, localVariables);

				i = inf.end+1;
			}
		} else if (token.type == TokenType::ControlFlowReturn) {
			const Token& next = tokens[++i];

			InstBase* operation = nullptr;

			bool returnVoid = declaration->returnType->type == Type::Void;

			if (next.type == TokenType::SemiColon) {
				if (!returnVoid) {
					Log::CompilerError(token, "Function must return something that matches the return type");
				}
				
				operation = new InstReturn;
			} else {
				if (returnVoid) {
					Log::CompilerError(token, "Unexpected symbol \"%s\" expected \";\". Function has return type void", next.string.str);
				}

				ParseInfo inf;
				inf.start = i;
				inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, inf.start);

				if (inf.end-- == ~0) {
					Log::CompilerError(token, "Expression is missing \";\"");
				}

				Symbol* res = ParseExpression(tokens, &inf, localVariables);

				i = inf.end+1;

				TypeBase* type = res->type;
				TypeBase* retType = declaration->returnType;

				ID* operandId;

				if (res->symbolType == SymbolType::Variable) {
					InstLoad* load = new InstLoad(type->typeId, res->id, 0);
					instructions.Add(load);

					operandId = load->id;
				} else {
					operandId = res->id;
				}

				if (*type != retType) {
					operandId = ImplicitCastId(retType, type, operandId, &next);
				}
				

				operation = new InstReturnValue(operandId);
			}

			instructions.Add(operation);

		} else if (token.type == TokenType::ControlFlowIf) {
			ParseIf(declaration, tokens, i--, localVariables);
		} else {
			Log::CompilerError(token, "Unexpected symbol \"%s\"", token.string.str);
		}
	}

	localVariables->PopStack();

	tokens.Remove(start, closeBracket);
}

void Compiler::ParseIf(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start, VariableStack* localVariables) {
	const Token& parenthesisOpen = tokens[start + 1];

	if (parenthesisOpen.type != TokenType::ParenthesisOpen) {
		Log::CompilerError(parenthesisOpen, "Unexpected symbol \"%s\" expected \"(\"", parenthesisOpen.string.str);
	}

	ParseInfo inf;
	inf.start = start + 2;
	inf.end = FindMatchingToken(tokens, start, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

	if (inf.end-- == ~0) {
		Log::CompilerError(parenthesisOpen, "\"(\" needs a closing \")\"");
	}


	Symbol* res = ParseExpression(tokens, &inf, localVariables);

	if (!Utils::CompareEnums(res->type->type, CompareOperation::Or, Type::Int, Type::Float, Type::Bool)) {
		Log::CompilerError(tokens[start + 2], "Expression must result in a scalar bool, int or float type. Is \"%s\"", res->type->typeString.str);
	}

	if (res->symbolType == SymbolType::Variable) {
		InstLoad* load = new InstLoad(res->type->typeId, res->id, 0);
		instructions.Add(load);

		res->id = load->id;
	}

	if (res->type->type != Type::Bool) {
		TypeBase* tmp = res->type;
		res = ImplicitCast(CreateTypeBool(), res->type, res->id, &tokens[start + 2]);
	}

	tokens.Remove(start, inf.end+1);

	const Token& bracket = tokens[start];

	InstBase* mergeBlock = new InstLabel;
	InstBase* trueBlock = new InstLabel;
	InstBase* falseBlock = new InstLabel;

	instructions.Add(new InstSelectionMerge(mergeBlock->id, 0));
	instructions.Add(new InstBranchConditional(res->id, trueBlock->id, falseBlock->id, 1, 1));
	instructions.Add(trueBlock);

	if (bracket.type == TokenType::CurlyBracketOpen) { 
		tokens.RemoveAt(start);
		ParseBody(declaration, tokens, start, localVariables);
		ParseElse(declaration, tokens, start, localVariables, mergeBlock, falseBlock);
	} else  {//One line if
		ParseInfo inf;
		inf.start = start;
		inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, start);

		if (inf.end-- == ~0) {
			Log::CompilerError(bracket, "Unexpected symbol \"%s\", expected expression or \"{\"", bracket.string.str);
		}

		ParseExpression(tokens, &inf, localVariables);

		tokens.Remove(start, inf.end + 1);

		ParseElse(declaration, tokens, start, localVariables, mergeBlock, falseBlock);
	}

	instructions.Add(mergeBlock);
	
}

void Compiler::ParseElse(FunctionDeclaration* declaration, List<Token>& tokens, uint64 start, VariableStack* localVariables, InstBase* mergeBlock, InstBase* falseBlock) {
	instructions.Add(new InstBranch(mergeBlock->id));
	instructions.Add(falseBlock);
	
	const Token& els = tokens[start];

	if (els.type == TokenType::ControlFlowElse) {
		tokens.RemoveAt(start);

		const Token& next = tokens[start];

		if (next.type == TokenType::ControlFlowIf) {
			ParseIf(declaration, tokens, start, localVariables);
		} else if (next.type == TokenType::CurlyBracketOpen) {
			tokens.RemoveAt(start);
			ParseBody(declaration, tokens, start, localVariables);
		} else {
			ParseInfo inf;
			inf.end = tokens.Find<TokenType>(TokenType::SemiColon, CmpFunc, start);

			if (inf.end-- == ~0) {
				Log::CompilerError(next, "Unexpected symbol \"%s\", expected expression or \"{\"", next.string.str);
			}

			ParseExpression(tokens, &inf, localVariables);

			tokens.Remove(start, inf.end + 1);
		}
	}

	instructions.Add(new InstBranch(mergeBlock->id));
}

Compiler::Symbol* Compiler::ParseName(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	uint64 offset = 0;

	const Token& name = tokens[info->start + offset++];

	Symbol* var = GetVariable(name.string, localVariables);
	Symbol* result = new Symbol;

	if (var == nullptr) {
		Log::CompilerError(name, "Unexpected symbol \"%s\" expected a variable", name.string.str);
	} 

	Token op = tokens[info->start + offset++];

	if (Utils::CompareEnums(op.type, CompareOperation::Or, TokenType::OperatorSelector, TokenType::BracketOpen)) {
		List<ID*> accessIds;
		
		String n = name.string;
		TypeBase* curr = var->type;

		while (true) {
			if (op.type == TokenType::OperatorSelector) {
				const Token& member = tokens[info->start + offset++];

				if (curr->type != Type::Struct && curr->type != Type::Vector) {
					Log::CompilerError(op, "Left of operator \".\" must be a struct or vector");
				}

				if (curr->type == Type::Vector) {
					offset--;
					break;
				}

				if (member.type != TokenType::Name) {
					Log::CompilerError(member, "Right of operator \".\" must be a valid name");
				}

				TypeStruct* s = (TypeStruct*)curr;

				uint64 index = s->GetMemberIndex(member.string);

				if (index == ~0) {
					Log::CompilerError(member, "\"%s\" doesn't have a member named \"%s\"", n.str, member.string.str);
				}

				n.Append(".").Append(member.string);
				curr = s->members[index].type;

				accessIds.Add(CreateConstantS32((int32)index));
			} else if (op.type == TokenType::BracketOpen) {
				if (curr->type != Type::Array) {
					Log::CompilerError(op, "\"%s\" is not an array", n.str);
				}

				TypeArray* arr = (TypeArray*)curr;

				ParseInfo inf;
				inf.start = info->start+offset;
				inf.end = tokens.Find<TokenType>(TokenType::BracketClose, CmpFunc, info->start+offset);

				if (inf.end-- == ~0) {
					Log::CompilerError(op, "\"[\" needs a closing \"]\"");
				}

				Symbol* index = ParseExpression(tokens, &inf, localVariables);

				if (index->type->type != Type::Int) {
					Log::CompilerError(op, "Array index must be a (signed) integer scalar");
				} else {
					TypePrimitive* p = (TypePrimitive*)index->type;

					if (!p->sign) {
						Log::CompilerWarning(op, "Array index is unsigned but will be treated as signed");
					}
				}

				if (index->symbolType == SymbolType::Variable) {
					InstLoad* load = new InstLoad(index->type->typeId, index->id, 0);
					instructions.Add(load);

					accessIds.Add(load->id);
				} else {
					accessIds.Add(index->id);
				}

				n.Append("[]");

				curr = arr->elementType;

				offset = (inf.end - info->start)+2;
			} else {
				break;
			}

			op = tokens[info->start + offset++];
		}
		
		if (accessIds.GetCount() != 0) {
			TypePointer* pointer = CreateTypePointer(curr, var->variable.scope);

			InstInBoundsAccessChain* access = new InstInBoundsAccessChain(pointer->typeId, var->id, (uint32)accessIds.GetCount(), accessIds.GetData());

			instructions.Add(access);
			
			result->symbolType = SymbolType::Variable;
			result->variable.scope = var->variable.scope;
			result->variable.name = n;
			result->type = curr;
			result->id = access->id;
			result->variable.isConst= var->variable.isConst;
		} else {
			delete result;
			result = var;
		}
	} else {
		delete result;
		result = var;
	}

	info->len = offset-1;

	return result;
}


bool Compiler::Process() {
	lines = preprocessor::PreProcessor::Run(code, filename, defines, includes);

	if (CompilerOptions::PPOnly()) {
		String tmp;

		for (uint64 i = 0; i < lines.GetCount(); i++) {
			tmp.Append(lines[i].string + "\n");
		}
		
		FILE* file = fopen((CompilerOptions::OutputFile() + ".pp").str, "wb");

		fwrite(tmp.str, tmp.length * sizeof(char), 1, file);
		fclose(file);

		return false;
	}

	List<Token> tokens = Tokenize();

	ParseTokens(tokens);

	return true;
}

bool Compiler::GenerateFile(const String& filename) {
	List<InstBase*> capabilities;

	capabilities.Add(new InstCapability(THC_SPIRV_CAPABILITY_SHADER)); // Currently shader is the only supported capability
	capabilities.Add(new InstMemoryModel(THC_SPIRV_ADDRESSING_MODEL_LOGICAL, THC_SPIRV_MEMORY_MODEL_GLSL450));

	List<ID*> ids;

	auto findIds = [&](VariableScope scope) {
		for (uint64 i = 0; i < globalVariables.GetCount(); i++) {
			Symbol* v = globalVariables[i];

			if (v->variable.scope == scope) {
				ids.Add(v->id);
			}
		}
	};

	findIds(VariableScope::Out);
	findIds(VariableScope::In);

	uint32 executionMode = CompilerOptions::VertexShader() ? THC_SPIRV_EXECUTION_MODEL_VERTEX : CompilerOptions::FragmentShader() ? THC_SPIRV_EXECUTION_MODEL_FRAGMENT : ~0;
	
	List<FunctionDeclaration*> main = GetFunctionDeclarations("main");

	if (main.GetCount() == 0) {
		Log::Error("No main function defined!");
	} else if (main.GetCount() > 1) {
		Log::Error("Multiple main functions defined!");
	}

	FunctionDeclaration* decl = main[0];

	if (!decl->defined) Log::Error("Main function not defined!");

	capabilities.Add(new InstEntryPoint(executionMode, decl->id, "main", (uint32)ids.GetCount(), ids.GetData()));
	if (CompilerOptions::FragmentShader()) capabilities.Add(new InstExecutionMode(decl->id, THC_SPIRV_EXECUTION_MODE_ORIGIN_LOWER_LEFT, 0, nullptr));

	FILE* file = fopen(filename.str, "wb");

	if (file == nullptr) {
		Log::Error("Failed to open file \"%s\"", filename.str);
		return false;
	}

	struct Header {
		uint32 magic = THC_SPIRV_MAGIC_NUMBER;
		uint32 version = 0x00010300;
		uint32 gen = THC_GENERATOR_ID;
		uint32 bound = IDManager::GetCount()+1;
		uint32 schema = 0;
	} header;

	fwrite(&header, sizeof(Header), 1, file);

	List<uint32> code;

	uint32 writtenInstructions = 0;

	auto writeCode = [&writtenInstructions, &code, &file](List<InstBase*>& list) {
		uint32 tmp[128];

		for (uint64 i = 0; i < list.GetCount(); i++) {
			InstBase* inst = list[i];

			inst->GetInstWords(tmp);

			code.Add(tmp, inst->wordCount);

			writtenInstructions++;
		}

		fwrite(code.GetData(), code.GetSize(), 1, file);

		code.Clear();
	};

	writeCode(capabilities);
	if (CompilerOptions::DebugInformation()) writeCode(debugInstructions);
	writeCode(annotationIstructions);
	writeCode(types);
	writeCode(instructions);

	fclose(file);

	return true;
}

Compiler::Compiler(const String& code, const String& filename, const List<String>& defines, const List<String>& includes) : code(code), filename(filename), defines(defines), includes(includes) {

}

bool Compiler::Run(const String& code, const String& filename, const List<String>& defines, const List<String>& includes, const String& outFile) {
	Compiler c(code, filename, defines, includes);

	bool res = c.Process();

	if (res == false) return true;

	return c.GenerateFile(outFile);
}

bool Compiler::Run(const String& filename, const List<String>& defines, const List<String>& includes, const String& outFile) {
	return Run(Utils::ReadFile(filename), filename, defines, includes, outFile);
}

}
}
}