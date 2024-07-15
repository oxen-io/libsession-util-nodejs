#pragma once

#include <napi.h>

#include "session/config/contacts.hpp"

namespace session::nodeapi {
class ConstantsWrapper : public Napi::ObjectWrap<ConstantsWrapper> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ConstantsWrapper(const Napi::CallbackInfo& info);

  private:
    static Napi::Number MAX_NAME_LENGTH;
};
}  // namespace session::nodeapi
