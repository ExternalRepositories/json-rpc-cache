#include "cache.hpp"

#include "helper.hpp"
#include "mockup.hpp"
#include "rapidjson/rapidjson.h"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <rapidjson/document.h>

TEST_CASE("Order of keys in json does not matter", "[normalization]")
{
    auto variant1 = helper::json_string("key0", 0, "key1", 1);
    auto variant2 = helper::json_string("key1", 1, "key0", 0);

    REQUIRE(caching::detail::normalize_json(variant1) == caching::detail::normalize_json(variant2));
}

TEST_CASE("Order of keys in embedded object does not matter", "[normalization]")
{
    auto depth2_1  = helper::json_string("key0", 0, "key1", 1);
    auto depth1_1  = helper::json_string("depth2", depth2_1, "value", 0);
    auto variant_1 = helper::json_string("embedded", depth1_1, "value", 0);

    auto depth2_2  = helper::json_string("key1", 1, "key0", 0);
    auto depth1_2  = helper::json_string("value", 0, "depth2", depth2_2);
    auto variant_2 = helper::json_string("value", 0, "embedded", depth1_1);

    // std::cout << variant_1 << std::endl << variant_2 << std::endl;

    REQUIRE(caching::detail::normalize_json(variant_1) == caching::detail::normalize_json(variant_2));
}

TEST_CASE("Additional whitespace is ignored", "[normalization]")
{
    auto variant1 = helper::json_string("key0", 0, "key1", 1);
    auto variant2 = "{\"key0\"    :    0,    \"key1\":  1 }";

    REQUIRE(caching::detail::normalize_json(variant1) == caching::detail::normalize_json(variant2));
}

TEST_CASE("Input is case sensitive", "[normalization]")
{
    auto variant1 = helper::json_string("KEY0", 0, "KEY1", 1);
    auto variant2 = helper::json_string("key1", 1, "key0", 0);

    REQUIRE(caching::detail::normalize_json(variant1) != caching::detail::normalize_json(variant2));
}

TEST_CASE("Inserted object can be found", "[cache]")
{
    auto const method = "aMethod";
    auto const params = helper::param_string(helper::json_string("key", "value"));

    caching::cache cache;
    cache.lookup_or_insert(method, params, mockup::factory::ignorable_value{});

    REQUIRE(cache.lookup(method, params).has_value());
}

TEST_CASE("Cached value is correct", "[cache]")
{
    auto const method = "aMethod";
    auto const params = helper::param_string(helper::json_string("key", "value"));
    auto const value  = helper::json_string("value", 1, "context", "test");

    caching::cache cache;
    cache.lookup_or_insert(method, params, mockup::factory::fixed_value{value});

    REQUIRE(cache.lookup(method, params).has_value());
    REQUIRE(cache.lookup(method, params).value() == value);
}

TEST_CASE("Refresh refreshes the cached value", "[cache]")
{
    auto const method = "aMethod";
    auto const params = helper::param_string(helper::json_string("key", "value"));
    auto const value  = helper::json_string("value", 1, "context", "test");

    caching::cache cache;
    cache.lookup_or_insert(method, params, mockup::factory::fixed_value{value});

    REQUIRE(cache.lookup(method, params).value() == value);

    auto const value2 = "other value";
    cache.refresh(method, params, value2);
    REQUIRE(cache.lookup(method, params).value() == value2);
}

TEST_CASE("Empty parameters equals no parameter", "[parameters]")
{
    auto const method  = "withEmptyParams";
    auto const method2 = "withNoParams";
    auto const params  = helper::param_string("");

    caching::cache cache;

    // insertion with empty, lookup with none
    cache.lookup_or_insert(method, params, mockup::factory::ignorable_value{});
    REQUIRE(cache.lookup(method).has_value());

    // insertion with none, lookup with empty
    cache.lookup_or_insert(method2, mockup::factory::ignorable_value{});
    REQUIRE(cache.lookup(method2, params).has_value());
}
