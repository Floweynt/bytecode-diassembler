// cSpell:ignore clazz
#include "clazz_parse.h"
    
template <typename... Args>
static constexpr inline uint16_t validate_constant_index(const class_file& clazz, uint16_t index)
{
    if (index == 0 || index > clazz.constant_pool.size())
        throw class_parse_error("bad index into constant pool");
    if(!(std::holds_alternative<Args>(clazz.constant_pool[index - 1]) || ...))
        throw class_parse_error("invalid type of constant");
    return index;
}
