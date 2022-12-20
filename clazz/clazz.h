// cSpell:ignore clazz
#pragma once
#include <cstdint>
#include <stdexcept>
#include <variant>
#include <vector>

namespace clazz
{
    struct class_file;

    class class_parse_error : public std::runtime_error
    {
    public:
        inline class_parse_error(const std::string& str) : std::runtime_error(str) {}
    };

    enum class_access_flags
    {
        CL_ACC_PUBLIC = 0x0001,
        CL_ACC_FINAL = 0x0010,
        CL_ACC_SUPER = 0x0020,
        CL_ACC_INTERFACE = 0x0200,
        CL_ACC_ABSTRACT = 0x0400,
        CL_ACC_SYNTHETIC = 0x1000,
        CL_ACC_ANNOTATION = 0x2000,
        CL_ACC_ENUM = 0x4000,
    };

    enum field_access_flags
    {
        FIELD_ACC_PUBLIC = 0x0001,
        FIELD_ACC_PRIVATE = 0x0002,
        FIELD_ACC_PROTECTED = 0x0004,
        FIELD_ACC_STATIC = 0x0008,
        FIELD_ACC_FINAL = 0x0010,
        FIELD_ACC_VOLATILE = 0x0040,
        FIELD_ACC_TRANSIENT = 0x0080,
        FIELD_ACC_SYNTHETIC = 0x1000,
        FIELD_ACC_ENUM = 0x4000,
    };

    enum method_access_flags
    {
        METHOD_ACC_PUBLIC = 0x0001,
        METHOD_ACC_PRIVATE = 0x0002,
        METHOD_ACC_PROTECTED = 0x0004,
        METHOD_ACC_STATIC = 0x0008,
        METHOD_ACC_FINAL = 0x0010,
        METHOD_ACC_SYNCHRONIZED = 0x0020,
        METHOD_ACC_BRIDGE = 0x0040,
        METHOD_ACC_VARARGS = 0x0080,
        METHOD_ACC_NATIVE = 0x0100,
        METHOD_ACC_ABSTRACT = 0x0400,
        METHOD_ACC_STRICT = 0x0800,
        METHOD_ACC_SYNTHETIC = 0x1000,
    };

    enum inner_class_access_flags
    {
        IC_ACC_PUBLIC = 0x0001,
        IC_ACC_PRIVATE = 0x0002,
        IC_ACC_PROTECTED = 0x0004,
        IC_ACC_STATIC = 0x0008,
        IC_ACC_FINAL = 0x0010,
        IC_ACC_INTERFACE = 0x0200,
        IC_ACC_ABSTRACT = 0x0400,
        IC_ACC_SYNTHETIC = 0x1000,
        IC_ACC_ANNOTATION = 0x2000,
        IC_ACC_ENUM = 0x4000,
    };

    struct nocheck_tag
    {
    };
    inline constexpr nocheck_tag nocheck = nocheck_tag{};
    template <typename... Types>
    class cp_ref
    {
        uint16_t index;

    public:
        constexpr cp_ref() : index(0){};
        constexpr cp_ref(const class_file& clazz, uint16_t index);
        constexpr cp_ref(nocheck_tag, uint16_t index) : index(index) {}
        constexpr cp_ref(const cp_ref&) = default;

        template <typename T>
        requires(std::disjunction_v<std::is_same<T, Types>...>) constexpr const T& get(const class_file& f) const;

        constexpr const auto& get(const class_file& f) const requires(sizeof...(Types) == 1);
        constexpr auto get(const class_file& f) const requires(sizeof...(Types) != 1);

        constexpr auto get_index() const { return index; }
    };

    template <typename... Types>
    class nullable_cp_ref
    {
        uint16_t index;

    public:
        constexpr nullable_cp_ref() : index(0){};
        constexpr nullable_cp_ref(const class_file& clazz, uint16_t index);
        constexpr nullable_cp_ref(nocheck_tag, uint16_t index) : index(index) {}

        template <typename T>
        requires(std::disjunction_v<std::is_same<T, Types>...>) constexpr const T& get(const class_file& f) const;

        constexpr const auto& get(const class_file& f) const requires(sizeof...(Types) == 1);
        constexpr auto get(const class_file& f) const requires(sizeof...(Types) != 1);

        constexpr auto get_index() const { return index; }
        constexpr auto has_value() const { return index; }
    };

    struct address_ref
    {
        uint16_t ip;
        constexpr address_ref(uint16_t v) : ip(v) {}
    };

    struct address_offset
    {
        int32_t off;
        constexpr address_offset(int32_t v) : off(v) {}
        constexpr address_ref resolve(size_t ip) const { return off + ip; }
    };

    inline static constexpr const char* PRIMITIVE_TYPE_NAMES[] = {"boolean", "char", "float", "double", "byte", "short", "int", "long"};

    struct primitive_type_ref
    {
        uint8_t ty;
        constexpr primitive_type_ref(uint8_t v) : ty(v)
        {
            if (ty < 4 || ty > 11)
                throw class_parse_error("invalid primitive");
        }
        constexpr const char* name() { return PRIMITIVE_TYPE_NAMES[ty - 4]; }
    };

    struct lvt_ref
    {
        uint16_t index;
        constexpr lvt_ref(uint16_t v) : index(v) {}
    };

    // ensure that ref has same semantics as regular values
    static_assert(sizeof(cp_ref<char>) == sizeof(uint16_t));
    static_assert(sizeof(nullable_cp_ref<char>) == sizeof(uint16_t));
    static_assert(sizeof(address_offset) == sizeof(int32_t));
    static_assert(sizeof(address_ref) == sizeof(uint16_t));
    static_assert(sizeof(lvt_ref) == sizeof(uint16_t));
    static_assert(alignof(cp_ref<char>) == alignof(uint16_t));
    static_assert(alignof(nullable_cp_ref<char>) == alignof(uint16_t));
    static_assert(alignof(address_offset) == alignof(int32_t));
    static_assert(alignof(address_ref) == alignof(uint16_t));
    static_assert(alignof(lvt_ref) == alignof(uint16_t));

    struct utf8_info
    {
        std::vector<uint8_t> bytes;
    };
    using utf8_ref = cp_ref<utf8_info>;

    struct class_info
    {
        utf8_ref name_index;
    };
    using class_ref = cp_ref<class_info>;

    struct name_and_type_info
    {
        utf8_ref name_index;
        utf8_ref descriptor_index;
    };
    using name_and_type_ref = cp_ref<name_and_type_info>;

    struct fieldref_info
    {
        class_ref class_index;
        name_and_type_ref name_and_type_index;
    };
    using fieldref_ref = cp_ref<fieldref_info>;

    struct methodref_info
    {
        class_ref class_index;
        name_and_type_ref name_and_type_index;
    };
    using methodref_ref = cp_ref<methodref_info>;

    struct interface_methodref_info
    {
        class_ref class_index;
        name_and_type_ref name_and_type_index;
    };
    using interface_methodref_ref = cp_ref<interface_methodref_info>;

    using member_ref = cp_ref<fieldref_info, methodref_info, interface_methodref_info>;

    struct string_info
    {
        utf8_ref string_index;
    };
    using string_ref = cp_ref<string_info>;

    struct integer_info
    {
        int value;
    };
    using integer_ref = cp_ref<integer_info>;

    struct float_info
    {
        float value;
    };
    using float_ref = cp_ref<float_info>;

    struct long_info
    {
        long value;
    };
    using long_ref = cp_ref<long_info>;

    struct double_info
    {
        double value;
    };
    using double_ref = cp_ref<double_info>;

    struct method_handle_info
    {
        uint8_t reference_kind;
        member_ref reference_index;
    };
    using method_handle_ref = cp_ref<method_handle_info>;

    struct method_type_info
    {
        utf8_ref descriptor_index;
    };
    using method_type_ref = cp_ref<method_type_info>;

    // TODO: dynamic_info

    struct invoke_dynamic_info
    {
        // TODO: give index custom type
        uint16_t bootstrap_method_attr_index;
        name_and_type_ref name_and_type_index;
    };
    using invoke_dynamic_ref = cp_ref<invoke_dynamic_info>;

    using any_cp_ref = cp_ref<class_info, fieldref_info, methodref_info, interface_methodref_info, string_info, integer_info, float_info, long_info,
                              double_info, name_and_type_info, utf8_info, method_handle_info, method_type_info, invoke_dynamic_info, std::monostate>;

    using primitive_ref = cp_ref<string_info, integer_info, float_info, long_info, double_info>;

    using cp_info =
        std::variant<class_info, fieldref_info, methodref_info, interface_methodref_info, string_info, integer_info, float_info, long_info,
                     double_info, name_and_type_info, utf8_info, method_handle_info, method_type_info, invoke_dynamic_info, std::monostate>;

    namespace detail
    {
        template <typename T, typename... Args>
        concept is_same_any = std::disjunction_v<std::is_same<T, Args>...>;
    }

    template <typename T>
    concept is_primitive = detail::is_same_any<T, string_info, integer_info, float_info, long_info, double_info, string_ref, integer_ref, float_ref,
                                               long_ref, double_ref>;

    template <typename T>
    concept is_member =
        detail::is_same_any<T, fieldref_info, methodref_info, interface_methodref_info, fieldref_ref, methodref_ref, interface_methodref_ref>;

    namespace stackmap
    {
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
                std::vector<verification_type_info> locals;
                std::vector<verification_type_info> stack;
            };

            std::variant<same_frame, same_locals_1_stack_item_frame, same_locals_1_stack_item_frame_extended, chop_frame, same_frame_extended,
                         append_frame, full_frame>
                data;
        };
    } // namespace stackmap

    namespace annotations
    {
        struct type_path
        {
            struct entry
            {
                uint8_t type_path_kind;
                uint8_t type_argument_index;
            };

            std::vector<entry> path;
        };

        struct annotation
        {
            utf8_ref type_index;
            struct entry;
            std::vector<entry> entries;
        };

        struct element_value
        {
            struct enum_const_value
            {
                utf8_ref type_name_index;
                utf8_ref const_name_index;
            };

            uint8_t tag;
            std::variant<integer_ref, double_ref, float_ref, long_ref, utf8_ref, enum_const_value, annotation, std::vector<element_value>> value;
        };

        struct annotation::entry
            {
                utf8_ref name;
                element_value value;
            };

        struct type_annotation
        {
            struct type_parameter_target
            {
                uint8_t type_parameter_index;
            };

            struct supertype_target
            {
                uint16_t supertype_index;
            };

            struct type_parameter_bound_target
            {
                uint8_t type_parameter_index;
                uint8_t bound_index;
            };

            struct empty_target
            {
            };

            struct formal_parameter_target
            {
                uint8_t formal_parameter_index;
            };

            struct throws_target
            {
                uint16_t throws_type_index;
            };

            struct localvar_target
            {
                struct target_entry
                {
                    uint16_t start_pc;
                    uint16_t length;
                    uint16_t index;
                };

                std::vector<target_entry> table;
            };

            struct catch_target
            {
                uint16_t exception_table_index;
            };

            struct offset_target
            {
                uint16_t offset;
            };

            struct type_argument_target
            {
                uint16_t offset;
                uint8_t type_argument_index;
            };

            using target_info_t =
                std::variant<type_parameter_target, supertype_target, type_parameter_bound_target, empty_target, formal_parameter_target,
                             throws_target, localvar_target, catch_target, offset_target, type_argument_target>;
            uint8_t target_type;
            target_info_t target_info;
            type_path target_path;
            utf8_ref type_index;
            std::vector<std::pair<utf8_ref, element_value>> entries;
        };
    } // namespace annotations
    struct attribute_info
    {
        utf8_ref attribute_name_index;
        std::vector<uint8_t> buffer;
    };

    struct signature_attribute
    {
        utf8_ref signature_index;
    };

    struct source_file_attribute
    {
        utf8_ref sourcefile_index;
    };

    struct lvt_attribute
    {
        struct lvt_entry
        {
            address_ref start_pc;
            uint16_t length;
            utf8_ref name_index;
            utf8_ref descriptor_index;
            lvt_ref index;
        };

        std::vector<lvt_entry> lvt;
    };

    struct lvt_type_attribute
    {
        struct lvt_type_entry
        {
            address_ref start_pc;
            uint16_t length;
            utf8_ref name_index;
            utf8_ref signature_index;
            lvt_ref index;
        };

        std::vector<lvt_type_entry> lvt;
    };

    struct inner_class_attribute
    {
        struct inner_class_entry
        {
            class_ref inner_class_info_index;
            nullable_cp_ref<class_info> outer_class_info_index;
            nullable_cp_ref<utf8_info> inner_name_index;
            uint16_t inner_class_access_flags;
        };
        std::vector<inner_class_entry> inner_classes;
    };

    struct lineno_attribute
    {
        struct lineno_entry
        {
            address_ref start_pc;
            uint16_t line_number;
        };
        std::vector<lineno_entry> line_number_table;
    };

    struct stack_map_table_attribute
    {
        std::vector<stackmap::stack_map_frame> entries;
    };

    struct bootstrap_methods_attribute
    {
        struct bootstrap_methods_entry
        {
            method_handle_ref bootstrap_method_ref;
            std::vector<any_cp_ref> bootstrap_arguments;
        };

        std::vector<bootstrap_methods_entry> bootstrap_methods;
    };

    struct nest_members_attribute
    {
        std::vector<class_ref> classes;
    };

    struct nest_host_attribute
    {
        class_ref host_class_index;
    };

    struct constant_value_attribute
    {
        primitive_ref constantvalue_index;
    };

    struct exceptions_attribute
    {
        std::vector<class_ref> exception_index_table;
    };

    struct enclosing_method_attribute
    {
        class_ref class_index;
        nullable_cp_ref<name_and_type_info> method_index;
    };

    struct runtime_invisible_type_annotations_attribute
    {
        std::vector<annotations::type_annotation> annotations;
    };

    struct runtime_invisible_parameter_annotations_attribute
    {
        std::vector<std::vector<annotations::annotation>> annotations;
    };

    struct runtime_invisible_annotations_attribute
    {
        std::vector<annotations::annotation> annotations;
    };

    struct runtime_visible_type_annotations_attribute
    {
        std::vector<annotations::type_annotation> annotations;
    };

    struct runtime_visible_parameter_annotations_attribute
    {
        std::vector<std::vector<annotations::annotation>> annotations;
    };

    struct runtime_visible_annotations_attribute
    {
        std::vector<annotations::annotation> annotations;
    };


    struct code_attribute;
    using attribute =
        std::variant<attribute_info, code_attribute, signature_attribute, source_file_attribute, lvt_attribute, inner_class_attribute,
                     lineno_attribute, stack_map_table_attribute, bootstrap_methods_attribute, lvt_type_attribute, nest_members_attribute,
                     nest_host_attribute, constant_value_attribute, exceptions_attribute, enclosing_method_attribute,
                         runtime_invisible_type_annotations_attribute, runtime_invisible_parameter_annotations_attribute,
                         runtime_invisible_annotations_attribute, runtime_visible_type_annotations_attribute, 
runtime_visible_parameter_annotations_attribute, runtime_visible_annotations_attribute
                         >;

    inline constexpr const char* opcodes[] = {
        "nop",           "aconst_null", "iconst_m1",     "iconst_0",      "iconst_1",     "iconst_2",
        "iconst_3",      "iconst_4",    "iconst_5",      "lconst_0",      "lconst_1",     "fconst_0",
        "fconst_1",      "fconst_2",    "dconst_0",      "dconst_1",      "bipush",       "sipush",
        "ldc",           "ldc_w",       "ldc2_w",        "iload",         "lload",        "fload",
        "dload",         "aload",       "iload_0",       "iload_1",       "iload_2",      "iload_3",
        "lload_0",       "lload_1",     "lload_2",       "lload_3",       "fload_0",      "fload_1",
        "fload_2",       "fload_3",     "dload_0",       "dload_1",       "dload_2",      "dload_3",
        "aload_0",       "aload_1",     "aload_2",       "aload_3",       "iaload",       "laload",
        "faload",        "daload",      "aaload",        "baload",        "caload",       "saload",
        "istore",        "lstore",      "fstore",        "dstore",        "astore",       "istore_0",
        "istore_1",      "istore_2",    "istore_3",      "lstore_0",      "lstore_1",     "lstore_2",
        "lstore_3",      "fstore_0",    "fstore_1",      "fstore_2",      "fstore_3",     "dstore_0",
        "dstore_1",      "dstore_2",    "dstore_3",      "astore_0",      "astore_1",     "astore_2",
        "astore_3",      "iastore",     "lastore",       "fastore",       "dastore",      "aastore",
        "bastore",       "castore",     "sastore",       "pop",           "pop2",         "dup",
        "dup_x1",        "dup_x2",      "dup2",          "dup2_x1",       "dup2_x2",      "swap",
        "iadd",          "ladd",        "fadd",          "dadd",          "isub",         "lsub",
        "fsub",          "dsub",        "imul",          "lmul",          "fmul",         "dmul",
        "idiv",          "ldiv",        "fdiv",          "ddiv",          "irem",         "lrem",
        "frem",          "drem",        "ineg",          "lneg",          "fneg",         "dneg",
        "ishl",          "lshl",        "ishr",          "lshr",          "iushr",        "lushr",
        "iand",          "land",        "ior",           "lor",           "ixor",         "lxor",
        "iinc",          "i2l",         "i2f",           "i2d",           "l2i",          "l2f",
        "l2d",           "f2i",         "f2l",           "f2d",           "d2i",          "d2l",
        "d2f",           "i2b",         "i2c",           "i2s",           "lcmp",         "fcmpl",
        "fcmpg",         "dcmpl",       "dcmpg",         "ifeq",          "ifne",         "iflt",
        "ifge",          "ifgt",        "ifle",          "if_icmpeq",     "if_icmpne",    "if_icmplt",
        "if_icmpge",     "if_icmpgt",   "if_icmple",     "if_acmpeq",     "if_acmpne",    "goto",
        "jsr",           "ret",         "tableswitch",   "lookupswitch",  "ireturn",      "lreturn",
        "freturn",       "dreturn",     "areturn",       "return",        "getstatic",    "putstatic",
        "getfield",      "putfield",    "invokevirtual", "invokespecial", "invokestatic", "invokeinterface",
        "invokedynamic", "new",         "newarray",      "anewarray",     "arraylength",  "athrow",
        "checkcast",     "instanceof",  "monitorenter",  "monitorexit",   "wide",         "multianewarray",
        "ifnull",        "ifnonnull",   "goto_w",        "jsr_w",         "breakpoint",
    };

    struct inst;

    struct code_attribute
    {
        struct exception_table_entry
        {
            address_ref start_pc;
            address_ref end_pc;
            address_ref handler_pc;
            nullable_cp_ref<class_info> catch_type;
        };

        uint16_t max_stack;
        uint16_t max_locals;
        std::size_t max_ip;
        std::vector<inst> code;
        std::vector<exception_table_entry> exception_table;
        std::vector<attribute> attributes;
    };

    struct field_info
    {
        uint16_t access_flags;
        utf8_ref name_index;
        utf8_ref descriptor_index;
        std::vector<attribute> attributes;
    };

    struct method_info
    {
        uint16_t access_flags;
        utf8_ref name_index;
        utf8_ref descriptor_index;

        std::vector<attribute> attributes;
    };

    struct class_file
    {
        uint32_t magic;
        uint16_t minor_version;
        uint16_t major_version;
        std::vector<cp_info> constant_pool;
        uint16_t access_flags;
        class_ref this_class;
        class_ref super_class;
        std::vector<class_ref> interfaces;
        std::vector<field_info> fields;
        std::vector<method_info> methods;
        std::vector<attribute> attributes;
        size_t bootstrap_index;
    };

    namespace detail
    {
        template <typename... Args>
        constexpr inline uint16_t validate_constant_index(const class_file& clazz, uint16_t index)
        {
            if (index == 0 || index > clazz.constant_pool.size())
                throw class_parse_error("bad index into constant pool");
            if (!(std::holds_alternative<Args>(clazz.constant_pool[index - 1]) || ...))
                throw class_parse_error("invalid type of constant");
            return index;
        }

        template <typename... Args>
        constexpr inline uint16_t validate_constant_index_z(const class_file& clazz, uint16_t index)
        {
            if (index == 0)
                return index;
            if (index > clazz.constant_pool.size())
                throw class_parse_error("bad index into constant pool");
            if (!(std::holds_alternative<Args>(clazz.constant_pool[index - 1]) || ...))
                throw class_parse_error("invalid type of constant");
            return index;
        }
    } // namespace detail

    template <typename... Types>
    constexpr cp_ref<Types...>::cp_ref(const class_file& clazz, uint16_t index) : index(detail::validate_constant_index<Types...>(clazz, index))
    {
    }

    template <typename... Types>
    constexpr nullable_cp_ref<Types...>::nullable_cp_ref(const class_file& clazz, uint16_t index)
        : index(detail::validate_constant_index_z<Types...>(clazz, index))
    {
    }

    template <typename... Types>
    template <typename T>
    requires(std::disjunction_v<std::is_same<T, Types>...>) constexpr const T& cp_ref<Types...>::get(const class_file& f) const
    {
        return std::get<T>(f.constant_pool[index - 1]);
    }

    template <typename... Types>
    constexpr const auto& cp_ref<Types...>::get(const class_file& f) const requires(sizeof...(Types) == 1)
    {
        return std::get<Types...>(f.constant_pool[index - 1]);
    }

    template <typename... Types>
    constexpr auto cp_ref<Types...>::get(const class_file& f) const requires(sizeof...(Types) != 1)
    {
        return std::visit(
            [](auto i) -> std::variant<Types...> {
                if constexpr ((std::is_same_v<decltype(i), Types> || ...))
                    return i;
                __builtin_unreachable();
            },
            f.constant_pool[index - 1]);
    }

    template <typename... Types>
    template <typename T>
    requires(std::disjunction_v<std::is_same<T, Types>...>) constexpr const T& nullable_cp_ref<Types...>::get(const class_file& f) const
    {
        return std::get<T>(f.constant_pool[index - 1]);
    }

    template <typename... Types>
    constexpr const auto& nullable_cp_ref<Types...>::get(const class_file& f) const requires(sizeof...(Types) == 1)
    {
        return std::get<Types...>(f.constant_pool[index - 1]);
    }

    template <typename... Types>
    constexpr auto nullable_cp_ref<Types...>::get(const class_file& f) const requires(sizeof...(Types) != 1)
    {
        return std::visit(
            [](auto i) -> std::variant<Types...> {
                if constexpr ((std::is_same_v<decltype(i), Types> || ...))
                    return i;
                __builtin_unreachable();
            },
            f.constant_pool[index - 1]);
    }

    struct tableswitch_data
    {
        address_offset def;
        int32_t low;
        int32_t high;
        std::vector<address_offset> lut;
    };

    struct lookupswitch_data
    {
        address_offset def;
        std::vector<std::pair<int32_t, address_offset>> lut;
    };

    struct wide_data
    {
        uint8_t op;
    };

    struct inst
    {
        using operand_1_t =
            std::variant<std::monostate, int, lvt_ref, address_offset, fieldref_ref, methodref_ref, interface_methodref_ref, string_ref, integer_ref,
                         float_ref, long_ref, double_ref, method_type_ref, method_handle_ref, class_ref, invoke_dynamic_ref, primitive_type_ref>;

        using operand_2_t = std::variant<std::monostate, int>;

        using special_data = std::variant<std::monostate, tableswitch_data, wide_data, lookupswitch_data>;

        uint8_t inst_sz;
        uint8_t opcode;
        special_data special;
        operand_1_t operand1;
        operand_2_t operand2;
    };

    inline static constexpr const char* METHOD_HANDLE_REF_TYPES[] = {
        nullptr,         "getField",     "getStatic",     "putField",         "putStatic",
        "invokeVirtual", "invokeStatic", "invokeSpecial", "newInvokeSpecial", "invokeInterface"};

    class_file parse_class(const std::string& file);
} // namespace clazz
