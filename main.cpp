// cSpell:ignore clazz bgreen
#include "clazz/attribute.h"
#include "clazz/clazz.h"
#include "clazz/clazz_parse.h"
#include "clazz/constant_pool.h"
#include "colors.h"
#include <bit>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>

constexpr std::string escape_str(const std::vector<uint8_t>& i)
{
    std::string out;
    for (const auto& e : i)
    {
        if (!isprint(e))
            out += "\\0" + std::to_string((e >> 6) & 7) + std::to_string((e >> 3) & 7) + std::to_string((e)&7);
        else
            out += e;
    }
    return out;
}

template <typename... Ts> // (7)
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

static constexpr std::pair<inner_class_access_flags, const char*> INNER_CLASS_FLAGS_NAMES[] = {
    {IC_ACC_PUBLIC, "public "},     {IC_ACC_PRIVATE, "private "},     {IC_ACC_PROTECTED, "protected "},
    {IC_ACC_STATIC, "static "},     {IC_ACC_FINAL, "final "},         {IC_ACC_INTERFACE, "interface "},
    {IC_ACC_ABSTRACT, "abstract "}, {IC_ACC_SYNTHETIC, "synthetic "}, {IC_ACC_ANNOTATION, "annotation "},
    {IC_ACC_ENUM, "enum "},
};

static constexpr std::pair<class_access_flags, const char*> CLASS_FLAGS_NAMES[] = {
    {CL_ACC_PUBLIC, "public "},         {CL_ACC_FINAL, "final "},       {CL_ACC_SUPER, "super "},
    {CL_ACC_INTERFACE, "interface "},   {CL_ACC_ABSTRACT, "abstract "}, {CL_ACC_SYNTHETIC, "synthetic "},
    {CL_ACC_ANNOTATION, "annotation "}, {CL_ACC_ENUM, "enum "},
};
static constexpr std::pair<field_access_flags, const char*> FIELD_FLAGS_NAMES[] = {
    {FIELD_ACC_PUBLIC, "public "},       {FIELD_ACC_PRIVATE, "private "},     {FIELD_ACC_PROTECTED, "protected "},
    {FIELD_ACC_STATIC, "static "},       {FIELD_ACC_FINAL, "final "},         {FIELD_ACC_VOLATILE, "volatile "},
    {FIELD_ACC_TRANSIENT, "transient "}, {FIELD_ACC_SYNTHETIC, "synthetic "}, {FIELD_ACC_ENUM, "enum "},

};
static constexpr std::pair<method_access_flags, const char*> METHOD_FLAGS_NAMES[] = {
    {METHOD_ACC_PUBLIC, "public "},     {METHOD_ACC_PRIVATE, "private "}, {METHOD_ACC_PROTECTED, "protected "},
    {METHOD_ACC_STATIC, "static "},     {METHOD_ACC_FINAL, "final "},     {METHOD_ACC_SYNCHRONIZED, "synchronized "},
    {METHOD_ACC_BRIDGE, "bridge "},     {METHOD_ACC_VARARGS, "varargs "}, {METHOD_ACC_NATIVE, "native "},
    {METHOD_ACC_ABSTRACT, "abstract "}, {METHOD_ACC_STRICT, "strict "},   {METHOD_ACC_SYNTHETIC, "synthetic "},
};

static std::string flags_to_string(const auto& map, auto flags)
{
    std::string str;
    for (const auto& i : map)
        if (flags & i.first)
            str += i.second;

    return ::flags(str);
}

constexpr std::string dup(const std::string& str, size_t n)
{
    std::string out;
    out.reserve(str.size() * n);
    for (size_t i = 0; i < n; i++)
        out += str;
    return str;
}

constexpr std::string demangle_type(const std::string& str, size_t index = 0, size_t* out = nullptr)
{
    if (out)
        *out = index + 1;
    switch (str[index])
    {
    case 'Z':
        return "boolean";
    case 'B':
        return "byte";
    case 'C':
        return "char";
    case 'S':
        return "short";
    case 'I':
        return "int";
    case 'J':
        return "long";
    case 'F':
        return "float";
    case 'V':
        return "void";
    case 'D':
        return "double";
    case 'L': {
        std::string res = "";
        size_t i;
        for (i = index + 1; i < str.size() && str[i] != ';'; i++)
            res += (str[i] == '/' || str[i] == '$') ? '.' : str[i];
        if (out)
            *out = i + 1;
        return res;
    }
    case '(': {
        std::string args;
        size_t idx = 1;

        while (str[idx] != ')')
        {
            size_t i;
            args += demangle_type(str, idx, &i) += ", ";
            idx = i;
        }

        if (!args.empty())
            args.erase(args.size() - 2);

        return demangle_type(str, idx + 1) + "(" + args + ")";
    }
    }

    size_t arr_count = 0;
    size_t i;
    for (i = index; i < str.size() && str[i] == '['; i++)
        arr_count++;
    return demangle_type(str, i, out) + dup("[]", i);
}

constexpr std::string get_type_string(const class_file& clazz, uint16_t index)
{
    return escape_str(clazz.get_utf8(clazz.get_constant<name_and_type_info>(index).descriptor_index));
}

constexpr std::string get_name_string(const class_file& clazz, uint16_t index)
{
    return escape_str(clazz.get_utf8(clazz.get_constant<name_and_type_info>(index).name_index));
}

constexpr std::string get_name_and_type_string(const class_file& clazz, uint16_t index)
{
    return get_name_string(clazz, index) + " " + get_type_string(clazz, index);
}

constexpr std::string get_class_name(const class_file& clazz, uint16_t index)
{
    return escape_str(clazz.get_utf8(clazz.get_constant<class_info>(index).name_index));
}

constexpr std::string ref(auto v) { return reference("#{}", v); }
constexpr std::string pretty(auto v) { return bright_green("{}", v); }
constexpr std::string ref_inst(auto v) { return code("@{}", v); }
constexpr std::string pretty_demangle(const std::string& v) { return bright_green("{}", demangle_type(v)); }
constexpr std::string obj(auto v) { return bright_yellow("{}", v); }
constexpr std::string member(auto v) { return bright_cyan("{}", v); }
constexpr std::string sig(auto v) { return bright_magenta("{}", v); }
constexpr std::string lit(auto v) { return constant("{}", v); }

constexpr std::string get_padding(auto i1, auto i2)
{
    return std::string(std::to_string(i1).size() - std::to_string(i2).size(), ' ');
}

static std::string dump_instruction(const class_file& clazz, const inst& i, size_t curr_op)
{
    std::string out = bright_yellow("{: <16}", i.opcode <= 0xca ? opcodes[i.opcode] : "");

    out +=
        std::visit(overload{[](std::monostate) -> std::string { return ""; },
                            [](lvt_reference t) { return fmt::format("{}({})", ref(t.val), italic("LVT")); },
                            [&](address_reference t) { return ref_inst(curr_op + t.val); }, [&](int t) { return lit(t); },
                            [&](cp_generic_ref t) {
                                std::string str = std::visit(
                                    overload{[clazz](string_info v) {
                                                 return constant("\"{}\"", escape_str(clazz.get_utf8(v.string_index)));
                                             },
                                             [](integer_info v) { return constant("{}i", v.value); },
                                             [](float_info v) { return constant("{}f", v.value); },
                                             [](long_info v) { return constant("{}l", v.value); },
                                             [](double_info v) { return constant("{}d", v.value); },
                                             [clazz](class_info v) { return escape_str(clazz.get_utf8(v.name_index)); },
                                             [](const auto&) -> std::string { throw std::runtime_error("unimplemented"); }},
                                    t.get_constant(clazz));
                                return fmt::format("{}({})", ref(t.get_index()), italic(str));
                            },
                            [&](cp_ref<fieldref_info> t) {
                                const auto& c = t.get_constant(clazz);
                                return fmt::format("{}({}/{} {} {})", ref(t.get_index()),
                                                   italic(obj(get_class_name(clazz, c.class_index))),
                                                   italic(member(get_name_string(clazz, c.name_and_type_index))),
                                                   italic(sig(get_type_string(clazz, c.name_and_type_index))),
                                                   italic(pretty_demangle(get_type_string(clazz, c.name_and_type_index))));
                            },
                            [&](cp_ref<methodref_info> t) {
                                auto name_type = t.get_constant(clazz).name_and_type_index;
                                return fmt::format("{}({}/{}{} {})", ref(t.get_index()),
                                                   italic(obj(get_class_name(clazz, t.get_constant(clazz).class_index))),
                                                   italic(member(get_name_string(clazz, name_type))),
                                                   italic(sig(get_type_string(clazz, name_type))),
                                                   italic(pretty_demangle(get_type_string(clazz, name_type))));
                            },
                            [&](cp_ref<interface_methodref_info> t) {
                                auto name_type = t.get_constant(clazz).name_and_type_index;
                                return fmt::format("{}({}/{}{} {})", ref(t.get_index()),
                                                   italic(obj(get_class_name(clazz, t.get_constant(clazz).class_index))),
                                                   italic(member(get_name_string(clazz, name_type))),
                                                   italic(sig(get_type_string(clazz, name_type))),
                                                   italic(pretty_demangle(get_type_string(clazz, name_type))));
                            },
                            [&](cp_ref<class_info> t) {
                                return fmt::format("{}({})", ref(t.get_index()),
                                                   obj(escape_str(clazz.get_utf8(t.get_constant(clazz).name_index))));
                            },
                            [&](cp_ref<invoke_dynamic_info> t) {
                                return fmt::format("{}.{}", ref(t.get_constant(clazz).bootstrap_method_attr_index),
                                                   ref(t.get_constant(clazz).name_and_type_index));
                            }},
                   i.operand1);

    out += std::visit(
        overload{[](std::monostate) -> std::string { return ""; }, [](int t) -> std::string { return constant(" {}", t); }},
        i.operand2);

    return out;
}

inline static constexpr const char* METHOD_HANDLE_REF_TYPES[] = {nullptr,        "getField",      "getStatic",
                                                                 "putField",     "putStatic",     "invokeVirtual",
                                                                 "invokeStatic", "invokeSpecial", "newInvokeSpecial"};

static std::string dump_constants(const class_file& clazz)
{
    std::string str;
    size_t index = 1;
    for (const auto& i : clazz.constant_pool)
    {
        str +=
            fmt::format("    {}:{} ", ref(index), get_padding(clazz.constant_pool.size(), index)) +
            std::visit(
                overload{
                    [clazz](class_info t) {
                        return fmt::format("class_info:               {} {}\n", ref(t.name_index),
                                           obj(escape_str(clazz.get_utf8(t.name_index))));
                    },
                    [clazz](fieldref_info t) {
                        return fmt::format("fieldref_info:            {}.{} {}/{} {}\n", ref(t.class_index),
                                           ref(t.name_and_type_index), obj(get_class_name(clazz, t.class_index)),
                                           member(get_name_string(clazz, t.name_and_type_index)),
                                           sig(get_type_string(clazz, t.name_and_type_index)));
                    },
                    [clazz](methodref_info t) {
                        return fmt::format("methodref_info:           {}.{} {}/{}{}\n", ref(t.class_index),
                                           ref(t.name_and_type_index), obj(get_class_name(clazz, t.class_index)),
                                           member(get_name_string(clazz, t.name_and_type_index)),
                                           sig(get_type_string(clazz, t.name_and_type_index)));
                    },
                    [clazz](interface_methodref_info t) {
                        return fmt::format("methodref_info:           {}.{} {}/{}{}\n", ref(t.class_index),
                                           ref(t.name_and_type_index), obj(get_class_name(clazz, t.class_index)),
                                           member(get_name_string(clazz, t.name_and_type_index)),
                                           sig(get_type_string(clazz, t.name_and_type_index)));
                    },
                    [clazz](string_info t) {
                        return fmt::format("string_info:              {} {}\n", ref(t.string_index),
                                           constant('"' + escape_str(clazz.get_utf8(t.string_index)) + '"'));
                    },
                    [clazz](integer_info t) { return fmt::format("integer_info:             {}", lit(t.value)); },
                    [clazz](float_info t) { return fmt::format("float_info:               {}", lit(t.value)); },
                    [clazz](long_info t) { return fmt::format("long_info:                {}", lit(t.value)); },
                    [clazz](double_info t) { return fmt::format("double_info:              {}", lit(t.value)); },
                    [clazz](name_and_type_info t) {
                        return fmt::format("name_and_type_info:       {}.{} {} {}\n", ref(t.name_index),
                                           ref(t.descriptor_index), member(escape_str(clazz.get_utf8(t.name_index))),
                                           sig(escape_str(clazz.get_utf8(t.descriptor_index))));
                    },
                    [clazz](utf8_info t) {
                        return fmt::format("utf8_info:                {}\n", bright_black(escape_str(t.bytes)));
                    },
                    [clazz](method_handle_info t) {
                        return fmt::format(
                            "method_handle_info:       {} {} {}\n", ref(t.reference_index),
                            key(METHOD_HANDLE_REF_TYPES[t.reference_kind]),
                            std::visit(overload{[&](fieldref_info t) {
                                                    return fmt::format("{}/{} {}",

                                                                       obj(get_class_name(clazz, t.class_index)),
                                                                       member(get_name_string(clazz, t.name_and_type_index)),
                                                                       sig(get_type_string(clazz, t.name_and_type_index)));
                                                },
                                                [&](methodref_info t) {
                                                    return fmt::format("{}/{}{}",

                                                                       obj(get_class_name(clazz, t.class_index)),
                                                                       member(get_name_string(clazz, t.name_and_type_index)),
                                                                       sig(get_type_string(clazz, t.name_and_type_index)));
                                                },
                                                [&](interface_methodref_info t) {
                                                    return fmt::format("{}/{}{}", obj(get_class_name(clazz, t.class_index)),
                                                                       member(get_name_string(clazz, t.name_and_type_index)),
                                                                       sig(get_type_string(clazz, t.name_and_type_index)));
                                                },

                                                [](auto) -> std::string { __builtin_unreachable(); }},
                                       clazz.constant_pool[t.reference_index - 1]));
                    },
                    [clazz](method_type_info t) {
                        return fmt::format("method_type_info:         {} {}\n", ref(t.descriptor_index),
                                           sig(escape_str(clazz.get_utf8(t.descriptor_index))));
                    },
                    [clazz](invoke_dynamic_info t) {
                        return fmt::format("invoke_dynamic_info:      {}.{} {}{}\n", ref(t.bootstrap_method_attr_index),
                                           ref(t.name_and_type_index), member(get_name_string(clazz, t.name_and_type_index)),
                                           sig(get_type_string(clazz, t.name_and_type_index))

                        );
                    },
                    [clazz](std::monostate t) -> std::string { return ""; }},
                i);
        index++;
    }
    return str;
}

static std::string dump_attribute(const std::string& prefix, const class_file& clazz, const attribute& attr);

static std::string dump_code(const class_file& clazz, const code_attribute& attr)
{

    std::string ret = fmt::format("        {}: {}\n        {}: {}\n", key("max_stack"), lit(attr.max_stack),
                                  key("max_locals"), lit(attr.max_locals));

    size_t off = 0;
    for (const auto& i : attr.code)
    {
        ret += fmt::format("            {}{}: {}\n", off, get_padding(attr.max_ip, off), dump_instruction(clazz, i, off));
        off += i.inst_sz;
    }

    for (const auto& i : attr.exception_table)
    {
        ret += fmt::format("           {} {} {} {} [{}, {})\n", key("catch"),
                           obj(i.catch_type ? get_class_name(clazz, i.catch_type) : "all"), key("in"),
                           ref_inst(i.handler_pc), ref_inst(i.start_pc), ref_inst(i.end_pc));
    }

    ret += fmt::format("        {} ({}):\n", key("attributes"), lit(attr.attributes.size()));
    for (const auto& i : attr.attributes)
        ret += dump_attribute("          ", clazz, i);

    return ret;
}

static std::string dump_lvt(const class_file& clazz, const lvt_attribute& attr)
{
    std::string ret;

    for (const auto& i : attr.lvt)
    {
        ret += fmt::format("            {} {} {} ({})"
                           " [{}, {}) ({} bytes)\n",
                           ref(i.index), member(escape_str(clazz.get_utf8(i.name_index))),
                           sig(escape_str(clazz.get_utf8(i.descriptor_index))),
                           pretty_demangle(escape_str(clazz.get_utf8(i.descriptor_index))), ref_inst(i.start_pc),
                           ref_inst(i.start_pc + i.length + 1), lit(i.length));
    }

    return ret;
}

static std::string dump_inner_class(const class_file& clazz, const inner_class_attribute& attr)
{
    std::string ret;
    for (const auto& i : attr.inner_classes)
    {
        ret += fmt::format("      {}{} {} {}\n", flags_to_string(INNER_CLASS_FLAGS_NAMES, i.inner_class_access_flags),
                           obj(get_class_name(clazz, i.inner_class_info_index)), key("in"),
                           obj(get_class_name(clazz, i.outer_class_info_index)));
    }
    return ret;
}

static std::string dump_lineno_table(const class_file& clazz, const lineno_attribute& attr)
{
    std::string ret;

    for (const auto& i : attr.line_number_table)
        ret += fmt::format("            {} {}\n", lit(i.line_number), ref_inst(i.start_pc));
    return ret;
}

static std::string dump_verification_type_info(const class_file& clazz, const verification_type_info& info)
{
    switch (info.tag)
    {
    case VERIFICATION_TOP:
        return blue("top");
    case VERIFICATION_INTEGER:
        return blue("int");
    case VERIFICATION_FLOAT:
        return blue("float");
    case VERIFICATION_DOUBLE:
        return blue("double");
    case VERIFICATION_LONG:
        return blue("long");
    case VERIFICATION_NULL:
        return blue("null");
    case VERIFICATION_UNINITIALIZED_THIS:
        return blue("uninitalized-this");
    case VERIFICATION_OBJECT:
        return obj(get_class_name(clazz, info.data));
    case VERIFICATION_UNINITIALIZED:
        return fmt::format("{} ({}{})", key("uninitalized"), key("new"), ref_inst(info.data));
    }

    __builtin_unreachable();
}

static std::string dump_stack_map_table(const class_file& clazz, const stack_map_table_attribute& attr)
{
    std::string ret;
    int64_t off = -1;
    for (const auto& i : attr.entries)
    {
        uint8_t frame_type = i.frame_type;
        std::string type;
        std::string extra;
        if (frame_type >= 0 && frame_type <= 63)
        {
            type = "same";
            off += frame_type + 1;
        }
        else if (frame_type >= 64 && frame_type <= 127)
        {
            type = "same_locals_1_stack_item";
            off += frame_type - 64 + 1;
            extra =
                dump_verification_type_info(clazz, std::get<stack_map_frame::same_locals_1_stack_item_frame>(i.data).stack);
        }
        else if (frame_type == 247)
        {
            type = "same_locals_1_stack_item_ext";
            off += std::get<stack_map_frame::same_locals_1_stack_item_frame_extended>(i.data).offset_delta + 1;
            extra = dump_verification_type_info(
                clazz, std::get<stack_map_frame::same_locals_1_stack_item_frame_extended>(i.data).stack);
        }
        else if (frame_type >= 248 && frame_type <= 250)
        {
            type = "chop";
            auto k = 251 - frame_type;
            off += std::get<stack_map_frame::chop_frame>(i.data).offset_delta + 1;
            extra = lit(k);
        }
        else if (frame_type == 251)
        {
            type = "same_ext";
            off += std::get<stack_map_frame::same_frame_extended>(i.data).offset_delta + 1;
        }
        else if (frame_type >= 252 && frame_type <= 254)
        {
            type = "append";
            off += std::get<stack_map_frame::append_frame>(i.data).offset_delta + 1;
            for (const auto& i : std::get<stack_map_frame::append_frame>(i.data).locals)
                extra = "\n                   " + dump_verification_type_info(clazz, i) + '\n';
            extra.pop_back();
        }
        else
        {
            type = "full";
            off += std::get<stack_map_frame::full_frame>(i.data).offset_delta + 1;
        }

        ret +=
            fmt::format("            {}: {} ({}) {} {}\n", key("type"), lit(i.frame_type), key(type), ref_inst(off), extra);
    }

    return ret;
}

static std::string dump_bootstrap_methods(const class_file& clazz, const bootstrap_methods_attribute& attr)
{
    std::string res;
    size_t index = 0;
    for (const auto& i : attr.bootstrap_methods)
    {
        auto t = clazz.get_constant<method_handle_info>(i.bootstrap_method_ref);
        auto r = clazz.get_constant<methodref_info>(t.reference_index);
        std::string p1 = fmt::format("{} {} {}\n", ref(t.reference_index), key(METHOD_HANDLE_REF_TYPES[t.reference_kind]),
                                     fmt::format("{}/{}{}", obj(get_class_name(clazz, r.class_index)),
                                                 member(get_name_string(clazz, r.name_and_type_index)),
                                                 sig(get_type_string(clazz, r.name_and_type_index))));

        // we must then dump the arguments
        std::string p2;

        for (const auto& j : i.bootstrap_arguments)
        {
            p2 += "    " +
                  std::visit(
                      overload{
                          [clazz](string_info v) {
                              return constant("{} {}", ref(v.string_index), constant("\"{}\"", escape_str(clazz.get_utf8(v.string_index))));
                          },
                          [](integer_info v) { return constant("{}i", v.value); },
                          [](float_info v) { return constant("{}f", v.value); },
                          [](long_info v) { return constant("{}l", v.value); },
                          [](double_info v) { return constant("{}d", v.value); },
                          [clazz](class_info v) { return escape_str(clazz.get_utf8(v.name_index)); },
                          [clazz](method_handle_info t) {
                              return fmt::format(
                                  "{} {} {}", ref(t.reference_index), key(METHOD_HANDLE_REF_TYPES[t.reference_kind]),
                                  std::visit(
                                      overload{[&](fieldref_info t) {
                                                   return fmt::format("{}/{} {}", obj(get_class_name(clazz, t.class_index)),
                                                                      member(get_name_string(clazz, t.name_and_type_index)),
                                                                      sig(get_type_string(clazz, t.name_and_type_index)));
                                               },
                                               [&](methodref_info t) {
                                                   return fmt::format("{}/{}{}", obj(get_class_name(clazz, t.class_index)),
                                                                      member(get_name_string(clazz, t.name_and_type_index)),
                                                                      sig(get_type_string(clazz, t.name_and_type_index)));
                                               },
                                               [&](interface_methodref_info t) {
                                                   return fmt::format("{}/{}{}", obj(get_class_name(clazz, t.class_index)),
                                                                      member(get_name_string(clazz, t.name_and_type_index)),
                                                                      sig(get_type_string(clazz, t.name_and_type_index)));
                                               },

                                               [](auto) -> std::string { __builtin_unreachable(); }},
                                      clazz.constant_pool[t.reference_index - 1]));
                          },
                          [clazz](method_type_info t) {
                              return fmt::format("{} {}", ref(t.descriptor_index),
                                                 sig(escape_str(clazz.get_utf8(t.descriptor_index))));
                          },
                          [](const auto&) -> std::string { __builtin_unreachable(); },
                      },
                      clazz.constant_pool[j - 1]) +
                  '\n';
        }

        res += fmt::format("  {}:{} {}{}\n", ref(index), get_padding(i.bootstrap_arguments.size(), index), p1, p2);
        index++;
    }
    return res;
}

static std::string dump_attribute(const std::string& prefix, const class_file& clazz, const attribute& attr)
{
    return std::visit(
        overload{[&](const attribute_info& attr) {
                     return fmt::format("{}- {}\n", prefix, escape_str(clazz.get_utf8(attr.attribute_name_index)));
                 },
                 [&](const code_attribute& attr) { return fmt::format("{}- Code\n", prefix) + dump_code(clazz, attr); },
                 [&](const signature_attribute& attr) {
                     return fmt::format("{}- Signature: {}", prefix, sig(escape_str(clazz.get_utf8(attr.signature_index))));
                 },
                 [&](const source_file_attribute& attr) {
                     return fmt::format("{}- SourceFile: {}\n", prefix, escape_str(clazz.get_utf8(attr.sourcefile_index)));
                 },
                 [&](const lvt_attribute& attr) {
                     return fmt::format("{}- LocalVariableTable\n", prefix) + dump_lvt(clazz, attr);
                 },
                 [&](const inner_class_attribute& attr) {
                     return fmt::format("{}- InnerClasses\n{} ({}):\n", prefix, key("    inner classes"),
                                        lit(attr.inner_classes.size())) +
                            dump_inner_class(clazz, attr);
                 },
                 [&](const lineno_attribute& attr) {
                     return fmt::format("{}- LineNumberTable\n", prefix) + dump_lineno_table(clazz, attr);
                 },
                 [&](const stack_map_table_attribute& attr) {
                     return fmt::format("{}- StackMapTable\n", prefix) + dump_stack_map_table(clazz, attr);
                 },
                 [&](const bootstrap_methods_attribute& attr) {
                     return fmt::format("{}- BootstrapMethods\n", prefix) + dump_bootstrap_methods(clazz, attr);
                     ;
                 }},
        attr);
}

/*
inline static constexpr auto METHOD_FMT = "  \x1b[34m{}\x1b[95m{}\x1b[0m (" bgreen("{}") "):\n    " key(
    "access") ": {}\n    " key("attributes") "(" green("{}") "):\n";

inline static constexpr auto FIELD_FMT = "  \x1b[34m{} \x1b[95m{}\x1b[0m (" bgreen("{}") "):\n    " key(
    "access") ": {}\n    " key("attributes") "(" green("{}") "):\n";
*/

static std::string dump_methods(const class_file& clazz)
{
    std::string str;

    for (const auto& i : clazz.methods)
    {
        str += fmt::format("  {}{} ({}):\n    {}: {}\n    {} ({}):\n", member(escape_str(clazz.get_utf8(i.name_index))),
                           sig(escape_str(clazz.get_utf8(i.descriptor_index))),
                           pretty(demangle_type(escape_str(clazz.get_utf8(i.descriptor_index)))), key("access"),
                           flags_to_string(METHOD_FLAGS_NAMES, i.access_flags), key("attributes"), lit(i.attributes.size()));

        for (const auto& j : i.attributes)
        {
            str += dump_attribute("      ", clazz, j);
        }
    }

    return str;
}

static std::string dump_fields(const class_file& clazz)
{
    std::string str;

    for (const auto& i : clazz.fields)
    {
        str += fmt::format("  {} {} ({}):\n    {}: {}\n    {} ({}):\n", member(escape_str(clazz.get_utf8(i.name_index))),
                           sig(escape_str(clazz.get_utf8(i.descriptor_index))),
                           pretty(demangle_type(escape_str(clazz.get_utf8(i.descriptor_index)))), key("access"),
                           flags_to_string(FIELD_FLAGS_NAMES, i.access_flags), key("attributes"), lit(i.attributes.size()));

        for (const auto& j : i.attributes)
        {
            str += dump_attribute("      ", clazz, j);
        }
    }

    return str;
}

static std::string dump_attributes(const class_file& clazz)
{
    std::string str;
    for (const auto& j : clazz.attributes)
    {
        str += dump_attribute("  ", clazz, j);
    }

    return str;
}

static std::string dump_interfaces(const class_file& clazz)
{
    std::string out;
    for (auto i : clazz.interfaces)
        out += fmt::format("  {}\n", obj(get_class_name(clazz, i)));
    return out;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << fmt::format("usage: {} [classfile]", argv[0]);
        exit(-1);
    }

    auto clazz = parse_class(argv[1]);

    std::string clazz_headers = fmt::format(
        "{}: {}\n{}: {}\n{} {} {} {}\n{}: {}\n", key("magic"), bright_red("0x{:x}", clazz.magic), key("version"),
        constant("{}.{}", clazz.major_version, clazz.minor_version), key("class"),
        obj(get_class_name(clazz, clazz.this_class)), key("extends"), obj(get_class_name(clazz, clazz.super_class)),
        key("access"), flags_to_string(CLASS_FLAGS_NAMES, clazz.access_flags));

    std::string constants =
        fmt::format("{} ({}):\n{}\n", key("constants"), lit(clazz.constant_pool.size()), dump_constants(clazz));
    std::string interfaces =
        fmt::format("{} ({}):\n{}\n", key("interfaces"), lit(clazz.interfaces.size()), dump_interfaces(clazz));
    std::string fields = fmt::format("{} ({})\n{}\n", key("fields"), lit(clazz.fields.size()), dump_fields(clazz));
    std::string methods = fmt::format("{} ({}):\n{}\n", key("methods"), lit(clazz.methods.size()), dump_methods(clazz));
    std::string attributes =
        fmt::format("{} ({}):\n{}\n", key("attributes"), lit(clazz.attributes.size()), dump_attributes(clazz));
    std::cout << clazz_headers + constants + interfaces + fields + methods + attributes;
}
