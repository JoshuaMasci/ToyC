#pragma once

#include "containers.hpp"
#include "ast/node.hpp"
#include "llvm/scope_block.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

enum class BlockResult
{
    None,
    Returned,
};

class Module
{
public:
    Module(const string& module_name, ModuleNode* module_node);
    llvm::Type* getType(shared_ptr<Type> type);

    void print_code();
    void write_to_file(const string& file_name);

protected:
    string module_name;
    unique_ptr<llvm::LLVMContext> context;
    unique_ptr<llvm::Module> module;
    unordered_map<shared_ptr<Type>, llvm::Type*> type_map;

    llvm::Function* generate_function_prototype(unique_ptr<FunctionNode>& function_node);
    void generate_function_body(llvm::Function* function, unique_ptr<FunctionNode>& function_node);
    BlockResult generate_block(llvm::IRBuilder<>* builder, ScopeBlock* parent_scope, unique_ptr<BlockNode>& block);
    llvm::Value* generate_expression(llvm::IRBuilder<>* builder, ScopeBlock* current_scope, unique_ptr<ExpressionNode>& expression);
};