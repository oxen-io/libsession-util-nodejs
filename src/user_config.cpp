#include "user_config.hpp"

#include "base_config.hpp"
#include "profile_pic.hpp"
#include "session/config/user_profile.hpp"

namespace session::nodeapi {

using config::UserProfile;

void UserConfigWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<UserConfigWrapper>(
            env,
            exports,
            "UserConfigWrapperNode",
            {
                    InstanceMethod("getUserInfo", &UserConfigWrapper::getUserInfo),
                    InstanceMethod("setUserInfo", &UserConfigWrapper::setUserInfo),
                    InstanceMethod(
                            "getEnableBlindedMsgRequest",
                            &UserConfigWrapper::getEnableBlindedMsgRequest),
                    InstanceMethod(
                            "setEnableBlindedMsgRequest",
                            &UserConfigWrapper::setEnableBlindedMsgRequest),
                    InstanceMethod("getNoteToSelfExpiry", &UserConfigWrapper::getNoteToSelfExpiry),
                    InstanceMethod("setNoteToSelfExpiry", &UserConfigWrapper::setNoteToSelfExpiry),
            });
}

UserConfigWrapper::UserConfigWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<config::UserProfile>(info, "UserConfig")},
        Napi::ObjectWrap<UserConfigWrapper>{info} {}

Napi::Value UserConfigWrapper::getUserInfo(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto user_info_obj = Napi::Object::New(env);

        auto name = config.get_name();
        auto priority = config.get_nts_priority();

        user_info_obj["name"] = toJs(env, name);
        user_info_obj["priority"] = toJs(env, priority);

        auto profile_pic_obj = object_from_profile_pic(env, config.get_profile_pic());
        if (profile_pic_obj) {
            user_info_obj["url"] = profile_pic_obj.Get("url");
            user_info_obj["key"] = profile_pic_obj.Get("key");
        } else {
            user_info_obj["url"] = env.Null();
            user_info_obj["key"] = env.Null();
        }

        return user_info_obj;
    });
}

Napi::Value UserConfigWrapper::setUserInfo(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 3);

        auto name = info[0];
        auto priority = info[1];
        auto profile_pic_obj = info[2];

        assertIsStringOrNull(name);
        assertIsNumber(priority);
        std::string new_name;

        if (name.IsString())
            new_name = name.As<Napi::String>().Utf8Value();

        config.set_name_truncated(new_name);

        auto new_priority = toPriority(priority, config.get_nts_priority());
        config.set_nts_priority(new_priority);

        if (!profile_pic_obj.IsNull() && !profile_pic_obj.IsUndefined())
            assertIsObject(profile_pic_obj);

        config.set_profile_pic(profile_pic_from_object(profile_pic_obj));

        return config.get_name();
    });
}

Napi::Value UserConfigWrapper::getEnableBlindedMsgRequest(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto blindedMsgRequest = toJs(env, config.get_blinded_msgreqs());

        return blindedMsgRequest;
    });
}

void UserConfigWrapper::setEnableBlindedMsgRequest(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto blindedMsgRequests = info[0];
        assertIsBoolean(blindedMsgRequests);

        auto blindedMsgReqCpp = toCppBoolean(blindedMsgRequests, "set_blinded_msgreqs");
        config.set_blinded_msgreqs(blindedMsgReqCpp);
    });
}

Napi::Value UserConfigWrapper::getNoteToSelfExpiry(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto nts_expiry = config.get_nts_expiry();
        if (nts_expiry) {
            return nts_expiry->count();
        }

        return static_cast<int64_t>(0);
    });
}

void UserConfigWrapper::setNoteToSelfExpiry(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto expirySeconds = info[0];
        assertIsNumber(expirySeconds);

        auto expiryCppSeconds = toCppInteger(expirySeconds, "set_nts_expiry", false);
        config.set_nts_expiry(std::chrono::seconds{expiryCppSeconds});
    });
}

}  // namespace session::nodeapi
