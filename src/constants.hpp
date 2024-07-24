#pragma once

#include <napi.h>

namespace session::nodeapi {
class ConstantsWrapper : public Napi::ObjectWrap<ConstantsWrapper> {
  public:
    ConstantsWrapper(const Napi::CallbackInfo& info);

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
};
}  // namespace session::nodeapi
