#include "compiler.h"
#include <util/utils.h>
#include <core/compiler/preprocessor/preprocessor.h>
#include <util/log.h>


namespace thc {
namespace core {
namespace compiler {

using namespace utils;
using namespace parsing;
using namespace type;
using namespace instruction;

Compiler::ResultVariable Compiler::ParseExpression(List<Token>& tokens, ParseInfo* info, VariableStack* localVariables) {
	List<Expression> expressions;
	List<Variable*> tmpVariables;
	List<InstBase*> postIncrements;


	for (uint64 i = info->start; i <= info->end; i++) {
		const Token& t = tokens[i];
		Expression e = { ExpressionType::Undefined };

		if (t.type == TokenType::Name) {
			const Token& next = tokens[i + (i == info->end ? 0 : 1)];
			if (next.type == TokenType::OperatorSelector || next.type == TokenType::BracketOpen) { //Member selection in a struct and/or array subscripting
				ParseInfo inf;
				inf.start = i;

				Variable* v = ParseName(tokens, &inf, localVariables);

				e.type = ExpressionType::Variable;
				e.variable = v;
				e.parent = tokens[i + inf.len - 1];

				tmpVariables.Add(v);

				i += inf.len - 1;
			} else if (t.string == "false" || t.string == "true") {
				e.type = ExpressionType::Constant;
				e.constant.isConstant = true;
				e.constant.type = CreateTypeBool();
				e.constant.id = CreateConstantBool(t.string == "true");
				e.parent = t;
			} else { //Variable or function
				if (next.type == TokenType::ParenthesisOpen) { //Function
					ParseInfo inf;
					inf.start = i;

					ResultVariable res = ParseFunctionCall(tokens, &inf, localVariables);

					e.type = ExpressionType::Result;
					e.result = res;

					i += inf.len;
				} else { //Variable
					e.type = ExpressionType::Variable;
					e.variable = GetVariable(t.string, localVariables);
					e.parent = t;

					if (e.variable == nullptr) {
						Log::CompilerError(t, "Unexpected symbol \"%s\" expected a variable or constant", t.string.str);
					}
				}
			}
		} else if (t.type == TokenType::Value) {
			e.type = ExpressionType::Constant;
			e.constant.type = CreateTypePrimitiveScalar(ConvertToType(t.valueType), 32, t.sign);
			e.constant.id = CreateConstant(e.constant.type, (uint32)t.value);
			e.parent = t;
		} else if (t.type >= TokenType::OperatorIncrement && t.type <= TokenType::OperatorCompoundDiv) {
			e.type = ExpressionType::Operator;
			e.operatorType = t.type;
			e.parent = t;
		} else if (t.type >= TokenType::TypeVoid && t.type <= TokenType::TypeMatrix) {
			if (info->start == info->end) {//Type for a cast
				//Log::CompilerError(t, "Unexpected symbol \"%s\"", t.string.str);
				if (!Utils::CompareEnums(t.type, CompareOperation::Or, TokenType::TypeInt, TokenType::TypeFloat)) {
					Log::CompilerError(t, "Cast type must be scalar of type integer or float");
				}

				e.type = ExpressionType::Type;
				e.castType = CreateTypePrimitiveScalar(ConvertToType(t.type), t.bits, t.sign);
				e.parent = t;
			} else {
				
			}

		} else if (t.type == TokenType::ParenthesisOpen) {
			ParseInfo inf;

			e.type = ExpressionType::Result;
			e.parent = t;

			uint64 parenthesisClose = FindMatchingToken(tokens, i, TokenType::ParenthesisOpen, TokenType::ParenthesisClose);

			if (parenthesisClose > info->end) {
				Log::CompilerError(t, "\"(\" needs a closing \")\"");
			}

			inf.start = i + 1;
			inf.end = --parenthesisClose;

			e.result = ParseExpression(tokens, &inf, localVariables);

			info->end -= parenthesisClose - inf.end;
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
				if (left.variable->isConstant) {
					Log::CompilerError(left.parent, "Left hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(left.variable->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(left.parent, "Left hand operand of must be a interger/float scalar");
				}

				const Variable* var = left.variable;

				InstLoad* load = new InstLoad(var->type->typeId, var->variableId, 0);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : ~0U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}

				InstStore* store = new InstStore(var->variableId, operation->id, 0);

				instructions.Add(load);
				instructions.Add(operation);

				postIncrements.Add(store);

				left.type = ExpressionType::Result;
				left.result.isVariable = false;
				left.result.type = var->type;
				left.result.id = load->id;

				expressions.RemoveAt(i--);
			} else if (rightVar) {
				Log::CompilerError(e.parent, "Left or right hand operand must be a lvalue");
			}
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
			ID* operandId = GetExpressionOperandId(&right, &type);


			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer or float");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar of type integer or float");
			}

			if (*e.castType == type) {
				Log::CompilerWarning(e.parent, "Unnecessary cast");
				expressions.RemoveAt(i);
			} else {
				right.type = ExpressionType::Result;
				right.result = Cast(e.castType, type, operandId, &e.parent);

				if (right.result.id == nullptr) {
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
				if (right.variable->isConstant) {
					Log::CompilerError(right.parent, "Right hand operand must be a modifiable value");
				} else if (!Utils::CompareEnums(right.variable->type->type, CompareOperation::Or, Type::Float, Type::Int)) {
					Log::CompilerError(right.parent, "Right hand operand of must be a interger/float scalar");
				}

				const Variable* var = right.variable;

				InstLoad* load = new InstLoad(var->type->typeId, var->variableId, 0);
				InstBase* operation = nullptr;

				switch (var->type->type) {
					case Type::Int:
						operation = new InstIAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1U : ~0U));
						break;
					case Type::Float:
						operation = new InstFAdd(var->type->typeId, load->id, CreateConstant(var->type, e.operatorType == TokenType::OperatorIncrement ? 1.0f : -1.0f));
						break;
				}

				InstStore* store = new InstStore(var->variableId, operation->id, 0);

				instructions.Add(load);
				instructions.Add(operation);
				instructions.Add(store);

				right.type = ExpressionType::Result;
				right.result.isVariable = false;
				right.result.type = var->type;
				right.result.id = operation->id;

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
			ID* operandId = GetExpressionOperandId(&right, &type);

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
			right.result.isVariable = false;
			right.result.type = type;
			right.result.id = operation->id;

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorLogicalNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* rType = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &rType);

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

			right.result.type = CreateTypeBool();
			ID* retTypeId = right.result.type->typeId;

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

			right.result.isVariable = false;
			right.result.id = operation->id;

			expressions.RemoveAt(i);
		} else if (e.operatorType == TokenType::OperatorBitwiseNot) {
			if (i >= expressions.GetCount() - 1) {
				Log::CompilerError(e.parent, "No right hand operand");
			}

			Expression& right = expressions[i + 1];

			TypePrimitive* type = nullptr;
			ID* operandId = GetExpressionOperandId(&right, &type);

			if (operandId == nullptr) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			if (!Utils::CompareEnums(type->type, CompareOperation::Or, Type::Int, Type::Vector)) {
				Log::CompilerError(e.parent, "Right hand operand must be a scalar or vector of type integer");
			}

			InstNot* operation = new InstNot(type->typeId, operandId);
			instructions.Add(operation);

			right.type = ExpressionType::Result;
			right.result.isVariable = false;
			right.result.type = type;
			right.result.id = operation->id;

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
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

			if (mul) {
				ret = Multiply(lType, lOperandId, rType, rOperandId, &e.parent);
			} else {
				ret = Divide(lType, lOperandId, rType, rOperandId, &e.parent);
			}

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

			if (add) {
				ret = Add(lType, lOperandId, rType, rOperandId, &e.parent);
			} else {
				ret = Subtract(lType, lOperandId, rType, rOperandId, &e.parent);
			}

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);


			if (lType->type != Type::Int && rType->type != Type::Int) {
				Log::CompilerError(e.parent, "Operands must be integers");
			}

			InstBase* instruction = nullptr;
			ResultVariable ret = { 0 };

			ID* rId = rOperandId;

			if (lType->bits != rType->bits) {
				rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;
			}

			if (e.operatorType == TokenType::OperatorRightShift) {
				instruction = new InstShiftRightLogical(lType->typeId, lOperandId, rId);
			} else {
				instruction = new InstShiftLeftLogical(lType->typeId, lOperandId, rId);
			}

			instructions.Add(instruction);

			ret.type = lType;
			ret.id = instruction->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;
			ResultVariable ret = { 0 };

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			bool floatCmp = false;

			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;
					} else if (lType->bits < rType->bits) {
						lId = ImplicitCast(rType, lType, lOperandId, &e.parent).id;
					}
				} else { // Int
					rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;
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
					lId = ImplicitCast(rType, lType, lOperandId, &e.parent).id;

					floatCmp = true;
				}
			}

			ret.type = CreateTypeBool();
			ID* retTypeId = ret.type->typeId;

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

			ret.id = instruction->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			InstBase* instruction = nullptr;
			InstBase* convInst = nullptr;
			ResultVariable ret = { 0 };

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of type int, float or bool");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			uint8 cmpType = 0;

			if (lType->type == Type::Float) {
				if (rType->type == Type::Float) { // Float
					if (lType->bits > rType->bits) {
						rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;
					} else if (lType->bits < rType->bits) {
						lId = ImplicitCast(rType, lType, rOperandId, &e.parent).id;
					}
				} else if (rType->type == Type::Int) { // Int
					rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;
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
					rId = ImplicitCast(lType, rType, rOperandId, &e.parent).id;

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

			ret.type = CreateTypeBool();
			ID* retTypeId = ret.type->typeId;

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

			ret.id = instruction->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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

		if (Utils::CompareEnums(e.operatorType, CompareOperation::Or, TokenType::OperatorEqual, TokenType::OperatorNotEqual)) {
			Expression& left = expressions[i - 1];
			Expression& right = expressions[1 + 1];

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

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

			ret.type = lType;
			ret.id = inst->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

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

			ret.type = lType;
			ret.id = inst->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

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

			ret.type = lType;
			ret.id = inst->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			ResultVariable ret = { 0 };

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

			ret.type = lType;
			ret.id = inst->id;

			left.type = ExpressionType::Result;
			left.result = ret;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of bool, int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			TypeBase* retType = CreateTypeBool();

			if (lType->type != Type::Bool) {
				ResultVariable r = Cast(retType, lType, lOperandId, &left.parent);
				lId = r.id;
			}

			if (rType->type != Type::Bool) {
				ResultVariable r = Cast(retType, rType, rOperandId, &right.parent);
				rId = r.id;
			}

			InstBase* instruction = new InstLogicalAnd(retType->typeId, lId, rId);

			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.result.type = retType;
			left.result.isConstant = false;
			left.result.isVariable = false;
			left.result.id = instruction->id;
			left.variable = nullptr;

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
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float)) {
				Log::CompilerError(e.parent, "Operands must be a scalar of bool, int or float");
			}

			ID* lId = lOperandId;
			ID* rId = rOperandId;

			TypeBase* retType = CreateTypeBool();

			if (lType->type != Type::Bool) {
				ResultVariable r = Cast(retType, lType, lOperandId, &left.parent);
				lId = r.id;
			}

			if (rType->type != Type::Bool) {
				ResultVariable r = Cast(retType, rType, rOperandId, &right.parent);
				rId = r.id;
			}

			InstBase* instruction = new InstLogicalOr(retType->typeId, lId, rId);

			instructions.Add(instruction);

			left.type = ExpressionType::Result;
			left.result.type = retType;
			left.result.isConstant = false;
			left.result.isVariable = false;
			left.result.id = instruction->id;
			left.variable = nullptr;

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
			Expression& right = expressions[1 + 1];

			if (left.type != ExpressionType::Variable) Log::CompilerError(e.parent, "Left hand operand must be a lvalue");

			TypePrimitive* lType = nullptr;
			TypePrimitive* rType = nullptr;
			ID* lOperandId = GetExpressionOperandId(&left, &lType);
			ID* rOperandId = GetExpressionOperandId(&right, &rType);

			if (!Utils::CompareEnums(lType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector, Type::Matrix) || !Utils::CompareEnums(rType->type, CompareOperation::Or, Type::Bool, Type::Int, Type::Float, Type::Vector, Type::Matrix)) {
				Log::CompilerError(e.parent, "Operands must be a of valid type");
			}

			if (lType->type != rType->type) {
				ResultVariable tmp = ImplicitCast(lType, rType, rOperandId, &right.parent);

				rType = (TypePrimitive*)tmp.type;
				rOperandId = tmp.id;
			}

			ResultVariable tmp;

			tmp.id = rOperandId;

			switch (e.operatorType) {
				case TokenType::OperatorAssign:
					instructions.RemoveAt(instructions.GetCount() - 1); //Variable is loaded in GetExpressionOperandId but isn't needed when only assigning a value.
					break;
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
			}


			instructions.Add(new InstStore(left.variable->variableId, tmp.id, 0));

			expressions.Remove(i, i + 1);
			i--;
		}
	}

#pragma endregion

	instructions.Add(postIncrements);

	ResultVariable result = { 0 };

	if (expressions.GetCount() > 1) {
		const Expression& e = expressions[1];
		Log::CompilerError(e.parent, "Unexpected symbol \"%s\"", e.parent.string.str);
	}

	Expression e = expressions[0];

	switch (e.type) {
		case ExpressionType::Variable:
			result.isVariable = true;
			result.type = e.variable->type;
			result.id = e.variable->variableId;
			break;
		case ExpressionType::Constant:
			result.isConstant = true;
		case ExpressionType::Result:
			result = e.result;
			break;
	}

	return result;
}

}}}