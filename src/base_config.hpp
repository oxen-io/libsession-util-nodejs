#pragma once

#include "session/config/base.hpp"
#include "session/types.hpp"
#include "utilities.hpp"

#include <nan.h>
#include <optional>

class ConfigBaseWrapperInsideWorker : public Nan::ObjectWrap {
public:
  static NAN_MODULE_INIT(Init);

  template <typename... More>
  static void
  RegisterNANMethods(v8::Local<v8::FunctionTemplate> &tpl, const char *name,
                     Nan::FunctionCallback callback, More &&...more) {

    Nan::SetPrototypeMethod(tpl, name, std::move(callback));

    if constexpr (sizeof...(More) > 0)
      // If we were given more arguments then it is more methods, so recurse:
      RegisterNANMethods(tpl, std::forward<More>(more)...);
    else {
      // We're at the end of the argument list, so now add the base class
      // methods:
      Nan::SetPrototypeMethod(tpl, "needsDump",
                              &ConfigBaseWrapperInsideWorker::NeedsDump);
      Nan::SetPrototypeMethod(tpl, "needsPush",
                              &ConfigBaseWrapperInsideWorker::NeedsPush);
      Nan::SetPrototypeMethod(tpl, "push",
                              &ConfigBaseWrapperInsideWorker::Push);
      Nan::SetPrototypeMethod(tpl, "dump",
                              &ConfigBaseWrapperInsideWorker::Dump);
      Nan::SetPrototypeMethod(tpl, "confirmPushed",
                              &ConfigBaseWrapperInsideWorker::ConfirmPushed);
      Nan::SetPrototypeMethod(tpl, "merge",
                              &ConfigBaseWrapperInsideWorker::Merge);
      Nan::SetPrototypeMethod(tpl, "storageNamespace",
                              &ConfigBaseWrapperInsideWorker::StorageNamespace);
      Nan::SetPrototypeMethod(tpl, "encryptionDomain",
                              &ConfigBaseWrapperInsideWorker::EncryptionDomain);
    }
  }

protected:
  void initWithConfig(session::config::ConfigBase *config) {
    if (this->isInitialized()) {
      throw std::invalid_argument(
          "this instance of ConfigBaseWrapperInsideWorker was already "
          "initialized");
    }
    this->config = config;
  }

  bool isInitialized() { return (this->config != nullptr); }

  /**
   * This function throws and exception if this instance has not been
   * initialized, but you still need to return in the parent if it returns
   * false. The reason is that just throwing an exception does not apparently
   * stop the executation of the current function.
   */
  bool isInitializedOrThrow() {
    if (!this->isInitialized()) {
      throw std::invalid_argument(
          "this instance of ConfigBaseWrapperInsideWorker was already "
          "initialized");
    }
    return true;
  }

  template <typename Subtype>
  static Subtype *to(const Nan::FunctionCallbackInfo<v8::Value> &info) {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());

    if (!obj->isInitializedOrThrow())
      return nullptr;
    return dynamic_cast<Subtype *>(obj->config);
  }

  virtual ~ConfigBaseWrapperInsideWorker() {
    if (config)
      delete config;
    config = nullptr;
  }

  ConfigBaseWrapperInsideWorker() { config = nullptr; }

  static NAN_METHOD(NeedsDump);
  static NAN_METHOD(NeedsPush);
  static NAN_METHOD(Push);
  static NAN_METHOD(Dump);
  static NAN_METHOD(ConfirmPushed);
  static NAN_METHOD(Merge);
  static NAN_METHOD(EncryptionDomain);
  static NAN_METHOD(StorageNamespace);

  session::config::ConfigBase *config;

private:
  static NAN_METHOD(New);
};
