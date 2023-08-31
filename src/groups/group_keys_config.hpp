#pragma once

#include <napi.h>

#include "../base_config.hpp"
#include "session/config/groups/keys.hpp"

namespace session::nodeapi {

class GroupKeysWrapper : public Napi::ObjectWrap<GroupKeysWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit GroupKeysWrapper(const Napi::CallbackInfo& info);

  private:
    std::shared_ptr<session::config::groups::Keys> config;
    // GroupKeys related methods
    Napi::Value get(const Napi::CallbackInfo& info);
    // Napi::Value getOrConstruct(const Napi::CallbackInfo& info);
    // Napi::Value getAll(const Napi::CallbackInfo& info);

    // // setters
    // Napi::Value setName(const Napi::CallbackInfo& info);
    // Napi::Value setInvited(const Napi::CallbackInfo& info);
    // Napi::Value setAccepted(const Napi::CallbackInfo& info);
    // Napi::Value setPromoted(const Napi::CallbackInfo& info);
    // Napi::Value setProfilePicture(const Napi::CallbackInfo& info);

    // // eraser
    // Napi::Value erase(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
