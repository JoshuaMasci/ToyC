%{
    #include "string_cache.hpp"
    #include "ast/node.hpp"
    #include "ast/expression.hpp"
    #include "ast/statement.hpp"
    #include "ast/types.hpp"

    extern int yylex();
    void yyerror(const char *s) { std::printf("Error: %s\n", s); std::exit(1); }

    unique_ptr<FunctionNode> main_func;
    unique_ptr<ModuleNode> module_node;
%}

/* Represents the many different ways we can access our data */
%union {
	long int_val;
	double double_val;
	size_t string_id;
	ModuleNode* module_node;
	ExpressionNode* expression_node;
	StatementNode* statement_node;
	BlockNode* block_node;
	FunctionNode* function_node;
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
%token <string_id> FLOAT
%token <string_id> IDENTIFIER

%type <module_node> module
%type <function_node> function
%type <block_node> block
%type <statement_node> statement
%type <expression_node> expression

//Supposedly enforces operator precedence
//Need to test
%left ADD SUB
%left MUL DIV MOD

%start file

%%
file: module { module_node = unique_ptr<ModuleNode>($<module_node>1); };

module: function { ModuleNode* module = new ModuleNode(); module->functions.push_back(unique_ptr<FunctionNode>($<function_node>1)); $$ = module; }
    | module function { $1->functions.push_back(unique_ptr<FunctionNode>($<function_node>2)); }
    ;

function: IDENTIFIER IDENTIFIER LPAREN RPAREN LBRACE block RBRACE SEMI { $$ = new FunctionNode(StringCache::get($<string_id>1), StringCache::get($<string_id>2), $<block_node>6); };

block: statement { BlockNode* node = new BlockNode(); node->push_back($<statement_node>1); $$ = node; }
	| block statement { $1->push_back($<statement_node>2); }
	;

statement: RETURN expression SEMI { $$ = new ReturnStatmentNode($<expression_node>2); }
		| IDENTIFIER IDENTIFIER ASSIGN expression SEMI { $$ = new DeclarationStatementNode(StringCache::get($<string_id>1), StringCache::get($<string_id>2), $<expression_node>4); }
        | IDENTIFIER ASSIGN expression SEMI { $$ = new AssignmentStatementNode(StringCache::get($<string_id>1), $<expression_node>3); }
		| IF LPAREN expression RPAREN LBRACE block RBRACE { $$ = new IfStatementNode($<expression_node>3, $<block_node>6, nullptr); }
        | IF LPAREN expression RPAREN LBRACE block RBRACE ELSE LBRACE block RBRACE { $$ = new IfStatementNode($<expression_node>3, $<block_node>6, $<block_node>10); }
		;

expression: INTEGER { $$ = new ConstantIntegerExpressionNode($<int_val>1); }
		| FLOAT {$$ = new ConstantDoubleExpressionNode(StringCache::get($<string_id>1)); }
		| IDENTIFIER { $$ = new IdentifierExpressionNode(StringCache::get($<string_id>1)); }
		| LPAREN expression RPAREN { $$ = $<expression_node>2; }
		| expression ADD expression { $$ = new BinaryOperatorExpressionNode(MathOperator::ADD, $<expression_node>1, $<expression_node>3); }
		| expression SUB expression { $$ = new BinaryOperatorExpressionNode(MathOperator::SUB, $<expression_node>1, $<expression_node>3); }
		| expression MUL expression { $$ = new BinaryOperatorExpressionNode(MathOperator::MUL, $<expression_node>1, $<expression_node>3); }
		| expression DIV expression { $$ = new BinaryOperatorExpressionNode(MathOperator::DIV, $<expression_node>1, $<expression_node>3); }
		| expression MOD expression { $$ = new BinaryOperatorExpressionNode(MathOperator::MOD, $<expression_node>1, $<expression_node>3); }
		| LPAREN IDENTIFIER RPAREN expression {}// casting
		| IDENTIFIER LPAREN RPAREN { $$ = new FunctionCallExpressionNode(StringCache::get($<string_id>1)); }
		;
%%
