// cSpell:ignore clazz
#include "clazz/clazz.h"
#include "colors.h"
#include "utils.h"
#include <iostream>
#include <stdexcept>

using namespace clazz;
namespace stackmap = stackmap;

template <typename T>
concept not_str = !std::is_convertible_v<std::decay_t<T>, std::string>;

constexpr std::string member(const std::string& str) { return cyan(str); }
constexpr std::string desc(const std::string& str) { return magenta(str); }
constexpr std::string type(const std::string& str) { return yellow(str); }
constexpr std::string utf8(const std::string& str) { return bright_black(str); }
constexpr std::string flags(const std::string& s) { return bright_red(s); }
constexpr std::string key(const std::string& str) { return bright_blue(str); }
constexpr std::string constant(const std::string& str) { return green(str); }
constexpr std::string instruction(const std::string& str) { return bright_yellow(str); }
constexpr std::string address(const std::string& str) { return bright_red(str); }
constexpr std::string reference(const std::string& str) { return bright_green(str); }
constexpr std::string pretty(const std::string& str) { return bright_green(str); }
constexpr std::string magic(const std::string& str) { return bright_red(str); }

constexpr std::string member(const not_str auto& str) { return cyan(std::to_string(str)); }
constexpr std::string desc(const not_str auto& str) { return magenta(std::to_string(str)); }
constexpr std::string type(const not_str auto& str) { return yellow(std::to_string(str)); }
constexpr std::string utf8(const not_str auto& str) { return bright_black(std::to_string(str)); }
constexpr std::string flags(const not_str auto& str) { return bright_red(std::to_string(str)); }
constexpr std::string key(const not_str auto& str) { return bright_blue(std::to_string(str)); }
constexpr std::string constant(const not_str auto& str) { return green(std::to_string(str)); }
constexpr std::string instruction(const not_str auto& str) { return bright_yellow(std::to_string(str)); }
constexpr std::string address(const not_str auto& str) { return bright_red(std::to_string(str)); }
constexpr std::string reference(const not_str auto& str) { return bright_green(std::to_string(str)); }
constexpr std::string pretty(const not_str auto& str) { return bright_green(std::to_string(str)); }
constexpr std::string magic(const not_str auto& str) { return magic(std::to_string(str)); }
inline static constexpr auto TAB_SIZE = 2;

constexpr std::string flags_to_string(const auto& map, auto flags)
{
    std::string str;
    for (const auto& i : map)
        if (flags & i.first)
            str += i.second;
    return ::flags(str);
}

class output_consumer
{
    const size_t tab_len;
    size_t indent;
    std::string buffer;
    std::string prefix;

public:
    constexpr output_consumer(size_t tab_len) : tab_len(tab_len), indent(0) {}

    constexpr output_consumer& w(const std::string& str)
    {
        buffer += std::string(tab_len * indent, ' ') + prefix + str + '\n';
        prefix.clear();
        return *this;
    }

    template <typename... Ts>
    constexpr output_consumer& w(const fmt::format_string<Ts...>& str, Ts&&... args)
    {
        buffer += std::string(tab_len * indent, ' ') + prefix + fmt::format(str, std::forward<Ts>(args)...) + '\n';
        prefix.clear();
        return *this;
    }

    constexpr output_consumer& wp(const std::string& str)
    {
        prefix = str;
        return *this;
    }

    template <typename... Ts>
    constexpr output_consumer& wp(const fmt::format_string<Ts...>& str, Ts&&... args)
    {
        prefix = fmt::format(str, std::forward<Ts>(args)...);
        return *this;
    }

    constexpr void push(size_t off = 1) { indent += off; }
    constexpr void pop(size_t off = 1) { indent -= off; }
    constexpr const std::string& data() const { return buffer; };
};

constexpr std::string dump_ref(const class_file& clazz, utf8_ref ref) { return escape_str(ref.get(clazz).bytes); }
constexpr std::string dump_info(const class_file&, const utf8_info& info) { return escape_str(info.bytes); }
constexpr std::string dump_info(const utf8_info& info) { return escape_str(info.bytes); }

constexpr std::string dump_ref(const class_file& clazz, integer_ref ref) { return constant(fmt::format("{}i", ref.get(clazz).value)); }
constexpr std::string dump_info(const class_file&, integer_info info) { return constant(fmt::format("{}i", info.value)); }
constexpr std::string dump_info(integer_info info) { return constant(fmt::format("{}i", info.value)); }

constexpr std::string dump_ref(const class_file& clazz, float_ref ref) { return constant(fmt::format("{}f", ref.get(clazz).value)); }
constexpr std::string dump_info(const class_file&, float_info info) { return constant(fmt::format("{}f", info.value)); }
constexpr std::string dump_info(float_info info) { return constant(fmt::format("{}f", info.value)); }

constexpr std::string dump_ref(const class_file& clazz, long_ref ref) { return constant(fmt::format("{}l", ref.get(clazz).value)); }
constexpr std::string dump_info(const class_file&, long_info info) { return constant(fmt::format("{}l", info.value)); }
constexpr std::string dump_info(long_info info) { return constant(fmt::format("{}l", info.value)); }

constexpr std::string dump_ref(const class_file& clazz, double_ref ref) { return constant(fmt::format("{}d", ref.get(clazz).value)); }
constexpr std::string dump_info(const class_file&, double_info info) { return constant(fmt::format("{}d", info.value)); }
constexpr std::string dump_info(double_info info) { return constant(fmt::format("{}d", info.value)); }

constexpr std::string dump_ref(const class_file& clazz, class_ref ref) { return type(dump_ref(clazz, ref.get(clazz).name_index)); }
constexpr std::string dump_info(const class_file& clazz, class_info ref) { return type(dump_ref(clazz, ref.name_index)); }

constexpr std::string dump_ref(const class_file& clazz, string_ref ref)
{
    return constant(fmt::format("\"{}\"", dump_ref(clazz, ref.get(clazz).string_index)));
}
constexpr std::string dump_info(const class_file& clazz, string_info info)
{
    return constant(fmt::format("\"{}\"", dump_ref(clazz, info.string_index)));
}

constexpr std::string dump_ref(const class_file& clazz, fieldref_ref ref)
{
    return fmt::format("{}/{} {}", dump_ref(clazz, ref.get(clazz).class_index),
                       member(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).descriptor_index)));
}
constexpr std::string dump_info(const class_file& clazz, fieldref_info info)
{
    return fmt::format("{}/{} {}", dump_ref(clazz, info.class_index), member(dump_ref(clazz, info.name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, info.name_and_type_index.get(clazz).descriptor_index)));
}

constexpr std::string dump_ref(const class_file& clazz, methodref_ref ref)
{
    return fmt::format("{}/{}{}", dump_ref(clazz, ref.get(clazz).class_index),
                       member(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).descriptor_index)));
}
constexpr std::string dump_info(const class_file& clazz, methodref_info ref)
{
    return fmt::format("{}/{}{}", dump_ref(clazz, ref.class_index), member(dump_ref(clazz, ref.name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.name_and_type_index.get(clazz).descriptor_index)));
}

constexpr std::string dump_ref(const class_file& clazz, interface_methodref_ref ref)
{
    return fmt::format("{}/{}{}", dump_ref(clazz, ref.get(clazz).class_index),
                       member(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).descriptor_index)));
}
constexpr std::string dump_info(const class_file& clazz, interface_methodref_info ref)
{
    return fmt::format("{}/{}{}", dump_ref(clazz, ref.class_index), member(dump_ref(clazz, ref.name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.name_and_type_index.get(clazz).descriptor_index)));
}

constexpr std::string dump_ref(const class_file& clazz, name_and_type_ref ref)
{
    return fmt::format("{} {}", member(dump_ref(clazz, ref.get(clazz).name_index)), desc(dump_ref(clazz, ref.get(clazz).descriptor_index)));
}
constexpr std::string dump_info(const class_file& clazz, name_and_type_info ref)
{
    return fmt::format("{} {}", member(dump_ref(clazz, ref.name_index)), desc(dump_ref(clazz, ref.descriptor_index)));
}

constexpr std::string dump_ref(const class_file& clazz, method_handle_ref ref)
{
    return std::visit(overload{[&clazz](auto e) { return dump_info(clazz, e); }}, ref.get(clazz).reference_index.get(clazz));
}
constexpr std::string dump_info(const class_file& clazz, method_handle_info ref)
{
    return std::visit(overload{[&clazz](auto e) { return dump_info(clazz, e); }}, ref.reference_index.get(clazz));
}

constexpr std::string dump_ref(const class_file& clazz, method_type_ref ref) { return desc(dump_ref(clazz, ref.get(clazz).descriptor_index)); }
constexpr std::string dump_info(const class_file& clazz, method_type_info ref) { return desc(dump_ref(clazz, ref.descriptor_index)); }

constexpr std::string dump_ref(const class_file& clazz, invoke_dynamic_ref ref)
{
    return fmt::format("{}{}", member(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.get(clazz).name_and_type_index.get(clazz).descriptor_index)));
}
constexpr std::string dump_info(const class_file& clazz, invoke_dynamic_info ref)
{
    return fmt::format("{}{}", member(dump_ref(clazz, ref.name_and_type_index.get(clazz).name_index)),
                       desc(dump_ref(clazz, ref.name_and_type_index.get(clazz).descriptor_index)));
}

constexpr std::string pretty_demangle(const class_file& clazz, const utf8_info& ref) { return pretty(demangle_type(escape_str(ref.bytes))); }
constexpr std::string pretty_demangle(const class_file& clazz, utf8_ref ref) { return pretty(demangle_type(escape_str(ref.get(clazz).bytes))); }

constexpr std::string pretty_demangle(const class_file& clazz, name_and_type_info ref) { return pretty_demangle(clazz, ref.descriptor_index); }
constexpr std::string pretty_demangle(const class_file& clazz, name_and_type_ref ref)
{
    return pretty_demangle(clazz, ref.get(clazz).descriptor_index);
}

template <typename... Ts>
struct fmt::formatter<cp_ref<Ts...>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(cp_ref<Ts...> p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", reference("#" + std::to_string(p.get_index())));
    }
};

template <>
struct fmt::formatter<lvt_ref>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(lvt_ref p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", reference("#" + std::to_string(p.index)));
    }
};

template <>
struct fmt::formatter<address_ref>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const address_ref& p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", address("@" + std::to_string(p.ip)));
    }
};

static std::string dump_class_header(const class_file& clazz)
{
    output_consumer s(TAB_SIZE);

    s.w("{}: {}", key("magic"), magic(fmt::format("0x{:x}", clazz.magic)));
    s.w("{}: {}.{}", key("version"), constant(clazz.major_version), constant(clazz.minor_version));
    s.w("{}{} {} {}", flags_to_string(CLASS_FLAGS_NAMES, clazz.access_flags), dump_ref(clazz, clazz.this_class), key("extends"),
        dump_ref(clazz, clazz.super_class));

    return s.data();
}

static std::string dump_constant_pool(const class_file& clazz)
{
    output_consumer s(TAB_SIZE);
    s.w("{} ({}):", key("constants"), constant(clazz.constant_pool.size()));

    std::size_t index = 1;
    for (const auto& cp_entry : clazz.constant_pool)
    {
        s.push();
        s.wp("{}:{} ", reference(fmt::format("#{}", index)), get_padding(clazz.constant_pool.size(), index));
        std::visit(
            overload{
                [](std::monostate) {},
                [&s](const utf8_info& info) { s.w("utf8_info:                 {}", utf8(dump_info(info))); },
                [&s](integer_info info) { s.w("integer_info:              {}", dump_info(info)); },
                [&s](float_info info) { s.w("float_info:                {}", dump_info(info)); },
                [&s](long_info info) { s.w("long_info:                 {}", dump_info(info)); },
                [&s](double_info info) { s.w("double_info:               {}", dump_info(info)); },
                [&s, &clazz](class_info info) { s.w("class_info:                {} {}", info.name_index, dump_info(clazz, info)); },
                [&s, &clazz](string_info info) { s.w("string_info:               {} {}", info.string_index, dump_info(clazz, info)); },
                [&s, &clazz](fieldref_info info) {
                    s.w("fieldref_info:             {}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                        pretty_demangle(clazz, info.name_and_type_index));
                },
                [&s, &clazz](methodref_info info) {
                    s.w("methodref_info:            {}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                        pretty_demangle(clazz, info.name_and_type_index));
                },
                [&s, &clazz](interface_methodref_info info) {
                    s.w("interface_methodref_info:  {}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                        pretty_demangle(clazz, info.name_and_type_index));
                },
                [&s, &clazz](name_and_type_info info) {
                    s.w("name_and_type_info:        {}.{} {} ({})", info.name_index, info.name_index, dump_info(clazz, info),
                        pretty_demangle(clazz, info));
                },
                [&s, &clazz](method_handle_info info) {
                    s.w("method_handle_info:        {} ({}) {} ({})", info.reference_index, key(METHOD_HANDLE_REF_TYPES[info.reference_kind]),
                        dump_info(clazz, info),
                        std::visit([&clazz](auto i) { return pretty_demangle(clazz, i.name_and_type_index); }, info.reference_index.get(clazz)));
                },
                [&s, &clazz](method_type_info info) {
                    s.w("method_type_info:          {} {} ({})", info.descriptor_index, desc(dump_ref(clazz, info.descriptor_index)),
                        pretty_demangle(clazz, info.descriptor_index));
                },
                [&s, &clazz](invoke_dynamic_info info) {
                    s.w("invoke_dynamic_info:       {}.{} {} ({})", reference(fmt::format("#{}", info.bootstrap_method_attr_index)),
                        info.name_and_type_index, dump_info(clazz, info), pretty_demangle(clazz, info.name_and_type_index));
                },
            },
            cp_entry);
        s.pop();
        index++;
    }

    return s.data();
}

static void dump_instruction(const class_file& clazz, const inst& i, output_consumer& s, size_t ip, size_t max_sz)
{
    std::string buf = instruction(fmt::format("{: <16} ", i.opcode <= 0xca ? opcodes[i.opcode] : "<bad opcode>"));

    if (i.opcode == 0xaa)
    {
        const tableswitch_data& data = std::get<tableswitch_data>(i.special);
        s.w("{}:{} {} (def, hi, lo) = {} {} {}", address_ref(ip), get_padding(max_sz, ip), buf, data.def.resolve(ip), constant(data.high),
            constant(data.low));
        s.push(2);
        for (auto off = 0; off < data.lut.size(); off++)
            s.w("{} -> {}", constant(data.low + off), data.lut[off].resolve(ip));
        s.pop(2);
        return;
    }
    else if (i.opcode == 0xab)
    {
        const lookupswitch_data& data = std::get<lookupswitch_data>(i.special);

        s.w("{}:{} {} def = {}", address_ref(ip), get_padding(max_sz, ip), buf, data.def.resolve(ip));
        s.push(2);
        for (auto off = 0; off < data.lut.size(); off++)
            s.w("{} -> {}", constant(data.lut[off].first), data.lut[off].second.resolve(ip));
        s.pop(2);

        return;
    }
    else if (i.opcode == 0xc4)
        buf = instruction(fmt::format("w.{: <14} ", opcodes[std::get<wide_data>(i.special).op]));

    buf += std::visit(
        overload{
            [](std::monostate) { return std::string(); }, [](int v) { return constant(v); },
            [](lvt_ref ref) { return reference("#" + std::to_string(ref.index)); },
            [ip](address_offset ref) { return address("@" + std::to_string(ref.resolve(ip).ip)); },
            [&clazz](is_member auto ref) {
                return fmt::format("{}({})", ref, italic("{} {}", dump_ref(clazz, ref), pretty_demangle(clazz, ref.get(clazz).name_and_type_index)));
            },
            [&clazz](is_primitive auto ref) { return fmt::format("{}({})", ref, italic(dump_ref(clazz, ref))); },
            [&clazz](method_type_ref ref) {
                return fmt::format("{}({})", ref, italic("{} {}", dump_ref(clazz, ref), pretty_demangle(clazz, ref.get(clazz).descriptor_index)));
            },
            [&clazz](method_handle_ref ref) {
                auto pretty = italic("{} {} {}", key(METHOD_HANDLE_REF_TYPES[ref.get(clazz).reference_kind]), dump_ref(clazz, ref),
                                     std::visit([&clazz](auto i) { return pretty_demangle(clazz, i.name_and_type_index); },
                                                ref.get(clazz).reference_index.get(clazz)));
                return fmt::format("{}({})", ref, pretty);
            },
            [&clazz](class_ref ref) { return fmt::format("{}({})", ref, italic("{}", dump_ref(clazz, ref))); },
            [&clazz](invoke_dynamic_ref ref) {
                return fmt::format("{}({})", ref, italic("{} {}", dump_ref(clazz, ref), pretty_demangle(clazz, ref.get(clazz).name_and_type_index)));
            },
            [](primitive_type_ref ref) { return type(ref.name()); }},
        i.operand1);

    buf += ' ' + std::visit(overload{[](std::monostate) { return std::string(); }, [](int v) { return constant(v); }}, i.operand2);

    s.w("{}:{} {}", address_ref(ip), get_padding(max_sz, ip), buf);
}

static void dump_attribute(const class_file& clazz, const attribute& attr, output_consumer& s);

static void dump_code_attribute(const class_file& clazz, const code_attribute& attr, output_consumer& s)
{
    s.w("- Code");
    s.push();
    s.w("{}: {}", key("max_locals"), constant(attr.max_locals));
    s.w("{}: {}", key("max_stack"), constant(attr.max_stack));
    size_t ip = 0;
    s.push();
    for (const auto& i : attr.code)
    {
        dump_instruction(clazz, i, s, ip, attr.max_ip);
        ip += i.inst_sz;
    }
    s.pop();
    s.w("{} ({}):", key("exception_table"), constant(attr.exception_table.size()));
    s.push();

    for (const auto& i : attr.exception_table)
    {
        s.w("{} {} {} [{}, {}) {}", key("catch"), i.catch_type.has_value() ? dump_info(clazz, i.catch_type.get(clazz)) : type("<all>"), key("in"),
            i.start_pc, i.end_pc, i.handler_pc);
    }
    s.pop();
    s.w("{} ({}):", key("attributes"), constant(attr.attributes.size()));
    s.push();
    for (const auto& i : attr.attributes)
    {
        dump_attribute(clazz, i, s);
    }
    s.pop(2);
}

static std::string dump_verification_type_info(const class_file& clazz, const stackmap::verification_type_info& info)
{
    switch (info.tag)
    {
    case stackmap::VERIFICATION_TOP:
        return type("top");
    case stackmap::VERIFICATION_INTEGER:
        return type("int");
    case stackmap::VERIFICATION_FLOAT:
        return type("float");
    case stackmap::VERIFICATION_DOUBLE:
        return type("double");
    case stackmap::VERIFICATION_LONG:
        return type("long");
    case stackmap::VERIFICATION_NULL:
        return type("null");
    case stackmap::VERIFICATION_UNINITIALIZED_THIS:
        return type("uninitalized-this");
    case stackmap::VERIFICATION_OBJECT:
        return dump_ref(clazz, class_ref(clazz, info.data));
    case stackmap::VERIFICATION_UNINITIALIZED:
        return fmt::format("{} ({}{})", key("uninitalized"), key("new"), info.data);
    }

    __builtin_unreachable();
}

static void dump_stack_map_table(const class_file& clazz, const stack_map_table_attribute& attr, output_consumer& s)
{
    int64_t off = -1;
    for (const auto& i : attr.entries)
    {
        uint8_t frame_type = i.frame_type;
        std::string type;
        std::string extra;
        if (frame_type >= 0 && frame_type <= 63)
        {
            off += frame_type + 1;
            s.w("{}: {} ({})", address_ref(off), key("same"), constant(frame_type));
        }
        else if (frame_type >= 64 && frame_type <= 127)
        {
            off += frame_type - 64 + 1;
            s.w("{}: {} ({}) {}", address_ref(off), key("same_locals_1_stack_item"), constant(frame_type),
                dump_verification_type_info(clazz, std::get<stackmap::stack_map_frame::same_locals_1_stack_item_frame>(i.data).stack));
        }
        else if (frame_type == 247)
        {
            off += std::get<stackmap::stack_map_frame::same_locals_1_stack_item_frame_extended>(i.data).offset_delta + 1;
            s.w("{}: {} ({}) {}", address_ref(off), key("same_locals_1_stack_item_ext"), constant(frame_type),
                dump_verification_type_info(clazz, std::get<stackmap::stack_map_frame::same_locals_1_stack_item_frame_extended>(i.data).stack));
        }
        else if (frame_type >= 248 && frame_type <= 250)
        {
            off += std::get<stackmap::stack_map_frame::chop_frame>(i.data).offset_delta + 1;
            s.w("{}: {} ({}) {}", address_ref(off), key("chop"), constant(frame_type), constant(251 - frame_type));
        }
        else if (frame_type == 251)
        {
            off += std::get<stackmap::stack_map_frame::same_frame_extended>(i.data).offset_delta + 1;
            s.w("{}: {} ({})", address_ref(off), key("same_ext"), constant(frame_type));
        }
        else if (frame_type >= 252 && frame_type <= 254)
        {
            off += std::get<stackmap::stack_map_frame::append_frame>(i.data).offset_delta + 1;
            s.w("{}: {} ({})", address_ref(off), key("append"), constant(frame_type));
            s.push();
            for (const auto& i : std::get<stackmap::stack_map_frame::append_frame>(i.data).locals)
                s.w(dump_verification_type_info(clazz, i));
            s.pop();
        }
        else
        {
            off += std::get<stackmap::stack_map_frame::full_frame>(i.data).offset_delta + 1;
            s.w("{}: {} ({})", address_ref(off), key("full"), constant(frame_type));
            s.push();
            s.w("{}:", key("stack"));

            s.push();
            for (const auto& i : std::get<stackmap::stack_map_frame::full_frame>(i.data).stack)
                s.w(dump_verification_type_info(clazz, i));
            s.pop();

            s.w("{}:", key("locals"));
            s.push();
            for (const auto& i : std::get<stackmap::stack_map_frame::full_frame>(i.data).locals)
                s.w(dump_verification_type_info(clazz, i));
            s.pop(2);
        }
    }
}

static void dump_bootstrap_methods_attribute(const class_file& clazz, const bootstrap_methods_attribute& attr, output_consumer& s)
{
    s.w("- BoostrapMethods");
    s.push();
    std::string res;
    size_t index = 0;
    for (const auto& i : attr.bootstrap_methods)
    {
        auto t = i.bootstrap_method_ref.get(clazz);
        auto r = std::get<methodref_info>(t.reference_index.get(clazz));
        s.wp("{}:{} ", reference(fmt::format("#{}", index)), get_padding(attr.bootstrap_methods.size(), index));
        s.w("{} {} {} ({})", t.reference_index, key(METHOD_HANDLE_REF_TYPES[t.reference_kind]), dump_info(clazz, r),
            pretty_demangle(clazz, r.name_and_type_index));

        s.push();
        for (const auto& j : i.bootstrap_arguments)
        {
            std::visit(
                overload{
                    [](std::monostate) {},
                    [&s](const utf8_info& info) { s.w(" {}", utf8(dump_info(info))); },
                    [&s](integer_info info) { s.w("{}", dump_info(info)); },
                    [&s](float_info info) { s.w("{}", dump_info(info)); },
                    [&s](long_info info) { s.w("{}", dump_info(info)); },
                    [&s](double_info info) { s.w("{}", dump_info(info)); },
                    [&s, &clazz](class_info info) { s.w("{} {}", info.name_index, dump_info(clazz, info)); },
                    [&s, &clazz](string_info info) { s.w("{} {}", info.string_index, dump_info(clazz, info)); },
                    [&s, &clazz](fieldref_info info) {
                        s.w("{}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                            pretty_demangle(clazz, info.name_and_type_index));
                    },
                    [&s, &clazz](methodref_info info) {
                        s.w("{}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                            pretty_demangle(clazz, info.name_and_type_index));
                    },
                    [&s, &clazz](interface_methodref_info info) {
                        s.w("{}.{} {} ({})", info.class_index, info.name_and_type_index, dump_info(clazz, info),
                            pretty_demangle(clazz, info.name_and_type_index));
                    },
                    [&s, &clazz](name_and_type_info info) {
                        s.w("{}.{} {} ({})", info.name_index, info.name_index, dump_info(clazz, info), pretty_demangle(clazz, info));
                    },
                    [&s, &clazz](method_handle_info info) {
                        s.w("{} ({}) {} ({})", info.reference_index, key(METHOD_HANDLE_REF_TYPES[info.reference_kind]), dump_info(clazz, info),
                            std::visit([&clazz](auto i) { return pretty_demangle(clazz, i.name_and_type_index); }, info.reference_index.get(clazz)));
                    },
                    [&s, &clazz](method_type_info info) {
                        s.w("{} {} ({})", info.descriptor_index, desc(dump_ref(clazz, info.descriptor_index)),
                            pretty_demangle(clazz, info.descriptor_index));
                    },
                    [&s, &clazz](invoke_dynamic_info info) {
                        s.w("{}.{} {} ({})", reference(fmt::format("#{}", info.bootstrap_method_attr_index)), info.name_and_type_index,
                            dump_info(clazz, info), pretty_demangle(clazz, info.name_and_type_index));
                    },
                },
                clazz.constant_pool[j.get_index() - 1]);
        }

        index++;
        s.pop();
    }

    s.pop();
}

static void dump_attribute(const class_file& clazz, const attribute& attr, output_consumer& s)
{
    std::visit(
        overload{[&s, &clazz](const attribute_info& info) { s.w("- {} (unknown)", dump_ref(clazz, info.attribute_name_index)); },
                 [&clazz, &s](const code_attribute& attr) { dump_code_attribute(clazz, attr, s); },
                 [&s, &clazz](const signature_attribute& attr) { s.w("- Signature: {}", type(escape_str(attr.signature_index.get(clazz).bytes))); },
                 [&s, &clazz](const source_file_attribute& attr) { s.w("- SourceFile: {}", escape_str(attr.sourcefile_index.get(clazz).bytes)); },
                 [&s, &clazz](const lvt_attribute& attr) {
                     s.w("- LocalVariableTable");
                     s.push();
                     for (const auto& i : attr.lvt)
                     {
                         s.w("{} {} {} ({}) [{}, {}) ({} bytes)", i.index, member(escape_str(i.name_index.get(clazz).bytes)),
                             desc(escape_str(i.descriptor_index.get(clazz).bytes)), pretty_demangle(clazz, i.descriptor_index), i.start_pc,
                             address_ref(i.start_pc.ip + i.length + 1), constant(i.length));
                     }
                     s.pop();
                 },
                 [&clazz, &s](const inner_class_attribute& attr) {
                     s.w("- InnerClasses");
                     s.push();
                     for (const auto& i : attr.inner_classes)
                     {
                         s.w("{}{} {} {}", flags_to_string(INNER_CLASS_FLAGS_NAMES, i.inner_class_access_flags),
                             type(dump_ref(clazz, i.inner_class_info_index.get(clazz).name_index)),
                             type(i.inner_name_index.has_value() ? dump_info(clazz, i.inner_name_index.get(clazz)) : "<anon>"),
                             type(i.outer_class_info_index.has_value() ? dump_info(clazz, i.outer_class_info_index.get(clazz)) : "<anon>"));
                     }
                     s.pop();
                 },
                 [&s](const lineno_attribute& attr) {
                     s.w("- LineNumberTable");
                     s.push();

                     for (const auto& i : attr.line_number_table)
                         s.w("{} {}", constant(i.line_number), i.start_pc);
                     s.pop();
                 },
                 [&s, &clazz](const stack_map_table_attribute& attr) {
                     s.w("- StackMapTable");
                     s.push();
                     dump_stack_map_table(clazz, attr, s);
                     s.pop();
                 },
                 [&s, &clazz](const bootstrap_methods_attribute& attr) { dump_bootstrap_methods_attribute(clazz, attr, s); },
                 [&clazz, &s](const lvt_type_attribute& attr) {
                     s.w("- LocalVariableTypeTable");
                     s.push();
                     for (const auto& i : attr.lvt)
                         s.w("{} {} {} [{}, {}) ({} bytes)", i.index, member(escape_str(i.name_index.get(clazz).bytes)),
                             desc(escape_str(i.signature_index.get(clazz).bytes)), i.start_pc, address_ref(i.start_pc.ip + i.length + 1),
                             constant(i.length));
                     s.pop();
                 },
                 [&s, &clazz](const nest_members_attribute& attr) {
                     s.w("- NestMembers");
                     s.push();
                     for (auto i : attr.classes)
                         s.w("{}", dump_ref(clazz, i));
                     s.pop();
                 },
                 [&s, &clazz](const nest_host_attribute& attr) { s.w("- NestHost {}", dump_ref(clazz, attr.host_class_index)); },
                 [&s, &clazz](const constant_value_attribute& attr) {
                     s.w("- ConstantValue {}", std::visit([&clazz](auto i) { return dump_info(clazz, i); }, attr.constantvalue_index.get(clazz)));
                 },
                 [&s, &clazz](const exceptions_attribute& attr) {
                     s.w("- Exceptions");
                     s.push();
                     for (auto i : attr.exception_index_table)
                         s.w("{}", dump_ref(clazz, i));
                     s.pop();
                 },
                 [&s, &clazz](const enclosing_method_attribute& attr) {
                     s.w("- EnclosingMethod");
                     s.push();
                     s.w("{}/{}", dump_ref(clazz, attr.class_index),
                         attr.method_index.has_value()
                             ? fmt::format("{}{} {}", member(dump_ref(clazz, attr.method_index.get(clazz).descriptor_index)),
                                           desc(dump_ref(clazz, attr.method_index.get(clazz).descriptor_index)),
                                           pretty_demangle(clazz, attr.method_index.get(clazz).descriptor_index))
                             : "<no method>");
                     s.pop();
                 }},
        attr);
}

static std::string dump_fields(const class_file& clazz)
{

    output_consumer s(TAB_SIZE);
    s.w("{} ({}):", key("fields"), constant(clazz.fields.size()));
    s.push();

    for (const auto& field : clazz.fields)
    {
        s.w("{}{} {} ({})", flags_to_string(FIELD_FLAGS_NAMES, field.access_flags), member(dump_ref(clazz, field.name_index)),
            desc(dump_ref(clazz, field.descriptor_index)), pretty_demangle(clazz, field.descriptor_index));

        s.push();
        s.w("{} ({}):", key("attributes"), constant(field.attributes.size()));
        s.push();
        for (const auto& attr : field.attributes)
            dump_attribute(clazz, attr, s);
        s.pop(2);
    }

    s.pop();
    return s.data();
}

static std::string dump_methods(const class_file& clazz)
{
    output_consumer s(TAB_SIZE);
    s.w("{} ({}):", key("methods"), constant(clazz.methods.size()));
    s.push();

    for (const auto& method : clazz.methods)
    {
        s.w("{}{}{} ({})", flags_to_string(METHOD_FLAGS_NAMES, method.access_flags), member(dump_ref(clazz, method.name_index)),
            desc(dump_ref(clazz, method.descriptor_index)), pretty_demangle(clazz, method.descriptor_index));

        s.push();
        s.w("{} ({}):", key("attributes"), constant(method.attributes.size()));
        s.push();
        for (const auto& attr : method.attributes)
            dump_attribute(clazz, attr, s);
        s.pop(2);
    }

    s.pop();
    return s.data();
}

static std::string dump_class_attributes(const class_file& clazz)
{
    output_consumer s(TAB_SIZE);
    s.w("{} ({}):", key("attributes"), constant(clazz.attributes.size()));
    s.push();
    for (const auto& i : clazz.attributes)
        dump_attribute(clazz, i, s);
    s.pop();
    return s.data();
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << fmt::format("usage: {} [classfiles...]", argv[0]);
        exit(-1);
    }

    for (size_t i = 1; i < argc; i++)
    {
        std::cout << "dumping class " << argv[i] << '\n';
        auto c = parse_class(argv[i]);
        std::cout << dump_class_header(c) << dump_constant_pool(c) << dump_fields(c) << dump_methods(c) << dump_class_attributes(c);
    }
}
