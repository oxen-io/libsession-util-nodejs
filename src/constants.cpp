#include "constants.hpp"

#include "session/config/contacts.hpp"

namespace session::nodeapi {
ConstantsWrapper::ConstantsWrapper(const Napi::CallbackInfo& info) :
        Napi::ObjectWrap<ConstantsWrapper>(info) {}

Napi::Object ConstantsWrapper::Init(Napi::Env env, Napi::Object exports) {
    // fetch cpp constants
    Napi::Number MAX_NAME_LENGTH =
            Napi::Number::New(env, session::config::contact_info::MAX_NAME_LENGTH);

    const char* class_name = "CONSTANTS";

    // construct javascript constants object
    Napi::Function cls = DefineClass(
            env,
            class_name,
            {ObjectWrap::StaticValue("MAX_NAME_LENGTH", MAX_NAME_LENGTH, napi_enumerable)});

    // export object as javascript module
    exports.Set(class_name, cls);
    return exports;
}

}  // namespace session::nodeapi

