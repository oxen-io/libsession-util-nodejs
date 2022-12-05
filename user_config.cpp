#include "user_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include <iostream>
NAN_MODULE_INIT(UserConfigWrapper::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("UserConfigWrapper").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "getName", GetName, "setName", SetName,
                     "getProfilePic", GetProfilePic, "setProfilePic",
                     SetProfilePic);

  // constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("UserConfigWrapper").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(UserConfigWrapper::New) {
  if (info.IsConstructCall()) {
    UserConfigWrapper *obj = new UserConfigWrapper();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Nan::ThrowError("You need to call the constructor with the `new` syntax");
    return;
  }
}

NAN_METHOD(UserConfigWrapper::GetName) {
  tryOrWrapStdException([&]() {
    v8::Isolate *isolate = info.GetIsolate();

    auto userProfile = to<session::config::UserProfile>(info);

    if (userProfile == nullptr || userProfile->get_name() == nullptr) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto name = userProfile->get_name();
    info.GetReturnValue().Set(
        v8::String::NewFromUtf8(isolate, name->c_str()).ToLocalChecked());
    return;
  });
}

NAN_METHOD(UserConfigWrapper::SetName) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    assertIsStringOrNull(info[0]);

    Nan::Utf8String str(info[0]);
    std::string cppStr(*str);

    auto userProfile = to<session::config::UserProfile>(info);
    if (!userProfile) {
      Nan::ThrowTypeError("User profile is null");
      return;
    }
    userProfile->set_name(cppStr);
  });
}

NAN_METHOD(UserConfigWrapper::GetProfilePic) {
  tryOrWrapStdException([&]() {
    auto isolate = info.GetIsolate();
    auto context = Nan::GetCurrentContext();
    auto userProfile = to<session::config::UserProfile>(info);

    if (userProfile == nullptr) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto [url, key] = userProfile->get_profile_pic();
    if (!url || !key) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    v8::Local<v8::Object> pictureObject = Nan::New<v8::Object>();

    auto urlStr = toJSString(isolate, *url);

    // key is a std::string not null terminated. We need to extract a
    // uint8Array out of it
    auto keyUint8Array = toJSUInt8Array(isolate, key);

    auto result =
        pictureObject->Set(context, toJSString(isolate, "url"), urlStr);
    result =
        pictureObject->Set(context, toJSString(isolate, "key"), keyUint8Array);

    info.GetReturnValue().Set(pictureObject);

    return;
  });
}

NAN_METHOD(UserConfigWrapper::SetProfilePic) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);
    assertIsStringOrNull(info[0]);
    assertIsUInt8ArrayOrNull(info[1]);

    auto pic = toCppString(info[0]);
    auto key = toCppString(info[1]);

    auto userProfile = to<session::config::UserProfile>(info);
    if (!userProfile) {
      throw std::invalid_argument("User profile is null");
    }
    userProfile->set_profile_pic(pic, key);
  });
}