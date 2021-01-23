#pragma once

#include "containers.hpp"
#include "ast/types.hpp"

#include <llvm/IR/Instructions.h>

class ScopeBlock
{
    protected:
    ScopeBlock* parent;
    unordered_map<string, llvm::AllocaInst*> variables;

    public:
    ScopeBlock(ScopeBlock* parent);
    void addLocalVariable(const string& name, llvm::AllocaInst* variable);
    llvm::AllocaInst* getLocalVariable(const string& name);
};