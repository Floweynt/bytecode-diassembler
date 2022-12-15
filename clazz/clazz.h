// cSpell:ignore clazz
#pragma once
#include "constant_pool.h"
#include "attribute.h"
#include "flags.h"
#include <string>

struct field_info
{
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    std::vector<attribute> attributes;
};

struct method_info
{
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;

    std::vector<attribute> attributes;
};

struct class_file
{
    uint32_t magic;
    uint16_t minor_version;
    uint16_t major_version;
    std::vector<cp_info> constant_pool;
    uint16_t access_flags;
    uint16_t this_class;
    uint16_t super_class;
    std::vector<uint16_t> interfaces;
    std::vector<field_info> fields;
    std::vector<method_info> methods;
    std::vector<attribute> attributes;
    size_t bootstrap_index;

    template <typename T>
    constexpr const T& get_constant(uint16_t index) const
    {
        return std::get<T>(constant_pool[index - 1]);
    }

    constexpr const std::vector<uint8_t>& get_utf8(uint16_t index) const
    {
        return get_constant<utf8_info>(index).bytes;
    }
};
