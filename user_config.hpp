#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_profile.hpp"

class UserConfigWrapper : public ConfigBaseWrapper
{
public:
  static NAN_MODULE_INIT(Init);

private:
  // session::config::UserProfile *userProfile;

  explicit UserConfigWrapper()
  {
    initWithConfig(new session::config::UserProfile());
  }

  ~UserConfigWrapper()
  {
  }

  static session::config::UserProfile *toUserProfile(const Nan::FunctionCallbackInfo<v8::Value> &info);

  static NAN_METHOD(New);
  static NAN_METHOD(GetName);
  static NAN_METHOD(SetName);
};

NODE_MODULE(session_util_wrapper, UserConfigWrapper::Init)
