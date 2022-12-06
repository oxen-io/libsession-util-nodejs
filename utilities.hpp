#pragma once

#include <nan.h>

void assertInfoLength(const Nan::FunctionCallbackInfo<v8::Value> &info,
                      const int expected);

void assertInfoMinLength(const Nan::FunctionCallbackInfo<v8::Value> &info,
                         const int minLength);

void assertIsStringOrNull(const v8::Local<v8::Value> value);
void assertIsNumber(const v8::Local<v8::Value> value);
void assertIsArray(const v8::Local<v8::Value> value);
void assertIsUInt8ArrayOrNull(const v8::Local<v8::Value> value);
void assertIsUInt8Array(const v8::Local<v8::Value> value);
void assertIsString(const v8::Local<v8::Value> value);

template <typename Call> void tryOrWrapStdException(Call &&call) {
  try {
    call();
  } catch (const std::exception &e) {
    Nan::ThrowError(e.what());
  }
}

v8::Local<v8::String> toJSString(std::string_view x);
std::string toCppString(v8::Local<v8::Value> x);
std::string toCppBuffer(v8::Local<v8::Value> x);

v8::Local<v8::Object> toJsBuffer(const std::string *x);
int64_t toCppInteger(v8::Local<v8::Value> x);
