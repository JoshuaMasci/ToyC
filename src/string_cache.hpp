#pragma once

#include "containers.hpp"

class StringCache
{
    protected:
    static vector<string> symbols;

    public:
    static size_t add(string symbol);
    static string& get(size_t id);
    static void clear();
};