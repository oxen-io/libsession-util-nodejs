#include <nan.h>

#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/config/encrypt.hpp"

#include <iostream>
#include <oxenc/hex.h>
#include <string_view>

using std::cerr;

using v8::Array;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

NAN_MODULE_INIT(ConfigBaseWrapperInsideWorker::Init) {
  Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ConfigBaseWrapperInsideWorker").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::Set(target, Nan::New("ConfigBaseWrapperInsideWorker").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::New) {
  tryOrWrapStdException([&]() {
    if (info.IsConstructCall()) {
      assertInfoLength(info, 0);
      ConfigBaseWrapperInsideWorker *obj = new ConfigBaseWrapperInsideWorker();
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      throw std::invalid_argument(
          "You need to call the constructor with the `new` syntax");

      return;
    }
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::NeedsDump) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 0);
    info.GetReturnValue().Set(obj->config->needs_dump());
    return;
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::NeedsPush) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    info.GetReturnValue().Set(obj->config->needs_push());
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::Push) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 0);
    auto context = Nan::GetCurrentContext();
    auto [seqno, to_push, hashes] = obj->config->push();

    Local<Object> to_push_js = toJsBuffer(&to_push);
    Local<v8::Number> seqno_js = Nan::New<v8::Number>(seqno);
    Local<Object> to_return = Nan::New<Object>();
    auto result = to_return->Set(context, toJsString("data"), to_push_js);
    result = to_return->Set(context, toJsString("seqno"), seqno_js);

    Local<Array> jsHashes = Nan::New<Array>(hashes.size());
    int index = 0;

    for (auto &hash : hashes) {
      ignore_result(
          jsHashes->Set(Nan::GetCurrentContext(), index, toJsString(hash)));
      index++;
    }
    result = to_return->Set(context, toJsString("hashes"), jsHashes);

    info.GetReturnValue().Set(to_return);
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::Dump) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 0);
    auto dumped = obj->config->dump();

    auto dumped_js = toJsBuffer(&dumped);

    info.GetReturnValue().Set(dumped_js);
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::ConfirmPushed) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 2);
    assertIsNumber(info[0]);
    assertIsString(info[1]);

    auto seqno = info[0];
    auto jsHash = info[1];
    int64_t seqNoInteger = toCppInteger(seqno, "ConfirmPushed", false);
    std::string cppHash = toCppString(jsHash, "ConfirmPushed");

    obj->config->confirm_pushed(seqNoInteger, cppHash);
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::Merge) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 1);
    assertIsArray(info[0]);
    Local<Array> asArray = info[0].As<Array>();

    uint32_t arrayLength = asArray->Length();
    std::vector<std::pair<std::string, session::ustring>> conf_strs;
    conf_strs.reserve(arrayLength);

    for (uint32_t i = 0; i < asArray->Length(); i++) {
      Local<Value> item =
          asArray->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      assertIsObject(item);
      if (item.IsEmpty()) {
        throw std::invalid_argument("Merge.item received empty");
      }

      Local<Object> itemObject = Nan::To<Object>(item).ToLocalChecked();

      auto hash = Nan::Get(itemObject, toJsString("hash")).ToLocalChecked();
      auto data = Nan::Get(itemObject, toJsString("data")).ToLocalChecked();
      assertIsString(hash);
      assertIsUInt8Array(data);

      std::string hashCpp = toCppString(hash, "base.merge");
      session::ustring dataCpp = toCppBuffer(data, "base.merge");
      std::pair<std::string, session::ustring> pair =
          std::make_pair(hashCpp, dataCpp);

      conf_strs.push_back(pair);
    }

    int accepted = obj->config->merge(conf_strs);
    info.GetReturnValue().Set(toJsNumber(accepted));
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::StorageNamespace) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 0);
    session::config::Namespace config_namespace =
        obj->config->storage_namespace();

    info.GetReturnValue().Set(
        toJsNumber(static_cast<int16_t>(config_namespace)));
  });
}

NAN_METHOD(ConfigBaseWrapperInsideWorker::EncryptionDomain) {
  tryOrWrapStdException([&]() {
    ConfigBaseWrapperInsideWorker *obj =
        Nan::ObjectWrap::Unwrap<ConfigBaseWrapperInsideWorker>(info.Holder());
    assertInfoLength(info, 0);

    const char *domain = obj->config->encryption_domain();

    info.GetReturnValue().Set(toJsString(domain));
  });
}