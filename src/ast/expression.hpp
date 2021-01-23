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

enum class BinaryOperator
{
    Iadd,
    Isub,
    Imul,
    Idiv,
    Imod,
    Udiv,
    Umod,

    Fadd,
    Fsub,
    Fmul,
    Fdiv,
    Fmod,

    Function,
    Invalid,
};

enum class ExpressionType
{
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

struct ConstantIntegerExpressionNode : ExpressionNode
{
    shared_ptr<Type> int_type;
    uint64_t value;

    ConstantIntegerExpressionNode(long value)
    :ExpressionNode(ExpressionType::ConstInt)
    {
        this->value = value;
    };

    void resolve_value(shared_ptr<Type> type)
    {
        if(type->get_class() != TypeClass::Int)
        {
            printf("Error: cannot cast int to type\n");
            exit(-1);
        }

        this->int_type = type;
    }
};

struct ConstantDoubleExpressionNode : ExpressionNode
{
    shared_ptr<Type> float_type;
    double value;

    ConstantDoubleExpressionNode(double value)
    :ExpressionNode(ExpressionType::ConstFloat)
    {
        this->value = value;
    };

    void resolve_value(shared_ptr<Type> type)
    {
        if(type->get_class() != TypeClass::Float)
        {
            printf("Error: cannot cast float to type\n");
            exit(-1);
        }

        this->float_type = type;
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

typedef vector<ExpressionNode*> FunctionArguments;
struct FunctionCallExpressionNode : ExpressionNode
{
    std::string function_name;
    vector<unique_ptr<ExpressionNode>> arguments;

    FunctionCallExpressionNode(const string& name, FunctionArguments* argument_list = nullptr)
    :ExpressionNode(ExpressionType::Function)
    {
        this->function_name = name;

        if(argument_list)
        {
            arguments.resize(argument_list->size());
            for(size_t i = 0; i < arguments.size(); i++)
            {
                arguments[i] = unique_ptr<ExpressionNode>(argument_list->at(i));
            }
            delete argument_list;
        }
    }
};

struct BinaryOperatorExpressionNode : ExpressionNode
{
    MathOperator op;
    BinaryOperator binary_op = BinaryOperator::Invalid;
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