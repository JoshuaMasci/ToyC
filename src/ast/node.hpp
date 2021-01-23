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

struct FunctionParameter
{
    shared_ptr<Type> type;
    string name;
};
typedef vector<FunctionParameter> FunctionParameters;

struct FunctionNode
{
    string name;
    shared_ptr<Type> return_type;
    vector<FunctionParameter> parameters;
    unique_ptr<BlockNode> block;

    FunctionNode(const string& type, const string& name, BlockNode* block, FunctionParameters* function_parameters = nullptr)
    {
        //printf("FunctionNode %s %s\n", type.c_str(), name.c_str());
        this->name = name;
        this->return_type = std::make_shared<UnresolvedType>(type);
        this->block = unique_ptr<BlockNode>(block);

        if(function_parameters)
        {
            parameters.resize(function_parameters->size());
            for(size_t i = 0; i < this->parameters.size(); i++)
            {
                this->parameters[i] = function_parameters->at(i);
            }
            delete function_parameters;
        }
    };
};

struct ModuleNode
{
    string name;
    //Globals
    //Stucts,Typedefs,etc
    vector<unique_ptr<FunctionNode>> functions;
};