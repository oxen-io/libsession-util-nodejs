#include "group_keys_config.hpp"

#include <optional>

#include "../base_config.hpp"
#include "../profile_pic.hpp"
#include "session/config/groups/keys.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

static InitHelper(Napi::Env env, Napi::Object exports) {
    Napi::Function cls = T::DefineClass(env, class_name, WithBaseMethods<T>(std::move(properties)));

    auto ref = std::make_unique<Napi::FunctionReference>();
    *ref = Napi::Persistent(cls);
    env.SetInstanceData(ref.release());

    exports.Set(class_name, cls);
}

using config::groups::Keys;

void GroupKeysWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<GroupKeysWrapper>(
            env,
            exports,
            "GroupKeysWrapperNode",
            {
                    // group related methods
                    InstanceMethod("get", &GroupKeysWrapper::get),
                    // InstanceMethod("getOrConstruct", &GroupKeysWrapper::getOrConstruct),
                    // InstanceMethod("getAll", &GroupKeysWrapper::getAll),
                    // InstanceMethod("setName", &GroupKeysWrapper::setName),

            });
}

GroupKeysWrapper::GroupKeysWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<GroupKeysWrapper>{info} {
    return wrapExceptions(info, [&] {
        if (!info.IsConstructCall())
            throw std::invalid_argument{"You need to call the constructor with the `new` syntax"};
        auto class_name = "GroupKeys"s;
        assertInfoLength(info, 3);

        // we should get ed25519_pubkey as string (with 03 prefix), as first arg, secret key as
        // second opt arg and optional dumped as third arg
        assertIsString(info[0]);
        assertIsUInt8ArrayOrNull(info[1]);
        assertIsUInt8ArrayOrNull(info[2]);
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
        this->config = std::make_shared<Keys>(ed25519_pubkey, secret_key, dump);
    });
}

/**
 * =================================================
 * ==================== GETTERS ====================
 * =================================================
 */

// Napi::Value GroupKeysWrapper::get(const Napi::CallbackInfo& info) {
//     return wrapResult(info, [&] {
//         auto env = info.Env();
//         assertInfoLength(info, 1);
//         assertIsString(info[0]);

//         auto pubkeyHex = toCppString(info[0], "GroupKeysWrapper::get");

//         return config.get(pubkeyHex);
//     });
// }

// Napi::Value GroupKeysWrapper::getOrConstruct(const Napi::CallbackInfo& info) {
//     return wrapResult(info, [&] {
//         auto env = info.Env();
//         assertInfoLength(info, 1);
//         assertIsString(info[0]);

//         auto pubkeyHex = toCppString(info[0], "GroupKeysWrapper::getOrConstruct");

//         return config.get_or_construct(pubkeyHex);
//     });
// }
}  // namespace session::nodeapi
