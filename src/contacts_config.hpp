#pragma once

#include <napi.h>

#include "base_config.hpp"
#include "session/config/contacts.hpp"

namespace session::nodeapi {

class ContactsConfigWrapper : public ConfigBaseImpl,
                              public Napi::ObjectWrap<ContactsConfigWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit ContactsConfigWrapper(const Napi::CallbackInfo& info);

  private:
    config::Contacts& config{get_config<config::Contacts>()};

    Napi::Value get(const Napi::CallbackInfo& info);
    Napi::Value getAll(const Napi::CallbackInfo& info);
    void set(const Napi::CallbackInfo& info);
    Napi::Value erase(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
