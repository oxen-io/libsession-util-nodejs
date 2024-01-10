#pragma once

#include <napi.h>

#include "base_config.hpp"
#include "session/config/convo_info_volatile.hpp"

namespace session::nodeapi {

class ConvoInfoVolatileWrapper : public ConfigBaseImpl,
                                 public Napi::ObjectWrap<ConvoInfoVolatileWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit ConvoInfoVolatileWrapper(const Napi::CallbackInfo& info);

  private:
    config::ConvoInfoVolatile& config{get_config<config::ConvoInfoVolatile>()};

    // 1o1 related methods
    Napi::Value get1o1(const Napi::CallbackInfo& info);
    Napi::Value getAll1o1(const Napi::CallbackInfo& info);
    void set1o1(const Napi::CallbackInfo& info);
    Napi::Value erase1o1(const Napi::CallbackInfo& info);

    // legacy group related methods
    Napi::Value getLegacyGroup(const Napi::CallbackInfo& info);
    Napi::Value getAllLegacyGroups(const Napi::CallbackInfo& info);
    void setLegacyGroup(const Napi::CallbackInfo& info);
    Napi::Value eraseLegacyGroup(const Napi::CallbackInfo& info);

    // group related methods
    Napi::Value getGroup(const Napi::CallbackInfo& info);
    Napi::Value getAllGroups(const Napi::CallbackInfo& info);
    void setGroup(const Napi::CallbackInfo& info);
    Napi::Value eraseGroup(const Napi::CallbackInfo& info);

    // communities related methods
    Napi::Value getCommunity(const Napi::CallbackInfo& info);
    Napi::Value getAllCommunities(const Napi::CallbackInfo& info);
    void setCommunityByFullUrl(const Napi::CallbackInfo& info);
    Napi::Value eraseCommunityByFullUrl(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
