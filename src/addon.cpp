#include "contacts_config.hpp"
#include "user_config.hpp"
#include <nan.h>

void InitAll(v8::Local<v8::Object> exports) {
  ContactsConfigWrapper::Init(exports);
  UserConfigWrapper::Init(exports);
}

NODE_MODULE(session_util_wrapper, InitAll)