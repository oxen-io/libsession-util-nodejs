#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_groups.hpp"

using namespace session;
using namespace session::config;
using namespace std;

class UserGroupsWrapperInsideWorker : public ConfigBaseWrapperInsideWorker {
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit UserGroupsWrapperInsideWorker(ustring_view ed25519_secretkey,
                                         optional<ustring_view> dumped) {
    tryOrWrapStdException(
        [&]() { initWithConfig(new UserGroups(ed25519_secretkey, dumped)); });
  }

  // creation of the wrapper
  static NAN_METHOD(New);

  // Communities related methods
  static NAN_METHOD(GetCommunityByFullUrl);
  static NAN_METHOD(SetCommunityByFullUrl);
  static NAN_METHOD(GetAllCommunities);
  static NAN_METHOD(EraseCommunityByFullUrl);

  static NAN_METHOD(BuildFullUrlFromDetails);

  // Legacy groups related methods
};
