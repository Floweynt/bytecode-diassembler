#include <fstream>

class byte_file
{
    std::ifstream ifs;

public:
    inline byte_file(std::string path) : ifs(path, std::ios::binary) {}

    inline uint8_t read_u8()
    {
        uint8_t n;
        ifs.read((char*)&n, 1);
        return n;
    }

    inline uint16_t read_u16()
    {
        uint16_t n;
        ifs.read((char*)&n, 2);
        return __builtin_bswap16(n);
    }

    inline uint32_t read_u32()
    {
        uint32_t n;
        ifs.read((char*)&n, 4);
        return __builtin_bswap32(n);
    }

    inline uint64_t read_u64()
    {
        uint64_t n;
        ifs.read((char*)&n, 8);
        return __builtin_bswap64(n);
    }

    constexpr const auto& get_stream() const { return ifs; }
    constexpr  auto& get_stream()  { return ifs; }
};

