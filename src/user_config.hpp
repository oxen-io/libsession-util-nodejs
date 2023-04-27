#pragma once

#include <napi.h>

#include "base_config.hpp"
#include "session/config/user_profile.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

class UserConfigWrapper : public ConfigBaseImpl, public Napi::ObjectWrap<UserConfigWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit UserConfigWrapper(const Napi::CallbackInfo& info);

  private:
    config::UserProfile& config{get_config<config::UserProfile>()};

    // This is exported via the `name` accessor rather than as a method:
    Napi::Value getName(const Napi::CallbackInfo& info);
    void setName(const Napi::CallbackInfo& info, const Napi::Value& name);

    // get/setProfilePicture -- returns/takes an object with "url" (string or null) and "key"
    // (buffer or null) value.  We *don't* expose this as an accessor because that might imply that
    // `conf.profilePicture.url = newUrl;` would work, but it won't: you have to get the object,
    // change key/url in it, then feed it back in via `setPictureProfile`.
    Napi::Value getProfilePicture(const Napi::CallbackInfo& info);
    void setProfilePicture(const Napi::CallbackInfo& info);

    // Accessors: .profilePictureUrl and .profilePicture
};

}  // namespace session::nodeapi
