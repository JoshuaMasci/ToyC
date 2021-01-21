#include "ast_resolver.hpp"

AstResolver::AstResolver()
{
    //Add Primitive Types
    this->type_map["bool"] = std::make_shared<IntType>(TypeEnum::Bool);
    this->type_map["char"] = std::make_shared<IntType>(TypeEnum::Char8);
    this->type_map["u8"] = std::make_shared<IntType>(TypeEnum::Uint8);
    this->type_map["u16"] = std::make_shared<IntType>(TypeEnum::Uint16);
    this->type_map["u32"] = std::make_shared<IntType>(TypeEnum::Uint32);
    this->type_map["u64"] = std::make_shared<IntType>(TypeEnum::Uint64);
    this->type_map["i8"] = std::make_shared<IntType>(TypeEnum::Int8);
    this->type_map["i16"] = std::make_shared<IntType>(TypeEnum::Int16);
    this->type_map["i32"] = std::make_shared<IntType>(TypeEnum::Int32);
    this->type_map["i64"] = std::make_shared<IntType>(TypeEnum::Int64);
    this->type_map["f32"] = std::make_shared<FloatType>(true);
    this->type_map["f64"] = std::make_shared<FloatType>(false);
    //this->type_map["void"] = llvm::Type::getVoidTy(*this->context);
}


void AstResolver::resolve(ModuleNode* module)
{
    //Resolve string types for all functions
    for(auto& function: module->functions)
    {
        this->resolve_types_function(function);
    }

    //TODO process constant expressions to determine exact and value


    //TODO process operators and create the expressions for those specific operations (ie int add, float add, func call)
    //TODO process condition expressions to create required casting/comparisons
}

void AstResolver::resolve_types_function(unique_ptr<FunctionNode> &function)
{
    //Resolve return and argument types
    function->return_type = this->resolve_type(function->return_type);
    this->resolve_types_block(function, function->block);
}

void AstResolver::resolve_types_block(unique_ptr<FunctionNode>& function, unique_ptr<BlockNode>& block)
{
    for(unique_ptr<StatementNode>& statement: block->statements)
    {
        switch (statement->statement_type)
        {
            case StatementType::Declaration:
            {
                DeclarationStatementNode *declaration_node = (DeclarationStatementNode *) statement.get();
                declaration_node->type = this->resolve_type(declaration_node->type);
                this->resolve_types_expression(declaration_node->expression, declaration_node->type);
            }
                break;
            case StatementType::Assignment:
            {
                //AssignmentStatementNode* assignment_node = (AssignmentStatementNode*)statement.get();
            }
                break;
            case StatementType::Block:
                this->resolve_types_block(function, ((BlockStatementNode*)statement.get())->block);
                break;
            case StatementType::If:
                break;
            case StatementType::While:
                break;
            case StatementType::Return:
                //TODO
                this->resolve_types_expression(((ReturnStatmentNode*)statement.get())->return_expression, function->return_type);
                break;
        }
    }
}

shared_ptr<Type> AstResolver::resolve_type(shared_ptr<Type> unresolved_type)
{
    string name = ((UnresolvedType*)unresolved_type.get())->get_name();
    auto iterator = this->type_map.find(name);
    if(iterator == this->type_map.end())
    {
        printf("Error: cannot resolve type: %s\n", name.c_str());
        exit(-1);
    }
    printf("Resolved %s to %d\n", name.c_str(), iterator->second->get_type());

    return iterator->second;
}

void AstResolver::resolve_types_expression(unique_ptr<ExpressionNode>& expression, shared_ptr<Type> required_type)
{
    switch (expression->expression_type)
    {
        case ExpressionType::ConstFloat:
            {
                ConstantDoubleExpressionNode* const_float = (ConstantDoubleExpressionNode*)expression.get();
                const_float->resolve_value(required_type);
            }
            break;
        case ExpressionType::Function:
            break;
        case ExpressionType::BinaryOperator:
            //Determine which bin op to use
            break;
    }
}
