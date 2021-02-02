#pragma once

#include "containers.hpp"
#include "struct.hpp"
#include "function.hpp"

struct Module
{
    string name;
    //TODO remove unique_ptr from these???
    vector<unique_ptr<Struct>> structs;
    vector<unique_ptr<Function>> functions;
    vector<unique_ptr<ExternFunction>> extern_functions;
};