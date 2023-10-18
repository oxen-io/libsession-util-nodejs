#pragma once

#include <napi.h>

#include "../base_config.hpp"
#include "../groups/meta_group.hpp"

namespace session::nodeapi {

class MetaBaseWrapper {

  public:
    explicit MetaBaseWrapper(){};

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

    static std::unique_ptr<session::nodeapi::MetaGroup> constructGroupWrapper(
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

            std::optional<ustring> dumped_meta = maybeNonemptyBuffer(
                    obj.Get("metaDumped"), class_name + ":constructGroupWrapper.metaDumped");

            std::optional<ustring_view> dumped_info;
            std::optional<ustring_view> dumped_members;
            std::optional<ustring_view> dumped_keys;

            if (dumped_meta) {
                auto dumped_meta_str = from_unsigned_sv(*dumped_meta);

                oxenc::bt_dict_consumer combined{dumped_meta_str};
                // NB: must read in ascii-sorted order:
                if (!combined.skip_until("info"))
                    throw std::runtime_error{"info dump not found in combined dump!"};
                dumped_info = session::to_unsigned_sv(combined.consume_string_view());

                if (!combined.skip_until("keys"))
                    throw std::runtime_error{"keys dump not found in combined dump!"};
                dumped_keys = session::to_unsigned_sv(combined.consume_string_view());

                if (!combined.skip_until("members"))
                    throw std::runtime_error{"members dump not found in combined dump!"};
                dumped_members = session::to_unsigned_sv(combined.consume_string_view());
            }

            // Note, we keep shared_ptr for those as the Keys one need a reference to Members and
            // Info on its own currently.
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

            return std::make_unique<session::nodeapi::MetaGroup>(info, members, keys);
        });
    }
};

}  // namespace session::nodeapi
