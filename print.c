#include <stdio.h>

void print_i32(int value)
{
    printf("I32: %d\n", value);
}

void print_u32(unsigned int value)
{
    printf("U32: %u\n", value);
}

void print_i64(int value)
{
    printf("I64: %ld\n", value);
}

void print_u64(int value)
{
    printf("U64: %lu\n", value);
}

void print_str(char* value)
{
    printf("string: %s\n", value);
}