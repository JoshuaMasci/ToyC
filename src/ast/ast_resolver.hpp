#pragma once

#include "types.hpp"
#include "ast/node.hpp"
#include "ast/expression.hpp"

class AstResolver
{
public:
    AstResolver();

    void resolve(ModuleNode* module);

protected:
    unordered_map<string, shared_ptr<Type>> type_map;

    void resolve_types_function(unique_ptr<FunctionNode>& function);
    void resolve_types_block(unique_ptr<FunctionNode> &function, unique_ptr<BlockNode>& block);
    shared_ptr<Type> resolve_type(shared_ptr<Type> unresolved_type);

    void resolve_types_expression(unique_ptr<ExpressionNode>& expression, shared_ptr<Type> required_type);
};