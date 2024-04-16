#pragma once

#include <napi.h>

#include "../meta/meta_base_wrapper.hpp"
#include "../profile_pic.hpp"
#include "../utilities.hpp"
#include "./meta_group.hpp"
#include "oxenc/bt_producer.h"

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
        obj["admin"] = toJs(env, info.admin);
        obj["removedStatus"] = toJs(env, info.removed_status);

        return obj;
    }
};

template <>
struct toJs_impl<Keys::swarm_auth> {
    Napi::Object operator()(const Napi::Env& env, const Keys::swarm_auth& auth) {
        auto obj = Napi::Object::New(env);

        obj["subaccount"] = toJs(env, auth.subaccount);
        obj["subaccount_sig"] = toJs(env, auth.subaccount_sig);
        obj["signature"] = toJs(env, auth.signature);
        return obj;
    }
};

class MetaGroupWrapper : public Napi::ObjectWrap<MetaGroupWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::unique_ptr<MetaGroup> meta_group;

    /* Shared Actions */
    Napi::Value needsPush(const Napi::CallbackInfo& info);
    Napi::Value push(const Napi::CallbackInfo& info);
    Napi::Value needsDump(const Napi::CallbackInfo& info);
    Napi::Value metaDump(const Napi::CallbackInfo& info);
    Napi::Value metaMakeDump(const Napi::CallbackInfo& info);
    void metaConfirmPushed(const Napi::CallbackInfo& info);
    Napi::Value metaMerge(const Napi::CallbackInfo& info);

    /** Info Actions */
    Napi::Value infoGet(const Napi::CallbackInfo& info);
    Napi::Value infoSet(const Napi::CallbackInfo& info);
    Napi::Value infoDestroy(const Napi::CallbackInfo& info);

    /** Members Actions */
    Napi::Value memberGetAll(const Napi::CallbackInfo& info);
    Napi::Value memberGetAllPendingRemovals(const Napi::CallbackInfo& info);
    Napi::Value memberGet(const Napi::CallbackInfo& info);
    Napi::Value memberGetOrConstruct(const Napi::CallbackInfo& info);
    Napi::Value memberSetName(const Napi::CallbackInfo& info);
    Napi::Value memberSetInvited(const Napi::CallbackInfo& info);
    Napi::Value memberSetAccepted(const Napi::CallbackInfo& info);
    Napi::Value memberSetPromoted(const Napi::CallbackInfo& info);
    Napi::Value memberSetAdmin(const Napi::CallbackInfo& info);
    Napi::Value memberSetProfilePicture(const Napi::CallbackInfo& info);
    Napi::Value memberEraseAndRekey(const Napi::CallbackInfo& info);
    Napi::Value membersMarkPendingRemoval(const Napi::CallbackInfo& info);

    /** Keys Actions */

    Napi::Value keysNeedsRekey(const Napi::CallbackInfo& info);
    Napi::Value keyRekey(const Napi::CallbackInfo& info);
    Napi::Value keyGetAll(const Napi::CallbackInfo& info);
    Napi::Value loadKeyMessage(const Napi::CallbackInfo& info);
    Napi::Value keyGetCurrentGen(const Napi::CallbackInfo& info);
    Napi::Value currentHashes(const Napi::CallbackInfo& info);
    Napi::Value encryptMessages(const Napi::CallbackInfo& info);
    Napi::Value decryptMessage(const Napi::CallbackInfo& info);
    Napi::Value makeSwarmSubAccount(const Napi::CallbackInfo& info);
    Napi::Value swarmSubAccountToken(const Napi::CallbackInfo& info);
    Napi::Value generateSupplementKeys(const Napi::CallbackInfo& info);
    Napi::Value swarmSubaccountSign(const Napi::CallbackInfo& info);
    Napi::Value swarmVerifySubAccount(const Napi::CallbackInfo& info);
    Napi::Value loadAdminKeys(const Napi::CallbackInfo& info);
};

}  // namespace session::nodeapi
