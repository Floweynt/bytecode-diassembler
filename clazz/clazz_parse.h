#pragma once
#include "decomp.h"
#include <stdexcept>

class class_parse_error : public std::runtime_error
{
public:
    inline class_parse_error(const std::string& str) : std::runtime_error(str) {}
};

class_file parse_class(const std::string& file);
