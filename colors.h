#include <string>
#include <fmt/core.h>
#include <utility>

constexpr std::string flags(const std::string& s) { return fmt::format("\x1b[91m{}\x1b[0m", s); }
constexpr std::string black(const std::string& s) { return fmt::format("\x1b[30m{}\x1b[0m", s); }
constexpr std::string red(const std::string& s) { return fmt::format("\x1b[31m{}\x1b[0m", s); }
constexpr std::string green(const std::string& s) { return fmt::format("\x1b[32m{}\x1b[0m", s); }
constexpr std::string yellow(const std::string& s) { return fmt::format("\x1b[33m{}\x1b[0m", s); }
constexpr std::string blue(const std::string& s) { return fmt::format("\x1b[34m{}\x1b[0m", s); }
constexpr std::string magenta(const std::string& s) { return fmt::format("\x1b[35m{}\x1b[0m", s); }
constexpr std::string cyan(const std::string& s) { return fmt::format("\x1b[36m{}\x1b[0m", s); }
constexpr std::string bright_black(const std::string& s) { return fmt::format("\x1b[90m{}\x1b[0m", s); }
constexpr std::string bright_red(const std::string& s) { return fmt::format("\x1b[91m{}\x1b[0m", s); }
constexpr std::string bright_green(const std::string& s) { return fmt::format("\x1b[92m{}\x1b[0m", s); }
constexpr std::string bright_yellow(const std::string& s) { return fmt::format("\x1b[93m{}\x1b[0m", s); }
constexpr std::string bright_blue(const std::string& s) { return fmt::format("\x1b[94m{}\x1b[0m", s); }
constexpr std::string bright_magenta(const std::string& s) { return fmt::format("\x1b[95m{}\x1b[0m", s); }
constexpr std::string bright_cyan(const std::string& s) { return fmt::format("\x1b[96m{}\x1b[0m", s); }
constexpr std::string ansi(const std::string& format, const std::string& s)
{
    return fmt::format("\x1b[{}m{}\x1b[0m", format, s);
}
constexpr std::string italic(const std::string& str) 
{
    return fmt::format("\x1b[3m{}\x1b[0m", str);
}
constexpr auto key(const std::string& str) { return bright_blue(str); }
constexpr auto obj(const std::string& str) { return yellow(str); }
constexpr auto constant(const std::string& str) { return green(str); }
constexpr auto reference(const std::string& str) { return bright_green(str); }
constexpr auto code(const std::string& str) { return bright_red(str); }

template <typename... Args>
constexpr std::string flags(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[91m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}

template <typename... Args>
constexpr std::string black(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[30m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string red(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[31m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string green(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[32m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string yellow(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[33m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string blue(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[34m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string magenta(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[35m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string cyan(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[36m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_black(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[90m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_red(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[91m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_green(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[92m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_yellow(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[93m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_blue(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[94m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_magenta(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[95m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string bright_cyan(const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[96m{}\x1b[0m", fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr std::string ansi(const std::string& format, const fmt::format_string<Args...>& s, Args&&... args)
{
    return fmt::format("\x1b[{}m{}\x1b[0,", format, fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr auto key(const fmt::format_string<Args...>& s, Args&&... args)
{
    return bright_cyan(fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr auto obj(const fmt::format_string<Args...>& s, Args&&... args)
{
    return yellow(fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr auto constant(const fmt::format_string<Args...>& s, Args&&... args)
{
    return green(fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr auto reference(const fmt::format_string<Args...>& s, Args&&... args)
{
    return bright_green(fmt::format(s, std::forward<Args>(args)...));
}
template <typename... Args>
constexpr auto code(const fmt::format_string<Args...>& s, Args&&... args)
{
    return bright_red(fmt::format(s, std::forward<Args>(args)...));
}
