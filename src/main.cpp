#include "containers.hpp"
#include "string_cache.hpp"
#include "ast/node.hpp"
#include "ast/ast_resolver.hpp"
#include "llvm/llvm_code_gen.hpp"

#include <stdio.h>

extern unique_ptr<FunctionNode> main_func;
extern unique_ptr<ModuleNode> module_node;

int yyparse(void);
extern "C" FILE *yyin;

int main(int argc, char **argv)
{
    const char* file_name = "test.c_not";
	FILE *myfile = fopen(file_name, "r");

	if(!myfile)
    {
	    printf("File failed to open!!!!");
	    return -1;
    }

	yyin = myfile;

	yyparse();
    fclose(myfile);
    StringCache::clear();

    if(!module_node)
    {
        fprintf(stderr, "Filed to parse file");
        return -2;
    }

    //Resolve types, functions, consts, etc
    AstResolver().resolve(module_node.get());

	printf("Code Gen!!!!\n");
    Module module(file_name, module_node.get());

    printf("Print Code!!!!\n");
    module.print_code();
    printf("\n");

    module.write_to_file("module.bc");

	return 0;
}