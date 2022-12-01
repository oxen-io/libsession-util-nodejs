#include "user_config.hpp"

NAN_MODULE_INIT(UserConfigWrapper::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("UserConfigWrapper").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "getName", GetName);
    Nan::SetPrototypeMethod(tpl, "setName", SetName);
    SESSION_LINK_BASE_CONFIG

    // constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("UserConfigWrapper").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(UserConfigWrapper::New)
{
    if (info.IsConstructCall())
    {
        UserConfigWrapper *obj = new UserConfigWrapper();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        Nan::ThrowError("You need to call the constructor with the `new` syntax");
        return;
    }
}

NAN_METHOD(UserConfigWrapper::GetName)
{
    v8::Isolate *isolate = info.GetIsolate();

    auto userProfile = toUserProfile(info);

    if (userProfile == nullptr || userProfile->get_name() == nullptr)
    {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    auto name = userProfile->get_name();
    info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, name->c_str()).ToLocalChecked());
    return;
}

NAN_METHOD(UserConfigWrapper::SetName)
{
    v8::Isolate *isolate = info.GetIsolate();

    if (info.Length() != 1)
    {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() && !info[0]->IsNull())
    {
        Nan::ThrowTypeError("Wrong arguments");
        return;
    }

    v8::String::Utf8Value str(isolate, info[0]);
    std::string cppStr(*str);

    auto userProfile = toUserProfile(info);
    userProfile->set_name(cppStr);
}

session::config::UserProfile *UserConfigWrapper::toUserProfile(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    UserConfigWrapper *obj = Nan::ObjectWrap::Unwrap<UserConfigWrapper>(info.Holder());

    if (!obj->isInitializedOrThrow())
    {
        return nullptr;
    }
    auto asUserProfile = static_cast<session::config::UserProfile *>(obj->config);

    return asUserProfile;
}
