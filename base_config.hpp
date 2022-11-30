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

    config = config;
  }

  ~ConfigBaseWrapper()
  {
    // FIXME
    //  if (config)
    //  {
    //    config_free(config)
    //    config = NULL;
    //  }
  }
  ConfigBaseWrapper()
  {
    config = NULL;
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
