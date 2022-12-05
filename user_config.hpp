#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_profile.hpp"

class UserConfigWrapper : public ConfigBaseWrapper
{
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit UserConfigWrapper()
  {
    initWithConfig(new session::config::UserProfile());
  }

  static NAN_METHOD(New);
  static NAN_METHOD(GetName);
  static NAN_METHOD(SetName);
  static NAN_METHOD(GetProfilePic);
  static NAN_METHOD(SetProfilePic);
};

NODE_MODULE(session_util_wrapper, UserConfigWrapper::Init)
