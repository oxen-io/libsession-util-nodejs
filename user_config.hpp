#pragma once

#include <nan.h>

#include "base_config.hpp"

#include "session/config/user_profile.hpp"

class UserConfigWrapper : public ConfigBaseWrapper
{
public:
  static NAN_MODULE_INIT(Init)
  {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("UserConfigWrapper").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "getName", GetName);
    Nan::SetPrototypeMethod(tpl, "setName", SetName);
    SESSION_LINK_BASE_CONFIG

    // constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("UserConfigWrapper").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
  }

private:
  // session::config::UserProfile *userProfile;

  explicit UserConfigWrapper()
  {
    initWithConfig(new session::config::UserProfile());
  }

  ~UserConfigWrapper()
  {
  }

  static NAN_METHOD(New)
  {
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    if (info.IsConstructCall())
    {
      UserConfigWrapper *obj = new UserConfigWrapper();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
    else
    {
      Nan::ThrowError("You need to call the constructor with the `new` syntax");
      return;
    }
  }

  static NAN_METHOD(GetName)
  {
    v8::Isolate *isolate = info.GetIsolate();
    UserConfigWrapper *obj = Nan::ObjectWrap::Unwrap<UserConfigWrapper>(info.Holder());
    auto asUserProfile = static_cast<session::config::UserProfile *>(obj->config);
    if (asUserProfile == nullptr || asUserProfile->get_name() == nullptr)
    {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto name = asUserProfile->get_name();
    info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, name->c_str()).ToLocalChecked());
    return;
  }

  static NAN_METHOD(SetName)
  {
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (info.Length() != 1)
    {
      Nan::ThrowTypeError("Wrong number of arguments");
      return;
    }

    if (!info[0]->IsString() && !info[0]->IsNull())
    {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }

    v8::String::Utf8Value str(isolate, info[0]);
    std::string cppStr(*str);

    UserConfigWrapper *obj = Nan::ObjectWrap::Unwrap<UserConfigWrapper>(info.Holder());
    auto asUserProfile = static_cast<session::config::UserProfile *>(obj->config);
    asUserProfile->set_name(cppStr);
  }
};

NODE_MODULE(session_util_wrapper, UserConfigWrapper::Init)
