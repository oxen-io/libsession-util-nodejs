#pragma once

#include <napi.h>

#include "../meta/meta_base_wrapper.hpp"
#include "../utilities.hpp"
#include "./meta_group.hpp"

namespace session::nodeapi {
using session::nodeapi::MetaGroup;

class MetaGroupWrapper : public MetaBaseWrapper, public Napi::ObjectWrap<MetaGroupWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::shared_ptr<MetaGroup> meta_group;

    /** Info Actions */

    Napi::Value infoNeedsPush(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            return meta_group->info_needs_push();
        });
    }

    /** Members Actions */

    Napi::Value membersNeedsPush(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->members_needs_push(); });
    }

    /** Keys Actions */

    Napi::Value keysNeedsRekey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->keys_needs_rekey(); });
    }

    Napi::Value keyRekey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->keys_rekey(); });
    }
};

}  // namespace session::nodeapi
