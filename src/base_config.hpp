#pragma once

#include <napi.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <unordered_set>

#include "session/config/base.hpp"
#include "session/types.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

class ConfigBaseImpl;
template <typename T>
inline constexpr bool is_derived_napi_wrapper =
        std::is_base_of_v<ConfigBaseImpl, T>&& std::is_base_of_v<Napi::ObjectWrap<T>, T>;

/// Base implementation class for config types; this provides the napi wrappers for the base
/// methods.  Subclasses should inherit from this (alongside Napi::ObjectWrap<ConfigBaseWrapper>)
/// and wrap their method list argument in `DefineClass` with a call to
/// `ConfigBaseImpl::WithBaseMethods<Subtype>({...})` to have the base methods added to the derived
/// type appropriately.
class ConfigBaseImpl {

    std::shared_ptr<config::ConfigBase> conf_;

  public:
    // These are exposed as read-only accessors rather than methods:
    Napi::Value needsDump(const Napi::CallbackInfo& info);
    Napi::Value needsPush(const Napi::CallbackInfo& info);
    Napi::Value storageNamespace(const Napi::CallbackInfo& info);

    Napi::Value push(const Napi::CallbackInfo& info);
    Napi::Value dump(const Napi::CallbackInfo& info);
    void confirmPushed(const Napi::CallbackInfo& info);
    Napi::Value merge(const Napi::CallbackInfo& info);

    // Called from a sub-type's Init function (typically indirectly, via InitHelper) to add the base
    // class properties/methods to the type.
    template <typename T, std::enable_if_t<is_derived_napi_wrapper<T>, int> = 0>
    static std::vector<typename T::PropertyDescriptor> WithBaseMethods(
            std::vector<typename T::PropertyDescriptor> properties) {
        std::unordered_set<std::string_view> seen;
        for (const napi_property_descriptor& prop : properties)
            if (prop.utf8name)
                seen.emplace(prop.utf8name);

        properties.push_back(T::InstanceAccessor("needsDump", &T::needsDump, nullptr));
        properties.push_back(T::InstanceAccessor("needsPush", &T::needsPush, nullptr));
        properties.push_back(
                T::InstanceAccessor("storageNamespace", &T::storageNamespace, nullptr));

        properties.push_back(T::InstanceMethod("push", &T::push));
        properties.push_back(T::InstanceMethod("dump", &T::dump));
        properties.push_back(T::InstanceMethod("confirmPushed", &T::confirmPushed));
        properties.push_back(T::InstanceMethod("merge", &T::merge));

        return properties;
    }

  protected:
    // Constructor (callable from a subclass): the wrapper subclass constructs its
    // ConfigBase-derived shared_ptr during *its* construction, passing it here.  For example:
    //
    //     ConfigWhateverWrapper(const Napi::CallbackInfo& info) :
    //         ConfigBaseImpl{construct<config::Whatever>(info), "Whatever"},
    //         Napi::ObjectWrap<UserWhateverWrapper>{info} {}
    ConfigBaseImpl(std::shared_ptr<session::config::ConfigBase> conf) : conf_{std::move(conf)} {
        if (!conf_)
            throw std::invalid_argument{
                    "ConfigBaseImpl initialization requires a live ConfigBase pointer"};
    }

    // Constructs a shared_ptr of some config::ConfigBase-derived type, taking a secret key and
    // optional dump.  This is what most Config types require, but a subclass could replace this if
    // it needs to do something else.
    template <
            typename Config,
            std::enable_if_t<std::is_base_of_v<config::ConfigBase, Config>, int> = 0>
    static std::shared_ptr<Config> construct(
            const Napi::CallbackInfo& info, const std::string& class_name) {
        return wrapExceptions(info, [&] {
            if (!info.IsConstructCall())
                throw std::invalid_argument{
                        "You need to call the constructor with the `new` syntax"};

            assertInfoLength(info, 2);

            // we should get secret key as first arg and optional dumped as second argument
            assertIsUInt8Array(info[0]);
            assertIsUInt8ArrayOrNull(info[1]);
            ustring_view secretKey = toCppBufferView(info[0], class_name + ".new");

            std::optional<ustring_view> dump;
            auto second = info[1];
            if (!second.IsEmpty() && !second.IsNull() && !second.IsUndefined())
                dump = toCppBufferView(second, class_name + ".new");

            return std::make_shared<Config>(secretKey, dump);
        });
    }

    virtual ~ConfigBaseImpl() = default;

    // Accesses a reference the stored config instance as `std::shared_ptr<T>` (if no template is
    // specified then as the base ConfigBase type).  `T` must be a subclass of ConfigBase for this
    // to compile.  Throws std::logic_error if not set.  Throws std::invalid_argument if the
    // instance is not castable to a `T`.
    template <typename T, std::enable_if_t<std::is_base_of_v<config::ConfigBase, T>, int> = 0>
    T& get_config() {
        assert(conf_);  // should not be possible to construct without this set
        if (auto* t = dynamic_cast<T*>(conf_.get()))
            return *t;
        throw std::invalid_argument{
                "Error retrieving config: config instance is not of the requested type"};
    }

    // Same as above, but return a shared ptr.
    template <
            typename T = config::ConfigBase,
            std::enable_if_t<std::is_base_of_v<config::ConfigBase, T>, int> = 0>
    std::shared_ptr<T> config_ptr() {
        assert(conf_);  // should not be possible to construct without this set
        if (auto t = std::dynamic_pointer_cast<T>(conf_))
            return t;
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
