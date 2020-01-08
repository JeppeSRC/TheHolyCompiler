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
#include <core/parsing/token.h>
#include <core/type/types.h>
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

		ID* typeId; //OpType id

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

	struct StructMember {
		utils::String name;
		TypeBase* type;

		bool operator==(const StructMember& other) const;
		bool operator!=(const StructMember& other) const;
	};

	struct TypeStruct : public TypeBase {
		utils::List<StructMember> members;

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
		TypeBase* baseType;
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

	void CheckTypeExist(type::InstTypeBase** type); 
	void CheckTypeExist(TypeBase** type);
	void CheckConstantExist(instruction::InstBase** constant);
	inline void CheckConstantExist(instruction::InstConstant** constant) { CheckConstantExist((instruction::InstBase**)constant); }
	inline void CheckConstantExist(instruction::InstConstantComposite** constant) { CheckConstantExist((instruction::InstBase**)constant); }
	

	TypePrimitive* CreateTypeBool();
	//start is the index of the type
	TypePrimitive* CreateTypePrimitive(utils::List<parsing::Token>& tokens, uint64 start, uint64* len);
	TypePrimitive* CreateTypePrimitiveScalar(type::Type type, uint8 bits, uint8 sign);
	TypePrimitive* CreateTypePrimitiveVector(type::Type componentType, uint8 bits, uint8 sign, uint8 rows);
	TypePrimitive* CreateTypePrimtiveMatrix(type::Type componentType, uint8 bits, uint8 sign, uint8 rows, uint8 columns);

	TypePrimitive* ModifyTypePrimitiveBitWidth(TypePrimitive* base, uint8 bits);

	//start is the index of the name of the struct
	TypeStruct* CreateTypeStruct(utils::List<parsing::Token>& tokens, uint64 start, uint64* len);
	//start is start of type
	TypeArray* CreateTypeArray(utils::List<parsing::Token>& tokens, uint64 start, uint64* len);

	TypeBase* CreateType(utils::List<parsing::Token>& tokens, uint64 start, uint64* len);

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

		TypeBase* type;
		TypePointer* typePointer;
		ID* variableId;

		bool isConstant;
	};

	struct Parameter : public Variable {
		bool isReference;
	};

	struct ResultVariable {
		TypeBase* type;
		ID* id;

		bool isVariable;
		bool isConstant;
	};

	utils::List<Variable*> globalVariables;

	class VariableStack;
	Variable* GetVariable(const utils::String& name, VariableStack* localVariables) const;

	bool CheckGlobalName(const utils::String& name) const; //returns true if name is available

	TypePointer* CreateTypePointer(const TypeBase* const type, VariableScope scope);
	Variable* CreateGlobalVariable(const TypeBase* const type, VariableScope scope, const utils::String& name);
	
	Variable* CreateLocalVariable(const TypeBase* const type, const utils::String& name, VariableStack* localVariables);

	ResultVariable Cast(TypeBase* cType, TypeBase* type, ID* operandId, const parsing::Token* t);
	ResultVariable ImplicitCast(TypeBase* cType, TypeBase* type, ID* operandId, const parsing::Token* t);
	ResultVariable Add(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const parsing::Token* parent);
	ResultVariable Subtract(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const parsing::Token* parent);
	ResultVariable Multiply(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const parsing::Token* parent);
	ResultVariable Divide(TypeBase* type1, ID* operand1, TypeBase* type2, ID* operand2, const parsing::Token* parent);

private:
	class VariableStack {
	public:
		utils::List<instruction::InstBase*> variableInstructions;
		utils::List<Variable*> variables;
		utils::List<Parameter*> parameters;
		utils::List<uint64> offsets;

		Compiler* compiler;
	public:
		VariableStack(Compiler* compiler, const utils::List<Parameter*>& parameters);
		~VariableStack();

		uint64 PushStack();
		void   PopStack();

		bool CheckName(const parsing::Token& name);
		bool CheckName(const utils::String& name, const parsing::Token& token);

		void AddVariable(Variable* variable, instruction::InstBase* inst);
		Variable* GetVariable(const utils::String& name);

		uint64 GetSize() const;
		uint64 GetStackSize(uint64 stack) const;
		uint64 GetCurrentStackSize() const;
	};

private: //Function stuff
	struct FunctionDeclaration {
		utils::String name;

		TypeBase* returnType;
		utils::List<Parameter*> parameters;

		bool defined;

		ID* typeId;
		ID* id;

		utils::List<instruction::InstBase*> declInstructions;

		bool operator==(const FunctionDeclaration* const other) const;
	};

	utils::List<FunctionDeclaration*> functionDeclarations;

	utils::List<FunctionDeclaration*> GetFunctionDeclarations(const utils::String& name); 
	void CreateFunctionType(FunctionDeclaration* decl);

	static bool CheckParameterName(const utils::List<Variable*>& params, const utils::String& name); //return true if name is available

private: //Constants
	ID* CreateConstantBool(bool value);
	ID* CreateConstantS32(int32 value);
	ID* CreateConstant(const TypeBase* const type, uint32 value);
	ID* CreateConstant(const TypeBase* const type, float32 value);
	ID* CreateConstantComposite(const TypeBase* const type, const utils::List<uint32>& values);
	ID* CreateConstantComposite(const TypeBase* const type, const uint32** values);
	ID* CreateConstantCompositeVector(const TypeBase* const type, const uint32** values);
	ID* CreateConstantCompositeMatrix(const TypeBase* const type, const uint32** values);
	ID* CreateConstantCompositeArray( const TypeBase* const type, const uint32** values);
	ID* CreateConstantCompositeStruct(const TypeBase* const type, const uint32** values);

	bool IsTypeComposite(const TypeBase* const type) const;

private: //Composites


private: //Expression parsing
	enum class ExpressionType {
		Undefined,
		Variable,
		Constant,
		Result,
		Operator,
		Type,
		SwizzleComponent
	};

	struct Expression {
		ExpressionType type;

		//type = Variable
		Variable* variable;

		union {
			//type = Constant
			ResultVariable constant;

			//type = Result
			ResultVariable result;
		};

		//type = Operator
		parsing::TokenType operatorType;

		//type = Type
		TypeBase* castType;

		//SwizzleInfo
		utils::List<uint32> swizzleIndices;
		bool swizzleWritable;

		parsing::Token parent;
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
	void CreateFunctionDeclaration(FunctionDeclaration* decl);
	void ParseBody(FunctionDeclaration* declaration, utils::List<parsing::Token>& tokens, uint64 start, VariableStack* localVariables);
	void ParseIf(FunctionDeclaration* declaration, utils::List<parsing::Token>& tokens, uint64 start, VariableStack* localVariables);
	void ParseElse(FunctionDeclaration* declaration, utils::List<parsing::Token>& tokens, uint64 start, VariableStack* localVariables, instruction::InstBase* mergeBlock, instruction::InstBase* falseBlock);
	
	struct ParseInfo {
		uint64 start;
		uint64 end;
		uint64 len;
	};

	Variable* ParseName(utils::List<parsing::Token>& tokens, ParseInfo* info, VariableStack* localVariables); //struct member selection, array subscripting and function calls
	ResultVariable ParseExpression(utils::List<parsing::Token>& tokens, ParseInfo* info, VariableStack* localVariables);
	ResultVariable ParseFunctionCall(utils::List<parsing::Token>& tokens, ParseInfo* info, VariableStack* localVariables);
	ResultVariable ParseTypeConstructor(utils::List<parsing::Token>& tokens, ParseInfo* info, VariableStack* localVariables);
	utils::List<ResultVariable> ParseParameters(utils::List<parsing::Token>& tokens, ParseInfo* inf, VariableStack* localVariables);

private: //Misc
	bool IsCharAllowedInName(const char c, bool first = true) const;
	bool IsCharWhitespace(const char c) const;
	void ProcessName(parsing::Token& t) const;
	uint64 FindMatchingToken(const utils::List<parsing::Token>& tokens, uint64 start, parsing::TokenType open, parsing::TokenType close) const;
	ID* GetExpressionOperandId(const Expression* e, TypePrimitive** type, bool swizzle, ID** ogID = nullptr);
	static utils::List<ID*> GetIDs(utils::List<ResultVariable>& things);
	utils::List<uint32> GetVectorShuffleIndices(const parsing::Token& token, const TypePrimitive* type);
	TypePrimitive* GetSwizzledType(TypePrimitive* base, const utils::List<uint32>& indices);
	ID* GetSwizzledVector(TypePrimitive** type, ID* load, const utils::List<uint32>& indices);
	void CheckIntrin(const parsing::Token& intrin, const Variable* var);

public:
	bool Process();
	bool GenerateFile(const utils::String& filename);

	Compiler(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes);
	static bool Run(const utils::String& code, const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes, const utils::String& outFile);
	static bool Run(const utils::String& filename, const utils::List<utils::String>& defines, const utils::List<utils::String>& includes, const utils::String& outFile);

};

}
}
}