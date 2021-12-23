/// @file cache.hpp
///
/// @brief Implementation of a simple in-memory cache for json-rpc requests.
///
/// Namespace caching::detail contains helper functions.
/// The actual caching is implemented and accessible via class caching::cache.
///

#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <unordered_map>

namespace caching
{
    namespace detail
    {
        using parameters = rapidjson::Document;
        using response = std::string;

        struct hashable_params
        {
            parameters params;
            std::string json;
        };

        struct request
        {
            std::string const method;
            hashable_params h_params;
        };

        inline auto operator==(request const& lhs, request const& rhs)
        {
            return lhs.method == rhs.method && lhs.h_params.json == rhs.h_params.json;
        }

        /// @brief hashing operation for request to enable map storage with request in key-place.
        struct request_hash
        {
            auto operator()(request const& rq) const noexcept -> size_t
            {
                auto const method_hash = std::hash<std::string>{}(rq.method);
                auto const params_hash = std::hash<std::string>{}(rq.h_params.json);
                return method_hash ^ (params_hash << 1);
            }
        };

        /// @brief Comparator for two keys from a rapidjson object.
        inline auto json_key_cmp(rapidjson::Value::Member const& lhs, rapidjson::Value::Member const& rhs)
        {
            return std::string_view(lhs.name.GetString()) < std::string_view(rhs.name.GetString());
        }

        /// @brief Recursive sort function for `rapidjson::GenericValue`s.
        /// @param json_object is e.g. an instance of `rapidjson::Document`.
        template <typename T> inline auto sort_recursively(T&& json_object) -> void
        {
            for (auto it{json_object.MemberBegin()}; it != json_object.MemberEnd(); ++it)
            {
                if (it->value.IsObject())
                {
                    sort_recursively(it->value.GetObject());
                }
            }
            std::sort(json_object.MemberBegin(), json_object.MemberEnd(), json_key_cmp);
        }

        /// @brief returns a string representation of the json document.
        inline auto to_string(rapidjson::Document const& doc) -> std::string
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer{buffer};
            doc.Accept(writer);
            return buffer.GetString();
        }

        /// @brief normalizes the given json document.
        /// @see normalize_json(std::string_view)
        inline auto normalize_json(rapidjson::Document& doc) { sort_recursively(doc); }

        /// @brief normalizes the given json string.
        ///
        /// Raw json may differ in layout and whitespacing. Normalizing first constructs a
        /// rapidjson object from the raw data, sorts recursively by key, then converts back to a string.
        /// @param json is a json object string.
        inline auto normalize_json(std::string_view json) -> std::string
        {
            rapidjson::Document doc;
            doc.Parse(json.data());
            normalize_json(doc);
            return to_string(doc);
        }
    } // namespace detail

    /// @brief Simple cache implementation for json-rpc request / response buffering.
    ///
    /// The interface expects and returns all string values.
    /// Internally, cache storage and lookup is done via conversion of the parameters to
    /// a rapidjson document, thereby eliminating false lookups for possibly reordered keys.
    ///
    /// No parameters equal an empty list of parameters here. Case sensitivity of all values
    /// is preserved, as is the case in json-rpc.
    class cache
    {
        using cache_map = std::unordered_map<detail::request, detail::response, detail::request_hash>;
        cache_map cache_;

        auto make_request(std::string const& method, std::string_view params) const -> detail::request
        {
            detail::request rq{.method = method};

            rq.h_params.params.Parse(params.data());
            rq.h_params.json = detail::to_string(rq.h_params.params);
            return rq;
        }

        auto insert(std::string const& method, std::string_view params, std::string_view value) -> std::string_view
        {
            cache_[make_request(method, params)] = value;
            return value;
        }

        auto get_if(detail::request const& req) const -> std::optional<detail::response>
        {
            return cache_.contains(req) ? std::make_optional(cache_.at(req)) : std::nullopt;
        }

    public:
        /// @brief Lookup the response for `method` with `params`.
        /// @param method is the name of the method to lookup.
        /// @param params is a json string of the parameter list, i.e. "[<json-key-values>]"
        /// @return an optional containing the response stored, if any.
        auto lookup(std::string const& method, std::string_view params) const -> std::optional<detail::response>
        {
            return get_if(detail::request{make_request(method, params)});
        }

        /// @brief Lookup the response for `method` with an empty parameter list, i.e. lookup(method, "[]").
        auto lookup(std::string const& method) const -> std::optional<detail::response> { return lookup(method, "[]"); }

        using factory = std::function<std::string(std::string const&, std::string_view)>;

        /// @brief Lookup or create the response for `method` with `params`.
        ///
        /// If the value for the given parameters is cached, this equals `lookup(method, params)`.
        /// Else, `f` is called with `method` and `params` and is expected to return a string value,
        /// which will then be cached and returned.
        /// @param method is the name of the method to lookup.
        /// @param params is a json string of the parameter list, i.e. "[<json-key-values>]"
        /// @param f is a factory for a string value.
        /// @return the cached or newly created value for the given arguments.
        auto lookup_or_insert(std::string const& method, std::string_view params, factory f) -> std::string_view
        {
            if (auto existing{lookup(method, params)}; existing)
            {
                return existing.value();
            }

            return insert(method, params, f(method, params));
        }

        /// @brief This corresponds to `lookup_or_insert(method, "[]", f)`.
        auto lookup_or_insert(std::string const& method, factory f) { return lookup_or_insert(method, "[]", f); }

        /// @brief Insert the value unconditionally for the given request.
        auto refresh(std::string const& method, std::string_view params, std::string_view value)
        {
            insert(method, params, value);
        }
    };
} // namespace caching
