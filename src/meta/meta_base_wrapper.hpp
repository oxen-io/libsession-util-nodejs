#pragma once

#include <napi.h>

#include "../base_config.hpp"
#include "../groups/meta_group.hpp"

namespace session::nodeapi {

class MetaBaseWrapper {

  public:
    explicit MetaBaseWrapper();

    virtual ~MetaBaseWrapper() = default;

    template <typename T, std::enable_if_t<is_derived_napi_wrapper<T>, int> = 0>
    static void NoBaseClassInitHelper(
            Napi::Env env,
            Napi::Object exports,
            const char* class_name,
            std::vector<typename T::PropertyDescriptor> properties) {

        // not adding the baseMethods here from withBaseMethods()
        Napi::Function cls = T::DefineClass(env, class_name, std::move(properties));

        auto ref = std::make_unique<Napi::FunctionReference>();
        *ref = Napi::Persistent(cls);
        env.SetInstanceData(ref.release());

        exports.Set(class_name, cls);
    }

    static std::shared_ptr<session::nodeapi::MetaGroup> constructGroupWrapper(
            const Napi::CallbackInfo& info, const std::string& class_name) {
        return wrapExceptions(info, [&] {
            if (!info.IsConstructCall())
                throw std::invalid_argument{
                        "You need to call the constructor with the `new` syntax"};

            assertInfoLength(info, 1);
            auto arg = info[0];
            assertIsObject(arg);
            auto obj = arg.As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("constructGroupWrapper received empty");

            assertIsUInt8Array(obj.Get("userEd25519Secretkey"));
            auto user_ed25519_secretkey = toCppBuffer(
                    obj.Get("userEd25519Secretkey"),
                    class_name + ":constructGroupWrapper.userEd25519Secretkey");

            assertIsUInt8Array(obj.Get("groupEd25519Pubkey"));
            auto group_ed25519_pubkey = toCppBuffer(
                    obj.Get("groupEd25519Pubkey"),
                    class_name + ":constructGroupWrapper.groupEd25519Pubkey");

            std::optional<ustring> group_ed25519_secretkey = maybeNonemptyBuffer(
                    obj.Get("groupEd25519Secretkey"),
                    class_name + ":constructGroupWrapper.groupEd25519Secretkey");

            std::optional<ustring> dumped_info = maybeNonemptyBuffer(
                    obj.Get("dumpedInfo"), class_name + ":constructGroupWrapper.dumpedInfo");
            std::optional<ustring> dumped_members = maybeNonemptyBuffer(
                    obj.Get("dumpedMembers"), class_name + ":constructGroupWrapper.dumpedMembers");
            std::optional<ustring> dumped_keys = maybeNonemptyBuffer(
                    obj.Get("dumpedKeys"), class_name + ":constructGroupWrapper.dumpedKeys");

            auto info = std::make_shared<config::groups::Info>(
                    group_ed25519_pubkey, group_ed25519_secretkey, dumped_info);

            auto members = std::make_shared<config::groups::Members>(
                    group_ed25519_pubkey, group_ed25519_secretkey, dumped_members);

            auto keys = std::make_shared<config::groups::Keys>(
                    user_ed25519_secretkey,
                    group_ed25519_pubkey,
                    group_ed25519_secretkey,
                    dumped_keys,
                    *info,
                    *members);

            return std::make_shared<session::nodeapi::MetaGroup>(info, members, keys);
        });
    }
};

}  // namespace session::nodeapi
