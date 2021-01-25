#pragma once

#include "containers.hpp"
#include "function.hpp"

struct Module
{
    string name;
    vector<unique_ptr<Function>> functions;
    vector<unique_ptr<ExternFunction>> extern_functions;
};