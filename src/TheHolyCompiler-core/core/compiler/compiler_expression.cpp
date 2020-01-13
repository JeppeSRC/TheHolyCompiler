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

Compiler::Symbol* Compiler::ParseExpression(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	List<Expression> expressions;
	List<Symbol*> tmpVariables;
	List<InstBase*> postIncrements;


	for (uint64 i = info->start; i <= info->end; i++) {
		const Token& t = tokens[i];
		Expression e = { ExpressionType::Undefined };

		if (t.type == TokenType::Name) {
			const Token& next = tokens[i + (i == info->end ? 0 : 1)];

			uint64 c = expressions.GetCount();

			//Checks if the previous thing was a vector
			if (c >= 2) {
				if (expressions[c - 1].type == ExpressionType::Operator && expressions[c - 1].operatorType == TokenType::OperatorSelector) {
					const Expression& exp = expressions[c - 2];

					if (exp.type == ExpressionType::Symbol) {
						if (exp.symbol->type->type == Type::Vector) {
							e.type = ExpressionType::SwizzleComponent;
							e.parent = t;
							c = ~0;
						}
					}
				}
			}
			
			if (next.type == TokenType::OperatorSelector || next.type == TokenType::BracketOpen && c != ~0) { //Member selection in a struct and/or array subscripting
				ParseInfo inf;
				inf.start = i;

				Symbol* v = ParseName(tokens, &inf, localVariables);

				e.type = ExpressionType::Variable;
				e.symbol = v;
				e.parent = tokens[i + inf.len - 1];

				tmpVariables.Add(v);

				i += inf.len - 1;
			} else if (t.string == "false" || t.string == "true") {
				e.type = ExpressionType::Constant;
				e.symbol = new Symbol;
				e.symbol->symbolType = SymbolType::Constant;
				e.symbol->type = CreateTypeBool();
				e.symbol->id = CreateConstantBool(t.string == "true");
				e.parent = t;
			} else if (c != ~0) { //Variable or function
				if (next.type == TokenType::ParenthesisOpen) { //Function
					ParseInfo inf;
					inf.start = i;

					Symbol* res = ParseFunctionCall(tokens, &inf, localVariables);

					e.type = res->symbolType == SymbolType::Variable ? ExpressionType::Variable : res->symbolType == SymbolType::Constant ? ExpressionType::Constant : ExpressionType::Result;
					e.symbol = res;

					i += inf.len;
				} else { //Variable
					e.type = ExpressionType::Variable;
					e.symbol = GetVariable(t.string, localVariables);
					e.parent = t;

					if (e.symbol == nullptr) {
						Log::CompilerError(t, "Unexpected symbol \"%s\" expected a variable or constant", t.string.str);
					}
				}
			}
		} else if (t.type == TokenType::Value) {
			e.type = ExpressionType::Constant;
			e.symbol = new Symbol(SymbolType::Constant);
			e.symbol->type = CreateTypePrimitiveScalar(ConvertToType(t.valueType), 32, t.sign);
			e.symbol->id = CreateConstant(e.symbol->type, (uint32)t.value);
			e.parent = t;
		} else if (t.type >= TokenType::OperatorSelector && t.type <= TokenType::OperatorCompoundDiv) {
			e.type = ExpressionType::Operator;
			e.operatorType = t.type;
			e.parent = t;
		} else if (t.type >= TokenType::TypeVoid && t.type <= TokenType::TypeMatrix) {
			ParseInfo inf;
			inf.start = i;

			Symbol* res = ParseTypeConstructor(tokens, &inf, localVariables);

			e.type = res->symbolType == SymbolType::Variable ? ExpressionType::Variable : res->symbolType == SymbolType::Constant ? ExpressionType::Constant : ExpressionType::Result;
			e.symbol = res;

			info->end -= inf.end;
			i += inf.len;
		} else if (t.type == TokenType::ParenthesisOpen) {
			const Token& next = tokens[i + (i == info->end ? 0 : 1)];

			if (next.type >= TokenType::TypeVoid && next.type <= TokenType::TypeMatrix) {
				if (!Utils::CompareEnums(next.type, CompareOperation::Or, TokenType::TypeInt, TokenType::TypeFloat)) {
					Log::CompilerError(t, "Cast type must be scalar of type integer or float");
				}

				e.type = ExpressionType::Type;
				e.castType = CreateTypePrimitiveScalar(ConvertToType(next.type), next.bits, next.sign);
				e.parent = t;
				i+=2;
			} else {
				ParseInfo inf;

				uint64 parenthesisClose = FindMatchingToken(tokens, i, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

				if (parenthesisClose > info->end) {
					Log::CompilerError(t, "\"(\" needs a closing \")\"");
				}

				inf.start = i + 1;
				inf.end = --parenthesisClose;

				Symbol* res = ParseExpression(tokens, &inf, localVariables);

				e.type = res->symbolType == SymbolType::Variable ? ExpressionType::Variable : res->symbolType == SymbolType::Constant ? ExpressionType::Constant : ExpressionType::Result;
				e.symbol = res;
				e.parent = t;
				
				info->end -= parenthesisClose - inf.end;

				i = inf.end + 1;
			}
		}

		if (e.type != ExpressionType::Undefined) expressions.Add(e);
	}

#pragma region precedence 1
	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		//post increment/decrement
		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement)) {
			if (i < 1) continue;
			Expression& left = expressions[i - 1];

			bool rightVar = false;

			if (i < expressions.GetCount() - 1) {
				const Expression& right = expressions[i + 1];

				rightVar = right.type == ExpressionType::Variable;
			}


			if (left.type == ExpressionType::Variable) {
				if (left.symbol->variable.isConst) {
					Log::CompilerError(left.parent, "Left hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(left.symbol->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(left.parent, "Left hand operand of must be a interger/float scalar");
				}

				Symbol* var = left.symbol;

				InstLoad* load = new InstLoad(var->type->typeId, var->id, 0);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : ~0U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}

				InstStore* store = new InstStore(var->id, operation->id, 0);

				instructions.Add(load);
				instructions.Add(operation);

				postIncrements.Add(store);

				left.type = ExpressionType::Result;
				left.symbol = new Symbol(SymbolType::Result, var->type, operation->id);

				expressions.RemoveAt(i--);
			} else if (!rightVar) {
				Log::CompilerError(e.parent, "Left or right hand operand must be a lvalue");
			}
		} else if (e.operatorType == TokenType::OperatorSelector) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[i + 1];

			TypePrimitive* lType = (TypePrimitive*)(left.type == ExpressionType::Symbol ? left.symbol->type : nullptr);

			if (lType == nullptr) {
				Log::CompilerError(e.parent, "Invalid left hand operand");
			}
			
			if (lType->type != Type::Vector) Log::CompilerError(e.parent, "Left of operator \".\" must be a struct or vector");

			if (right.type != ExpressionType::SwizzleComponent) Log::CompilerError(e.parent, "Right of operator \".\" must be a valid set of components for the left hand vector");
			
			Symbol* symbol = new Symbol(left.symbol);

			symbol->swizzleIndices = GetVectorShuffleIndices(right.parent, lType);
			symbol->swizzleWritable = true;

			for (uint64 i = 0; i < symbol->swizzleIndices.GetCount()-1; i++) {
				if (symbol->swizzleIndices.Find(symbol->swizzleIndices[i], i + 1) != ~0) {
					symbol->swizzleWritable = false;
					break;
				}
			}

			left.symbol = symbol;

			expressions.Remove(i, i + 1);
		}
	}

#pragma endregion

#pragma region precedence 2

	for (uint64 i = expressions.GetCount() - 1; (int64)i >= 0; i--) {
		const Expression& e = expressions[i];

		if (e.type == ExpressionType::Type) { //Cast
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type, true);

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Float) || operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer or float");
			}

			if (*e.castType == type) {
				Log::CompilerWarning(e.parent, "Unnecessary cast");
				expressions.RemoveAt(i);
			} else {
				right.type = ExpressionType::Result;
				right.symbol = Cast(e.castType, type, operandId, &e.parent);

				if (right.symbol->id == nullptr) {
					Log::CompilerError(e.parent, "The only castable types are scalar integers or floats");
				}

				expressions.RemoveAt(i);
			}

		} else if (e.type != ExpressionType::Operator) {
			continue;
		}

		//pre increment/decrement
		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorIncrement, TokenType::OperatorDecrement)) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "Left or right hand operand must be a lvalue");
			}

			Expression& right = expressions[i + 1];

			if (right.type == ExpressionType::Variable) {
				if (right.symbol->variable.isConst) {
					Log::CompilerError(right.parent, "Right hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(right.symbol->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(right.parent, "Right hand operand of must be a interger/float scalar");
				}

				Symbol* var = right.symbol;

				ID* load = LoadVariable(var, true);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : ~0U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}

				instructions.Add(operation);

				StoreVariable(var, operation->id, true);

				//right.type = ExpressionType::Result;
				//right.symbol = new Symbol(SymbolType::Result, var->type, operation->id);

				expressions.RemoveAt(i);
			} else {
				Log::CompilerError(e.parent, "Right hand operand must be lvalue");
			}

		} else if (e.operatorType == TokenType::OperatorNegate) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type, true);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			InstBase* operation = nullptr;

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Float, Type::Vector)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			if (type->componentType == Type::Int) {
				if (!type->sign) {
					if (type->type == Type::Vector) {
						type = CreateTypePrimitiveVector(Type::Int, type->bits, 1, type->rows);
					} else {
						type = CreateTypePrimitiveScalar(Type::Int, type->bits, 1);
					}
				}

				operation = new InstSNegate(type->typeId, operandId);
			} else if (type->componentType == Type::Float) {
				operation = new InstFNegate(type->typeId, operandId);
			} else {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer or float");
			}

			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.symbol = new Symbol(SymbolType::Result, type, operation->id);

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorLogicalNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* rType = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &rType, false);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a variable or value");
			}

			if (!Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer, float or a bool result from an expression");
			}

			InstBase* operation = nullptr;

			ID* constantId = nullptr;

			if (rType->type == Type::Int) {
				constantId = CreateConstant(rType, 0U);
			} else {
				constantId = CreateConstant(rType, 0.0f);
			}

			TypeBase* type = CreateTypeBool();
			ID* retTypeId = type->typeId;

			switch (rType->type) {
				case Type::Bool:
					operation = new InstLogicalNot(retTypeId, operandId);
					break;
				case Type::Int:
					operation = new InstINotEqual(retTypeId, operandId, constantId);
					break;
				case Type::Float:
					operation = new InstFOrdNotEqual(retTypeId, operandId, constantId);
					break;
			}


			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.symbol = new Symbol(SymbolType::Result, type, operation->id);

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorBitwiseNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type, true);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Vector)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			InstNot* operation = new InstNot(type->typeId, operandId);
			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.symbol = new Symbol(SymbolType::Result, type, operation->id);

			expressions.RemoveAt(i);
		}
	}

#pragma endregion

#pragma region precedence 3

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorMul || e.operatorType == TokenType::OperatorDiv) {
			bool mul = e.operatorType == TokenType::OperatorMul;
			Expression& left = expressions[i - 1];
			Expression& right = expressions[i + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			left.type = ExpressionType::Result;

			if (mul) {
				left.symbol = Multiply(lType, lOperandId, rType, rOperandId, &e.parent);
			} else {
				left.symbol = Divide(lType, lOperandId, rType, rOperandId, &e.parent);
			}
			
			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 4

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorAdd || e.operatorType == TokenType::OperatorSub) {
			bool add = e.operatorType == TokenType::OperatorAdd;
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			left.type = ExpressionType::Result;

			if (add) {
				left.symbol = Add(lType, lOperandId, rType, rOperandId, &e.parent);
			} else {
				left.symbol = Subtract(lType, lOperandId, rType, rOperandId, &e.parent);
			}

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 5

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (e.operatorType == TokenType::OperatorRightShift || e.operatorType == TokenType::OperatorLeftShift) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, false);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, false);


			if (lType->type != Type::Int && rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be integers");
			}

			InstBase* instruction = nullptr;

			ID* rId = rOperandId;

			if (lType->bits != rType->bits) {
				rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);
			}

			if (e.operatorType == TokenType::OperatorRightShift) {
				instruction = new InstShiftRightLogical(lType->typeId, lOperandId, rId);
			} else {
				instruction = new InstShiftLeftLogical(lType->typeId, lOperandId, rId);
			}

			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, lType, instruction->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 6

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorLess, TokenType::OperatorLessEqual, TokenType::OperatorGreater, TokenType::OperatorGreaterEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			bool floatCmp = false;

			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);
					} else if (lType->bits < rType->bits) {
						lId = ImplicitCastId(rType, lType, lOperandId, &e.parent);
					}
				} else { // Int
					rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);
				}

				floatCmp = true;
			} else { //Int
				if (rType->type == Type::Int) {
					if (lType->sign != rType->sign) {
						Log::CompilerWarning(right.parent, "Sign missmatch");
					}

					TypePrimitive* tmp = nullptr;

					if (lType->bits > rType->bits) {
						convInst = rType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 1))->typeId, rOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 0))->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, tmp->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = lType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 1))->typeId, lOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 0))->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, tmp->typeString.str);
					}
				} else { // Float
					lId = ImplicitCastId(rType, lType, lOperandId, &e.parent);

					floatCmp = true;
				}
			}

			TypeBase* retType = CreateTypeBool();
			ID* retTypeId = retType->typeId;

			if (floatCmp) {
				switch (e.operatorType) {
					case TokenType::OperatorLess:
						instruction = new InstFOrdLessThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorLessEqual:
						instruction = new InstFOrdLessThanEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreater:
						instruction = new InstFOrdGreaterThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreaterEqual:
						instruction = new InstFOrdGreaterThanEqual(retTypeId, lId, rId);
						break;
				}
			} else {
				switch (e.operatorType) {
					case TokenType::OperatorLess:
						instruction = lType->sign ? new InstSLessThan(retTypeId, lId, rId) : (InstBase*)new InstULessThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorLessEqual:
						instruction = lType->sign ? new InstSLessThanEqual(retTypeId, lId, rId) : (InstBase*)new InstULessThanEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreater:
						instruction = lType->sign ? new InstSGreaterThan(retTypeId, lId, rId) : (InstBase*)new InstUGreaterThan(retTypeId, lId, rId);
						break;
					case TokenType::OperatorGreaterEqual:
						instruction = lType->sign ? new InstSGreaterThanEqual(retTypeId, lId, rId) : (InstBase*)new InstUGreaterThanEqual(retTypeId, lId, rId);
						break;
				}
			}

			if (convInst) instructions.Add(convInst);
			instructions.Add(instruction);


			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, retType, instruction->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 7

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorEqual, TokenType::OperatorNotEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int, float or bool");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			uint8 cmpType = 0;

			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);
					} else if (lType->bits < rType->bits) {
						lId = ImplicitCastId(rType, lType, rOperandId, &e.parent);
					}
				} else if (rType->type == Type::Int) { // Int
					rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);
				} else {
					Log::CompilerError(e.parent, "Type missmatch, cannot compare %s to bool", lType->typeString.str);
				}

				cmpType = 1;
			} else if (lType->type == Type::Int) { //Int
				if (rType->type == Type::Int) {
					TypePrimitive* tmp = nullptr;

					if (lType->bits > rType->bits) {
						convInst = rType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 1))->typeId, rOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, lType->bits, 0))->typeId, rOperandId);
						rId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, tmp->typeString.str);
					} else if (lType->bits < rType->bits) {
						convInst = lType->sign ? new InstSConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 1))->typeId, lOperandId) : (InstBase*)new InstUConvert((tmp = CreateTypePrimitiveScalar(Type::Int, rType->bits, 0))->typeId, lOperandId);
						lId = convInst->id;
						Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", lType->typeString.str, tmp->typeString.str);
					}
				} else if (rType->type == Type::Float) { // Float
					rId = ImplicitCastId(lType, rType, rOperandId, &e.parent);

					cmpType = 1;
				} else {
					Log::CompilerError(e.parent, "Type missmatch, cannot compare %s to bool", lType->typeString.str);
				}
			} else {
				if (rType->type != Type::Bool) {
					Log::CompilerError(e.parent, "Type missmatch, cannot compare %s to bool", rType->typeString.str);
				}

				cmpType = 2;
			}

			TypeBase* retType = CreateTypeBool();
			ID* retTypeId = retType->typeId;

			if (cmpType == 1) {
				switch (e.operatorType) {
					case TokenType::OperatorEqual:
						instruction = new InstFOrdEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorNotEqual:
						instruction = new InstFOrdNotEqual(retTypeId, lId, rId);
						break;
				}
			} else if (cmpType == 0) {
				switch (e.operatorType) {
					case TokenType::OperatorEqual:
						instruction = new InstIEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorNotEqual:
						instruction = new InstINotEqual(retTypeId, lId, rId);
						break;
				}
			} else {
				switch (e.operatorType) {
					case TokenType::OperatorEqual:
						instruction = new InstLogicalEqual(retTypeId, lId, rId);
						break;
					case TokenType::OperatorNotEqual:
						instruction = new InstLogicalNotEqual(retTypeId, lId, rId);
						break;
				}
			}

			if (convInst) instructions.Add(convInst);
			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, retType, instruction->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 8

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorBitwiseAnd)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			if (lType->type != Type::Int || rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int");
			}

			ID* rId = rOperandId;

			InstBase* conv = nullptr;

			if (lType->bits != rType->bits) {
				conv = rType->sign ? new InstSConvert(lType->typeId, rOperandId) : (InstBase*)new InstUConvert(lType->typeId, rOperandId);
				rId = conv->id;
				Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
			}

			InstBase* inst = new InstBitwiseAnd(lType->typeId, lOperandId, rId);

			if (conv) instructions.Add(conv);
			instructions.Add(inst);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, lType, inst->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 9

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorBitwiseXor)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			if (lType->type != Type::Int || rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int");
			}

			ID* rId = rOperandId;

			InstBase* conv = nullptr;

			if (lType->bits != rType->bits) {
				conv = rType->sign ? new InstSConvert(lType->typeId, rOperandId) : (InstBase*)new InstUConvert(lType->typeId, rOperandId);
				rId = conv->id;
				Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
			}

			InstBase* inst = new InstBitwiseXor(lType->typeId, lOperandId, rId);

			if (conv) instructions.Add(conv);
			instructions.Add(inst);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, lType, inst->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 10

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorBitwiseOr)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			if (lType->type != Type::Int || rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int");
			}

			ID* rId = rOperandId;

			InstBase* conv = nullptr;

			if (lType->bits != rType->bits) {
				conv = rType->sign ? new InstSConvert(lType->typeId, rOperandId) : (InstBase*)new InstUConvert(lType->typeId, rOperandId);
				rId = conv->id;
				Log::CompilerWarning(right.parent, "Implicit conversion from %s to %s", rType->typeString.str, lType->typeString.str);
			}

			InstBase* inst = new InstBitwiseOr(lType->typeId, lOperandId, rId);

			if (conv) instructions.Add(conv);
			instructions.Add(inst);
			
			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, lType, inst->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 11

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorLogicalAnd)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of bool, int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			TypeBase* retType = CreateTypeBool();

			if (lType->type != Type::Bool) {
				Symbol* r = Cast(retType, lType, lOperandId, &left.parent);
				lId = r->id;
				delete r;
			}

			if (rType->type != Type::Bool) {
				Symbol* r = Cast(retType, rType, rOperandId, &right.parent);
				rId = r->id;
				delete r;
			}

			InstBase* instruction = new InstLogicalAnd(retType->typeId, lId, rId);

			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, retType, instruction->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 12

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorLogicalOr)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType, true);
			ID* rOperandId = GetExpressionOperandId(&right, &rType, true);

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of bool, int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			TypeBase* retType = CreateTypeBool();

			if (lType->type != Type::Bool) {
				Symbol* r = Cast(retType, lType, lOperandId, &left.parent);
				lId = r->id;
				delete r;
			}

			if (rType->type != Type::Bool) {
				Symbol* r = Cast(retType, rType, rOperandId, &right.parent);
				rId = r->id;
				delete r;
			}

			InstBase* instruction = new InstLogicalOr(retType->typeId, lId, rId);

			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.symbol = new Symbol(SymbolType::Result, retType, instruction->id);

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

#pragma region precedence 13

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];


	}

#pragma endregion

#pragma region precedence 14

	for (uint64 i = 0; i < expressions.GetCount(); i++) {
		const Expression& e = expressions[i];

		if (e.type != ExpressionType::Operator) continue;

		if (i < 1) {
			Log::CompilerError(e.parent, "No left hand operand");
		} else if (i >= expressions.GetCount() - 1) {
			Log::CompilerError(e.parent, "No right hand operand");
		}

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorAssign, TokenType::OperatorCompoundAdd, TokenType::OperatorCompoundSub, TokenType::OperatorCompoundMul, TokenType::OperatorCompoundDiv)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[i + 1];

			TypePrimitive* lBaseType = nullptr;

			if (left.type == ExpressionType::Variable) {
				lBaseType = (TypePrimitive*)left.symbol->type;
			} else {
				Log::CompilerError(e.parent, "Left hand operand must be a lvalue");
			}

			bool lSwizzle = left.symbol->swizzleIndices.GetCount() > 0;
			bool rSwizzle = right.symbol->swizzleIndices.GetCount() > 0;
			bool opAssign = e.operatorType == TokenType::OperatorAssign;

			bool lSwizzled = lSwizzle && !opAssign;
			bool rSwizzled = rSwizzle && ((opAssign != lSwizzle) || !opAssign);
			
			if (lSwizzle && !left.symbol->swizzleWritable) {
				Log::CompilerError(left.parent, "Left hand operand must be lvalue");
			}

			ID* lBaseId = nullptr;
			

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = nullptr;
			
			if (opAssign) {
				lType = lBaseType;

				if (lSwizzle) {
					lBaseId = GetExpressionOperandId(&left, &lType, false);
				}

			} else {
				lOperandId = GetExpressionOperandId(&left, &lType, lSwizzled, &lBaseId);
			}

			ID* rOperandId = GetExpressionOperandId(&right, &rType, rSwizzled);


			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector, Type::Matrix) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector, Type::Matrix)) {
				Log::CompilerError(e.parent, "Operands must be a of valid type");
			}

			TypePrimitive* lSwizzledType = !lSwizzled ? GetSwizzledType(lType, left.symbol->swizzleIndices) : lType;
			TypePrimitive* rSwizzledType = !rSwizzled ? GetSwizzledType(rType, right.symbol->swizzleIndices) : rType;

			if (opAssign) {
				if (*lSwizzledType != rSwizzledType) {
					if (!rSwizzled) {
						rSwizzled = true;
						rOperandId = GetSwizzledVector(&rType, rOperandId, right.symbol->swizzleIndices);
					}

					rOperandId = ImplicitCastId(lSwizzledType, rSwizzledType, rOperandId, &right.parent);

					rType = lSwizzledType;
				}
			}

			Symbol* tmp = nullptr;

			switch (e.operatorType) {
				case TokenType::OperatorCompoundAdd:
					tmp = Add(lType, lOperandId, rType, rOperandId, &e.parent);
					break;
				case TokenType::OperatorCompoundSub:
					tmp = Subtract(lType, lOperandId, rType, rOperandId, &e.parent);
					break;
				case TokenType::OperatorCompoundMul:
					tmp = Multiply(lType, lOperandId, rType, rOperandId, &e.parent);
					break;
				case TokenType::OperatorCompoundDiv:
					tmp = Divide(lType, lOperandId, rType, rOperandId, &e.parent);
					break;
				default:
					tmp = new Symbol;
					tmp->id = rOperandId;
			}

			//vec3.zx = vec3.yx;
			if (lSwizzle) {
				uint64 rows = lBaseType->rows;

				List<uint32> indices;

				const List<uint32>& lIndices = left.symbol->swizzleIndices;
				List<uint32> rIndices = right.symbol->swizzleIndices;
				
				if (!rSwizzle) {
					uint32 tmp[4] = { 0, 1, 2, 3 };
					rIndices.Add(tmp);
				}

				InstBase* inst = nullptr;

				if (lIndices.GetCount() == 1) {
					if (!rSwizzled && rSwizzle) {
						rSwizzled = true;
						tmp->id = GetSwizzledVector(&rType, rOperandId, right.symbol->swizzleIndices);
					}

					inst = new InstCompositeInsert(lBaseType->typeId, tmp->id, lBaseId, 1, lIndices.GetData());
				} else {
					for (uint64 i = 0; i < rows; i++) {
						uint64 lIndex = lIndices.Find((uint32)i);

						if (lIndex != ~0) {
							uint64 index = rSwizzled ? lIndex : rIndices[lIndex];
							indices.Add((uint32)(index + rows));
						} else {
							indices.Add((uint32)i);
						}
					}

					inst = new InstVectorShuffle(lBaseType->typeId, lBaseId, tmp->id, (uint32)rows, indices.GetData());
				} 

				instructions.Add(inst);
				tmp->id = inst->id;
			}

			StoreVariable(left.symbol, tmp->id);

			delete tmp;

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

	instructions.Add(postIncrements);

	if (expressions.GetCount() > 1) {
		const Expression& e = expressions[1];
		Log::CompilerError(e.parent, "Unexpected symbol \"%s\"", e.parent.string.str);
	}

	Expression e = expressions[0];

	return e.symbol;
}

}}}