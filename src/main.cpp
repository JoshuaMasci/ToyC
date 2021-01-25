#include "containers.hpp"
#include "string_cache.hpp"
#include "ast/module.hpp"
#include "ast/ast_resolver.hpp"
#include "llvm/llvm_code_gen.hpp"

#include <stdio.h>

extern unique_ptr<Module> ast_module;

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

    if(!ast_module)
    {
        fprintf(stderr, "Filed to parse file");
        return -2;
    }

    //Resolve types, functions, consts, etc
    AstResolver().resolve(ast_module.get());

	printf("Code Gen!!!!\n");
    llvmModule module(file_name, ast_module.get());

    printf("Print Code!!!!\n");
    module.print_code();
    printf("\n");

    //module.write_to_file("module.bc");
    module.compile("module.o");

	return 0;
}