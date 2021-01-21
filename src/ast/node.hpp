#pragma once

#include "containers.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"

struct BlockNode
{
    vector<unique_ptr<StatementNode>> statements;

    void push_back(StatementNode* statement)
    {
        this->statements.push_back(unique_ptr<StatementNode>(statement));
    };
};

struct ArgumentValue
{
    shared_ptr<Type> type;
    string name;
};

struct FunctionNode
{
    shared_ptr<Type> return_type;
    string name;
    vector<ArgumentValue> arguments;
    unique_ptr<BlockNode> block;

    FunctionNode(const string& type, const string& name, BlockNode* block)
    {
        //printf("FunctionNode %s %s\n", type.c_str(), name.c_str());
        this->return_type = std::make_shared<UnresolvedType>(type);
        this->name = name;
        this->block = unique_ptr<BlockNode>(block);
    };
};

struct ModuleNode
{
    string name;
    //Globals
    //Stucts,Typedefs,etc
    vector<unique_ptr<FunctionNode>> functions;
};