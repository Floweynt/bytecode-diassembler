// cSpell:ignore clazz
#include "attribute.h"
#include "byte_file.cpp"
#include "clazz_parse.h"
#include "constant_pool.h"
#include "decomp.h"
#include "utils.cpp"
#include <bit>

static attribute parse_attribute(class_file& clazz, byte_file& bf, size_t index);

static code_attribute parse_code_attribute(class_file& clazz, byte_file& bf)
{
    code_attribute attr;
    attr.max_stack = bf.read_u16();
    attr.max_locals = bf.read_u16();
    uint32_t code_len = attr.max_ip = bf.read_u32();

    for (size_t i = 0; i < code_len; i++)
    {
        inst curr;
        curr.inst_sz = 1;
        uint8_t opcode = bf.read_u8();
        curr.opcode = opcode;
        if (opcode <= 0x0f || (opcode >= 0x1a && opcode <= 0x35) || (opcode >= 0x3b && opcode <= 0x98 && opcode != 0x84) ||
            (opcode >= 0xac && opcode <= 0xb1))
            ;
        else if (opcode == 0xbe || opcode == 0xbf || opcode == 0xc2 || opcode == 0xc3 || opcode == 0xca)
            ;
        else if (opcode == 0x10)
        {
            i++;
            curr.inst_sz++;
            curr.operand1 = (int)std::bit_cast<int8_t>(bf.read_u8());
        }
        else if (opcode == 0x11)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = (int)std::bit_cast<int16_t>(bf.read_u16());
        }
        else if (opcode == 0x12)
        {
            i++;
            curr.inst_sz++;
            curr.operand1 =
                (cp_generic_ref)validate_constant_index<string_info, integer_info, float_info, class_info, method_type_info,
                                                        method_handle_info>(clazz, bf.read_u8());
        }
        else if (opcode == 0x13 | opcode == 0x14)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 =
                (cp_generic_ref)validate_constant_index<string_info, integer_info, float_info, long_info, double_info,
                                                        class_info, method_type_info, method_handle_info>(clazz,
                                                                                                          bf.read_u16());
        }
        else if (opcode >= 0x15 && opcode <= 0x19)
        {
            i++;
            curr.inst_sz++;
            curr.operand1 = lvt_reference{bf.read_u8()};
        }
        else if (opcode >= 0x36 && opcode <= 0x3a)
        {
            i++;
            curr.inst_sz++;
            curr.operand1 = lvt_reference{bf.read_u8()};
        }
        else if (opcode == 0x84)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = lvt_reference{bf.read_u8()};
            curr.operand2 = (int)bf.read_u8();
        }
        else if ((opcode >= 0x99 && opcode <= 0xa8) || opcode == 0xc6 || opcode == 0xc7)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = address_reference{std::bit_cast<int16_t>(bf.read_u16())};
        }
        else if (opcode >= 0xb2 && opcode <= 0xb5)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = (cp_ref<fieldref_info>)validate_constant_index<fieldref_info>(clazz, bf.read_u16());
        }
        else if (opcode >= 0xb6 && opcode <= 0xb8)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = (cp_ref<methodref_info>)validate_constant_index<methodref_info>(clazz, bf.read_u16());
        }
        else if (opcode == 0xb9)
        {
            i += 4;
            curr.inst_sz += 4;
            curr.operand1 =
                (cp_ref<interface_methodref_info>)validate_constant_index<interface_methodref_info>(clazz, bf.read_u16());
            curr.operand2 = (int)bf.read_u8();
            bf.read_u8(); // move cursor forward to skip the zero
        }
        else if (opcode == 0xba)
        {
            i += 4;
            curr.inst_sz += 4;
            curr.operand1 = (cp_ref<invoke_dynamic_info>)validate_constant_index<invoke_dynamic_info>(clazz, bf.read_u16());
            bf.read_u16(); // move cursor forward to skip the zero
        }
        else if (opcode == 0xbc)
        {
            i++;
            curr.inst_sz++;
            curr.operand1 = lvt_reference{bf.read_u8()};
        }
        else if (opcode == 0xbd || opcode == 0xc0 || opcode == 0xc1 || opcode == 0xbb)
        {
            i += 2;
            curr.inst_sz += 2;
            curr.operand1 = (cp_ref<class_info>)validate_constant_index<class_info>(clazz, bf.read_u16());
        }
        else if (opcode == 0xc4)
        {
            throw std::runtime_error("not implemented");
        }
        else if (opcode == 0xc5)
        {
            i += 3;
            curr.inst_sz += 3;
            curr.operand1 = (cp_ref<class_info>)validate_constant_index<class_info>(clazz, bf.read_u16());
            curr.operand2 = (int)bf.read_u8();
        }
        else if (opcode == 0xc8 || opcode == 0xc9)
        {
            i += 4;
            curr.inst_sz += 4;
            curr.operand1 = address_reference{std::bit_cast<int32_t>(bf.read_u32())};
        }
        else
        {
            throw class_parse_error(std::string("not implemented: ") + std::to_string(opcode));
        }

        attr.code.push_back(curr);
    }

    uint16_t exception_table_length = bf.read_u16();
    attr.exception_table.reserve(exception_table_length);
    for (size_t i = 0; i < exception_table_length; i++)
    {
        uint16_t v;
        attr.exception_table.push_back({
            bf.read_u16(),
            bf.read_u16(),
            bf.read_u16(),
            v = bf.read_u16(),
        });
        if (v)
            validate_constant_index<class_info>(clazz, v);
    }

    uint16_t attribute_count = bf.read_u16();
    attr.attributes.reserve(attribute_count);
    for (size_t i = 0; i < attribute_count; i++)
        attr.attributes.push_back(parse_attribute(clazz, bf, 0));
    return attr;
}

static verification_type_info parse_verification_type_info(const class_file& clazz, byte_file& bf)
{
    verification_type_info res;
    res.tag = bf.read_u8();
    if (res.tag > 8)
        throw class_parse_error("invalid verification tag");

    if (res.tag >= 7)
        res.data = bf.read_u16();
    return res;
}

static bootstrap_methods_attribute parse_boostrap_method(const class_file& clazz, byte_file& bf)
{
    uint16_t len = bf.read_u16();
    bootstrap_methods_attribute attr;
    attr.bootstrap_methods.reserve(len);
    for (size_t i = 0; i < len; i++)
    {
        bootstrap_methods_attribute::bootstrap_methods_entry entry;
        entry.bootstrap_method_ref = validate_constant_index<method_handle_info>(clazz, bf.read_u16());
        auto kind = clazz.get_constant<method_handle_info>(entry.bootstrap_method_ref).reference_kind;
        if(kind != 6 && kind != 8)
            throw class_parse_error("expected kind to be 6 or 8");

        uint16_t num_bootstrap_args = bf.read_u16();
        entry.bootstrap_arguments.reserve(num_bootstrap_args);

        for (size_t j = 0l; j < num_bootstrap_args; j++)
            entry.bootstrap_arguments.push_back(
                validate_constant_index<string_info, class_info, integer_info, long_info, float_info, double_info,
                                        method_handle_info, method_type_info>(clazz, bf.read_u16()));
        attr.bootstrap_methods.push_back(entry);
    }

    return attr;
}

static stack_map_table_attribute parse_stack_map(const class_file& clazz, byte_file& bf)
{
    uint16_t len = bf.read_u16();
    stack_map_table_attribute attr;
    attr.entries.reserve(len);
    for (size_t i = 0; i < len; i++)
    {
        uint8_t frame_type = bf.read_u8();
        stack_map_frame frame;
        frame.frame_type = frame_type;

        if (frame_type >= 0 && frame_type <= 63)
        {
            frame.data = stack_map_frame::same_frame{};
        }
        else if (frame_type >= 64 && frame_type <= 127)
        {
            frame.data = stack_map_frame::same_locals_1_stack_item_frame{parse_verification_type_info(clazz, bf)};
        }
        else if (frame_type == 247)
        {
            frame.data = stack_map_frame::same_locals_1_stack_item_frame_extended{bf.read_u16(),
                                                                                  parse_verification_type_info(clazz, bf)};
        }
        else if (frame_type >= 248 && frame_type <= 250)
        {
            frame.data = stack_map_frame::chop_frame{bf.read_u16()};
        }
        else if (frame_type == 251)
        {
            frame.data = stack_map_frame::same_frame_extended{bf.read_u16()};
        }
        else if (frame_type >= 252 && frame_type <= 254)
        {
            size_t n = frame_type - 251;
            stack_map_frame::append_frame curr_frame{bf.read_u16()};

            curr_frame.locals.reserve(n);
            for (size_t i = 0; i < n; i++)
                curr_frame.locals.push_back(parse_verification_type_info(clazz, bf));
            frame.data = curr_frame;
        }
        else
        {
            stack_map_frame::full_frame curr_frame{bf.read_u16(), bf.read_u16()};

            uint16_t number_of_locals = bf.read_u16();
            curr_frame.locals.reserve(number_of_locals);
            for (size_t i = 0; i < number_of_locals; i++)
                curr_frame.locals.push_back(parse_verification_type_info(clazz, bf));

            uint16_t number_of_stack_items = bf.read_u16();

            curr_frame.stack.reserve(number_of_stack_items);
            for (size_t i = 0; i < number_of_stack_items; i++)
                curr_frame.stack.push_back(parse_verification_type_info(clazz, bf));

            frame.data = curr_frame;
        }

        attr.entries.push_back(frame);
    }

    return attr;
}

static attribute parse_attribute(class_file& clazz, byte_file& bf, size_t index)
{
    uint16_t name = bf.read_u16();
    const auto& tmp = clazz.get_utf8(name);
    std::string str_name(tmp.begin(), tmp.end());
    if (str_name == "Code")
    {
        bf.read_u32();
        return parse_code_attribute(clazz, bf);
    }
    else if (str_name == "Signature")
    {
        bf.read_u32();
        return signature_attribute{validate_constant_index<utf8_info>(clazz,bf.read_u16())};
    }
    else if (str_name == "SourceFile")
    {
        bf.read_u32();
        return source_file_attribute{validate_constant_index<utf8_info>(clazz,bf.read_u16())};
    }
    else if (str_name == "LocalVariableTable")
    {
        bf.read_u32();
        lvt_attribute attr;
        uint16_t len = bf.read_u16();
        attr.lvt.reserve(len);
        for (size_t i = 0; i < len; i++)
        {
            attr.lvt.push_back({
                bf.read_u16(),
                bf.read_u16(),
                validate_constant_index<utf8_info>(clazz, bf.read_u16()),
                validate_constant_index<utf8_info>(clazz, bf.read_u16()),
                bf.read_u16(),
            });
        }

        return attr;
    }
    else if (str_name == "InnerClasses")
    {
        bf.read_u32();
        inner_class_attribute attr;
        uint16_t len = bf.read_u16();
        attr.inner_classes.reserve(len);
        for (size_t i = 0; i < len; i++)
        {
            attr.inner_classes.push_back({
                validate_constant_index<class_info>(clazz, bf.read_u16()),
                validate_constant_index<class_info>(clazz, bf.read_u16()),
                validate_constant_index<utf8_info>(clazz, bf.read_u16()),
                bf.read_u16(),
            });
        }
        return attr;
    }
    else if (str_name == "LineNumberTable")
    {
        bf.read_u32();
        lineno_attribute attr;
        uint16_t len = bf.read_u16();
        attr.line_number_table.reserve(len);
        for (size_t i = 0; i < len; i++)
        {
            attr.line_number_table.push_back({
                bf.read_u16(),
                bf.read_u16(),
            });
        }
        return attr;
    }
    else if (str_name == "StackMapTable")
    {
        bf.read_u32();
        return parse_stack_map(clazz, bf);
    }
    else if (str_name == "BootstrapMethods")
    {
        bf.read_u32();
        clazz.bootstrap_index = index;
        return parse_boostrap_method(clazz, bf);
    }

    attribute_info info;
    info.attribute_name_index = name;
    uint32_t sz = bf.read_u32();
    for (size_t i = 0; i < sz; i++)
        info.buffer.push_back(bf.read_u8());
    return info;
}

