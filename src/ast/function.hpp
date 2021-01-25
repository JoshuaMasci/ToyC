#pragma once

#include "containers.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"

struct FunctionParameter
{
    shared_ptr<Type> type;
    string name;
};
typedef vector<FunctionParameter> FunctionParameters;

struct Function
{
    string name;
    shared_ptr<Type> return_type;
    vector<FunctionParameter> parameters;
    unique_ptr<Block> block;

    Function(const string& return_type, const string& name, FunctionParameters* parameters = nullptr, Block* block = nullptr)
    {
        this->name = name;
        this->return_type = std::make_shared<UnresolvedType>(return_type);
        this->block = unique_ptr<Block>(block);
        if(parameters)
        {
            this->parameters.resize(parameters->size());
            for(size_t i = 0; i < this->parameters.size(); i++)
            {
                this->parameters[i] = parameters->at(i);
            }
            delete parameters;
        }
    };
};

struct ExternFunction
{
    string name;
    shared_ptr<Type> return_type;
    vector<FunctionParameter> parameters;

    ExternFunction(const string& return_type, const string& name, FunctionParameters* parameters = nullptr)
    {
        this->name = name;
        this->return_type = std::make_shared<UnresolvedType>(return_type);
        if(parameters)
        {
            this->parameters.resize(parameters->size());
            for(size_t i = 0; i < this->parameters.size(); i++)
            {
                this->parameters[i] = parameters->at(i);
            }
            delete parameters;
        }
    };
};