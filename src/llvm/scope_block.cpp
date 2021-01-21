#include "scope_block.hpp"

ScopeBlock::ScopeBlock(ScopeBlock* parent)
{
    this->parent = parent;
};

void ScopeBlock::addLocalVariable(const string& name, VariableAllocation variable)
{
    if(this->variables.find(name) != this->variables.end())
    {
        printf("Error: Local variable %s redefined\n", name.c_str());
    }

    this->variables[name] = variable;
};

VariableAllocation ScopeBlock::getLocalVariable(const string& name)
{
    auto find_it = this->variables.find(name);
    if(find_it != this->variables.end())
    {
        return find_it->second;
    }
    else
    {
        if(this->parent != nullptr)
        {
            return this->parent->getLocalVariable(name);
        }
    }
    
    return VariableAllocation();
};