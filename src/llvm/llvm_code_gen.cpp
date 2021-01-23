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

    arg_types.reserve(function_node->parameters.size());
    for (int i = 0; i < function_node->parameters.size(); ++i)
    {
        arg_types.push_back(this->getType(function_node->parameters[i].type));
    }

    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, makeArrayRef(arg_types), false);
    llvm::Function* llvm_function = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, function_node->name,*this->module);
    llvm_function->setCallingConv(llvm::CallingConv::C);
    return llvm_function;
}

void Module::generate_function_body(llvm::Function* function, unique_ptr<FunctionNode>& function_node)
{
    llvm::BasicBlock* llvm_block = llvm::BasicBlock::Create(*this->context, "entry", function);
    llvm::IRBuilder<> builder(llvm_block);

    ScopeBlock function_scope(nullptr);

    size_t i = 0;
    for (auto& argument : function->args())
    {
        llvm::Type* variable_type = this->getType(function_node->parameters[i].type);
        llvm::AllocaInst* alloc = builder.CreateAlloca(variable_type, nullptr, function_node->parameters[i].name);
        builder.CreateStore(&argument, alloc);
        function_scope.addLocalVariable(function_node->parameters[i].name, alloc);
        i++;
    }

    if(this->generate_block(&builder, &function_scope, function_node->block) != BlockResult::Returned)
    {
        builder.CreateRet(nullptr);
    }
}

BlockResult Module::generate_block(llvm::IRBuilder<>* builder, ScopeBlock* parent_scope, unique_ptr<BlockNode>& block)
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
                current_scope.addLocalVariable(declaration_node->name, alloc);
                if (declaration_node->expression) {
                    llvm::Value *value = this->generate_expression(current_builder, &current_scope, declaration_node->expression);
                    current_builder->CreateStore(value, alloc);
                }
            }
                break;
            case StatementType::Assignment:
            {
                AssignmentStatementNode* assignment_node = (AssignmentStatementNode*)statement.get();
                llvm::AllocaInst* variable = current_scope.getLocalVariable(assignment_node->name);
                llvm::Value* value = this->generate_expression(current_builder, &current_scope, assignment_node->expression);
                current_builder->CreateStore(value, variable);
            }
                break;
            case StatementType::Block:
                if(this->generate_block(current_builder, &current_scope, ((BlockStatementNode*)statement.get())->block) == BlockResult::Returned);
                {
                    return BlockResult::Returned;
                }
                break;
            case StatementType::If:
            {
                IfStatementNode* if_statement_node = (IfStatementNode*)statement.get();
                llvm::Type* temp_type = llvm::Type::getInt32Ty(*this->context);//TODO dynamic if condition type
                llvm::Value* condition_value = this->generate_expression(current_builder, &current_scope, if_statement_node->condition);
                condition_value = current_builder->CreateICmpNE(condition_value, llvm::ConstantInt::get(temp_type, 0));
                llvm::Function* function = current_builder->GetInsertBlock()->getParent();

                // If only
                if(!if_statement_node->else_block)
                {
                    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_block", function);
                    llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(current_builder->getContext(), "if_continue", function);

                    llvm::IRBuilder<> if_builder(if_block);
                    if(this->generate_block(&if_builder, &current_scope, if_statement_node->if_block) != BlockResult::Returned)
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
                    if(this->generate_block(&if_builder, &current_scope, if_statement_node->if_block) != BlockResult::Returned)
                    {
                        if_builder.CreateBr(continue_block);
                    }

                    llvm::IRBuilder<> else_builder(else_block);
                    if(this->generate_block(&else_builder, &current_scope, if_statement_node->else_block) != BlockResult::Returned)
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
                    return_value = this->generate_expression(current_builder, &current_scope, return_statement->return_expression);
                }
                current_builder->CreateRet(return_value);
                return BlockResult::Returned;
            }
        }
    }
    return BlockResult::None;
}

llvm::Value* Module::generate_expression(llvm::IRBuilder<>* builder, ScopeBlock* current_scope, unique_ptr<ExpressionNode>& expression)
{
    switch (expression->expression_type)
    {
        case ExpressionType::ConstInt:
        {
            ConstantIntegerExpressionNode* int_node = (ConstantIntegerExpressionNode*)expression.get();
            IntType* int_type = (IntType*)int_node->int_type.get();
            return llvm::ConstantInt::get(this->getType(int_node->int_type), int_node->value, int_type->is_signed());
        }
        case ExpressionType::ConstFloat:
        {
            ConstantDoubleExpressionNode* const_float = (ConstantDoubleExpressionNode*)expression.get();
            return llvm::ConstantFP::get(this->getType(const_float->float_type), const_float->value);
        }
        case ExpressionType::Identifier:
            return builder->CreateLoad(current_scope->getLocalVariable(((IdentifierExpressionNode*)expression.get())->identifier_name), "load");
        case ExpressionType::Function:
        {
            FunctionCallExpressionNode* function_call = (FunctionCallExpressionNode*)expression.get();
            llvm::Function* called_function = this->module->getFunction(function_call->function_name);
            vector<llvm::Value*> arguments(function_call->arguments.size());
            for(size_t i = 0; i < function_call->arguments.size(); i++)
            {
                arguments[i] = this->generate_expression(builder, current_scope, function_call->arguments[i]);
            }

            return builder->CreateCall(called_function, arguments);
        }
        case ExpressionType::BinaryOperator:
        {
            BinaryOperatorExpressionNode* bin_op = (BinaryOperatorExpressionNode*)expression.get();
            llvm::Value* lhs_value = this->generate_expression(builder, current_scope, bin_op->lhs);
            llvm::Value* rhs_value = this->generate_expression(builder, current_scope, bin_op->rhs);

            switch (bin_op->binary_op)
            {
                case BinaryOperator::Iadd:
                    return builder->CreateBinOp(llvm::Instruction::Add, lhs_value, rhs_value);
                case BinaryOperator::Isub:
                    return builder->CreateBinOp(llvm::Instruction::Sub, lhs_value, rhs_value);
                case BinaryOperator::Imul:
                    return builder->CreateBinOp(llvm::Instruction::Mul, lhs_value, rhs_value);
                case BinaryOperator::Idiv:
                    return builder->CreateBinOp(llvm::Instruction::SDiv, lhs_value, rhs_value);
                case BinaryOperator::Imod:
                    return builder->CreateBinOp(llvm::Instruction::SRem, lhs_value, rhs_value);
                case BinaryOperator::Udiv:
                    return builder->CreateBinOp(llvm::Instruction::UDiv, lhs_value, rhs_value);
                case BinaryOperator::Umod:
                    return builder->CreateBinOp(llvm::Instruction::URem, lhs_value, rhs_value);

                case BinaryOperator::Fadd:
                    return builder->CreateBinOp(llvm::Instruction::FAdd, lhs_value, rhs_value);
                case BinaryOperator::Fsub:
                    return builder->CreateBinOp(llvm::Instruction::FSub, lhs_value, rhs_value);
                case BinaryOperator::Fmul:
                    return builder->CreateBinOp(llvm::Instruction::FMul, lhs_value, rhs_value);
                case BinaryOperator::Fdiv:
                    return builder->CreateBinOp(llvm::Instruction::FDiv, lhs_value, rhs_value);
                case BinaryOperator::Fmod:
                    return builder->CreateBinOp(llvm::Instruction::FRem, lhs_value, rhs_value);

                case BinaryOperator::Function:
                    //TODO
                default:
                    return nullptr;
            }
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
