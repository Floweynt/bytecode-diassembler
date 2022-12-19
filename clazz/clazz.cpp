// cSpell:ignore clazz
#include "clazz.h"
#include <bit>
#include <cstdint>
#include <fstream>
#include <stdexcept>
namespace clazz
{
    class byte_file
    {
        std::ifstream ifs;
        size_t cursor;

    public:
        inline byte_file(std::string path) : ifs(path, std::ios::binary), cursor(0) {}

        inline uint8_t read_u8()
        {
            uint8_t n;
            ifs.read((char*)&n, 1);
            cursor++;
            return n;
        }

        inline int8_t read_i8() { return std::bit_cast<int8_t>(read_u8()); }
        inline int16_t read_i16() { return std::bit_cast<int16_t>(read_u16()); }
        inline int32_t read_i32() { return std::bit_cast<int32_t>(read_u32()); }
        inline int64_t read_i64() { return std::bit_cast<int64_t>(read_u64()); }

        inline uint16_t read_u16()
        {
            uint16_t n;
            ifs.read((char*)&n, 2);
            cursor += 2;
            return __builtin_bswap16(n);
        }

        inline uint32_t read_u32()
        {
            uint32_t n;
            ifs.read((char*)&n, 4);
            cursor += 4;
            return __builtin_bswap32(n);
        }

        inline uint64_t read_u64()
        {
            uint64_t n;
            ifs.read((char*)&n, 8);
            cursor += 8;
            return __builtin_bswap64(n);
        }

        constexpr const auto& get_stream() const { return ifs; }
        constexpr auto& get_stream() { return ifs; }
        constexpr auto get_cursor() const { return cursor; }
    };

    template <class... Args>
    struct variant_cast_proxy
    {
        std::variant<Args...> v;

        template <class... ToArgs>
        operator std::variant<ToArgs...>() const
        {
            return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg; }, v);
        }
    };

    template <class... Args>
    auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
    {
        return {v};
    }

    template <typename... Ts>
    std::variant<cp_ref<Ts>...> expand_ref(const class_file& clazz, uint16_t index)
    {
        if (index == 0 || index > clazz.constant_pool.size())
            throw class_parse_error("invalid index into constant pool");

        return std::visit(
            [index, &clazz]<typename T>(const T& args) -> std::variant<cp_ref<Ts>...> {
                if constexpr (std::disjunction_v<std::is_same<T, Ts>...>)
                    return cp_ref<T>(clazz, index);
                throw class_parse_error("invalid constant pool type");
            },
            clazz.constant_pool[index - 1]);
    }

    static attribute parse_attribute(class_file& clazz, byte_file& bf, size_t index);

    static code_attribute parse_code_attribute(class_file& clazz, byte_file& bf)
    {
        code_attribute attr;
        attr.max_stack = bf.read_u16();
        attr.max_locals = bf.read_u16();
        const uint32_t code_len = attr.max_ip = bf.read_u32();
        const size_t expected = code_len + bf.get_cursor();
        size_t ip = 0;
        for (; ip < code_len; ip++)
        {
            // TODO: remove
            size_t ip0 = ip;
            size_t cur0 = bf.get_cursor();

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
                ip++;
                curr.inst_sz++;
                curr.operand1 = (int)bf.read_i8();
            }
            else if (opcode == 0x11)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = (int)bf.read_i16();
            }
            else if (opcode == 0x12)
            {
                ip++;
                curr.inst_sz++;
                curr.operand1 = variant_cast(
                    expand_ref<string_info, integer_info, float_info, class_info, method_type_info, method_handle_info>(clazz, bf.read_u8()));
            }
            else if (opcode == 0x13 || opcode == 0x14)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = variant_cast(
                    expand_ref<string_info, integer_info, float_info, long_info, double_info, class_info, method_type_info, method_handle_info>(
                        clazz, bf.read_u16()));
            }
            else if (opcode >= 0x15 && opcode <= 0x19)
            {
                ip++;
                curr.inst_sz++;
                curr.operand1 = (lvt_ref)bf.read_u8();
            }
            else if (opcode >= 0x36 && opcode <= 0x3a)
            {
                ip++;
                curr.inst_sz++;
                curr.operand1 = (lvt_ref)bf.read_u8();
            }
            else if (opcode == 0x84)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = (lvt_ref)bf.read_u8();
                curr.operand2 = (int)bf.read_u8();
            }
            else if ((opcode >= 0x99 && opcode <= 0xa8) || opcode == 0xc6 || opcode == 0xc7)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = (address_offset)bf.read_i16();
            }
            else if (opcode >= 0xb2 && opcode <= 0xb5)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = fieldref_ref(clazz, bf.read_u16());
            }
            else if (opcode >= 0xb6 && opcode <= 0xb8)
            {
                ip += 2;
                curr.inst_sz += 2;
                uint16_t ref = bf.read_u16();
                if (opcode == 0xb8 && ref != 0 && ref <= clazz.constant_pool.size() &&
                    std::holds_alternative<interface_methodref_info>(clazz.constant_pool[ref - 1]))
                    curr.operand1 = interface_methodref_ref(clazz, ref);
                else
                    curr.operand1 = methodref_ref(clazz, ref);
            }
            else if (opcode == 0xb9)
            {
                ip += 4;
                curr.inst_sz += 4;
                curr.operand1 = interface_methodref_ref(clazz, bf.read_u16());
                curr.operand2 = (int)bf.read_u8();
                bf.read_u8(); // move cursor forward to skip the zero
            }
            else if (opcode == 0xba)
            {
                ip += 4;
                curr.inst_sz += 4;
                curr.operand1 = invoke_dynamic_ref(clazz, bf.read_u16());
                bf.read_u16(); // move cursor forward to skip the zero
            }
            else if (opcode == 0xbc)
            {
                ip++;
                curr.inst_sz++;
                curr.operand1 = (primitive_type_ref)bf.read_u8();
            }
            else if (opcode == 0xbd || opcode == 0xc0 || opcode == 0xc1 || opcode == 0xbb)
            {
                ip += 2;
                curr.inst_sz += 2;
                curr.operand1 = class_ref(clazz, bf.read_u16());
            }
            else if (opcode == 0xc4)
            {
                uint8_t real_op = bf.read_u8();
                ip++;
                curr.inst_sz++;

                wide_data data{real_op};

                if (real_op == 0x84)
                {
                    ip += 4;
                    curr.inst_sz += 4;
                    curr.operand1 = (lvt_ref)bf.read_u16();
                    curr.operand2 = (int)bf.read_u16();
                }
                else if ((real_op >= 0x15 && real_op <= 0x19) || (real_op >= 0x36 && real_op <= 0x3a))
                {
                    ip += 2;
                    curr.inst_sz += 2;
                    curr.operand1 = lvt_ref{bf.read_u16()};
                }
                else
                {
                    throw class_parse_error("invalid operand for wide");
                }

                curr.special = data;
            }
            else if (opcode == 0xc5)
            {
                ip += 3;
                curr.inst_sz += 3;
                curr.operand1 = class_ref(clazz, bf.read_u16());
                curr.operand2 = (int)bf.read_u8();
            }
            else if (opcode == 0xc8 || opcode == 0xc9)
            {
                ip += 4;
                curr.inst_sz += 4;
                curr.operand1 = (address_offset)bf.read_i32();
            }
            else if (opcode == 0xaa)
            {
                while (ip + 1 & 0b11)
                {
                    curr.inst_sz++;
                    ip++;
                    bf.read_u8(); // move on
                }

                curr.inst_sz += 12;
                ip += 12;
                tableswitch_data data{
                    bf.read_i32(),
                    bf.read_i32(),
                    bf.read_i32(),
                };

                if (data.low > data.high)
                    throw class_parse_error("tableswitch low must <= high");

                curr.inst_sz += 4 * (data.high - data.low + 1);
                ip += 4 * (data.high - data.low + 1);

                for (size_t j = 0; j < (data.high - data.low + 1); j++)
                    data.lut.push_back(bf.read_i32());

                curr.special = std::move(data);
            }
            else if (opcode == 0xab)
            {
                while (ip + 1 & 0b11)
                {
                    curr.inst_sz++;
                    ip++;
                    bf.read_u8(); // move on
                }

                curr.inst_sz += 8;
                ip += 8;

                lookupswitch_data data{
                    bf.read_i32(),
                };

                uint32_t len = bf.read_u32();

                curr.inst_sz += 8 * len;
                ip += 8 * len;
                for (size_t j = 0; j < len; j++)
                    data.lut.push_back({bf.read_i32(), bf.read_i32()});

                curr.special = std::move(data);
            }
            else
            {
                throw class_parse_error(std::string("not implemented: ") + std::to_string(opcode));
            }

            attr.code.push_back(curr);

            if (bf.get_cursor() - cur0 != (ip - ip0 + 1))
                throw class_parse_error("bad");
        }

        if (expected != bf.get_cursor())
            throw class_parse_error("internal state inconsistency");

        uint16_t exception_table_length = bf.read_u16();
        attr.exception_table.reserve(exception_table_length);

        for (size_t i = 0; i < exception_table_length; i++)
        {
            attr.exception_table.push_back({
                (address_ref)bf.read_u16(),
                (address_ref)bf.read_u16(),
                (address_ref)bf.read_u16(),
                nullable_cp_ref<class_info>(clazz, bf.read_u16()),
            });
        }

        uint16_t attribute_count = bf.read_u16();
        attr.attributes.reserve(attribute_count);
        for (size_t i = 0; i < attribute_count; i++)
            attr.attributes.push_back(parse_attribute(clazz, bf, 0));
        return attr;
    }

    using namespace clazz::stackmap;
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
            entry.bootstrap_method_ref = method_handle_ref(clazz, bf.read_u16());
            auto kind = entry.bootstrap_method_ref.get(clazz).reference_kind;
            if (kind != 6 && kind != 8)
                throw class_parse_error("expected kind to be 6 or 8");

            uint16_t num_bootstrap_args = bf.read_u16();
            entry.bootstrap_arguments.reserve(num_bootstrap_args);

            for (size_t j = 0l; j < num_bootstrap_args; j++)
                entry.bootstrap_arguments.push_back(any_cp_ref(clazz, bf.read_u16()));
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
                frame.data = stack_map_frame::same_frame{};
            else if (frame_type >= 64 && frame_type <= 127)
                frame.data = stack_map_frame::same_locals_1_stack_item_frame{parse_verification_type_info(clazz, bf)};
            else if (frame_type == 247)
                frame.data = stack_map_frame::same_locals_1_stack_item_frame_extended{bf.read_u16(), parse_verification_type_info(clazz, bf)};
            else if (frame_type >= 248 && frame_type <= 250)
                frame.data = stack_map_frame::chop_frame{bf.read_u16()};
            else if (frame_type == 251)
                frame.data = stack_map_frame::same_frame_extended{bf.read_u16()};
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
                stack_map_frame::full_frame curr_frame{bf.read_u16()};

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

    using namespace clazz::annotations;

    static type_path parse_type_path(const class_file& clazz, byte_file& bf)
    {
        type_path p;
        uint8_t len = bf.read_u8();
        p.path.reserve(len);
        for (size_t i = 0; i < len; i++)
            p.path.push_back({bf.read_u8(), bf.read_u8()});
        return p;
    }

    static element_value parse_element_value(const class_file& clazz, byte_file& bf)
    {
        element_value value;
        value.tag = bf.read_u8();
    }

    static type_annotation parse_type_annotation(const class_file& clazz, byte_file& bf)
    {
        type_annotation annotation{bf.read_u8()};

        type_annotation::target_info_t info;
        switch (annotation.target_type)
        {
        case 0x00:
        case 0x01:
            info = type_annotation::type_parameter_target{bf.read_u8()};
            break;
        case 0x10:
            info = type_annotation::supertype_target{bf.read_u16()};
            break;
        case 0x11:
        case 0x12:
            info = type_annotation::type_parameter_bound_target{bf.read_u8(), bf.read_u8()};
            break;
        case 0x13:
        case 0x14:
        case 0x15:
            info = type_annotation::empty_target{};
            break;
        case 0x16:
            info = type_annotation::formal_parameter_target{bf.read_u8()};
            break;
        case 0x17:
            info = type_annotation::throws_target{bf.read_u16()};
            break;
        case 0x40:
        case 0x41: {
            type_annotation::localvar_target target;
            uint16_t len = bf.read_u16();
            target.table.reserve(len);
            for (size_t i = 0; i < len; i++)
                target.table.push_back({bf.read_u16(), bf.read_u16(), bf.read_u16()});
            info = std::move(target);
            break;
        }
        case 0x42:
            info = type_annotation::catch_target{bf.read_u16()};
            break;
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
            info = type_annotation::offset_target{bf.read_u16()};
            break;
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
            info = type_annotation::type_argument_target{bf.read_u16(), bf.read_u8()};
        }

        annotation.target_path = parse_type_path(clazz, bf);
        annotation.type_index = bf.read_u16();

        uint16_t len = bf.read_u16();
        annotation.element_value_pairs.reserve(len);
        for (size_t i = 0; i < len; i++)
        {

            /*
            annotation.element_value_pairs.push_back({
            bf.read_u16(),

                    });*/
        }

        return annotation;
    }

    template <typename T>
    class raii_guard
    {
        T v;

    public:
        raii_guard(T&& v) : v(v) {}
        ~raii_guard()
        {
            if (!std::uncaught_exceptions())
                v();
        }
    };

    template <typename T>
    raii_guard(T&& v) -> raii_guard<T>;

    static attribute parse_attribute(class_file& clazz, byte_file& bf, size_t index)
    {
        auto name = utf8_ref(clazz, bf.read_u16());
        const auto& tmp = name.get(clazz).bytes;
        std::string str_name(tmp.begin(), tmp.end());

        uint32_t sz = bf.read_u32();
        size_t target = bf.get_cursor() + sz;

        raii_guard g([&]() {
            if (bf.get_cursor() != target)
                throw std::runtime_error("internal IO fail: " + str_name);
        });

        if (str_name == "Code")
            return parse_code_attribute(clazz, bf);
        else if (str_name == "Signature")
            return signature_attribute{utf8_ref(clazz, bf.read_u16())};
        else if (str_name == "SourceFile")
            return source_file_attribute{utf8_ref(clazz, bf.read_u16())};
        else if (str_name == "LocalVariableTable")
        {
            lvt_attribute attr;
            uint16_t len = bf.read_u16();
            attr.lvt.reserve(len);
            for (size_t i = 0; i < len; i++)
            {
                attr.lvt.push_back({
                    (address_ref)bf.read_u16(),
                    bf.read_u16(),
                    utf8_ref(clazz, bf.read_u16()),
                    utf8_ref(clazz, bf.read_u16()),
                    (lvt_ref)bf.read_u16(),
                });
            }

            return attr;
        }
        else if (str_name == "LocalVariableTypeTable")
        {
            lvt_type_attribute attr;
            uint16_t len = bf.read_u16();
            attr.lvt.reserve(len);
            for (size_t i = 0; i < len; i++)
            {
                attr.lvt.push_back({
                    (address_ref)bf.read_u16(),
                    bf.read_u16(),
                    utf8_ref(clazz, bf.read_u16()),
                    utf8_ref(clazz, bf.read_u16()),
                    (lvt_ref)bf.read_u16(),
                });
            }

            return attr;
        }
        else if (str_name == "InnerClasses")
        {
            inner_class_attribute attr;
            uint16_t len = bf.read_u16();
            attr.inner_classes.reserve(len);
            for (size_t i = 0; i < len; i++)
            {
                attr.inner_classes.push_back({
                    class_ref(clazz, bf.read_u16()),
                    nullable_cp_ref<class_info>(clazz, bf.read_u16()),
                    nullable_cp_ref<utf8_info>(clazz, bf.read_u16()),
                    bf.read_u16(),
                });
            }
            return attr;
        }
        else if (str_name == "LineNumberTable")
        {
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
            return parse_stack_map(clazz, bf);
        else if (str_name == "BootstrapMethods")
        {
            clazz.bootstrap_index = index;
            return parse_boostrap_method(clazz, bf);
        }
        else if (str_name == "NestMembers")
        {
            nest_members_attribute attr;
            uint16_t len = bf.read_u16();
            attr.classes.reserve(len);
            for (size_t i = 0; i < len; i++)
                attr.classes.push_back(class_ref(clazz, bf.read_u16()));
            return attr;
        }
        else if (str_name == "NestHost")
            return nest_host_attribute{class_ref(clazz, bf.read_u16())};
        else if (str_name == "ConstantValue")
            return constant_value_attribute{primitive_ref(clazz, bf.read_u16())};
        else if (str_name == "Exceptions")
        {
            exceptions_attribute attr;
            uint16_t len = bf.read_u16();
            attr.exception_index_table.reserve(len);
            for (size_t i = 0; i < len; i++)
                attr.exception_index_table.push_back(class_ref(clazz, bf.read_u16()));
            return attr;
        }
        else if (str_name == "EnclosingMethod")
        {
            return enclosing_method_attribute{class_ref(clazz, bf.read_u16()), nullable_cp_ref<name_and_type_info>(clazz, bf.read_u16())};
        }

        attribute_info info;
        info.attribute_name_index = name;
        for (size_t i = 0; i < sz; i++)
            info.buffer.push_back(bf.read_u8());
        return info;
    }

    template <typename... Ts>
    struct overload : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overload(Ts...) -> overload<Ts...>;

    static inline void validate_fmim_ref(const class_file& clazz, auto t)
    {
        clazz::detail::validate_constant_index<class_info>(clazz, t.class_index.get_index());
        clazz::detail::validate_constant_index<name_and_type_info>(clazz, t.name_and_type_index.get_index());
    }

    static void validate_constant_pool(const class_file& clazz)
    {
        for (const auto& i : clazz.constant_pool)
        {
            using namespace clazz::detail;
            std::visit(overload{
                           [&](class_info c) { validate_constant_index<utf8_info>(clazz, c.name_index.get_index()); },
                           [&](fieldref_info c) { validate_fmim_ref(clazz, c); },
                           [&](methodref_info c) { validate_fmim_ref(clazz, c); },
                           [&](interface_methodref_info c) { validate_fmim_ref(clazz, c); },
                           [&](string_info c) { validate_constant_index<utf8_info>(clazz, c.string_index.get_index()); },
                           [&](method_type_info c) { validate_constant_index<utf8_info>(clazz, c.descriptor_index.get_index()); },
                           [&](name_and_type_info c) {
                               validate_constant_index<utf8_info>(clazz, c.name_index.get_index());
                               validate_constant_index<utf8_info>(clazz, c.descriptor_index.get_index());
                           },
                           [&](invoke_dynamic_info c) {
                               if (clazz.bootstrap_index == -1ull)
                                   throw class_parse_error("expected attribute BootstrapMethods");
                               clazz::detail::validate_constant_index<name_and_type_info>(clazz, c.name_and_type_index.get_index());
                           },
                           [&](method_handle_info c) {
                               switch (c.reference_kind)
                               {
                               case 1:
                               case 2:
                               case 3:
                               case 4:
                                   validate_constant_index<fieldref_info>(clazz, c.reference_index.get_index());
                                   break;
                               case 5:
                               case 6:
                               case 7:
                               case 8:
                                   validate_constant_index<methodref_info>(clazz, c.reference_index.get_index());
                                   break;
                               case 9:
                                   validate_constant_index<interface_methodref_info>(clazz, c.reference_index.get_index());
                                   break;
                               default:
                                   throw class_parse_error("invalid reference kind for CONSTANT_MethodHandle_info");
                               }
                           },

                           [&](auto c) {},
                       },
                       i);
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
                clazz.constant_pool.push_back(class_info{{nocheck, bf.read_u16()}});
                break;
            case 8:
                clazz.constant_pool.push_back(string_info{{nocheck, bf.read_u16()}});
                break;
            case 9:
                clazz.constant_pool.push_back(fieldref_info{{nocheck, bf.read_u16()}, {nocheck, bf.read_u16()}});
                break;
            case 10:
                clazz.constant_pool.push_back(methodref_info{{nocheck, bf.read_u16()}, {nocheck, bf.read_u16()}});
                break;
            case 11:
                clazz.constant_pool.push_back(interface_methodref_info{{nocheck, bf.read_u16()}, {nocheck, bf.read_u16()}});
                break;
            case 12:
                clazz.constant_pool.push_back(name_and_type_info{{nocheck, bf.read_u16()}, {nocheck, bf.read_u16()}});
                break;
            case 15: {
                uint8_t kind = bf.read_u8();
                if (kind > 9)
                    throw class_parse_error("invalid kind for method handle info");
                clazz.constant_pool.push_back(method_handle_info{kind, {nocheck, bf.read_u16()}});
                break;
            }
            case 16:
                clazz.constant_pool.push_back(method_type_info{{nocheck, bf.read_u16()}});
                break;
            case 18:
                clazz.constant_pool.push_back(invoke_dynamic_info{bf.read_u16(), {nocheck, bf.read_u16()}});
                break;
            default:
                throw class_parse_error("invalid constant type");
            }
        }

        clazz.access_flags = bf.read_u16();
        clazz.this_class = {clazz, bf.read_u16()};
        clazz.super_class = {clazz, bf.read_u16()};

        uint16_t interfaces_count = bf.read_u16();
        clazz.interfaces.reserve(interfaces_count);
        for (int i = 0; i < interfaces_count; i++)
        {
            uint16_t ref = bf.read_u16();
            clazz.interfaces.push_back({clazz, ref});
        }

        uint16_t fields_count = bf.read_u16();
        clazz.fields.reserve(fields_count);
        for (int i = 0; i < fields_count; i++)
        {
            field_info info;
            info.access_flags = bf.read_u16();
            info.name_index = {clazz, bf.read_u16()};
            info.descriptor_index = {clazz, bf.read_u16()};
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
            info.name_index = {clazz, bf.read_u16()};
            info.descriptor_index = {clazz, bf.read_u16()};
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
} // namespace clazz
