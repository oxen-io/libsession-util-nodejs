#pragma once

#include <napi.h>

#include "../base_config.hpp"
#include "session/config/groups/info.hpp"

namespace session::nodeapi {

class GroupInfoWrapper : public ConfigBaseImpl, public Napi::ObjectWrap<GroupInfoWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit GroupInfoWrapper(const Napi::CallbackInfo& info);

  private:
    session::config::groups::Info& config{get_config<session::config::groups::Info>()};

    // GroupInfo related methods
    Napi::Value getInfo(const Napi::CallbackInfo& info);
    Napi::Value setInfo(const Napi::CallbackInfo& info);
    Napi::Value destroy(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
