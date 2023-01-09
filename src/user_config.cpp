#include "user_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include <iostream>

using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

using session::ustring;
using session::ustring_view;

using std::cerr;

NAN_MODULE_INIT(UserConfigWrapper::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("UserConfigWrapper").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "getName", GetName, "setName", SetName,
                     "getProfilePic", GetProfilePic, "setProfilePic",
                     SetProfilePic);
  Nan::Set(target, Nan::New("UserConfigWrapper").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(UserConfigWrapper::New) {
  tryOrWrapStdException([&]() {
    if (info.IsConstructCall()) {
      assertInfoLength(info, 2);

      // we should get secret key as first arg and optional dumped as second
      // argument
      assertIsUInt8Array(info[0]);
      assertIsUInt8ArrayOrNull(info[1]);

      ustring secretKey = toCppBuffer(info[0]);

      std::optional<ustring_view> dumped = std::nullopt;

      UserConfigWrapper *obj = new UserConfigWrapper(secretKey, dumped);

      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {

      throw std::invalid_argument(
          "You need to call the constructor with the `new` syntax");
      return;
    }
  });
}

NAN_METHOD(UserConfigWrapper::GetName) {
  tryOrWrapStdException([&]() {
    auto userProfile = to<session::config::UserProfile>(info);

    if (!userProfile || !userProfile->get_name()) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto name = userProfile->get_name();

    if (name) {
      info.GetReturnValue().Set(toJsString(*name));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }

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
      throw std::invalid_argument("User profile is null");
      return;
    }

    userProfile->set_name(cppStr);
  });
}

NAN_METHOD(UserConfigWrapper::GetProfilePic) {
  tryOrWrapStdException([&]() {
    auto context = Nan::GetCurrentContext();
    auto userProfile = to<session::config::UserProfile>(info);

    if (!userProfile) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    std::optional<session::config::profile_pic> profile_pic =
        userProfile->get_profile_pic();
    if (profile_pic) {
      Local<Object> pictureObject = Nan::New<Object>();

      auto urlStr = toJsString(profile_pic->url);

      // key is a std::string not null terminated. We need to extract a
      // uint8Array out of it
      auto keyUint8Array = toJsBuffer(profile_pic->key);

      auto result = pictureObject->Set(context, toJsString("url"), urlStr);
      result = pictureObject->Set(context, toJsString("key"), keyUint8Array);

      info.GetReturnValue().Set(pictureObject);

      return;
    }
    Local<Object> pictureObject = Nan::New<Object>();

    auto result = pictureObject->Set(context, toJsString("url"), Nan::Null());
    result = pictureObject->Set(context, toJsString("key"), Nan::Null());

    info.GetReturnValue().Set(pictureObject);
    return;
  });
}

NAN_METHOD(UserConfigWrapper::SetProfilePic) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    auto second = info[1];
    assertIsStringOrNull(first);
    assertIsUInt8ArrayOrNull(second);

    std::string pic = toCppString(first);
    session::ustring_view key = toCppBuffer(second);

    auto userProfile = to<session::config::UserProfile>(info);
    if (!userProfile) {
      throw std::invalid_argument("User profile is null");
    }
    userProfile->set_profile_pic(pic, key);
  });
}