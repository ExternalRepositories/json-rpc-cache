#pragma once

#include <string>
#include <type_traits>

namespace helper
{

    template <typename V, std::enable_if_t<std::is_arithmetic_v<V>, void*> = nullptr>
    inline auto json_pair(std::string_view key, V value)
    {
        return std::string{"\""} + key.data() + "\":" + std::to_string(value);
    }

    inline auto json_pair(std::string_view key, std::string_view value)
    {
        return std::string{"\""} + key.data() + "\":" + value.data();
    }

    inline auto json_pair_list() { return std::string{}; }

    template <typename V> inline auto json_pair_list(std::string_view key, V value) { return json_pair(key, value); }

    template <typename V, typename... Args> inline auto json_pair_list(std::string_view key, V value, Args... args)
    {
        return json_pair(key, value) + "," + json_pair_list(args...);
    }

    template <typename... Args> inline auto json_string(Args... args) { return "{" + json_pair_list(args...) + "}"; }

    inline auto param_string(std::string contents) { return "[" + contents + "]"; }
} // namespace helper
