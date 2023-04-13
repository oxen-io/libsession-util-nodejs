#pragma once

// #include <oxenc/hex.h>
#include <string_view>

#include <nan.h>

#include "session/types.hpp"
#include <iostream>

using session::ustring;
using session::ustring_view;

using v8::Boolean;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

void assertInfoLength(const Nan::FunctionCallbackInfo<Value> &info,
                      const int expected);

void assertInfoMinLength(const Nan::FunctionCallbackInfo<Value> &info,
                         const int minLength);

void assertIsStringOrNull(const Local<Value> value);
void assertIsNumber(const Local<Value> value);
void assertIsArray(const Local<Value> value);
void assertIsObject(const Local<Value> value);
void assertIsUInt8ArrayOrNull(const Local<Value> value);
void assertIsUInt8Array(const Local<Value> value);
void assertIsString(const Local<Value> value);
void assertIsBoolean(const Local<Value> val);

template <typename Call> void tryOrWrapStdException(Call &&call) {
  try {
    call();
  } catch (const std::exception &e) {
    Nan::ThrowError(e.what());
  }
}

std::string toCppString(Local<Value> x, std::string identifier);
session::ustring toCppBuffer(Local<Value> x, std::string identifier);
int64_t toCppInteger(Local<Value> x, std::string identifier,
                     bool allowUndefined);
bool toCppBoolean(Local<Value> x, std::string identifier);

Local<String> toJsString(std::string_view x);
Local<Object> toJsBuffer(const ustring *x);
Local<Object> toJsBuffer(const ustring &x);
Local<Object> toJsBuffer(const ustring_view *x);
Local<Object> toJsBuffer(const ustring_view &x);
Local<Number> toJsNumber(int x);
Local<Number> toJsNumber(long int x);

Local<Boolean> toJsBoolean(bool x);

std::string printable(std::string_view x);
std::string printable(session::ustring_view x);
std::string printable(const char *x) = delete;
std::string printable(const char *x, size_t n);

std::string toCppDetailString(const Local<Value> val, std::string identifier);

template <typename T>
inline void ignore_result(const T & /* unused result */) {}

/**
 * Keep the current priority if a wrapper
 */
int64_t toPriority(Local<Value> x, int64_t currentPriority);
