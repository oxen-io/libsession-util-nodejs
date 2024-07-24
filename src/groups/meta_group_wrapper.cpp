#include "meta_group_wrapper.hpp"

#include <napi.h>

#include <memory>

#include "oxenc/bt_producer.h"
#include "session/types.hpp"

namespace session::nodeapi {

MetaGroupWrapper::MetaGroupWrapper(const Napi::CallbackInfo& info) :
        meta_group{std::move(MetaBaseWrapper::constructGroupWrapper(info, "MetaGroupWrapper"))},
        Napi::ObjectWrap<MetaGroupWrapper>{info} {}

void MetaGroupWrapper::Init(Napi::Env env, Napi::Object exports) {
    MetaBaseWrapper::NoBaseClassInitHelper<MetaGroupWrapper>(
            env,
            exports,
            "MetaGroupWrapperNode",
            {
                    // shared exposed functions

                    InstanceMethod("needsPush", &MetaGroupWrapper::needsPush),
                    InstanceMethod("push", &MetaGroupWrapper::push),
                    InstanceMethod("needsDump", &MetaGroupWrapper::needsDump),
                    InstanceMethod("metaDump", &MetaGroupWrapper::metaDump),
                    InstanceMethod("metaMakeDump", &MetaGroupWrapper::metaMakeDump),
                    InstanceMethod("metaConfirmPushed", &MetaGroupWrapper::metaConfirmPushed),
                    InstanceMethod("metaMerge", &MetaGroupWrapper::metaMerge),

                    // infos exposed functions
                    InstanceMethod("infoGet", &MetaGroupWrapper::infoGet),
                    InstanceMethod("infoSet", &MetaGroupWrapper::infoSet),
                    InstanceMethod("infoDestroy", &MetaGroupWrapper::infoDestroy),

                    // members exposed functions
                    InstanceMethod("memberGet", &MetaGroupWrapper::memberGet),
                    InstanceMethod("memberGetOrConstruct", &MetaGroupWrapper::memberGetOrConstruct),
                    InstanceMethod(
                            "memberConstructAndSet", &MetaGroupWrapper::memberConstructAndSet),

                    InstanceMethod("memberGetAll", &MetaGroupWrapper::memberGetAll),
                    InstanceMethod(
                            "memberGetAllPendingRemovals",
                            &MetaGroupWrapper::memberGetAllPendingRemovals),
                    InstanceMethod(
                            "membersMarkPendingRemoval",
                            &MetaGroupWrapper::membersMarkPendingRemoval),
                    InstanceMethod("memberSetName", &MetaGroupWrapper::memberSetName),
                    InstanceMethod("memberSetInvited", &MetaGroupWrapper::memberSetInvited),
                    InstanceMethod("memberSetAccepted", &MetaGroupWrapper::memberSetAccepted),
                    InstanceMethod("memberSetPromoted", &MetaGroupWrapper::memberSetPromoted),
                    InstanceMethod("memberSetAdmin", &MetaGroupWrapper::memberSetAdmin),
                    InstanceMethod(
                            "memberSetProfilePicture", &MetaGroupWrapper::memberSetProfilePicture),
                    InstanceMethod("memberEraseAndRekey", &MetaGroupWrapper::memberEraseAndRekey),

                    // keys exposed functions

                    InstanceMethod("keysNeedsRekey", &MetaGroupWrapper::keysNeedsRekey),
                    InstanceMethod("keyRekey", &MetaGroupWrapper::keyRekey),
                    InstanceMethod("keyGetAll", &MetaGroupWrapper::keyGetAll),

                    InstanceMethod("currentHashes", &MetaGroupWrapper::currentHashes),
                    InstanceMethod("loadKeyMessage", &MetaGroupWrapper::loadKeyMessage),
                    InstanceMethod("keyGetCurrentGen", &MetaGroupWrapper::keyGetCurrentGen),

                    InstanceMethod("encryptMessages", &MetaGroupWrapper::encryptMessages),
                    InstanceMethod("decryptMessage", &MetaGroupWrapper::decryptMessage),
                    InstanceMethod("makeSwarmSubAccount", &MetaGroupWrapper::makeSwarmSubAccount),
                    InstanceMethod("swarmSubAccountToken", &MetaGroupWrapper::swarmSubAccountToken),
                    InstanceMethod(
                            "swarmVerifySubAccount", &MetaGroupWrapper::swarmVerifySubAccount),
                    InstanceMethod("loadAdminKeys", &MetaGroupWrapper::loadAdminKeys),

                    InstanceMethod("swarmSubaccountSign", &MetaGroupWrapper::swarmSubaccountSign),
                    InstanceMethod(
                            "generateSupplementKeys", &MetaGroupWrapper::generateSupplementKeys),
            });
}

/* #region SHARED ACTIONS */

Napi::Value MetaGroupWrapper::needsPush(const Napi::CallbackInfo& info) {

    return wrapResult(info, [&] {
        return this->meta_group->members->needs_push() || this->meta_group->info->needs_push() ||
               this->meta_group->keys->pending_config();
    });
}

Napi::Value MetaGroupWrapper::push(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto to_push = Napi::Object::New(env);

        if (this->meta_group->members->needs_push())
            to_push["groupMember"s] = push_result_to_JS(
                    env,
                    this->meta_group->members->push(),
                    this->meta_group->members->storage_namespace());
        else
            to_push["groupMember"s] = env.Null();

        if (this->meta_group->info->needs_push())
            to_push["groupInfo"s] = push_result_to_JS(
                    env,
                    this->meta_group->info->push(),
                    this->meta_group->info->storage_namespace());
        else
            to_push["groupInfo"s] = env.Null();

        if (auto pending_config = this->meta_group->keys->pending_config())
            to_push["groupKeys"s] = push_key_entry_to_JS(
                    env, *(pending_config), this->meta_group->keys->storage_namespace());
        else
            to_push["groupKeys"s] = env.Null();

        return to_push;
    });
}

Napi::Value MetaGroupWrapper::needsDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        return this->meta_group->members->needs_dump() || this->meta_group->info->needs_dump() ||
               this->meta_group->keys->needs_dump();
    });
}

Napi::Value MetaGroupWrapper::metaDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        oxenc::bt_dict_producer combined;

        // NOTE: the keys have to be in ascii-sorted order:
        combined.append("info", session::from_unsigned_sv(this->meta_group->info->dump()));
        combined.append("keys", session::from_unsigned_sv(this->meta_group->keys->dump()));
        combined.append("members", session::from_unsigned_sv(this->meta_group->members->dump()));
        auto to_dump = std::move(combined).str();

        return session::ustring{to_unsigned_sv(to_dump)};
    });
}

Napi::Value MetaGroupWrapper::metaMakeDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        oxenc::bt_dict_producer combined;

        // NOTE: the keys have to be in ascii-sorted order:
        combined.append("info", session::from_unsigned_sv(this->meta_group->info->make_dump()));
        combined.append("keys", session::from_unsigned_sv(this->meta_group->keys->make_dump()));
        combined.append(
                "members", session::from_unsigned_sv(this->meta_group->members->make_dump()));
        auto to_dump = std::move(combined).str();

        return ustring{to_unsigned_sv(to_dump)};
    });
}

void MetaGroupWrapper::metaConfirmPushed(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&]() {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsArray(groupInfo);
            auto groupInfoArr = groupInfo.As<Napi::Array>();
            if (groupInfoArr.Length() != 2) {
                throw std::invalid_argument("groupInfoArr length was not 2");
            }

            auto seqno = maybeNonemptyInt(
                    groupInfoArr.Get("0"), "MetaGroupWrapper::metaConfirmPushed groupInfo seqno");
            auto hash = maybeNonemptyString(
                    groupInfoArr.Get("1"), "MetaGroupWrapper::metaConfirmPushed groupInfo hash");
            if (seqno && hash)
                this->meta_group->info->confirm_pushed(*seqno, *hash);
        }

        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsArray(groupMember);
            auto groupMemberArr = groupMember.As<Napi::Array>();
            if (groupMemberArr.Length() != 2) {
                throw std::invalid_argument("groupMemberArr length was not 2");
            }

            auto seqno = maybeNonemptyInt(
                    groupMemberArr.Get("0"),
                    "MetaGroupWrapper::metaConfirmPushed groupMemberArr seqno");
            auto hash = maybeNonemptyString(
                    groupMemberArr.Get("1"),
                    "MetaGroupWrapper::metaConfirmPushed groupMemberArr hash");
            if (seqno && hash)
                this->meta_group->members->confirm_pushed(*seqno, *hash);
        }
    });
};

Napi::Value MetaGroupWrapper::metaMerge(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");
        auto groupKeys = obj.Get("groupKeys");

        auto count_merged = 0;

        // Note: we need to process keys first as they might allow us the incoming info+members
        // details
        if (!groupKeys.IsNull() && !groupKeys.IsUndefined()) {
            assertIsArray(groupKeys);
            auto asArr = groupKeys.As<Napi::Array>();

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupKeys received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupKeys merge");
                assertIsNumber(itemObject.Get("timestampMs"), "timestampMs groupKeys");

                auto hash = toCppString(itemObject.Get("hash"), "meta.merge keys hash");
                auto data = toCppBuffer(itemObject.Get("data"), "meta.merge keys data");
                auto timestamp_ms = toCppInteger(
                        itemObject.Get("timestampMs"), "meta.merge keys timestampMs", false);

                this->meta_group->keys->load_key_message(
                        hash,
                        data,
                        timestamp_ms,
                        *(this->meta_group->info),
                        *(this->meta_group->members));
                count_merged++;  // load_key_message doesn't necessarely merge something as not
                                 // all keys are for us.
            }
        }

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsArray(groupInfo);
            auto asArr = groupInfo.As<Napi::Array>();

            std::vector<std::pair<std::string, ustring_view>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupInfo received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupInfo merge");
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBufferView(itemObject.Get("data"), "meta.merge"));
            }

            auto info_merged = this->meta_group->info->merge(conf_strs);
            count_merged += info_merged.size();
        }

        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsArray(groupMember);
            auto asArr = groupMember.As<Napi::Array>();

            std::vector<std::pair<std::string, ustring_view>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupMember received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"), "groupMember merge");
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBufferView(itemObject.Get("data"), "meta.merge"));
            }

            auto member_merged = this->meta_group->members->merge(conf_strs);

            count_merged += member_merged.size();
        }

        if (this->meta_group->keys->needs_rekey()) {
            this->meta_group->keys->rekey(*(this->meta_group->info), *(this->meta_group->members));
        }
        return count_merged;
    });
}

/* #endregion */

/* #region INFO ACTIONS */

Napi::Value MetaGroupWrapper::infoGet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto obj = Napi::Object::New(env);

        obj["name"] = toJs(env, this->meta_group->info->get_name());
        obj["createdAtSeconds"] = toJs(env, this->meta_group->info->get_created());
        obj["deleteAttachBeforeSeconds"] =
                toJs(env, this->meta_group->info->get_delete_attach_before());
        obj["deleteBeforeSeconds"] = toJs(env, this->meta_group->info->get_delete_before());

        if (auto expiry = this->meta_group->info->get_expiry_timer(); expiry)
            obj["expirySeconds"] = toJs(env, expiry->count());
        else
            obj["expirySeconds"] = env.Null();

        obj["isDestroyed"] = toJs(env, this->meta_group->info->is_destroyed());
        obj["profilePicture"] = toJs(env, this->meta_group->info->get_profile_pic());

        return obj;
    });
}

Napi::Value MetaGroupWrapper::infoSet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        if (auto name = maybeNonemptyString(obj.Get("name"), "MetaGroupWrapper::setInfo name"))
            this->meta_group->info->set_name(*name);

        if (auto created = maybeNonemptyInt(
                    obj.Get("createdAtSeconds"), "MetaGroupWrapper::setInfo set_created"))
            this->meta_group->info->set_created(std::move(*created));

        if (auto expiry = maybeNonemptyInt(
                    obj.Get("expirySeconds"), "MetaGroupWrapper::setInfo set_expiry_timer"))
            this->meta_group->info->set_expiry_timer(std::chrono::seconds{*expiry});

        if (auto deleteBefore = maybeNonemptyInt(
                    obj.Get("deleteBeforeSeconds"), "MetaGroupWrapper::setInfo set_delete_before"))
            this->meta_group->info->set_delete_before(std::move(*deleteBefore));

        if (auto deleteAttachBefore = maybeNonemptyInt(
                    obj.Get("deleteAttachBeforeSeconds"),
                    "MetaGroupWrapper::setInfo set_delete_attach_before"))
            this->meta_group->info->set_delete_attach_before(std::move(*deleteAttachBefore));

        if (auto profilePicture = obj.Get("profilePicture")) {
            auto profilePic = profile_pic_from_object(profilePicture);
            this->meta_group->info->set_profile_pic(profilePic);
        }

        return this->infoGet(info);
    });
}

Napi::Value MetaGroupWrapper::infoDestroy(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        meta_group->info->destroy_group();
        return this->infoGet(info);
    });
}

/* #endregion */

/* #region MEMBERS ACTIONS */

Napi::Value MetaGroupWrapper::memberGetAll(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        std::vector<session::config::groups::member> allMembers;
        for (auto& member : *this->meta_group->members) {
            allMembers.push_back(member);
        }
        return allMembers;
    });
}
Napi::Value MetaGroupWrapper::memberGetAllPendingRemovals(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        std::vector<session::config::groups::member> allMembersRemoved;
        for (auto& member : *this->meta_group->members) {
            if (member.is_removed()) {
                allMembersRemoved.push_back(member);
            }
        }
        return allMembersRemoved;
    });
}

Napi::Value MetaGroupWrapper::memberGet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        return meta_group->members->get(pubkeyHex);
    });
}

Napi::Value MetaGroupWrapper::memberGetOrConstruct(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        return meta_group->members->get_or_construct(pubkeyHex);
    });
}

void MetaGroupWrapper::memberConstructAndSet(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto created = meta_group->members->get_or_construct(pubkeyHex);
        meta_group->members->set(created);
    });
}

void MetaGroupWrapper::memberSetName(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);
        assertIsString(info[1]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto newName = toCppString(info[1], __PRETTY_FUNCTION__);
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_name(newName);
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetInvited(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertIsString(info[0]);
        assertIsBoolean(info[1]);
        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);

        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_invited(failed);
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetAccepted(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_accepted();
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetPromoted(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsBoolean(info[1]);
        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->set_promoted(failed);
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetAdmin(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);
        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        // Note: this step might add an admin which was removed back once he accepts the promotion,
        // but there is not much we can do about it
        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->admin = true;
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::memberSetProfilePicture(const Napi::CallbackInfo& info) {

    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsObject(info[1]);

        auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
        auto profilePicture = profile_pic_from_object(info[1]);

        auto m = this->meta_group->members->get(pubkeyHex);
        if (m) {
            m->profile_picture = profilePicture;
            this->meta_group->members->set(*m);
        }
    });
}

void MetaGroupWrapper::membersMarkPendingRemoval(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto toUpdateJSValue = info[0];
        auto withMessageJSValue = info[1];

        assertIsArray(toUpdateJSValue);
        assertIsBoolean(withMessageJSValue);
        bool withMessages = toCppBoolean(withMessageJSValue, __PRETTY_FUNCTION__);

        auto toUpdateJS = toUpdateJSValue.As<Napi::Array>();
        for (uint32_t i = 0; i < toUpdateJS.Length(); i++) {
            auto pubkeyHex = toCppString(toUpdateJS[i], __PRETTY_FUNCTION__);
            auto existing = this->meta_group->members->get(pubkeyHex);
            if (existing) {
                existing->set_removed(withMessages);
                this->meta_group->members->set(*existing);
            }
        }
    });
}

Napi::Value MetaGroupWrapper::memberEraseAndRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto toRemoveJSValue = info[0];

        assertIsArray(toRemoveJSValue);

        auto toRemoveJS = toRemoveJSValue.As<Napi::Array>();
        auto rekeyed = false;
        for (uint32_t i = 0; i < toRemoveJS.Length(); i++) {
            auto pubkeyHex = toCppString(toRemoveJS[i], __PRETTY_FUNCTION__);
            rekeyed |= this->meta_group->members->erase(pubkeyHex);
        }

        if (rekeyed) {
            meta_group->keys->rekey(*(this->meta_group->info), *(this->meta_group->members));
        }

        return rekeyed;
    });
}

/* #endregion */

/* #region KEYS ACTIONS */
Napi::Value MetaGroupWrapper::keysNeedsRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return meta_group->keys->needs_rekey(); });
}

Napi::Value MetaGroupWrapper::keyRekey(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        return meta_group->keys->rekey(*(meta_group->info), *(meta_group->members));
    });
}

Napi::Value MetaGroupWrapper::keyGetAll(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return meta_group->keys->group_keys(); });
}

Napi::Value MetaGroupWrapper::loadKeyMessage(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 3);
        assertIsString(info[0]);
        assertIsUInt8Array(info[1], __PRETTY_FUNCTION__);
        assertIsNumber(info[2], __PRETTY_FUNCTION__);

        auto hash = toCppString(info[0], __PRETTY_FUNCTION__);
        auto data = toCppBuffer(info[1], __PRETTY_FUNCTION__);
        auto timestamp_ms = toCppInteger(info[2], __PRETTY_FUNCTION__);

        return meta_group->keys->load_key_message(
                hash, data, timestamp_ms, *(this->meta_group->info), *(this->meta_group->members));
    });
}

Napi::Value MetaGroupWrapper::keyGetCurrentGen(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);
        return meta_group->keys->current_generation();
    });
}

Napi::Value MetaGroupWrapper::currentHashes(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto keysHashes = meta_group->keys->current_hashes();
        auto infoHashes = meta_group->info->current_hashes();
        auto memberHashes = meta_group->members->current_hashes();
        std::vector<std::string> merged;
        std::copy(std::begin(keysHashes), std::end(keysHashes), std::back_inserter(merged));
        std::copy(std::begin(infoHashes), std::end(infoHashes), std::back_inserter(merged));
        std::copy(std::begin(memberHashes), std::end(memberHashes), std::back_inserter(merged));
        return merged;
    });
}

Napi::Value MetaGroupWrapper::encryptMessages(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsArray(info[0]);

        auto plaintextsJS = info[0].As<Napi::Array>();
        uint32_t arrayLength = plaintextsJS.Length();
        std::vector<session::ustring> encryptedMessages;
        encryptedMessages.reserve(arrayLength);

        for (uint32_t i = 0; i < plaintextsJS.Length(); i++) {
            auto plaintext = toCppBuffer(plaintextsJS[i], __PRETTY_FUNCTION__);

            encryptedMessages.push_back(this->meta_group->keys->encrypt_message(plaintext));
        }
        return encryptedMessages;
    });
}

Napi::Value MetaGroupWrapper::decryptMessage(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], __PRETTY_FUNCTION__);

        auto ciphertext = toCppBuffer(info[0], __PRETTY_FUNCTION__);
        auto decrypted = this->meta_group->keys->decrypt_message(ciphertext);

        return decrypt_result_to_JS(info.Env(), decrypted);
    });
}

Napi::Value MetaGroupWrapper::makeSwarmSubAccount(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto memberPk = toCppString(info[0], __PRETTY_FUNCTION__);
        ustring subaccount = this->meta_group->keys->swarm_make_subaccount(memberPk);

        session::nodeapi::checkOrThrow(
                subaccount.length() == 100, "expected subaccount to be 100 bytes long");

        return subaccount;
    });
}

Napi::Value MetaGroupWrapper::swarmSubAccountToken(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto memberPk = toCppString(info[0], __PRETTY_FUNCTION__);
        ustring subaccount = this->meta_group->keys->swarm_subaccount_token(memberPk);

        session::nodeapi::checkOrThrow(
                subaccount.length() == 36, "expected subaccount token to be 36 bytes long");

        return oxenc::to_hex(subaccount);
    });
}

Napi::Value MetaGroupWrapper::swarmVerifySubAccount(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], __PRETTY_FUNCTION__);

        auto signingValue = toCppBuffer(info[0], __PRETTY_FUNCTION__);
        return this->meta_group->keys->swarm_verify_subaccount(signingValue);
    });
}

Napi::Value MetaGroupWrapper::loadAdminKeys(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsUInt8Array(info[0], __PRETTY_FUNCTION__);

        auto secret = toCppBuffer(info[0], __PRETTY_FUNCTION__);
        this->meta_group->keys->load_admin_key(
                secret, *(this->meta_group->info), *(this->meta_group->members));
        return info.Env().Null();
    });
}


Napi::Value MetaGroupWrapper::keysAdmin(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 0);
        return this->meta_group->keys->admin();
    });
}

Napi::Value MetaGroupWrapper::generateSupplementKeys(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto membersJSValue = info[0];
        assertIsArray(membersJSValue);

        auto membersJS = membersJSValue.As<Napi::Array>();
        uint32_t arrayLength = membersJS.Length();
        std::vector<std::string> membersToAdd;
        membersToAdd.reserve(arrayLength);
        std::vector<std::string> membersCpp;
        membersCpp.reserve(arrayLength);

        for (uint32_t i = 0; i < membersJS.Length(); i++) {
            auto memberPk = toCppString(membersJS[i], __PRETTY_FUNCTION__);
            membersCpp.push_back(memberPk);
        }
        return this->meta_group->keys->key_supplement(membersCpp);
    });
}

Napi::Value MetaGroupWrapper::swarmSubaccountSign(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsUInt8Array(info[0], "swarmSubaccountSign 0");
        assertIsUInt8Array(info[1], "swarmSubaccountSign 1");

        auto message = toCppBuffer(info[0], __PRETTY_FUNCTION__);
        auto authdata = toCppBuffer(info[1], __PRETTY_FUNCTION__);
        auto subaccountSign = this->meta_group->keys->swarm_subaccount_sign(message, authdata);

        return subaccountSign;
    });
}
/* #endregion */

}  // namespace session::nodeapi
