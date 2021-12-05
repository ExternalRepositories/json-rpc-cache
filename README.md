# json-rpc-cache

Header only implementation for simple in-memory caching of json-rpc requests in a single class `cache`.

This is just the definition of the caching container, using an `unordered_map` internally.
Periodic refreshing must be triggered externally; response values to be cached for individual requests
must also be provided externally.

This class is simply concerned with storage and normalization of json encoded request and response strings.

#### Usage

Various tests are defined in test/cache.cpp, which show the usage of the class `caching::cache`.
