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

struct Expression
{
    Expression(ExpressionType type) : expression_type(type){};
    const ExpressionType expression_type;
};

struct ConstantIntegerExpression : Expression
{
    shared_ptr<Type> int_type;
    uint64_t value;

    ConstantIntegerExpression(long value)
    :Expression(ExpressionType::ConstInt)
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

struct ConstantDoubleExpression : Expression
{
    shared_ptr<Type> float_type;
    double value;

    ConstantDoubleExpression(double value)
    :Expression(ExpressionType::ConstFloat)
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

struct IdentifierExpression : Expression
{
    std::string identifier_name;

    IdentifierExpression(const string& name)
    :Expression(ExpressionType::Identifier)
    {
        this->identifier_name = name;
    }
};

typedef vector<Expression*> FunctionArguments;
struct FunctionCallExpression : Expression
{
    std::string function_name;
    vector<unique_ptr<Expression>> arguments;

    FunctionCallExpression(const string& name, FunctionArguments* argument_list = nullptr)
    :Expression(ExpressionType::Function)
    {
        this->function_name = name;

        if(argument_list)
        {
            arguments.resize(argument_list->size());
            for(size_t i = 0; i < arguments.size(); i++)
            {
                arguments[i] = unique_ptr<Expression>(argument_list->at(i));
            }
            delete argument_list;
        }
    }
};

struct BinaryOperatorExpression : Expression
{
    MathOperator op;
    BinaryOperator binary_op = BinaryOperator::Invalid;
    unique_ptr<Expression> lhs;
    unique_ptr<Expression> rhs;

    BinaryOperatorExpression(MathOperator op, Expression* l, Expression* r)
    :Expression(ExpressionType::BinaryOperator)
    {
        this->op = op;
        this->lhs = unique_ptr<Expression>(l);
        this->rhs = unique_ptr<Expression>(r);
    };
};