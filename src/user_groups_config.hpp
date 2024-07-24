#pragma once

#include <napi.h>

#include "base_config.hpp"
#include "session/config/user_groups.hpp"

namespace session::nodeapi {

class UserGroupsWrapper : public ConfigBaseImpl, public Napi::ObjectWrap<UserGroupsWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit UserGroupsWrapper(const Napi::CallbackInfo& info);

  private:
    config::UserGroups& config{get_config<config::UserGroups>()};

    // Communities related methods
    Napi::Value getCommunityByFullUrl(const Napi::CallbackInfo& info);
    void setCommunityByFullUrl(const Napi::CallbackInfo& info);
    Napi::Value getAllCommunities(const Napi::CallbackInfo& info);
    Napi::Value eraseCommunityByFullUrl(const Napi::CallbackInfo& info);
    Napi::Value buildFullUrlFromDetails(const Napi::CallbackInfo& info);

    // Legacy groups related methods
    Napi::Value getLegacyGroup(const Napi::CallbackInfo& info);
    Napi::Value getAllLegacyGroups(const Napi::CallbackInfo& info);
    void setLegacyGroup(const Napi::CallbackInfo& info);
    Napi::Value eraseLegacyGroup(const Napi::CallbackInfo& info);

    // groups related methods
    Napi::Value createGroup(const Napi::CallbackInfo& info);
    Napi::Value getGroup(const Napi::CallbackInfo& info);
    Napi::Value getAllGroups(const Napi::CallbackInfo& info);
    Napi::Value setGroup(const Napi::CallbackInfo& info);
    Napi::Value eraseGroup(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
