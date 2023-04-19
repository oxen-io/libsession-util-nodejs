#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"
#include <nan.h>

void InitAll(v8::Local<v8::Object> exports) {
  ContactsConfigWrapperInsideWorker::Init(exports);
  UserGroupsWrapperInsideWorker::Init(exports);
  UserConfigWrapperInsideWorker::Init(exports);
  ConvoInfoVolatileWrapperInsideWorker::Init(exports);
}

NODE_MODULE(libsession_util_nodejs, InitAll)