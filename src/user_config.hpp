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

    Napi::Value getUserInfo(const Napi::CallbackInfo& info);
    void setUserInfo(const Napi::CallbackInfo& info);

    Napi::Value getEnableBlindedMsgRequest(const Napi::CallbackInfo& info);
    void setEnableBlindedMsgRequest(const Napi::CallbackInfo& info);

    Napi::Value getExpiry(const Napi::CallbackInfo& info);
    void setExpiry(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
