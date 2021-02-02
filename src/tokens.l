%{
    #include "string_cache.hpp"
    #include "ast/module.hpp"
    #include "parser.hpp"
    #include <stdio.h>
%}

%option noyywrap noinput nounput nodefault

%%

"struct"                        return STRUCT;
"return"				        return RETURN;
"if"							return IF;
"else"							return ELSE;
"while"							return WHILE;
"for"							return FOR;
"do"							return DO;
"continue"						return CONTINUE;
"break"							return BREAK;

";"								return SEMI;
"("	          					return LPAREN;
")"					          	return RPAREN;
"{"         					return LBRACE;
"}"					          	return RBRACE;
"["         					return LBRACK;
""					          	return RBRACK;
"<"         					return LARROW;
">"					          	return RARROW;
"."         					return DOT;
","				          		return COMMA;
"="						        return ASSIGN;

"+"				          		return ADD;
"-"		          				return SUB;
"*"		          				return MUL;
"/"				          		return DIV;
"%"                             return MOD;

"=="				          	return EQUAL;
"!="			          		return NOT_EQUAL;
"<="	          				return LESS_EQUAL;
">="					        return GREATER_EQUAL;

-?[0-9]+						yylval.int_val = strtol(yytext, nullptr, 10); return INTEGER;
-?[0-9]+[.][0-9]+				yylval.double_val = strtod(yytext, nullptr); return FLOAT;
[a-zA-Z_]+[a-zA-Z_0-9]*?		yylval.string_id = StringCache::add(string(yytext, yyleng)); return IDENTIFIER;

[ \n\t\r\f]+			        ;//Whitespace
.                               printf("Unknown token!\n\n"); yyterminate();

%%