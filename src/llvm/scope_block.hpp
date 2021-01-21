#pragma once

#include "containers.hpp"
#include "ast/types.hpp"

#include <llvm/IR/Instructions.h>

struct VariableAllocation
{
    shared_ptr<Type> type;
    llvm::AllocaInst* allocation = nullptr;
};

class ScopeBlock
{
    protected:
    ScopeBlock* parent;
    unordered_map<string, VariableAllocation> variables;

    public:
    ScopeBlock(ScopeBlock* parent);
    void addLocalVariable(const string& name, VariableAllocation variable);
    VariableAllocation getLocalVariable(const string& name);
};