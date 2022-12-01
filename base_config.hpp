#pragma once

#include <nan.h>
#include "session/config/base.hpp"

#define SESSION_LINK_BASE_CONFIG                        \
  Nan::SetPrototypeMethod(tpl, "needsDump", NeedsDump); \
  Nan::SetPrototypeMethod(tpl, "needsPush", NeedsPush);

class ConfigBaseWrapper : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init)
  {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ConfigBaseWrapper").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("ConfigBaseWrapper").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
  }

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

  static NAN_METHOD(NeedsDump)
  {
    ConfigBaseWrapper *obj = Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
    info.GetReturnValue().Set(obj->config->needs_dump());
    return;
  }

  static NAN_METHOD(NeedsPush)
  {
    ConfigBaseWrapper *obj = Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
    info.GetReturnValue().Set(obj->config->needs_push());
  }

  session::config::ConfigBase *config;

private:
  static NAN_METHOD(New)
  {
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    if (info.IsConstructCall())
    {
      if (info.Length() != 0)
      {

        Nan::ThrowTypeError("Wrong number of arguments");
        return;
      }
      ConfigBaseWrapper *obj = new ConfigBaseWrapper();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
    else
    {
      Nan::ThrowError("You need to call the constructor with the `new` syntax");
      return;
    }
  }
};
