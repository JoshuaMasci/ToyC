#pragma once

#include "containers.hpp"
#include "ast/expression.hpp"

enum class StatementType
{
    Declaration,
    Assignment,
    Block,
    FunctionCall,
    If,
    While,
    Return,
};

struct Statement
{
    Statement(StatementType type) : statement_type(type){};
    const StatementType statement_type;
};

struct Block
{
    vector<unique_ptr<Statement>> statements;

    void push_back(Statement* statement)
    {
        this->statements.push_back(unique_ptr<Statement>(statement));
    };
};

struct DeclarationStatement : Statement
{
    shared_ptr<Type> type;

    std::string name;
    unique_ptr<Expression> expression;

    DeclarationStatement(const string& type, const string& name, Expression* expression)
    : Statement(StatementType::Declaration)
    {
        this->type = std::make_shared<UnresolvedType>(type);
        this->name = name;
        this->expression = unique_ptr<Expression>(expression);
    };
};

struct AssignmentStatement : Statement
{
    std::string name;
    unique_ptr<Expression> expression;

    AssignmentStatement(const string& name, Expression* expression)
    : Statement(StatementType::Assignment)
    {
        this->name = name;
        this->expression = unique_ptr<Expression>(expression);
    };
};

struct BlockStatement : Statement
{
    unique_ptr<Block> block;

    BlockStatement(Block* block)
    : Statement(StatementType::Assignment)
    {
        this->block = unique_ptr<Block>(block);
    };
};

struct FunctionCallStatement : Statement
{
    std::string function_name;
    vector<unique_ptr<Expression>> arguments;

    FunctionCallStatement(const string& name, FunctionArguments* argument_list = nullptr)
            : Statement(StatementType::FunctionCall)
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

struct IfStatement : Statement
{
    unique_ptr<Expression> condition;
    unique_ptr<Block> if_block;
    unique_ptr<Block> else_block;

    IfStatement(Expression* condition, Block* if_block, Block* else_block)
    : Statement(StatementType::If)
    {
        this->condition = unique_ptr<Expression>(condition);
        this->if_block = unique_ptr<Block>(if_block);
        this->else_block = unique_ptr<Block>(else_block);
    };
};

struct WhileLoopStatement : Statement
{
    unique_ptr<Expression> condition;
    unique_ptr<Block> loop_block;

    WhileLoopStatement(Expression* condition, Block* block)
    : Statement(StatementType::While)
    {
        this->condition = unique_ptr<Expression>(condition);
        this->loop_block = unique_ptr<Block>(block);
    };
};

struct ReturnStatement : Statement
{
    unique_ptr<Expression> return_expression;

    ReturnStatement(Expression* expression)
    : Statement(StatementType::Return)
    {
        this->return_expression = unique_ptr<Expression>(expression);
    };
};