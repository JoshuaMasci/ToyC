#pragma once

#include "containers.hpp"

/*enum class PrimitiveType
{
    VOID,
    BOOL,
    CHAR8,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT32,
    FLOAT64,
    STRUCT,
    INVALID,
};

class AstType
{
protected:
    PrimitiveType primitive_type;
    string type_name;
    AstType(PrimitiveType type, const string& name = ""):primitive_type(type), type_name(name){};

public:
    AstType():primitive_type(PrimitiveType::INVALID), type_name(""){};

    PrimitiveType get_type() { return this->primitive_type; };
    string get_name() { return this->type_name; };

    bool is_unresolved() const { return this->primitive_type == PrimitiveType::INVALID; };

    //Create Functions
    static AstType new_void() { return AstType(PrimitiveType::VOID, "void"); };
    static AstType new_bool() { return AstType(PrimitiveType::BOOL, "bool"); };
    static AstType new_char() { return AstType(PrimitiveType::CHAR8, "char8"); };
    static AstType new_uint8() { return AstType(PrimitiveType::UINT8, "u8"); };
    static AstType new_uint16() { return AstType(PrimitiveType::UINT16, "u16"); };
    static AstType new_uint32() { return AstType(PrimitiveType::UINT32, "u32"); };
    static AstType new_uint64() { return AstType(PrimitiveType::UINT64, "u64"); };
    static AstType new_int8() { return AstType(PrimitiveType::INT8, "i8"); };
    static AstType new_int16() { return AstType(PrimitiveType::INT16, "i16"); };
    static AstType new_int32() { return AstType(PrimitiveType::INT32, "i32"); };
    static AstType new_int64() { return AstType(PrimitiveType::INT64, "i64"); };
    static AstType new_f32() { return AstType(PrimitiveType::FLOAT32, "f32"); };
    static AstType new_f64() { return AstType(PrimitiveType::FLOAT64, "f64"); };
    static AstType new_struct(const string& struct_name) { return AstType(PrimitiveType::STRUCT, struct_name); };
    static AstType new_unresolved(const string& type_name) { return AstType(PrimitiveType::INVALID, type_name); };
};*/
