// cSpell:ignore clazz
#include "attribute.cpp"
#include "clazz_parse.h"
#include "constant_pool.h"
#include <bit>
#include <cstdint>
#include <fstream>
#include <stdexcept>

// TODO: validate class names

template <typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

static inline void validate_fmim_ref(const class_file& clazz, auto t)
{
    validate_constant_index<class_info>(clazz, t.class_index);
    validate_constant_index<name_and_type_info>(clazz, t.name_and_type_index);
}

static void validate_constant_pool(const class_file& clazz)
{
    for (const auto& i : clazz.constant_pool)
    {
        std::visit(overload{
            [&](class_info c) { validate_constant_index<utf8_info>(clazz, c.name_index); },
            [&](fieldref_info c) { validate_fmim_ref(clazz, c); },
            [&](methodref_info c) { validate_fmim_ref(clazz, c); },
            [&](interface_methodref_info c) { validate_fmim_ref(clazz, c); },
            [&](string_info c) { validate_constant_index<utf8_info>(clazz, c.string_index); },
            [&](method_type_info c) { validate_constant_index<utf8_info>(clazz, c.descriptor_index); },
            [&](name_and_type_info c) { 
                validate_constant_index<utf8_info>(clazz, c.name_index); 
                validate_constant_index<utf8_info>(clazz, c.descriptor_index);    
            },
            [&](invoke_dynamic_info c) { 
                if(clazz.bootstrap_index == -1ull) throw new class_parse_error("expected attribute BootstrapMethods");
                validate_constant_index<name_and_type_info>(clazz, c.name_and_type_index);    
            },
            [&](method_handle_info c) { 
                switch(c.reference_kind)
                {
                case 1:
                case 2:
                case 3:
                case 4:
                    validate_constant_index<fieldref_info>(clazz, c.reference_index); break;
                case 5:
                case 6:
                case 7:
                case 8:
                    validate_constant_index<methodref_info>(clazz, c.reference_index); break;
                case 9:
                    validate_constant_index<interface_methodref_info>(clazz, c.reference_index); break;
                default: throw class_parse_error("invalid reference kind for CONSTANT_MethodHandle_info");
                }        
            },

            [&](auto c) { },
        }, i);
    }
}

class_file parse_class(const std::string& file)
{
    byte_file bf(file);
    if (!bf.get_stream())
        throw std::runtime_error("unable to open file");

    class_file clazz;
    clazz.magic = bf.read_u32();

    if (clazz.magic != 0xcafebabe)
        throw class_parse_error("bad signature, expected 0xcafebabe");
    clazz.minor_version = bf.read_u16();
    clazz.major_version = bf.read_u16();
    clazz.bootstrap_index = -1ull;

    uint16_t constant_pool_count = bf.read_u16();
    clazz.constant_pool.reserve(constant_pool_count);

    for (size_t i = 1; i < constant_pool_count; i++)
    {
        uint8_t tag = bf.read_u8();
        switch (tag)
        {
        case 1: {
            utf8_info info;
            uint16_t len = bf.read_u16();
            info.bytes.reserve(len);
            for (int j = 0; j < len; j++)
                info.bytes.push_back(bf.read_u8());
            clazz.constant_pool.push_back(info);
            break;
        }
        case 3:
            clazz.constant_pool.push_back(integer_info{std::bit_cast<int>(bf.read_u32())});
            break;
        case 4:
            clazz.constant_pool.push_back(float_info{std::bit_cast<float>(bf.read_u32())});
            break;
        case 5:
            clazz.constant_pool.push_back(long_info{std::bit_cast<long>(bf.read_u64())});
            clazz.constant_pool.push_back(std::monostate{});
            i++;
            break;
        case 6:
            clazz.constant_pool.push_back(double_info{std::bit_cast<double>(bf.read_u64())});
            clazz.constant_pool.push_back(std::monostate{});
            i++;
            break;
        case 7:
            clazz.constant_pool.push_back(class_info{bf.read_u16()});
            break;
        case 8:
            clazz.constant_pool.push_back(string_info{bf.read_u16()});
            break;
        case 9:
            clazz.constant_pool.push_back(fieldref_info{bf.read_u16(), bf.read_u16()});
            break;
        case 10:
            clazz.constant_pool.push_back(methodref_info{bf.read_u16(), bf.read_u16()});
            break;
        case 11:
            clazz.constant_pool.push_back(interface_methodref_info{bf.read_u16(), bf.read_u16()});
            break;
        case 12:
            clazz.constant_pool.push_back(name_and_type_info{bf.read_u16(), bf.read_u16()});
            break;
        case 15:
            clazz.constant_pool.push_back(method_handle_info{bf.read_u8(), bf.read_u16()});
            break;
        case 16:
            clazz.constant_pool.push_back(method_type_info{bf.read_u16()});
            break;
        case 18:
            clazz.constant_pool.push_back(invoke_dynamic_info{bf.read_u16(), bf.read_u16()});
            break;
        default:
            throw class_parse_error("invalid constant type");
        }
    }

    clazz.access_flags = bf.read_u16();
    validate_constant_index<class_info>(clazz, clazz.this_class = bf.read_u16());
    validate_constant_index<class_info>(clazz, clazz.super_class = bf.read_u16());

    uint16_t interfaces_count = bf.read_u16();
    clazz.interfaces.reserve(interfaces_count);
    for (int i = 0; i < interfaces_count; i++)
    {
        uint16_t ref = bf.read_u16();
        validate_constant_index<class_info>(clazz, ref);
        clazz.interfaces.push_back(ref);
    }

    uint16_t fields_count = bf.read_u16();
    clazz.fields.reserve(fields_count);
    for (int i = 0; i < fields_count; i++)
    {
        field_info info;
        info.access_flags = bf.read_u16();
        validate_constant_index<utf8_info>(clazz, info.name_index = bf.read_u16());
        validate_constant_index<utf8_info>(clazz, info.descriptor_index = bf.read_u16());
        uint16_t attributes_count = bf.read_u16();
        info.attributes.reserve(attributes_count);
        for (size_t j = 0; j < attributes_count; j++)
            info.attributes.push_back(parse_attribute(clazz, bf, 0));
        clazz.fields.push_back(info);
    }

    uint16_t methods_count = bf.read_u16();
    clazz.methods.reserve(fields_count);
    for (int i = 0; i < methods_count; i++)
    {
        method_info info;
        info.access_flags = bf.read_u16();
        validate_constant_index<utf8_info>(clazz, info.name_index = bf.read_u16());
        validate_constant_index<utf8_info>(clazz, info.descriptor_index = bf.read_u16());
        uint16_t attributes_count = bf.read_u16();
        info.attributes.reserve(attributes_count);
        for (size_t j = 0; j < attributes_count; j++)
            info.attributes.push_back(parse_attribute(clazz, bf, 0));
        clazz.methods.push_back(info);
    }

    uint16_t attributes_count = bf.read_u16();
    clazz.attributes.reserve(attributes_count);
    for (size_t i = 0; i < attributes_count; i++)
        clazz.attributes.push_back(parse_attribute(clazz, bf, i));

    validate_constant_pool(clazz);
    return clazz;
}

