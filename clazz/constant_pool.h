#pragma once
#include <cstdint>
#include <vector>
#include <variant>

struct class_info
{
    uint16_t name_index;
};

struct fieldref_info
{
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct methodref_info
{
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct interface_methodref_info
{
    uint16_t class_index;
    uint16_t name_and_type_index;
};

struct string_info
{
    uint16_t string_index;
};
struct integer_info
{
    int value;
};
struct float_info
{
    float value;
};

struct long_info
{
    long value;
};

struct double_info
{
    double value;
};

struct name_and_type_info
{
    uint16_t name_index;
    uint16_t descriptor_index;
};

struct utf8_info
{
    std::vector<uint8_t> bytes;
};

struct method_handle_info
{
    uint8_t reference_kind;
    uint16_t reference_index;
};

struct method_type_info
{
    uint16_t descriptor_index;
};

struct invoke_dynamic_info
{
    uint16_t bootstrap_method_attr_index;
    uint16_t name_and_type_index;
};

using cp_info = std::variant<class_info, fieldref_info, methodref_info, interface_methodref_info, string_info, integer_info,
                             float_info, long_info, double_info, name_and_type_info, utf8_info, method_handle_info,
                             method_type_info, invoke_dynamic_info, std::monostate>;
