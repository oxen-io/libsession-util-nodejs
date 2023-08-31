#pragma once

#include <napi.h>
#include <oxenc/hex.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_set>

#include "session/config/base.hpp"
#include "session/types.hpp"
#include "utilities.hpp"

using ustring_view = std::basic_string_view<unsigned char>;

namespace session::nodeapi {

class ConfigSigImpl;
template <typename T>
inline constexpr bool is_derived_napi_wrapper =
        std::is_base_of_v<ConfigSigImpl, T>&& std::is_base_of_v<Napi::ObjectWrap<T>, T>;

/// Base implementation class for config types; this provides the napi wrappers for the base
/// methods.  Subclasses should inherit from this (alongside Napi::ObjectWrap<ConfigBaseWrapper>)
/// and wrap their method list argument in `DefineClass` with a call to
/// `ConfigSigImpl::WithBaseMethods<Subtype>({...})` to have the base methods added to the derived
/// type appropriately.
class ConfigSigImpl {
    std::shared_ptr<config::ConfigSig> conf_;

  public:
    // Called from a sub-type's Init function (typically indirectly, via InitHelper) to add the base
    // class properties/methods to the type.
    template <typename T, std::enable_if_t<is_derived_napi_wrapper<T>, int> = 0>
    static std::vector<typename T::PropertyDescriptor> WithBaseMethods(
            std::vector<typename T::PropertyDescriptor> properties) {
        std::unordered_set<std::string_view> seen;
        for (const napi_property_descriptor& prop : properties)
            if (prop.utf8name)
                seen.emplace(prop.utf8name);

        return properties;
    }

  protected:
    // Constructor (callable from a subclass): the wrapper subclass constructs its
    // ConfigSig-derived shared_ptr during *its* construction, passing it here.
    ConfigSigImpl(std::shared_ptr<config::ConfigSig> conf) : conf_{std::move(conf)} {
        if (!conf_)
            throw std::invalid_argument{
                    "ConfigSigImpl initialization requires a live ConfigSig pointer"};
    }

    // Constructs a shared_ptr of some config::ConfigBase-derived type, taking a secret key and
    // optional dump.  This is what most Config types require, but a subclass could replace this if
    // it needs to do something else.
    template <
            typename Config,
            std::enable_if_t<std::is_base_of_v<config::ConfigBase, Config>, int> = 0>
    static std::shared_ptr<Config> constructFromKeys(
            const Napi::CallbackInfo& info, const std::string& class_name) {
        return wrapExceptions(info, [&] {
            if (!info.IsConstructCall())
                throw std::invalid_argument{
                        "You need to call the constructor with the `new` syntax"};

            assertInfoLength(info, 6);

            // types come from constructor of Keys::Keys
            assertIsUInt8Array(info[0]);        // user_ed25519_secretkey
            assertIsUInt8Array(info[1]);        // group_ed25519_pubkey
            assertIsUInt8ArrayOrNull(info[2]);  // group_ed25519_secretkey
            assertIsUInt8ArrayOrNull(info[3]);  // dumped
            assertIsUInt8ArrayOrNull(info[4]);  // dumped
            assertIsUInt8ArrayOrNull(info[5]);  // dumped

            std::string ed25519_pubkey_str = toCppString(info[0], class_name + ".new.pubkey");
            std::optional<ustring> secret_key;
            auto second = info[1];
            if (!second.IsEmpty() && !second.IsNull() && !second.IsUndefined())
                secret_key = toCppBufferView(second, class_name + ".new.secret");

            std::optional<ustring> dump;
            auto third = info[2];
            if (!third.IsEmpty() && !third.IsNull() && !third.IsUndefined())
                dump = toCppBufferView(third, class_name + ".new.dump");

            auto withoutPrefix = ed25519_pubkey_str.substr(2);
            ustring ed25519_pubkey = (const unsigned char*)oxenc::from_hex(withoutPrefix).c_str();
            return std::make_shared<Config>(ed25519_pubkey, secret_key, dump);
        });
    }

    virtual ~ConfigSigImpl() = default;

    // Accesses a reference the stored config instance as `std::shared_ptr<T>` (if no template is
    // specified then as the base ConfigSig type).  `T` must be a subclass of ConfigSig for this
    // to compile.  Throws std::logic_error if not set.  Throws std::invalid_argument if the
    // instance is not castable to a `T`.
    template <typename T, std::enable_if_t<std::is_base_of_v<config::ConfigSig, T>, int> = 0>
    T& get_config() {
        assert(conf_);  // should not be possible to construct without this set
        if (auto* t = dynamic_cast<T*>(conf_.get()))
            return *t;
        throw std::invalid_argument{
                "Error retrieving config: config instance is not of the requested type"};
    }

    // Helper function for doing the subtype napi Init call.  This sets up the class registration,
    // sets it in the exports, and appends the base methods and properties (needsDump, etc.) to the
    // given methods/properties list.
    template <typename T, std::enable_if_t<is_derived_napi_wrapper<T>, int> = 0>
    static void InitHelper(
            Napi::Env env,
            Napi::Object exports,
            const char* class_name,
            std::vector<typename T::PropertyDescriptor> properties) {

        Napi::Function cls =
                T::DefineClass(env, class_name, WithBaseMethods<T>(std::move(properties)));

        auto ref = std::make_unique<Napi::FunctionReference>();
        *ref = Napi::Persistent(cls);
        env.SetInstanceData(ref.release());

        exports.Set(class_name, cls);
    }
};

}  // namespace session::nodeapi
