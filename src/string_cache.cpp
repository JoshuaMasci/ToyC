#include "string_cache.hpp"

vector<string> StringCache::symbols;

size_t StringCache::add(string symbol)
{
    for(size_t i = 0; i < StringCache::symbols.size(); i++)
    {
        if(symbol == StringCache::symbols[i])
        {
            return i;
        }
    }

    StringCache::symbols.push_back(symbol);
    return StringCache::symbols.size() - 1;
};

string& StringCache::get(size_t id)
{
    return StringCache::symbols[id];
};

void StringCache::clear()
{
    StringCache::symbols.clear();
};