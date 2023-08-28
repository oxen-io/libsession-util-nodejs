#include "group_info_config.hpp"

#include <optional>

#include "base_config.hpp"
#include "profile_pic.hpp"
#include "session/config/groups/info.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

// namespace convo = config::convo;

using config::groups::Info;

void GroupInfoWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<GroupInfoWrapper>(
            env,
            exports,
            "GroupInfoWrapperNode",
            {
                    // group related methods
                    InstanceMethod("getInfo", &GroupInfoWrapper::getInfo),
                    InstanceMethod("setInfo", &GroupInfoWrapper::setInfo),
                    // InstanceMethod("destroy", &GroupInfoWrapper::destroy),

            });
}

GroupInfoWrapper::GroupInfoWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct3Args<session::config::groups::Info>(info, "GroupsInfo")},
        Napi::ObjectWrap<GroupInfoWrapper>{info} {}

// /**
//  * =================================================
//  * ==================== GETTERS ====================
//  * =================================================
//  */

Napi::Value GroupInfoWrapper::getInfo(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto obj = Napi::Object::New(env);

        obj["name"] = toJs(env, config.get_name());
        obj["createdAtSeconds"] = toJs(env, config.get_created());
        obj["deleteAttachBeforeSeconds"] = toJs(env, config.get_delete_attach_before());
        obj["deleteBeforeSeconds"] = toJs(env, config.get_delete_before());

        if (auto expiry = config.get_expiry_timer(); expiry)
            obj["expirySeconds"] = toJs(env, expiry->count());
        obj["isDestroyed"] = toJs(env, config.is_destroyed());
        obj["profilePicture"] = object_from_profile_pic(env, config.get_profile_pic());

        return obj;
    });
}

Napi::Value GroupInfoWrapper::setInfo(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        if (auto name = maybeNonemptyString(obj.Get("name"), "GroupInfoWrapper::setInfo name"))
            config.set_name(*name);

        if (auto created = maybeNonemptyInt(
                    obj.Get("createdAtSeconds"), "GroupInfoWrapper::setInfo set_created"))
            config.set_created(std::move(*created));

        if (auto expiry = maybeNonemptyInt(
                    obj.Get("expirySeconds"), "GroupInfoWrapper::setInfo set_expiry_timer"))
            config.set_expiry_timer(std::chrono::seconds{*expiry});

        if (auto deleteBefore = maybeNonemptyInt(
                    obj.Get("deleteBeforeSeconds"), "GroupInfoWrapper::setInfo set_delete_before"))
            config.set_delete_before(std::move(*deleteBefore));

        if (auto deleteAttachBefore = maybeNonemptyInt(
                    obj.Get("deleteAttachBeforeSeconds"),
                    "GroupInfoWrapper::setInfo set_delete_attach_before"))
            config.set_delete_attach_before(std::move(*deleteAttachBefore));

        if (auto profilePicture = obj.Get("profilePicture")) {
            auto profilePic = profile_pic_from_object(profilePicture);
            config.set_profile_pic(profilePic);
        }

        return this->getInfo(info);
    });
}

Napi::Value GroupInfoWrapper::destroy(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        config.destroy_group();
        return this->getInfo(info);
    });
}

}  // namespace session::nodeapi
