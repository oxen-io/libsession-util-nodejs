#pragma once

#include <napi.h>

#include "../meta/meta_base_wrapper.hpp"
#include "../profile_pic.hpp"
#include "../utilities.hpp"
#include "./meta_group.hpp"

namespace session::nodeapi {
using config::groups::Members;
using session::config::groups::member;
using session::nodeapi::MetaGroup;

template <>
struct toJs_impl<member> {
    Napi::Object operator()(const Napi::Env& env, const member& info) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info.session_id);
        obj["name"] = toJs(env, info.name);
        obj["profilePicture"] = toJs(env, info.profile_picture);
        obj["invitePending"] = toJs(env, info.invite_pending());
        obj["inviteFailed"] = toJs(env, info.invite_failed());
        obj["promotionPending"] = toJs(env, info.promotion_pending());
        obj["promotionFailed"] = toJs(env, info.promotion_failed());
        obj["promoted"] = toJs(env, info.promoted());

        return obj;
    }
};

class MetaGroupWrapper : public MetaBaseWrapper, public Napi::ObjectWrap<MetaGroupWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports) {
        NoBaseClassInitHelper<MetaGroupWrapper>(
                env,
                exports,
                "MetaGroupWrapperNode",
                {
                        // shared exposed functions
                        InstanceMethod("needsPush", &MetaGroupWrapper::needsPush),

                        // infos exposed functions
                        // InstanceMethod("infoGet", &MetaGroupWrapper::infoGet),
                        // InstanceMethod("infoSet", &MetaGroupWrapper::infoSet),
                        // InstanceMethod("infoDestroy", &MetaGroupWrapper::infoDestroy),

                        // members exposed functions
                        InstanceMethod("memberGet", &MetaGroupWrapper::memberGet),
                        InstanceMethod(
                                "memberGetOrConstruct", &MetaGroupWrapper::memberGetOrConstruct),
                        InstanceMethod("memberGetAll", &MetaGroupWrapper::memberGetAll),
                        InstanceMethod("memberSetName", &MetaGroupWrapper::memberSetName),
                        InstanceMethod("memberSetInvited", &MetaGroupWrapper::memberSetInvited),
                        InstanceMethod("memberSetAccepted", &MetaGroupWrapper::memberSetAccepted),
                        InstanceMethod("memberSetPromoted", &MetaGroupWrapper::memberSetPromoted),
                        InstanceMethod(
                                "memberSetProfilePicture",
                                &MetaGroupWrapper::memberSetProfilePicture),
                        InstanceMethod("memberErase", &MetaGroupWrapper::memberErase),

                        // keys exposed functions
                        InstanceMethod("keysNeedsRekey", &MetaGroupWrapper::keysNeedsRekey),
                });
    }

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::shared_ptr<MetaGroup> meta_group;

    /* Shared Actions */

    Napi::Value needsPush(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->needs_push(); });
    }

    /** Info Actions */

    // Napi::Value infoGet(const Napi::CallbackInfo& info);

    // Napi::Value infoSet(const Napi::CallbackInfo& info);

    // Napi::Value infoDestroy(const Napi::CallbackInfo& info) {
    //     return wrapResult(info, [&] {
    //         meta_group->destroy_group();
    //         return meta_group->info_get(info);
    //     });
    // }

    /** Members Actions */

    Napi::Value memberGetAll(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->member_get_all(); });
    }

    Napi::Value memberGet(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            auto env = info.Env();
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            return meta_group->member_get(pubkeyHex);
        });
    }

    Napi::Value memberGetOrConstruct(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            return meta_group->member_get_or_construct(pubkeyHex);
        });
    }

    Napi::Value memberSetName(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsString(info[1]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto newName = toCppString(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->member_get_or_construct(pubkeyHex);
            m.set_name(newName);
            return this->meta_group->member_set(m);
        });
    }

    Napi::Value memberSetInvited(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsBoolean(info[1]);
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->member_get_or_construct(pubkeyHex);
            m.set_invited(failed);
            return this->meta_group->member_set(m);
        });
    }

    Napi::Value memberSetAccepted(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto m = this->meta_group->member_get_or_construct(pubkeyHex);
            m.set_accepted();

            return this->meta_group->member_set(m);
        });
    }

    Napi::Value memberSetPromoted(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsBoolean(info[1]);
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->member_get_or_construct(pubkeyHex);
            m.set_promoted(failed);

            return this->meta_group->member_set(m);
        });
    }

    Napi::Value memberSetProfilePicture(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            assertInfoLength(info, 2);
            assertIsString(info[0]);
            assertIsObject(info[1]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto profilePicture = profile_pic_from_object(info[1]);

            auto m = this->meta_group->member_get_or_construct(pubkeyHex);
            m.profile_picture = profilePicture;
            return this->meta_group->member_set(m);
        });
    }

    Napi::Value memberErase(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            std::optional<ustring> result;
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto erased = this->meta_group->member_erase(pubkeyHex);
            if (erased) {
                meta_group->keys_rekey();
            }

            return erased;
        });
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
