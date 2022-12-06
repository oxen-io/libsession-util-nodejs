#include "utilities.hpp"
#include "base_config.hpp"

using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

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

void assertIsNumber(const Local<Value> val) {
  if (!val->IsNumber()) {
    auto errorMsg = "Wrong arguments: expected number";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsArray(const Local<Value> val) {
  if (!val->IsArray()) {
    auto errorMsg = "Wrong arguments: expected array";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsUInt8ArrayOrNull(const Local<Value> val) {
  if (!val->IsUint8Array() && !val->IsNull()) {
    auto errorMsg = "Wrong arguments: expected uint8Array or null";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsUInt8Array(const Local<Value> val) {
  if (!val->IsUint8Array()) {
    auto errorMsg = "Wrong arguments: expected uint8Array";

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
    auto asStr = Nan::To<String>(x).ToLocalChecked();

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

int64_t toCppInteger(Local<Value> x) {

  if (x->IsNumber()) {
    auto asNumber = x.As<v8::Number>();
    return asNumber->Value();
  }

  auto errorMsg = "toCppInteger unsupported type";

  throw std::invalid_argument(errorMsg);
}