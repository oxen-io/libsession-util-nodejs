#include "base_config.hpp"

NAN_MODULE_INIT(ConfigBaseWrapper::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ConfigBaseWrapper").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("ConfigBaseWrapper").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ConfigBaseWrapper::New)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    if (info.IsConstructCall())
    {
        if (info.Length() != 0)
        {

            Nan::ThrowTypeError("Wrong number of arguments");
            return;
        }
        ConfigBaseWrapper *obj = new ConfigBaseWrapper();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        Nan::ThrowError("You need to call the constructor with the `new` syntax");
        return;
    }
}

NAN_METHOD(ConfigBaseWrapper::NeedsDump)
{
    ConfigBaseWrapper *obj = Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
    info.GetReturnValue().Set(obj->config->needs_dump());
    return;
}

NAN_METHOD(ConfigBaseWrapper::NeedsPush)
{
    ConfigBaseWrapper *obj = Nan::ObjectWrap::Unwrap<ConfigBaseWrapper>(info.Holder());
    info.GetReturnValue().Set(obj->config->needs_push());
}