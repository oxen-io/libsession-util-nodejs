#ifndef NATIVE_EXTENSION_GRAB_H
#define NATIVE_EXTENSION_GRAB_H

#include <nan.h>
#include "session/config/user_profile.hpp"

class UserConfigWrapper : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("UserConfigWrapper").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "getHandle", GetHandle);
    Nan::SetPrototypeMethod(tpl, "getValue", GetValue);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("UserConfigWrapper").ToLocalChecked(),
      Nan::GetFunction(tpl).ToLocalChecked());
  }

 private:
  explicit UserConfigWrapper(std::string restoreData) {

    auto profile = new session::config::UserProfile(restoreData);

  }



  ~UserConfigWrapper() {}

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      auto restoreData = Nan::To<v8::String>(info[0]).ToLocalChecked();
      UserConfigWrapper *obj = new UserConfigWrapper(*Nan::Utf8String(restoreData));

      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = {info[0]};
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
    }
  }

  static NAN_METHOD(GetHandle) {
    UserConfigWrapper* obj = Nan::ObjectWrap::Unwrap<UserConfigWrapper>(info.Holder());
    info.GetReturnValue().Set(obj->handle());
  }

  static NAN_METHOD(GetValue) {
    // UserConfigWrapper* obj = Nan::ObjectWrap::Unwrap<UserConfigWrapper>(info.Holder());
    // info.GetReturnValue().Set(obj->value_);
    info.GetReturnValue().Set(42);
  }

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
  // session::config::UserProfile* userProfile;


};

NODE_MODULE(session_util_wrapper, UserConfigWrapper::Init)

#endif
