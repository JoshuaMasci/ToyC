#pragma once

#include "types.hpp"
#include "ast/node.hpp"
#include "ast/expression.hpp"

struct FunctionType
{
    shared_ptr<Type> return_type;
    vector<shared_ptr<Type>> arguments;
};

class GlobalScope
{
protected:
    unordered_map<string, FunctionType> functions_types;

public:
    void add_function(const string& name, const FunctionType& variable_type);
    FunctionType get_function_type(const string& name);
};

class LocalScope
{
protected:
    GlobalScope* global_scope;
    LocalScope* parent_scope;
    unordered_map<string, shared_ptr<Type>> variable_types;

public:
    LocalScope(LocalScope* parent_scope, GlobalScope* global_scope = nullptr);
    void add_variable(const string& name, shared_ptr<Type> variable_type);
    shared_ptr<Type> get_variable_type(const string& name);
    FunctionType get_function_type(const string& name);
};

class AstResolver
{
public:
    AstResolver();

    void resolve(ModuleNode* module);

protected:
    unordered_map<string, shared_ptr<Type>> type_map;

    void resolve_types_function(unique_ptr<FunctionNode>& function, GlobalScope* global_scope);
    void resolve_types_function_block(unique_ptr<FunctionNode>& function, GlobalScope* global_scope);
    void resolve_types_block(unique_ptr<FunctionNode> &function, unique_ptr<BlockNode>& block, LocalScope* parent_scope);
    shared_ptr<Type> resolve_type(shared_ptr<Type> unresolved_type);

    void resolve_types_expression(unique_ptr<ExpressionNode>& expression, shared_ptr<Type> required_type, LocalScope* local_scope);
};