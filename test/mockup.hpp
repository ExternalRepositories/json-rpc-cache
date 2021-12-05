#pragma once

#include <string>

namespace mockup::factory
{
    struct ignorable_value
    {
        auto operator()(std::string const&, std::string_view) const -> std::string { return ""; }
    };

    struct fixed_value
    {
        std::string const value;
        auto operator()(std::string const&, std::string_view) const -> std::string { return value; }
    };
} // namespace mockup::factory
