#include "constants.hpp"

namespace session::nodeapi {
Napi::Number ConstantsWrapper::MAX_NAME_LENGTH;

ConstantsWrapper::ConstantsWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<ConstantsWrapper>(info) {}

Napi::Object ConstantsWrapper::Init(Napi::Env env, Napi::Object exports) {
    MAX_NAME_LENGTH = Napi::Number::New(env, session::config::contact_info::MAX_NAME_LENGTH);

    Napi::Object constants = Napi::Object::New(env);
    constants.Set("MAX_NAME_LENGTH", MAX_NAME_LENGTH);

    exports.Set("CONSTANTS", constants);
    return exports;
}

}  // namespace session::nodeapi
