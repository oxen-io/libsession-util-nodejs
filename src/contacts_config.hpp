#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/contacts.hpp"

using session::ustring_view;

using std::optional;

class ContactsConfigWrapperInsideWorker : public ConfigBaseWrapperInsideWorker {
public:
  static NAN_MODULE_INIT(Init);

private:
  explicit ContactsConfigWrapperInsideWorker(ustring_view ed25519_secretkey,
                                             optional<ustring_view> dumped) {
    tryOrWrapStdException([&]() {
      initWithConfig(new session::config::Contacts(ed25519_secretkey, dumped));
    });
  }

  static NAN_METHOD(New);
  static NAN_METHOD(Get);
  static NAN_METHOD(GetOrConstruct);
  static NAN_METHOD(GetAll);
  static NAN_METHOD(Set);
  static NAN_METHOD(Erase);
};
