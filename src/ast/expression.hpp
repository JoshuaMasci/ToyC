#pragma once

#include "containers.hpp"
#include "ast/types.hpp"

enum class MathOperator
{
    ADD,
    SUB,
    MUL,
    DIV,
    MOD
};

enum class ExpressionType
{
    NumericalConst,
    ConstInt,
    ConstFloat,
    Identifier,
    Function,
    BinaryOperator,
};

struct ExpressionNode
{
    ExpressionNode(ExpressionType type) : expression_type(type){};
    const ExpressionType expression_type;
};

struct NumericalConstantExpressionNode : ExpressionNode
{
    string constant;

    NumericalConstantExpressionNode(const string& constant)
            :ExpressionNode(ExpressionType::NumericalConst)
    {
        this->constant = constant;
    }
};

struct ConstantIntegerExpressionNode : ExpressionNode
{
    uint64_t value;

    ConstantIntegerExpressionNode(uint64_t value)
    :ExpressionNode(ExpressionType::ConstInt)
    {
        //printf("ConstantIntegerExpressionNode %d\n", value);
        this->value = value;
    };
};

struct ConstantDoubleExpressionNode : ExpressionNode
{
    enum ConstDoubleType
    {
        Unresolved,
        Float,
        Double,
    };

    ConstDoubleType float_type;
    string unresolved_value;
    union Data
    {
        double value_64;
        float value_32;
    } data;

    ConstantDoubleExpressionNode(const string& value)
    :ExpressionNode(ExpressionType::ConstFloat)
    {
        this->float_type = ConstDoubleType::Unresolved;
        this->unresolved_value = value;
    };

    void resolve_value(shared_ptr<Type> type)
    {
        if(type->get_class() == TypeClass::Float)
        {
            FloatType* new_type = (FloatType*)type.get();

            if(new_type->is_f32())
            {
                this->float_type = ConstDoubleType::Float;
                this->data.value_32 = stof(this->unresolved_value);
            }
            else
            {
                this->float_type = ConstDoubleType::Double;
                this->data.value_32 = stod(this->unresolved_value);
            }
        }
        else
        {
            //Panic here
        }
    }
};

struct IdentifierExpressionNode : ExpressionNode
{
    std::string identifier_name;

    IdentifierExpressionNode(const string& name)
    :ExpressionNode(ExpressionType::Identifier)
    {
        this->identifier_name = name;
    }
};

struct FunctionCallExpressionNode : ExpressionNode
{
    std::string function_name;
    vector<unique_ptr<ExpressionNode>> arguments;

    FunctionCallExpressionNode(const string& name)
    :ExpressionNode(ExpressionType::Function)
    {
        this->function_name = name;
    }
};

struct BinaryOperatorExpressionNode : ExpressionNode
{  
    MathOperator op;
    unique_ptr<ExpressionNode> lhs;
    unique_ptr<ExpressionNode> rhs;

    BinaryOperatorExpressionNode(MathOperator op, ExpressionNode* l, ExpressionNode* r)
    :ExpressionNode(ExpressionType::BinaryOperator)
    {
        //printf("BinaryOperatorExpressionNode op: %d\n", (int)op);
        this->op = op;
        this->lhs = unique_ptr<ExpressionNode>(l);
        this->rhs = unique_ptr<ExpressionNode>(r);
    };
};