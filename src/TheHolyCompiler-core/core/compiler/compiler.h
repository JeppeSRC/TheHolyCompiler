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
private: //Type stuff
	struct TypeBase {
		type::Type type; //Type
		utils::String typeString; //Type as a string
		utils::String name; // Only used in TypeStruct as member

		uint32 typeId; //OpType id

		virtual bool operator==(const TypeBase* const other) const;
		virtual bool operator!=(const TypeBase* const other) const;

		virtual uint32 GetSize() const = 0;
	};

	struct TypePrimitive : public TypeBase {
		type::Type componentType;

		uint8 bits;
		uint8 sign;
		uint8 rows;
		uint8 columns;

		bool operator==(const TypeBase* const other) const override;
		bool operator!=(const TypeBase* const other) const override;
		
		uint32 GetSize() const override;
	};

	struct TypeStruct : public TypeBase {
		utils::List<TypeBase*> members;

		bool operator==(const TypeBase* const other) const override;
		bool operator!=(const TypeBase* const other) const override;

		uint32 GetSize() const override;

		uint32 GetMemberIndex(const utils::String& name);
	};

	struct TypeArray : public TypeBase {
		uint32 elementCount;
		TypeBase* elementType;

		bool operator==(const TypeBase* const other) const override;
		bool operator!=(const TypeBase* const other) const override;

		uint32 GetSize() const override;
	};

	struct TypePointer : public TypeBase {
		TypeBase* pointerType;
		uint32 storageClass;

		bool operator==(const TypeBase* const other) const override;
		bool operator!=(const TypeBase* const other) const override;

		uint32 GetSize() const override { return ~0; }
	};

	/*struct TypeFunction : public TypeBase {
		TypeBase* returnType;

		utils::List<TypeBase*> parameters;

		bool operator==(const TypeBase* const other) const override;

		uint32 GetSize() const override { return ~0; }
	};*/

	utils::List<TypeBase*> typeDefinitions;

	
	utils::List<instruction::InstBase*> debugInstructions;
	utils::List<instruction::InstBase*> annotationIstructions;
	utils::List<instruction::InstBase*> types;
	utils::List<instruction::InstBase*> instructions;

	instruction::InstBase* GetInstFromID(uint32 id);
	void CheckTypeExist(type::InstTypeBase** type); 
	void CheckTypeExist(TypeBase** type);
	void CheckConstantExist(instruction::InstBase** constant);
	inline void CheckConstantExist(instruction::InstConstant** constant) { CheckConstantExist((instruction::InstBase**)constant); }
	inline void CheckConstantExist(instruction::InstConstantComposite** constant) { CheckConstantExist((instruction::InstBase**)constant); }
	

	//start is the index of the type
	TypePrimitive* CreateTypePrimitive(const Token& token);
	TypePrimitive* CreateTypePrimitive(utils::List<parsing::Token>& tokens, uint64 start);
	//start is the index of the name of the struct
	TypeStruct* CreateTypeStruct(utils::List<parsing::Token>& tokens, uint64 start);
	//start is start of type
	TypeArray* CreateTypeArray(utils::List<parsing::Token>& tokens, uint64 start);

	TypeBase* CreateType(utils::List<parsing::Token>& tokens, uint64 start);

	utils::String GetTypeString(const TypeBase* const type) const;

private:
	static bool findStructFunc(TypeBase* const& curr, const utils::String& name);

private: //Variable stuff
	enum class VariableScope {
		None,
		In,
		Out,
		Private,
		Uniform,
		Function
	};

	static uint32 ScopeToStorageClass(VariableScope scope);

	struct Variable {
		VariableScope scope;
		utils::String name;

		const TypeBase* type;
		uint32 typePointerId;
		uint32 variableId;
	};

	struct ResultVariable {
		TypeBase* type;
		uint32 id;

		bool isVariable;
	};

	utils::List<Variable*> globalVariables;
	utils::List<Variable*> localVariables;

	Variable* GetVariable(const utils::String& name) const;

	bool CheckLocalName(const utils::String& name) const; //return true if name is available
	bool CheckGlobalName(const utils::String& name) const; //returns true if name is available

	TypePointer* CreateTypePointer(const TypeBase* const type, VariableScope scope);
	Variable* CreateGlobalVariable(const TypeBase* const type, VariableScope scope, const utils::String& name);
	Variable* CreateLocalVariable(const TypeBase* const type, const utils::String& name);

	struct FunctionParameter;
	Variable* CreateParameterVariable(const FunctionParameter* const param, instruction::InstFunctionParameter** opParam);

private: //Function stuff
	struct FunctionParameter {
		utils::String name;

		TypeBase* type;

		bool constant;
		bool reference;

		bool operator==(const FunctionParameter* const other) const;
	};

	struct FunctionDeclaration {
		utils::String name;

		TypeBase* returnType;
		utils::List<FunctionParameter*> parameters;

		bool defined;

		uint32 typeId;
		uint32 id;

		bool operator==(const FunctionDeclaration* const other) const;
	};

	utils::List<FunctionDeclaration*> functionDeclarations;

	utils::List<FunctionDeclaration*> GetFunctionDeclarations(const utils::String& name); 

	static bool CheckParameterName(const utils::List<FunctionParameter*>& params, const utils::String& name); //return true if name is available

private: //Constants
	uint32 CreateConstant(const TypeBase* const type, uint32 value);
	uint32 CreateConstant(const TypeBase* const type, float32 value);
	uint32 CreateConstantComposite(const TypeBase* const type, const utils::List<uint32>& values);
	uint32 CreateConstantComposite(const TypeBase* const type, const uint32** values);
	uint32 CreateConstantCompositeVector(const TypeBase* const type, const uint32** values);
	uint32 CreateConstantCompositeMatrix(const TypeBase* const type, const uint32** values);
	uint32 CreateConstantCompositeArray( const TypeBase* const type, const uint32** values);
	uint32 CreateConstantCompositeStruct(const TypeBase* const type, const uint32** values);

	bool IsTypeComposite(const TypeBase* const type) const;

private: //Misc
	bool IsCharAllowedInName(const char c, bool first = true) const;
	bool IsCharWhitespace(const char c) const;
	void ProcessName(parsing::Token& t) const;
	uint64 FindMatchingToken(const utils::List<parsing::Token>& tokens, uint64 start, parsing::TokenType open, parsing::TokenType close) const;

private: //Expression parsing
	enum class ExpressionType {
		Variable,
		Constant,
		Result,
		FunctionCall,
		Operator
	};

	struct Expression {
		ExpressionType type;

		//type = Variable
		Variable* variable;

		union {
			//type = Constant
			ResultVariable constant;

			//Result
			ResultVariable result;
		};

		//Operator
		parsing::TokenType operatorType;

	};

private:
	utils::String code;
	utils::String filename;
	utils::List<parsing::Line> lines;
	utils::List<utils::String> defines;
	utils::List<utils::String> includes;

	utils::List<parsing::Token> Tokenize();
	void ParseTokens(utils::List<parsing::Token>& tokens);
	void ParseLayout(utils::List<parsing::Token>& tokens, uint64 start);
	void ParseInOut(utils::List<parsing::Token>& tokens, uint64 start, VariableScope scope);
	void ParseFunction(utils::List<parsing::Token>& tokens, uint64 start);
	void ParseFunctionBody(FunctionDeclaration* declaration, utils::List<parsing::Token>& tokens, uint64 start);
	void ParseAssignment(Variable* variable, utils::List<parsing::Token>& tokens, uint64 start);

	ResultVariable ParseExpression(utils::List<parsing::Token>& tokens, uint64 start, uint64 end);
	ResultVariable ParseFunctionCall(utils::List<parsing::Token>& tokens, uint64 start, uint64* len);

	bool Process();

	Compiler(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
public:
	static bool Run(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
	static bool Run(const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);

};

}
}
}