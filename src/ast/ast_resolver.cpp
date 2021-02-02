#include "ast_resolver.hpp"

void GlobalScope::add_function(const string &name, const FunctionType& function_type)
{
    if(this->functions_types.find(name) != this->functions_types.end())
    {
        printf("Error: Function %s redefined\n", name.c_str());
        exit(-1);
    }

    this->functions_types[name] = function_type;
}

FunctionType GlobalScope::get_function_type(const string &name)
{
    auto find_it = this->functions_types.find(name);
    if(find_it != this->functions_types.end())
    {
        return find_it->second;
    }

    printf("Error: No variable with name %s \n", name.c_str());
    exit(-1);
}

LocalScope::LocalScope(LocalScope *parent_scope, GlobalScope* global_scope)
:parent_scope(parent_scope)
{
    if(parent_scope != nullptr)
    {
        this->global_scope = parent_scope->global_scope;
    }
    else
    {
        this->global_scope = global_scope;
    }
}

void LocalScope::add_variable(const string &name, shared_ptr<Type> variable_type)
{
    if(this->variable_types.find(name) != this->variable_types.end())
    {
        printf("Error: Variable %s redefined\n", name.c_str());
        exit(-1);
    }

    this->variable_types[name] = variable_type;
}

shared_ptr<Type> LocalScope::get_variable_type(const string &name)
{
    auto find_it = this->variable_types.find(name);
    if(find_it != this->variable_types.end())
    {
        return find_it->second;
    }
    else
    {
        if(this->parent_scope != nullptr)
        {
            return this->parent_scope->get_variable_type(name);
        }
    }

    printf("Error: No variable with name %s \n", name.c_str());
    exit(-1);
}

FunctionType LocalScope::get_function_type(const string &name)
{
    return this->global_scope->get_function_type(name);
}

AstResolver::AstResolver()
{
    //Add Primitive Types
    this->type_map["void"] = std::make_shared<VoidType>();
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
}

//This (admittedly poorly named) function will process the whole module and remove any ambiguity from the AST.
//Most notably this function will determine the appropriate Bin Op to use
void AstResolver::resolve(Module* module)
{
    GlobalScope global_scope;

    for(auto& struct_object: module->structs)
    {
        this->resolve_types_struct(struct_object);
    }

    for(auto& function: module->extern_functions)
    {
        this->resolve_types_extern(function, &global_scope);
    }

    for(auto& function: module->functions)
    {
        this->resolve_types_function(function, &global_scope);
    }

    for(auto& function: module->functions)
    {
        this->resolve_types_function_block(function, &global_scope);
    }

    //TODO process condition expressions to create required casting/comparisons for If/Loop statements
}

void AstResolver::resolve_types_struct(unique_ptr<Struct> &struct_object)
{
    printf("Struct Type: %s\n", struct_object->name.c_str());

    for(size_t i = 0; i < struct_object->members.size(); i++)
    {
        struct_object->members[i].type = this->resolve_type(struct_object->members[i].type);
    }
}

void AstResolver::resolve_types_extern(unique_ptr<ExternFunction>& function, GlobalScope* global_scope)
{
    FunctionType function_type;

    function->return_type = this->resolve_type(function->return_type);
    function_type.return_type = function->return_type;

    for(size_t i = 0; i < function->parameters.size(); i++)
    {
        function->parameters[i].type = this->resolve_type(function->parameters[i].type);
        function_type.arguments.push_back(function->parameters[i].type);
    }
    global_scope->add_function(function->name, function_type);
}

void AstResolver::resolve_types_function(unique_ptr<Function> &function, GlobalScope* global_scope)
{
    FunctionType function_type;

    function->return_type = this->resolve_type(function->return_type);
    function_type.return_type = function->return_type;

    for(size_t i = 0; i < function->parameters.size(); i++)
    {
        function->parameters[i].type = this->resolve_type(function->parameters[i].type);
        function_type.arguments.push_back(function->parameters[i].type);
    }
    global_scope->add_function(function->name, function_type);
}


void AstResolver::resolve_types_function_block(unique_ptr<Function> &function, GlobalScope *global_scope)
{
    //TODO add global variables
    LocalScope function_scope(nullptr, global_scope);
    for(FunctionParameter& parameter: function->parameters)
    {
        function_scope.add_variable(parameter.name, parameter.type);
    }
    this->resolve_types_block(function, function->block, &function_scope);
}

void AstResolver::resolve_types_block(unique_ptr<Function>& function, unique_ptr<Block>& block, LocalScope* parent_scope)
{
    LocalScope block_scope(parent_scope);

    for(unique_ptr<Statement>& statement: block->statements)
    {
        switch (statement->statement_type)
        {
            case StatementType::Declaration:
            {
                DeclarationStatement *declaration_node = (DeclarationStatement *)statement.get();
                declaration_node->type = this->resolve_type(declaration_node->type);
                this->resolve_types_expression(declaration_node->expression, declaration_node->type, &block_scope);
                block_scope.add_variable(declaration_node->name, declaration_node->type);
            }
                break;
            case StatementType::Assignment:
            {
                AssignmentStatement* assignment_node = (AssignmentStatement*)statement.get();
                shared_ptr<Type> type = block_scope.get_variable_type(assignment_node->name);
                this->resolve_types_expression(assignment_node->expression, type, &block_scope);
            }
                break;
            case StatementType::Block:
                this->resolve_types_block(function, ((BlockStatement*)statement.get())->block, &block_scope);
                break;
            case StatementType::FunctionCall:
            {
                //Function Call statement doesn't care about return type
                FunctionCallStatement* function_call = (FunctionCallStatement*)statement.get();
                FunctionType function_type = block_scope.get_function_type(function_call->function_name);
                for(size_t i = 0; i < function_call->arguments.size(); i++)
                {
                    this->resolve_types_expression(function_call->arguments[i], function_type.arguments[i], &block_scope);
                }
            }
                break;
            case StatementType::If:
                break;
            case StatementType::While:
                break;
            case StatementType::Return:
                this->resolve_types_expression(((ReturnStatement*)statement.get())->return_expression, function->return_type, &block_scope);
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
    return iterator->second;
}

TypeClass get_type_class(unique_ptr<Expression>& expression, LocalScope* local_scope)
{
    switch (expression->expression_type)
    {
        case ExpressionType::ConstInt:
            return TypeClass::Int;
        case ExpressionType::ConstFloat:
            return TypeClass::Float;
        case ExpressionType::Identifier:
            return local_scope->get_variable_type(((IdentifierExpression*) expression.get())->identifier_name)->get_class();
        case ExpressionType::BinaryOperator:
        {
            BinaryOperatorExpression* bin_op = (BinaryOperatorExpression*)expression.get();
            return get_type_class(bin_op->lhs, local_scope);
        }
        case ExpressionType::Function:
            return local_scope->get_function_type(((FunctionCallExpression*)expression.get())->function_name).return_type->get_class();
    }

    return TypeClass::Invalid;
}

void AstResolver::resolve_types_expression(unique_ptr<Expression>& expression, shared_ptr<Type> required_type, LocalScope* local_scope)
{
    switch (expression->expression_type)
    {
        case ExpressionType::ConstInt:
        {
            ConstantIntegerExpression* const_int = (ConstantIntegerExpression*)expression.get();
            const_int->resolve_value(required_type);
        }
            break;
        case ExpressionType::ConstFloat:
        {
            ConstantDoubleExpression* const_float = (ConstantDoubleExpression*)expression.get();
            const_float->resolve_value(required_type);
        }
            break;
        case ExpressionType::Identifier:
        {
            shared_ptr<Type> variable_type = local_scope->get_variable_type(((IdentifierExpression*) expression.get())->identifier_name);
            if(variable_type != required_type)
            {
                printf("Error: type mismatch");
                exit(-1);
            }
        }
            break;
        case ExpressionType::Function:
        {
            FunctionCallExpression* function_call = (FunctionCallExpression*)expression.get();
            FunctionType function_type = local_scope->get_function_type(function_call->function_name);

            if(required_type != function_type.return_type)
            {
                printf("Error: type mismatch!!!\n");
                exit(-1);
            }

            for(size_t i = 0; i < function_type.arguments.size(); i++)
            {
                this->resolve_types_expression(function_call->arguments[i], function_type.arguments[i], local_scope);
            }
        }
            break;
        case ExpressionType::BinaryOperator:
        {
            BinaryOperatorExpression* bin_op_node = (BinaryOperatorExpression*) expression.get();
            TypeClass lhs_type = get_type_class(bin_op_node->lhs, local_scope);

            //In the case of int or float, both lhs and rhs are assumed to be the same type and that type should match the required type
            if(lhs_type == TypeClass::Int)
            {
                IntType* int_type = (IntType*) required_type.get();
                this->resolve_types_expression(bin_op_node->lhs, required_type, local_scope);
                this->resolve_types_expression(bin_op_node->rhs, required_type, local_scope);

                switch (bin_op_node->op)
                {
                    case MathOperator::ADD:
                        bin_op_node->binary_op = BinaryOperator::Iadd;
                        break;
                    case MathOperator::SUB:
                        bin_op_node->binary_op = BinaryOperator::Isub;
                        break;
                    case MathOperator::MUL:
                        bin_op_node->binary_op = BinaryOperator::Imul;
                        break;
                    case MathOperator::DIV:
                    {
                        if (int_type->is_signed())
                        {
                            bin_op_node->binary_op = BinaryOperator::Idiv;
                        }
                        else
                        {
                            bin_op_node->binary_op = BinaryOperator::Udiv;
                        }
                    }
                        break;
                    case MathOperator::MOD:
                    {
                        if (int_type->is_signed())
                        {
                            bin_op_node->binary_op = BinaryOperator::Imod;
                        }
                        else
                        {
                            bin_op_node->binary_op = BinaryOperator::Umod;
                        }
                    }
                        break;
                }
            }
            else if(lhs_type == TypeClass::Float)
            {
                this->resolve_types_expression(bin_op_node->lhs, required_type, local_scope);
                this->resolve_types_expression(bin_op_node->rhs, required_type, local_scope);

                switch (bin_op_node->op)
                {
                    case MathOperator::ADD:
                        bin_op_node->binary_op = BinaryOperator::Fadd;
                        break;
                    case MathOperator::SUB:
                        bin_op_node->binary_op = BinaryOperator::Fsub;
                        break;
                    case MathOperator::MUL:
                        bin_op_node->binary_op = BinaryOperator::Fmul;
                        break;
                    case MathOperator::DIV:
                        bin_op_node->binary_op = BinaryOperator::Fdiv;
                        break;
                    case MathOperator::MOD:
                        bin_op_node->binary_op = BinaryOperator::Fmod;
                        break;
                }
            }
            else if(lhs_type == TypeClass::Struct)
            {
                //TODO
            }

            /* If lhs type class is int:
             * - resolve lhs for required_type, error if not a type_match
             * - resolve rhs for required_type, error if not a type_match
             * - determine correct int op (signed or unsigned)
             *
             * If lhs type class is float:
             * - resolve lhs for required_type, error if not a type_match
             * - resolve rhs for required_type, error if not a type_match
             * - determine correct float op
             *
             * TODO If lhs type class is struct
             * - Will have to determine correct operator overload function based on type of LHS and return_type/required_type
             * */
        }
            break;
    }
}