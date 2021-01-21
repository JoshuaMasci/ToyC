#pragma once

#include "containers.hpp"
#include "ast/expression.hpp"
#include "ast/type.hpp"

enum class StatementType
{
    Declaration,
    Assignment,
    Block,
    If,
    While,
    Return,
};

struct BlockNode;

struct StatementNode
{
    StatementNode(StatementType type) : statement_type(type){};
    const StatementType statement_type;
};

struct DeclarationStatementNode : StatementNode
{
    shared_ptr<Type> type;

    std::string name;
    unique_ptr<ExpressionNode> expression;

    DeclarationStatementNode(const string& type, const string& name, ExpressionNode* expression)
    :StatementNode(StatementType::Declaration)
    {
        //printf("DeclarationStatementNode %s %s\n", type.c_str(), name.c_str());
        this->type = std::make_shared<UnresolvedType>(type);
        this->name = name;
        this->expression = unique_ptr<ExpressionNode>(expression);
    };
};

struct AssignmentStatementNode : StatementNode
{
    std::string name;
    unique_ptr<ExpressionNode> expression;

    AssignmentStatementNode(const string& name, ExpressionNode* expression)
    :StatementNode(StatementType::Assignment)
    {
        //printf("AssignmentStatementNode %s\n", name.c_str());
        this->name = name;
        this->expression = unique_ptr<ExpressionNode>(expression);
    };
};

struct BlockStatementNode : StatementNode
{
    unique_ptr<BlockNode> block;

    BlockStatementNode(BlockNode* block)
    :StatementNode(StatementType::Assignment)
    {
        //printf("BlockStatementNode\n");
        this->block = unique_ptr<BlockNode>(block);
    };
};

struct IfStatementNode : StatementNode
{
    unique_ptr<ExpressionNode> condition;
    unique_ptr<BlockNode> if_block;
    unique_ptr<BlockNode> else_block;

    IfStatementNode(ExpressionNode* condition, BlockNode* if_block, BlockNode* else_block)
    :StatementNode(StatementType::If)
    {
        this->condition = unique_ptr<ExpressionNode>(condition);
        this->if_block = unique_ptr<BlockNode>(if_block);
        this->else_block = unique_ptr<BlockNode>(else_block);
    };
};

struct WhileLoopStatmentNode : StatementNode
{
    unique_ptr<ExpressionNode> condition;
    unique_ptr<BlockNode> loop_block;

    WhileLoopStatmentNode(ExpressionNode* condition, BlockNode* block)
    :StatementNode(StatementType::While)
    {
        this->condition = unique_ptr<ExpressionNode>(condition);
        this->loop_block = unique_ptr<BlockNode>(block);
    };
};

struct ReturnStatmentNode : StatementNode
{
    unique_ptr<ExpressionNode> return_expression;

    ReturnStatmentNode(ExpressionNode* expression)
    :StatementNode(StatementType::Return)
    {
        //printf("ReturnStatmentNode: %p\n", expression);
        this->return_expression = unique_ptr<ExpressionNode>(expression);
    };
};