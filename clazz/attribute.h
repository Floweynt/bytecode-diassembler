#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include "constant_pool.h"

struct attribute_info
{
    uint16_t attribute_name_index;
    std::vector<uint8_t> buffer;
};

struct signature_attribute
{
    uint16_t signature_index;
};

struct source_file_attribute
{
    uint16_t sourcefile_index;
};

struct lvt_attribute
{
    struct lvt_entry
    {
        uint16_t start_pc;
        uint16_t length;
        uint16_t name_index;
        uint16_t descriptor_index;
        uint16_t index;
    };

    std::vector<lvt_entry> lvt;
};

struct inner_class_attribute
{
    struct inner_class_entry
    {
        uint16_t inner_class_info_index;
        uint16_t outer_class_info_index;
        uint16_t inner_name_index;
        uint16_t inner_class_access_flags;
    };
    std::vector<inner_class_entry> inner_classes;
};

struct lineno_attribute
{
    struct lineno_entry
    {
        uint16_t start_pc;
        uint16_t line_number;
    };
    std::vector<lineno_entry> line_number_table;
};

enum verification_type_info_tag
{
    VERIFICATION_TOP = 0,
    VERIFICATION_INTEGER,
    VERIFICATION_FLOAT,
    VERIFICATION_DOUBLE,
    VERIFICATION_LONG,
    VERIFICATION_NULL,
    VERIFICATION_UNINITIALIZED_THIS,
    VERIFICATION_OBJECT,
    VERIFICATION_UNINITIALIZED
};

struct verification_type_info
{
    uint8_t tag;
    uint16_t data;
};

struct stack_map_frame
{
    uint8_t frame_type;

    struct same_frame
    {
    };
    struct same_locals_1_stack_item_frame
    {
        verification_type_info stack;
    };
    struct same_locals_1_stack_item_frame_extended
    {
        uint16_t offset_delta;
        verification_type_info stack;
    };
    struct chop_frame
    {
        uint16_t offset_delta;
    };
    struct same_frame_extended
    {
        uint16_t offset_delta;
    };
    struct append_frame
    {
        uint16_t offset_delta;
        std::vector<verification_type_info> locals;
    };
    struct full_frame
    {
        uint16_t offset_delta;
        uint16_t number_of_locals;
        std::vector<verification_type_info> locals;
        std::vector<verification_type_info> stack;
    };

    std::variant<same_frame, same_locals_1_stack_item_frame, same_locals_1_stack_item_frame_extended, chop_frame,
                 same_frame_extended, append_frame, full_frame> data;
};

struct stack_map_table_attribute
{
    std::vector<stack_map_frame> entries;
};

struct bootstrap_methods_attribute
{
    struct bootstrap_methods_entry
    {
        uint16_t bootstrap_method_ref;
        std::vector<uint16_t> bootstrap_arguments;
    };

    std::vector<bootstrap_methods_entry> bootstrap_methods;
};

struct code_attribute;
using attribute = std::variant<attribute_info, code_attribute, signature_attribute, source_file_attribute, lvt_attribute,
                 inner_class_attribute, lineno_attribute, stack_map_table_attribute, bootstrap_methods_attribute>;


struct inst;
struct code_attribute
{
    struct exception_table_entry
    {
        uint16_t start_pc;
        uint16_t end_pc;
        uint16_t handler_pc;
        uint16_t catch_type;
    };

    uint16_t max_stack;
    uint16_t max_locals;
    std::size_t max_ip;
    std::vector<inst> code;
    std::vector<exception_table_entry> exception_table;
    std::vector<attribute> attributes;
};

