#include <nan.h>

#include "base_config.hpp"
#include "oxenc/hex.h"

#include <iostream>
#include <oxenc/hex.h>
#include <string_view>

using namespace std::literals;
using namespace oxenc::literals;

namespace {

std::string printable(std::string_view x) {
  std::string p;
  for (auto c : x) {
    if (c >= 0x20 && c <= 0x7e)
      p += c;
    else
      p += "\\x" + oxenc::to_hex(&c, &c + 1);
  }
  return p;
}
std::string printable(const char *x) = delete;
std::string printable(const char *x, size_t n) { return printable({x, n}); }

} // namespace

using std::cerr;

using v8::Array;
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

NAN_METHOD(ConfigBaseWrapper::Push) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
  auto context = Nan::GetCurrentContext();

  auto [to_push, seqno] = obj->config->push();

  auto to_push_js = toJsBuffer(&to_push);
  auto seqno_js = Nan::New<v8::Number>(seqno);
  Local<Object> to_return = Nan::New<Object>();
  auto result = to_return->Set(context, toJSString("data"), to_push_js);
  result = to_return->Set(context, toJSString("seqno"), seqno_js);

  info.GetReturnValue().Set(to_return);
}

NAN_METHOD(ConfigBaseWrapper::Dump) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());

  auto dumped = obj->config->dump();

  auto dumped_js = toJsBuffer(&dumped);

  info.GetReturnValue().Set(dumped_js);
}

NAN_METHOD(ConfigBaseWrapper::ConfirmPushed) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
  assertIsNumber(info[0]);

  auto seqno = info[0];
  int64_t seqNoInteger = toCppInteger(seqno);

  obj->config->confirm_pushed(seqNoInteger);
}

NAN_METHOD(ConfigBaseWrapper::Merge) {
  ConfigBaseWrapper *obj =
      Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
  assertIsArray(info[0]);

  Local<Array> asArray = info[0].As<Array>();

  uint32_t arrayLength = asArray->Length();
  std::vector<std::string> conf_strs;

  std::vector<std::string_view>
      to_merge; // FIXME: drop this with new libsession-util ustring changes
  to_merge.reserve(arrayLength);

  for (uint32_t i = 0; i < asArray->Length(); i++) {
    Local<Value> item =
        asArray->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    assertIsUInt8Array(item);
    conf_strs.push_back(toCppBuffer(item));
    to_merge.push_back(conf_strs.back());
  }

  obj->config->merge(to_merge);
}
