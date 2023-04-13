#include "utilities.hpp"
#include <iostream>

using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

using session::ustring;
using session::ustring_view;
using namespace std;

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
  if (!val->IsNumber() || val.IsEmpty() || val->IsNullOrUndefined()) {
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

void assertIsObject(const Local<Value> val) {
  if (!val->IsObject() || val.IsEmpty() || val->IsNullOrUndefined()) {
    auto errorMsg = "Wrong arguments: expected object";

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
    auto errorMsg = "assertIsUInt8Array: Wrong arguments: expected uint8Array";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsString(const Local<Value> val) {
  if (!val->IsString()) {
    auto errorMsg = "assertIsString: Wrong arguments: expected string";

    throw std::invalid_argument(errorMsg);
  }
}

void assertIsBoolean(const Local<Value> val) {
  if (!val->IsBoolean()) {
    auto errorMsg = "assertIsBoolean: Wrong arguments: expected boolean";

    throw std::invalid_argument(errorMsg);
  }
}

Local<String> toJsString(std::string_view x) {

  return Nan::New<String>(x.data(), x.size()).ToLocalChecked();
}

Local<Number> toJsNumber(const int x) { return Nan::New<Number>(x); }
Local<Number> toJsNumber(const long int x) { return Nan::New<Number>(x); }

Local<Boolean> toJsBoolean(bool x) { return Nan::New<Boolean>(x); }

std::string toCppString(Local<Value> x, std::string identifier) {
  if (x->IsNullOrUndefined()) {
    throw std::invalid_argument(
        "toCppString called with null or undefined with identifier: " +
        identifier);
  }
  if (x->IsString()) {
    auto asStr = Nan::To<String>(x).ToLocalChecked();

    Nan::Utf8String xUtf(x);
    std::string xStr{*xUtf, static_cast<unsigned long int>(asStr->Length())};
    return xStr;
  }

  if (x->IsUint8Array()) {
    auto aUint8Array = x.As<Uint8Array>();

    std::string xStr;
    xStr.resize(aUint8Array->Length());
    aUint8Array->CopyContents(xStr.data(), xStr.size());
    return xStr;
  }

  auto errorMsg = "toCppString unsupported type with identifier: " + identifier;

  throw std::invalid_argument(errorMsg);
}

session::ustring toCppBuffer(Local<Value> x, std::string identifier) {
  if (x->IsNullOrUndefined()) {
    throw std::invalid_argument(
        "toCppBuffer called with null or undefined with identifier: " +
        identifier);
  }

  if (x->IsUint8Array()) {
    auto aUint8Array = x.As<Uint8Array>();

    session::ustring xStr;
    xStr.resize(aUint8Array->Length());
    aUint8Array->CopyContents(xStr.data(), xStr.size());

    return xStr;
  }

  auto errorMsg = "toCppBuffer unsupported type with identifier: " + identifier;

  throw std::invalid_argument(errorMsg);
}

Local<Object> toJsBuffer(const ustring *x) {
  auto buf =
      Nan::CopyBuffer((const char *)x->data(), x->size()).ToLocalChecked();
  return buf;
}

Local<Object> toJsBuffer(const ustring &x) {
  auto buf = Nan::CopyBuffer((const char *)x.data(), x.size()).ToLocalChecked();
  return buf;
}

Local<Object> toJsBuffer(const ustring_view *x) {
  auto buf =
      Nan::CopyBuffer((const char *)x->data(), x->size()).ToLocalChecked();
  return buf;
}

Local<Object> toJsBuffer(const ustring_view &x) {
  auto buf = Nan::CopyBuffer((const char *)x.data(), x.size()).ToLocalChecked();
  return buf;
}

std::string toCppDetailString(const Local<Value> val, std::string identifier) {
  auto context = Nan::GetCurrentContext();

  return toCppString(val->ToDetailString(context).ToLocalChecked(), identifier);
}

int64_t toCppInteger(Local<Value> x, std::string identifier,
                     bool allowUndefined) {

  if (allowUndefined && x->IsNullOrUndefined()) {
    return 0;
  }
  if (x->IsNumber()) {
    auto asNumber = x.As<v8::Number>();
    return asNumber->Value();
  }

  std::string errorMsg =
      "toCppInteger unsupported type with identifier: " + identifier +
      " and detailString: " + toCppDetailString(x, "toCppInteger");

  throw std::invalid_argument(errorMsg);
}

bool toCppBoolean(Local<Value> x, std::string identifier) {

  if (x->IsNullOrUndefined()) {
    return false;
  }

  if (x->IsBoolean() || x->IsNumber()) {
    return x.As<v8::Boolean>()->Value();
  }

  std::string errorMsg =
      "toCppBoolean unsupported type with identifier: " + identifier +
      " and detailString: " + toCppDetailString(x, "toCppBoolean");

  throw std::invalid_argument(errorMsg);
}

int64_t toPriority(Local<Value> x, int64_t currentPriority) {
  auto newPriority = toCppInteger(x, "toPriority", true);
  if (newPriority > 0) {
    // keep the existing priority if it is already set
    return max(currentPriority, (int64_t)1);
  }

  // newPriority being < 0 means that that conversation is hidden (and so
  // unpinned)

  return newPriority;
}