#include "meta_group_wrapper.hpp"

#include <napi.h>

#include <memory>

namespace session::nodeapi {

void MetaGroupWrapper::Init(Napi::Env env, Napi::Object exports) {
    NoBaseClassInitHelper<MetaGroupWrapper>(
            env,
            exports,
            "MetaGroupWrapperNode",
            {
                    InstanceMethod("infoNeedsPush", &MetaGroupWrapper::infoNeedsPush),
                    InstanceMethod("membersNeedsPush", &MetaGroupWrapper::membersNeedsPush),
                    InstanceMethod("keysNeedsRekey", &MetaGroupWrapper::keysNeedsRekey),
            });
}

MetaGroupWrapper::MetaGroupWrapper(const Napi::CallbackInfo& info) :
        MetaBaseWrapper{},
        Napi::ObjectWrap<MetaGroupWrapper>{info},
        meta_group{constructGroupWrapper(info, "MetaGroupWrapper")} {}

}  // namespace session::nodeapi
