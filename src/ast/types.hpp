#pragma once

#include "containers.hpp"

enum class TypeClass
{
    Invalid = 0,
    Int,
    Float,
    Struct,
};

enum class TypeEnum
{
    Invalid = 0,
    Void,
    Bool,
    Char8,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Int8,
    Int16,
    Int32,
    Int64,
    Float32,
    Float64,
    Struct,
};

class Type
{
public:
    virtual TypeClass get_class() = 0;
    virtual TypeEnum get_type() = 0;
};

class VoidType: public Type
{
public:
    TypeClass get_class() override { return TypeClass::Invalid; };
    TypeEnum get_type() override { return TypeEnum::Void; };
};

class UnresolvedType: public Type
{
protected:
    string name;

public:
    TypeClass get_class() override { return TypeClass::Invalid; };
    TypeEnum get_type() override { return TypeEnum::Invalid; };

    UnresolvedType(const string& name): name(name){};
    string get_name() {return this->name; };
};

class IntType: public Type
{
protected:
    bool is_signed_int;
    size_t number_of_bits;
    TypeEnum type;

    void set(bool is_signed, size_t bits)
    {
       this->is_signed_int = is_signed;
       this->number_of_bits = bits;
    };
    
public:
    TypeClass get_class() override { return TypeClass::Int; };
    TypeEnum get_type() override { return this->type; };
    bool is_signed() { return this->is_signed_int; };
    size_t size_in_bits() { return this->number_of_bits; };

    IntType(TypeEnum type)
    {
        this->type = type;
        switch (this->type)
        {
            case TypeEnum::Bool:
                this->set(false, 1);
                break;
            case TypeEnum::Char8:
            case TypeEnum::Uint8:
                this->set(false, 8);
                break;
            case TypeEnum::Uint16:
                this->set(false, 16);
                break;
            case TypeEnum::Uint32:
                this->set(false, 32);
                break;
            case TypeEnum::Uint64:
                this->set(false, 64);
                break;
            case TypeEnum::Int8:
                this->set(true, 8);
                break;
            case TypeEnum::Int16:
                this->set(true, 16);
                break;
            case TypeEnum::Int32:
                this->set(true, 32);
                break;
            case TypeEnum::Int64:
                this->set(true, 64);
                break;
            default:
                printf("Error: invalid int type!!!!!!!!!");
        }
    };
};

class FloatType: public Type
{
protected:
    //If true f32
    //If false f64
    bool f32;

public:
    TypeClass get_class() override { return TypeClass::Float; };
    TypeEnum get_type() override { return this->f32 ? TypeEnum::Float32 : TypeEnum::Float64; };
    FloatType(bool is_f32) { this->f32 = is_f32; };
    bool is_f32() { return this->f32; };
};

//TODO
class StructType: public Type
{
protected:
public:
    TypeClass get_class() override { return TypeClass::Struct; };
    TypeEnum get_type() override { return TypeEnum::Struct; };
};