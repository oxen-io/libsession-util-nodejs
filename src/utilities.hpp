#pragma once

#include <napi.h>

#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "session/config/namespaces.hpp"
#include "session/types.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

using namespace std::literals;

// template <typename T>
// T object_wrap_impl(const Napi::ObjectWrap<T>&);
// template <typename T>
// using object_wrap_t = decltype(object_wrap_impl(std::declval<const T&>()));

void assertInfoLength(const Napi::CallbackInfo& info, const int expected);

void assertInfoMinLength(const Napi::CallbackInfo& info, const int minLength);

void assertIsStringOrNull(const Napi::Value& value);
void assertIsNumber(const Napi::Value& value);
void assertIsArray(const Napi::Value& value);
void assertIsObject(const Napi::Value& value);
void assertIsUInt8ArrayOrNull(const Napi::Value& value);
void assertIsUInt8Array(const Napi::Value& value);
void assertIsString(const Napi::Value& value);
void assertIsBoolean(const Napi::Value& value);

// Checks for and returns exactly N string arguments.  If N == 1 this return just a string; if > 1
// this returns an std::array of strings of size N.
template <size_t N>
auto getStringArgs(const Napi::CallbackInfo& info) {
    assertInfoLength(info, N);
    std::array<std::string, N> args;
    for (int i = 0; i < args.size(); i++) {
        auto arg = info[i];
        assertIsString(arg);
        args[i] = arg.As<Napi::String>().Utf8Value();
    }
    if constexpr (N == 1)
        return std::move(args[0]);
    else
        return args;
}

std::string toCppString(Napi::Value x, const std::string& identifier);
ustring toCppBuffer(Napi::Value x, const std::string& identifier);
ustring_view toCppBufferView(Napi::Value x, const std::string& identifier);
int64_t toCppInteger(Napi::Value x, const std::string& identifier, bool allowUndefined = false);
std::optional<int64_t> maybeNonemptyInt(Napi::Value x, const std::string& identifier);

bool toCppBoolean(Napi::Value x, const std::string& identifier);

// If the object is null/undef/empty returns nullopt, otherwise if a String returns a std::string of
// the value.  Throws if something else.
std::optional<std::string> maybeNonemptyString(Napi::Value x, const std::string& identifier);

// If the object is null/undef/empty returns nullopt, otherwise if a Uint8Array returns a ustring of
// the value.  Throws if something else.
std::optional<ustring> maybeNonemptyBuffer(Napi::Value x, const std::string& identifier);

// Implementation struct of toJs(); we add specializations of this for any C++ types we want to be
// able to convert into JS types.
template <typename T, typename SFINAE = void>
struct toJs_impl {
    // If this gets instantiated it means we're missing a specialization and so fail to compile:
    static_assert(!std::is_same_v<T, T>, "toJs() does not have an implementation for type T");
};

// Converts C++ values of various types to a Napi value; in particular if you just include this
// header then you get:
//
// - bool -> Boolean
// - other arithmetic types -> Number
// - string, string_view -> String
// - ustring, ustring_view -> Buffer
// - std::vector<T> -> Array, where elements are created via toJs calls on the vector elements.
// - std::optional<T> -> Null if empty, otherwise the result of toJs on the contained value
// - Napi::Value (or derived) -> itself (this is mainly so that you can return a std::vector or
//   std::optional of a Napi::Value and get the Right Thing).
//
// but others can be added in other headers by adding additional specializations of toJs_impl.
template <typename T>
auto toJs(const Napi::Env& env, const T& val) {
    return toJs_impl<T>{}(env, val);
}

template <>
struct toJs_impl<bool> {
    auto operator()(const Napi::Env& env, bool b) const { return Napi::Boolean::New(env, b); }
};

template <>
struct toJs_impl<session::config::Namespace> {
    auto operator()(const Napi::Env& env, session::config::Namespace b) const {
        return Napi::Number::New(env, static_cast<int16_t>(b));
    }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    auto operator()(const Napi::Env& env, T n) const { return Napi::Number::New(env, n); }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_convertible_v<T, std::string_view>>> {
    auto operator()(const Napi::Env& env, std::string_view s) const {
        return Napi::String::New(env, s.data(), s.size());
    }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_convertible_v<T, ustring_view>>> {
    auto operator()(const Napi::Env& env, ustring_view b) const {
        return Napi::Buffer<uint8_t>::Copy(env, b.data(), b.size());
    }
};

template <typename T>
struct toJs_impl<T, std::enable_if_t<std::is_base_of_v<Napi::Value, T>>> {
    auto operator()(const Napi::Env& env, const T& val) { return val; }
};

template <typename T>
struct toJs_impl<std::vector<T>> {
    auto operator()(const Napi::Env& env, const std::vector<T>& val) {
        auto arr = Napi::Array::New(env, val.size());
        for (size_t i = 0; i < val.size(); i++)
            arr[i] = toJs(env, val[i]);
        return arr;
    }
};

template <typename T>
struct toJs_impl<std::unordered_set<T>> {
    auto operator()(const Napi::Env& env, const std::unordered_set<T>& set) {
        std::vector<T> as_array(set.begin(), set.end());

        auto arr = Napi::Array::New(env, as_array.size());
        for (size_t i = 0; i < as_array.size(); i++)
            arr[i] = toJs(env, as_array[i]);

        return arr;
    }
};

template <typename T>
struct toJs_impl<std::optional<T>> {
    Napi::Value operator()(const Napi::Env& env, const std::optional<T>& val) {
        if (val)
            return toJs(env, *val);
        return env.Null();
    }
};

// Helper for various "get_all" functions that copy [it...end) into a Napi::Array via toJs().
// Throws a Napi::Error on any exception.
template <typename It, typename EndIt>
static Napi::Array get_all_impl(const Napi::CallbackInfo& info, size_t size, It it, EndIt end) {
    auto env = info.Env();
    return wrapResult(env, [&] {
        assertInfoLength(info, 0);
        auto result = Napi::Array::New(env, size);
        int i = 0;
        for (; it != end; it++)
            result[i++] = toJs(env, *it);

        return result;
    });
}

// Wraps a string in an optional<string_view> which will be nullopt if the input string is empty.
// This is particularly useful with `toJs` to convert empty strings into Null.
inline std::optional<std::string_view> maybe_string(std::string_view val) {
    if (val.empty())
        return std::nullopt;
    return val;
}

// Calls the given callable and wraps it suitable for an Napi call:
// - Napi::Error exceptions will be uncaught
// - Other std::exceptions will be caught, converted to Napi::Errors, and rethrown
// - The return value will be returned as-is if already a Napi::Value (or subtype)
// - The return will be void if void
// - Otherwise the return value will be passed through toJs() to convert it to a Napi::Value.  See
//   toJs below, but generally this supports numeric types, bools, strings, ustrings, and vectors of
//   any of those.
//
// General use is:
//
//     return wrapResult(env, [&] { return foo(); });
//
template <typename Call>
auto wrapResult(const Napi::Env& env, Call&& call) {
    using Result = decltype(call());
    try {
        if constexpr (std::is_void_v<Result>) {
            call();
        } else {
            auto res = call();
            if constexpr (std::is_base_of_v<Napi::Value, Result>)
                return res;
            else
                return toJs(env, std::move(res));
        }
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}

// Same as above, but a small shortcut to allow passing `info` instead of `info.Env()` as the first
// argument.
template <typename Call>
auto wrapResult(const Napi::CallbackInfo& info, Call&& call) {
    return wrapResult(info.Env(), std::forward<Call>(call));
}

// Similar to wrapResult, above, but this only applies the exception wrapping (i.e. no wrapping of
// the result: we return it exactly as-is).
template <typename Call>
auto wrapExceptions(const Napi::Env& env, Call&& call) {
    try {
        return call();
    } catch (const std::exception& e) {
        throw Napi::Error::New(env, e.what());
    }
}
template <typename Call>
auto wrapExceptions(const Napi::CallbackInfo& info, Call&& call) {
    return wrapExceptions(info.Env(), std::forward<Call>(call));
}

std::string printable(std::string_view x);
std::string printable(const char* x) = delete;
std::string printable(const char* x, size_t n);
std::string printable(ustring_view x);

/**
 * Keep the current priority if a wrapper
 */
int64_t toPriority(Napi::Value x, int64_t currentPriority);

int64_t unix_timestamp_now();

using push_entry_t = std::tuple<
        session::config::seqno_t,
        session::ustring,
        std::vector<std::string, std::allocator<std::string>>>;

Napi::Object push_result_to_JS(
        const Napi::Env& env,
        const push_entry_t& push_entry,
        const session::config::Namespace& push_namespace);

Napi::Object push_key_entry_to_JS(
        const Napi::Env& env,
        const session::ustring_view& key_data,
        const session::config::Namespace& push_namespace);

Napi::Object decrypt_result_to_JS(
        const Napi::Env& env, const std::pair<std::string, ustring> decrypted);

}  // namespace session::nodeapi
