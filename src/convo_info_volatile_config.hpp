#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/convo_info_volatile.hpp"

using namespace session;
using namespace session::config;
using namespace std;

class ConvoInfoVolatileWrapperInsideWorker
    : public ConfigBaseWrapperInsideWorker {
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit ConvoInfoVolatileWrapperInsideWorker(ustring_view ed25519_secretkey,
                                                optional<ustring_view> dumped) {
    tryOrWrapStdException([&]() {
      initWithConfig(new ConvoInfoVolatile(ed25519_secretkey, dumped));
    });
  }

  // creation of the wrapper
  static NAN_METHOD(New);

  // 1o1 related methods
  static NAN_METHOD(Get1o1);
  static NAN_METHOD(GetAll1o1);
  static NAN_METHOD(Set1o1);

  // legacy group related methods
  static NAN_METHOD(GetLegacyGroup);
  static NAN_METHOD(GetAllLegacyGroups);
  static NAN_METHOD(SetLegacyGroup);

  // TODO erase?

  // communities related methods
  static NAN_METHOD(GetCommunity);
  static NAN_METHOD(GetAllCommunities);
  static NAN_METHOD(SetCommunityByFullUrl);
};
