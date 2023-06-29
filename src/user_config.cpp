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
        auto expirySeconds = config.get_nts_expiry();

        user_info_obj["name"] = toJs(env, name);
        user_info_obj["priority"] = toJs(env, priority);

        if (expirySeconds) {
            user_info_obj["expirySeconds"] = toJs(env, expirySeconds->count());
        } else {
            user_info_obj["expirySeconds"] = env.Null();
        }

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

void UserConfigWrapper::setUserInfo(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 4);

        auto name = info[0];
        auto priority = info[1];
        auto profile_pic_obj = info[2];
        auto expirySeconds = info[3];

        assertIsStringOrNull(name);
        assertIsNumber(priority);
        assertIsNumber(expirySeconds);
        std::string new_name;

        if (name.IsString())
            new_name = name.As<Napi::String>().Utf8Value();

        config.set_name(new_name);

        auto new_priority = toPriority(priority, config.get_nts_priority());
        config.set_nts_priority(new_priority);

        auto expiryCppSeconds = toCppInteger(expirySeconds, "set_nts_expiry", false);
        config.set_nts_expiry(std::chrono::seconds{expiryCppSeconds});

        if (!profile_pic_obj.IsNull() && !profile_pic_obj.IsUndefined())
            assertIsObject(profile_pic_obj);

        config.set_profile_pic(profile_pic_from_object(profile_pic_obj));
    });
}

}  // namespace session::nodeapi
