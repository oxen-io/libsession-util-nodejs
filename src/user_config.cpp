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
            "UserConfig",
            {
                    InstanceAccessor(
                            "name", &UserConfigWrapper::getName, &UserConfigWrapper::setName),
                    InstanceMethod("getProfilePicture", &UserConfigWrapper::getProfilePicture),
                    InstanceMethod("setProfilePicture", &UserConfigWrapper::setProfilePicture),
            });
}

UserConfigWrapper::UserConfigWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<config::UserProfile>(info, "UserConfig")},
        Napi::ObjectWrap<UserConfigWrapper>{info} {}

Napi::Value UserConfigWrapper::getName(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_name(); });
}

void UserConfigWrapper::setName(const Napi::CallbackInfo& info, const Napi::Value& name) {
    return wrapResult(info, [&] {
        assertIsStringOrNull(name);
        std::string new_name;
        if (name.IsString())
            new_name = name.As<Napi::String>().Utf8Value();

        config.set_name(new_name);
    });
}

Napi::Value UserConfigWrapper::getProfilePicture(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    return wrapResult(env, [&] { return object_from_profile_pic(env, config.get_profile_pic()); });
}

void UserConfigWrapper::setProfilePicture(const Napi::CallbackInfo& info) {
    wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        if (!arg.IsNull())
            assertIsObject(arg);

        config.set_profile_pic(profile_pic_from_object(arg));
    });
}

}  // namespace session::nodeapi
