#pragma once

#include "containers.hpp"
#include "ast/types.hpp"

enum class AccessType
{
    Public,
    Private,
};

struct StructMember
{
    AccessType access;
    shared_ptr<Type> type;
    string name;

    StructMember(bool is_public, const string& type, const string& name)
    {
        this->access = is_public ? AccessType::Public : AccessType::Private;
        this->type = std::make_shared<UnresolvedType>(type);
        this->name = name;
    }
};

typedef vector<StructMember> StructMembers;

struct Struct
{
    string name;
    StructMembers members;
    //TODO add Functions, Operators, Create/Delete Functions

    Struct(const string& name, StructMembers* members)
    {
        this->name = name;

        if(members)
        {
            this->members = *members;
            delete members;
        }
    }
};