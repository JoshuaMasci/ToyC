%{
    #include "string_cache.hpp"
    #include "ast/module.hpp"
    #include "ast/expression.hpp"
    #include "ast/statement.hpp"
    #include "ast/types.hpp"

    extern int yylex();
    void yyerror(const char *s) { std::printf("Error: %s\n", s); std::exit(1); }

    unique_ptr<Module> ast_module;
%}

/* Represents the many different ways we can access our data */
%union {
	long int_val;
	double double_val;
	size_t string_id;
	Module* module_ptr;
    ExternFunction* extern_function;
    Function* function_ptr;
	FunctionParameters* function_parameters;
    Block* block_ptr;
    Statement* statement_ptr;
	Expression* expression_ptr;
    FunctionArguments* function_arguments;
}

//Keywords
%token RETURN IF ELSE WHILE FOR DO CONTINUE BREAK
%token STRUCT ENUM UNION INTERFACE

//Symbols
%token SEMI LPAREN RPAREN LBRACE RBRACE LBRACK RBRACK LARROW RARROW DOT COMMA ASSIGN

//Binary Ops
%token ADD SUB MUL DIV MOD

//Comparison Ops
%token EQUAL NOT_EQUAL LESS LESS_EQUAL GREATER GREATER_EQUAL

%token <int_val> INTEGER
%token <double_val> FLOAT
%token <string_id> IDENTIFIER

%type <module_ptr> module
%type <extern_function> extern
%type <function_ptr> function
%type <function_parameters> parameters
%type <block_ptr> block
%type <statement_ptr> statement
%type <expression_ptr> expression
%type <function_arguments> arguments

//Supposedly enforces operator precedence
//Need to test
%left ADD SUB
%left MUL DIV MOD

%start file

%%
file: module { ast_module = unique_ptr<Module>($<module_ptr>1); };

module: function { Module* module = new Module(); module->functions.push_back(unique_ptr<Function>($<function_ptr>1)); $$ = module; }
    | extern { Module* module = new Module(); module->extern_functions.push_back(unique_ptr<ExternFunction>($<extern_function>1)); $$ = module; }
    | module function { $1->functions.push_back(unique_ptr<Function>($<function_ptr>2)); }
    | module extern { $1->extern_functions.push_back(unique_ptr<ExternFunction>($<extern_function>2)); }
    ;

function: IDENTIFIER IDENTIFIER LPAREN RPAREN LBRACE block RBRACE { $$ = new Function(StringCache::get($<string_id>1), StringCache::get($<string_id>2), nullptr, $<block_ptr>6); }
        | IDENTIFIER IDENTIFIER LPAREN parameters RPAREN LBRACE block RBRACE { $$ = new Function(StringCache::get($<string_id>1), StringCache::get($<string_id>2), $<function_parameters>4, $<block_ptr>7); }
        ;

extern: IDENTIFIER IDENTIFIER LPAREN RPAREN SEMI { $$ = new ExternFunction(StringCache::get($<string_id>1), StringCache::get($<string_id>2)); }
      | IDENTIFIER IDENTIFIER LPAREN parameters RPAREN SEMI { $$ = new ExternFunction(StringCache::get($<string_id>1), StringCache::get($<string_id>2), $<function_parameters>4); }
      ;

parameters: IDENTIFIER IDENTIFIER { FunctionParameters* parameters = new FunctionParameters(); parameters->push_back({std::make_shared<UnresolvedType>(StringCache::get($<string_id>1)), StringCache::get($<string_id>2)}); $$ = parameters; }
        | parameters COMMA IDENTIFIER IDENTIFIER { $1->push_back({std::make_shared<UnresolvedType>(StringCache::get($<string_id>3)), StringCache::get($<string_id>4)}); }
        ;

block: statement { Block* block = new Block(); block->push_back($<statement_ptr>1); $$ = block; }
	| block statement { $1->push_back($<statement_ptr>2); }
	;

statement: RETURN expression SEMI { $$ = new ReturnStatement($<expression_ptr>2); }
		| IDENTIFIER IDENTIFIER ASSIGN expression SEMI { $$ = new DeclarationStatement(StringCache::get($<string_id>1), StringCache::get($<string_id>2), $<expression_ptr>4); }
        | IDENTIFIER ASSIGN expression SEMI { $$ = new AssignmentStatement(StringCache::get($<string_id>1), $<expression_ptr>3); }
        | IDENTIFIER LPAREN RPAREN SEMI { $$ = new FunctionCallStatement(StringCache::get($<string_id>1)); }
		| IDENTIFIER LPAREN arguments RPAREN SEMI { $$ = new FunctionCallStatement(StringCache::get($<string_id>1), $<function_arguments>3); }
		| IF LPAREN expression RPAREN LBRACE block RBRACE { $$ = new IfStatement($<expression_ptr>3, $<block_ptr>6, nullptr); }
        | IF LPAREN expression RPAREN LBRACE block RBRACE ELSE LBRACE block RBRACE { $$ = new IfStatement($<expression_ptr>3, $<block_ptr>6, $<block_ptr>10); }
		;

expression: INTEGER { $$ = new ConstantIntegerExpression($<int_val>1); }
		| FLOAT {$$ = new ConstantDoubleExpression($<double_val>1); }
		| IDENTIFIER { $$ = new IdentifierExpression(StringCache::get($<string_id>1)); }
		| LPAREN expression RPAREN { $$ = $<expression_ptr>2; }
		| expression ADD expression { $$ = new BinaryOperatorExpression(MathOperator::ADD, $<expression_ptr>1, $<expression_ptr>3); }
		| expression SUB expression { $$ = new BinaryOperatorExpression(MathOperator::SUB, $<expression_ptr>1, $<expression_ptr>3); }
		| expression MUL expression { $$ = new BinaryOperatorExpression(MathOperator::MUL, $<expression_ptr>1, $<expression_ptr>3); }
		| expression DIV expression { $$ = new BinaryOperatorExpression(MathOperator::DIV, $<expression_ptr>1, $<expression_ptr>3); }
		| expression MOD expression { $$ = new BinaryOperatorExpression(MathOperator::MOD, $<expression_ptr>1, $<expression_ptr>3); }
		| IDENTIFIER LPAREN RPAREN { $$ = new FunctionCallExpression(StringCache::get($<string_id>1)); }
		| IDENTIFIER LPAREN arguments RPAREN { $$ = new FunctionCallExpression(StringCache::get($<string_id>1), $<function_arguments>3); }
		;

arguments: expression { FunctionArguments* function_arguments = new FunctionArguments(); function_arguments->push_back($<expression_ptr>1); $$ = function_arguments; }
            | arguments COMMA expression { $1->push_back($<expression_ptr>3); }
            ;
%%
