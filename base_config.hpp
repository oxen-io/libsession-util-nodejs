#pragma once

#include <nan.h>
#include "session/config/base.hpp"

#define SESSION_LINK_BASE_CONFIG                        \
  Nan::SetPrototypeMethod(tpl, "needsDump", NeedsDump); \
  Nan::SetPrototypeMethod(tpl, "needsPush", NeedsPush);

class ConfigBaseWrapper : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);

protected:
  void initWithConfig(session::config::ConfigBase *config)
  {

    if (this->isInitialized())
    {
      Nan::ThrowError("this instance of ConfigBaseWrapper was already initialized");
      return;
    }
    this->config = config;
  }

  bool isInitialized()
  {
    return (this->config != nullptr);
  }

  /**
   * This function throws and exception if this instance has not been initialized, but you still need to return in the parent if it returns false.
   * The reason is that just throwing an exception does not apparently stop the executation of the current function.
   */
  bool isInitializedOrThrow()
  {
    if (!this->isInitialized())
    {
      Nan::ThrowError("this instance of ConfigBaseWrapper was already initialized");
      return false;
    }
    return true;
  }

  ~ConfigBaseWrapper()
  {
    // FIXME
    //  if (config)
    //  {
    //    config_free(config)
    //    config = nullptr;
    //  }
  }
  ConfigBaseWrapper()
  {
    config = nullptr;
  }

  static NAN_METHOD(NeedsDump);
  static NAN_METHOD(NeedsPush);

  session::config::ConfigBase *config;

private:
  static NAN_METHOD(New);
};
