#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_profile.hpp"

using session::ustring_view;

class UserConfigWrapperInsideWorker : public ConfigBaseWrapperInsideWorker {
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit UserConfigWrapperInsideWorker(ustring_view ed25519_secretkey,
                                         std::optional<ustring_view> dumped) {
    tryOrWrapStdException([&]() {
      initWithConfig(
          new session::config::UserProfile(ed25519_secretkey, dumped));
    });
  }

  static NAN_METHOD(New);
  static NAN_METHOD(GetName);
  static NAN_METHOD(SetName);
  static NAN_METHOD(GetProfilePicture);
  static NAN_METHOD(SetProfilePicture);
};
