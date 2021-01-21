#include "llvm/llvm_code_gen.hpp"

#include "scope_block.hpp"

#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/Optional.h"

Module::Module(const string& module_name, ModuleNode* module_node)
{
    this->context = std::make_unique<llvm::LLVMContext>();
    this->module = std::make_unique<llvm::Module>(module_name, *this->context);

    ScopeBlock global_block(nullptr);

    //Functions
    vector<llvm::Function*> function_prototypes(module_node->functions.size());
    for(size_t i = 0; i < module_node->functions.size(); i++)
    {
        function_prototypes[i] = this->generate_function_prototype(module_node->functions[i]);
    }
    for(size_t i = 0; i < module_node->functions.size(); i++)
    {
        this->generate_function_body(function_prototypes[i], module_node->functions[i]);
    }
}

llvm::Type* Module::getType(shared_ptr<Type> type)
{
    auto type_it = this->type_map.find(type);
    if(type_it == this->type_map.end())
    {
        llvm::Type* llvm_type = nullptr;
        switch (type->get_class())
        {
            case TypeClass::Int:
            {
                IntType* int_type = (IntType*)type.get();
                llvm_type = llvm::Type::getIntNTy(*this->context, int_type->size_in_bits());
            }
                break;
            case TypeClass::Float:
            {
                FloatType* float_type = (FloatType*)type.get();
                if(float_type->is_f32())
                {
                    llvm_type = llvm::Type::getFloatTy(*this->context);
                }
                else
                {
                    llvm_type = llvm::Type::getDoubleTy(*this->context);
                }
            }
                break;
            case TypeClass::Struct:
                //TODO: this
                break;
            default:
                printf("Invalid Type during codegen!!!");
                exit(-1);
        }

        this->type_map[type] = llvm_type;
    }

    return this->type_map[type];
}

llvm::Function* Module::generate_function_prototype(unique_ptr<FunctionNode>& function_node)
{
    llvm::Type* return_type = this->getType(function_node->return_type);
    vector<llvm::BasicBlock*> llvm_basic_block_list;
    vector<llvm::Type*> arg_types;
    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, makeArrayRef(arg_types), false);
    llvm::Function* llvm_function = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, function_node->name,*this->module);
    llvm_function->setCallingConv(llvm::CallingConv::C);
    return llvm_function;
}

void Module::generate_function_body(llvm::Function* function, unique_ptr<FunctionNode>& function_node)
{
    llvm::BasicBlock* llvm_block = llvm::BasicBlock::Create(*this->context, "entry", function);
    llvm::IRBuilder<> builder(llvm_block);

    ScopeBlock global_scope(nullptr);
    if(this->generate_block(&builder, nullptr, function_node->block, function_node->return_type) != BlockResult::Returned)
    {
        builder.CreateRet(nullptr);
    }
}

BlockResult Module::generate_block(llvm::IRBuilder<>* builder, ScopeBlock* parent_scope, unique_ptr<BlockNode>& block, shared_ptr<Type> return_type)
{
    llvm::IRBuilder<>* current_builder = builder;

    ScopeBlock current_scope(parent_scope);
    for(unique_ptr<StatementNode>& statement : block->statements)
    {
        switch (statement->statement_type)
        {
            case StatementType::Declaration:
            {
                DeclarationStatementNode *declaration_node = (DeclarationStatementNode*) statement.get();
                llvm::Type* variable_type = this->getType(declaration_node->type);

                llvm::AllocaInst* alloc = current_builder->CreateAlloca(variable_type, nullptr, declaration_node->name);
                current_scope.addLocalVariable(declaration_node->name, {declaration_node->type, alloc});
                if (declaration_node->expression) {
                    llvm::Value *value = this->generate_expression(current_builder, &current_scope, declaration_node->expression, declaration_node->type);
                    current_builder->CreateStore(value, alloc);
                }
            }
                break;
            case StatementType::Assignment:
            {
                AssignmentStatementNode* assignment_node = (AssignmentStatementNode*)statement.get();
                VariableAllocation variable = current_scope.getLocalVariable(assignment_node->name);
                llvm::Value* value = this->generate_expression(current_builder, &current_scope, assignment_node->expression, variable.type);
                current_builder->CreateStore(value, variable.allocation);
            }
                break;
            case StatementType::Block:
                if(this->generate_block(current_builder, &current_scope, ((BlockStatementNode*)statement.get())->block, return_type) == BlockResult::Returned);
                {
                    return BlockResult::Returned;
                }
                break;
            case StatementType::If:
            {
                IfStatementNode* if_statement_node = (IfStatementNode*)statement.get();
                llvm::Type* temp_type = llvm::Type::getInt32Ty(*this->context);//TODO dynamic if condition type
                llvm::Value* condition_value = this->generate_expression(current_builder, &current_scope, if_statement_node->condition, shared_ptr<Type>());
                condition_value = current_builder->CreateICmpNE(condition_value, llvm::ConstantInt::get(temp_type, 0));
                llvm::Function* function = current_builder->GetInsertBlock()->getParent();

                // If only
                if(!if_statement_node->else_block)
                {
                    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_block", function);
                    llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_continue", function);

                    llvm::IRBuilder<> if_builder(if_block);
                    if(this->generate_block(&if_builder, &current_scope, if_statement_node->if_block, return_type) != BlockResult::Returned)
                    {
                        if_builder.CreateBr(continue_block);
                    }

                    current_builder->CreateCondBr(condition_value, if_block, continue_block);
                    current_builder->SetInsertPoint(continue_block);
                }
                    // If Else
                else
                {
                    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_block", function);
                    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(current_builder->getContext(), "else_block", function);
                    llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_continue", function);

                    llvm::IRBuilder<> if_builder(if_block);
                    if(this->generate_block(&if_builder, &current_scope, if_statement_node->if_block, return_type) != BlockResult::Returned)
                    {
                        if_builder.CreateBr(continue_block);
                    }

                    llvm::IRBuilder<> else_builder(else_block);
                    if(this->generate_block(&else_builder, &current_scope, if_statement_node->else_block, return_type) != BlockResult::Returned)
                    {
                        else_builder.CreateBr(continue_block);
                    }

                    current_builder->CreateCondBr(condition_value, if_block, else_block);
                    current_builder->SetInsertPoint(continue_block);
                }
            }
                break;
            case StatementType::While:
                break;
            case StatementType::Return:
            {
                ReturnStatmentNode* return_statement = (ReturnStatmentNode*)statement.get();
                llvm::Value* return_value = nullptr;
                if(return_statement->return_expression)
                {
                    return_value = this->generate_expression(current_builder, &current_scope, return_statement->return_expression, return_type);
                }
                current_builder->CreateRet(return_value);
                return BlockResult::Returned;
            }
        }
    }
    return BlockResult::None;
}

llvm::Value* Module::generate_expression(llvm::IRBuilder<>* builder, ScopeBlock* current_scope, unique_ptr<ExpressionNode>& expression, shared_ptr<Type> expected_type)
{
    switch (expression->expression_type)
    {
        case ExpressionType::ConstInt:
        {
            IntType* int_type = (IntType*)expected_type.get();
            return llvm::ConstantInt::get(this->getType(expected_type), ((ConstantIntegerExpressionNode*)expression.get())->value, int_type->is_signed());
        }
        case ExpressionType::ConstFloat:
        {
            ConstantDoubleExpressionNode* const_float = (ConstantDoubleExpressionNode*)expression.get();
            return llvm::ConstantFP::get(this->getType(expected_type), 0.0);//TODO use newer method
        }
        case ExpressionType::Identifier:
            return builder->CreateLoad(current_scope->getLocalVariable(((IdentifierExpressionNode*)expression.get())->identifier_name).allocation, "load");
        case ExpressionType::Function:
        {
            FunctionCallExpressionNode* function_call = (FunctionCallExpressionNode*)expression.get();
            llvm::Function* called_function = this->module->getFunction(function_call->function_name);
            return builder->CreateCall(called_function);
        }
        case ExpressionType::BinaryOperator:
        {
            //TODO take into account signed/unsigned, floats, struct operator overloading
            BinaryOperatorExpressionNode* bin_op = (BinaryOperatorExpressionNode*)expression.get();
            llvm::Instruction::BinaryOps instr;
            switch (bin_op->op)
            {
                case MathOperator::ADD: instr = llvm::Instruction::Add; break;
                case MathOperator::SUB: instr = llvm::Instruction::Sub; break;
                case MathOperator::MUL: instr = llvm::Instruction::Mul; break;
                case MathOperator::DIV: instr = llvm::Instruction::UDiv; break;
                case MathOperator::MOD: instr = llvm::Instruction::URem; break;
            }
            llvm::Value* lhs_value = this->generate_expression(builder, current_scope, bin_op->lhs, expected_type);
            llvm::Value* rhs_value = this->generate_expression(builder, current_scope, bin_op->rhs, expected_type);

            //Only works for unsigned ints right now
            return builder->CreateBinOp(instr, lhs_value, rhs_value);
        }
    }

    return nullptr;
}

void Module::print_code()
{
    this->module->print(llvm::errs(), nullptr);
}

void Module::write_to_file(const string& file_name)
{
    std::error_code EC;
    llvm::sys::fs::OpenFlags flags = (llvm::sys::fs::OpenFlags)0;
    llvm::raw_fd_ostream OS(file_name, EC, flags);
    llvm::WriteBitcodeToFile(*module, OS);
    OS.flush();
    OS.close();
}
