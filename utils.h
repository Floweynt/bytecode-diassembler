// cSpell:ignore clazz
#pragma once
#include "clazz/clazz.h"
#include <stdexcept>
#include <string>

template <typename... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
overload(Ts...) -> overload<Ts...>;

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
    case '[': {
        size_t arr_count = 0;
        size_t i;
        for (i = index; i < str.size() && str[i] == '['; i++)
            arr_count++;
        return demangle_type(str, i, out) + dup("[]", i);
    }
    default: {
        throw std::runtime_error("unable to demangle: " +  str);
    }
    }
}

constexpr std::string get_padding(auto i1, auto i2)
{
    return std::string(std::to_string(i1).size() - std::to_string(i2).size(), ' ');
}

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

static constexpr std::pair<clazz::inner_class_access_flags, const char*> INNER_CLASS_FLAGS_NAMES[] = {
    {clazz::IC_ACC_PUBLIC, "public "},         {clazz::IC_ACC_PRIVATE, "private "},
    {clazz::IC_ACC_PROTECTED, "protected "},   {clazz::IC_ACC_STATIC, "static "},
    {clazz::IC_ACC_FINAL, "final "},           {clazz::IC_ACC_INTERFACE, "interface "},
    {clazz::IC_ACC_ABSTRACT, "abstract "},     {clazz::IC_ACC_SYNTHETIC, "synthetic "},
    {clazz::IC_ACC_ANNOTATION, "annotation "}, {clazz::IC_ACC_ENUM, "enum "},
};
static constexpr std::pair<clazz::class_access_flags, const char*> CLASS_FLAGS_NAMES[] = {
    {clazz::CL_ACC_PUBLIC, "public "},         {clazz::CL_ACC_FINAL, "final "},
    {clazz::CL_ACC_SUPER, "super "},           {clazz::CL_ACC_INTERFACE, "interface "},
    {clazz::CL_ACC_ABSTRACT, "abstract "},     {clazz::CL_ACC_SYNTHETIC, "synthetic "},
    {clazz::CL_ACC_ANNOTATION, "annotation "}, {clazz::CL_ACC_ENUM, "enum "},
};
static constexpr std::pair<clazz::field_access_flags, const char*> FIELD_FLAGS_NAMES[] = {
    {clazz::FIELD_ACC_PUBLIC, "public "},       {clazz::FIELD_ACC_PRIVATE, "private "},
    {clazz::FIELD_ACC_PROTECTED, "protected "}, {clazz::FIELD_ACC_STATIC, "static "},
    {clazz::FIELD_ACC_FINAL, "final "},         {clazz::FIELD_ACC_VOLATILE, "volatile "},
    {clazz::FIELD_ACC_TRANSIENT, "transient "}, {clazz::FIELD_ACC_SYNTHETIC, "synthetic "},
    {clazz::FIELD_ACC_ENUM, "enum "},

};
static constexpr std::pair<clazz::method_access_flags, const char*> METHOD_FLAGS_NAMES[] = {
    {clazz::METHOD_ACC_PUBLIC, "public "},       {clazz::METHOD_ACC_PRIVATE, "private "},
    {clazz::METHOD_ACC_PROTECTED, "protected "}, {clazz::METHOD_ACC_STATIC, "static "},
    {clazz::METHOD_ACC_FINAL, "final "},         {clazz::METHOD_ACC_SYNCHRONIZED, "synchronized "},
    {clazz::METHOD_ACC_BRIDGE, "bridge "},       {clazz::METHOD_ACC_VARARGS, "varargs "},
    {clazz::METHOD_ACC_NATIVE, "native "},       {clazz::METHOD_ACC_ABSTRACT, "abstract "},
    {clazz::METHOD_ACC_STRICT, "strict "},       {clazz::METHOD_ACC_SYNTHETIC, "synthetic "},
};
