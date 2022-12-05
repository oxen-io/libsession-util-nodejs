#include <nan.h>

#include "base_config.hpp"
#include "oxenc/hex.h"
#include <iostream>

using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

NAN_MODULE_INIT(ConfigBaseWrapper::Init) {
  Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ConfigBaseWrapper").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::Set(target, Nan::New("ConfigBaseWrapper").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ConfigBaseWrapper::New) {
  if (info.IsConstructCall()) {
    if (info.Length() != 0) {

      Nan::ThrowTypeError("Wrong number of arguments");
      return;
    }
    ConfigBaseWrapper *obj = new ConfigBaseWrapper();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Nan::ThrowError("You need to call the constructor with the `new` syntax");
    return;
  }
}

NAN_METHOD(ConfigBaseWrapper::NeedsDump) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
  info.GetReturnValue().Set(obj->config->needs_dump());
  return;
}

NAN_METHOD(ConfigBaseWrapper::NeedsPush) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
  info.GetReturnValue().Set(obj->config->needs_push());
}

void assertInfoLength(const Nan::FunctionCallbackInfo<Value> &info,
                      const int expected) {
  if (info.Length() != expected) {
    auto errorMsg = "Invalid number of arguments";
    throw std::invalid_argument(errorMsg);
  }
}

void assertInfoMinLength(const Nan::FunctionCallbackInfo<Value> &info,
                         const int minLength) {
  if (info.Length() < minLength) {
    auto errorMsg = "Invalid number of min length arguments";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsStringOrNull(const Local<Value> val) {
  if (!val->IsString() && !val->IsNull()) {
    auto errorMsg = "Wrong arguments: expected string or null";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsUInt8ArrayOrNull(const Local<Value> val) {
  if (!val->IsUint8Array() && !val->IsNull()) {
    auto errorMsg = "Wrong arguments: expected uint8Array or null";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsString(const Local<Value> val) {
  if (!val->IsString()) {
    auto errorMsg = "Wrong arguments: expected string";

    throw std::invalid_argument(errorMsg);
  }
}

Local<String> toJSString(std::string_view x) {

  return Nan::New<String>(x.data(), x.size()).ToLocalChecked();
}

std::string toCppString(Local<Value> x) {

  if (x->IsString()) {
    auto asStr = x.As<String>();

    Nan::Utf8String xUtf(x);
    std::string xStr{*xUtf, asStr->Length()};
    return xStr;
  }

  if (x->IsUint8Array()) {
    auto aUint8Array = x.As<Uint8Array>();

    std::string xStr;
    xStr.resize(aUint8Array->Length());
    aUint8Array->CopyContents(xStr.data(), xStr.size());
    return xStr;
  }

  auto errorMsg = "toCppString unsupported type";

  throw std::invalid_argument(errorMsg);
}

std::string toCppBuffer(Local<Value> x) {
  if (x->IsUint8Array()) {
    auto aUint8Array = x.As<Uint8Array>();

    std::string xStr;
    xStr.resize(aUint8Array->Length());
    aUint8Array->CopyContents(xStr.data(), xStr.size());
    return xStr;
  }

  auto errorMsg = "toCppBuffer unsupported type";

  throw std::invalid_argument(errorMsg);
}

Local<Object> toJsBuffer(const std::string *x) {
  std::string as = *x;

  auto buf = Nan::CopyBuffer(x->data(), x->size()).ToLocalChecked();
  return buf;
}