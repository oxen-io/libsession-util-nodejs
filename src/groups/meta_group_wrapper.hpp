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

        return obj;
    }
};

class MetaGroupWrapper : public Napi::ObjectWrap<MetaGroupWrapper> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::shared_ptr<MetaGroup> meta_group;

    /* Shared Actions */

    Napi::Value needsPush(const Napi::CallbackInfo& info);

    Napi::Value push(const Napi::CallbackInfo& info) {
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

    Napi::Value needsDump(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            return this->meta_group->members->needs_dump() ||
                   this->meta_group->info->needs_dump() || this->meta_group->keys->needs_dump();
        });
    }

    Napi::Value metaDump(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();

            oxenc::bt_dict_producer combined;

            // NB: the keys have to be in ascii-sorted order:
            combined.append("info", session::from_unsigned_sv(this->meta_group->info->dump()));
            combined.append("keys", session::from_unsigned_sv(this->meta_group->keys->dump()));
            combined.append(
                    "members", session::from_unsigned_sv(this->meta_group->members->dump()));
            auto to_dump = std::move(combined).str();

            return ustring{to_unsigned_sv(to_dump)};
        });
    }

    Napi::Value metaMakeDump(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();

            oxenc::bt_dict_producer combined;

            // NB: the keys have to be in ascii-sorted order:
            combined.append("info", session::from_unsigned_sv(this->meta_group->info->make_dump()));
            combined.append("keys", session::from_unsigned_sv(this->meta_group->keys->make_dump()));
            combined.append(
                    "members", session::from_unsigned_sv(this->meta_group->members->make_dump()));
            auto to_dump = std::move(combined).str();

            return ustring{to_unsigned_sv(to_dump)};
        });
    }

    void metaConfirmPushed(const Napi::CallbackInfo& info);
    Napi::Value metaMerge(const Napi::CallbackInfo& info);

    /** Info Actions */

    Napi::Value infoGet(const Napi::CallbackInfo& info);

    Napi::Value infoSet(const Napi::CallbackInfo& info);

    Napi::Value infoDestroy(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            meta_group->info->destroy_group();
            return this->infoGet(info);
        });
    }

    /** Members Actions */

    Napi::Value memberGetAll(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            std::vector<session::config::groups::member> allMembers;
            for (auto& member : *this->meta_group->members) {
                allMembers.push_back(member);
            }
            return allMembers;
        });
    }

    Napi::Value memberGet(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            return meta_group->members->get(pubkeyHex);
        });
    }

    Napi::Value memberGetOrConstruct(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            return meta_group->members->get_or_construct(pubkeyHex);
        });
    }

    Napi::Value memberSetName(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsString(info[1]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto newName = toCppString(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->members->get_or_construct(pubkeyHex);
            m.set_name(newName);
            this->meta_group->members->set(m);
            return this->meta_group->members->get_or_construct(m.session_id);
        });
    }

    Napi::Value memberSetInvited(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsBoolean(info[1]);
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->members->get_or_construct(pubkeyHex);
            m.set_invited(failed);
            this->meta_group->members->set(m);
            return this->meta_group->members->get_or_construct(m.session_id);
        });
    }

    Napi::Value memberSetAccepted(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto m = this->meta_group->members->get_or_construct(pubkeyHex);
            m.set_accepted();

            this->meta_group->members->set(m);
            return this->meta_group->members->get_or_construct(m.session_id);
        });
    }

    Napi::Value memberSetPromoted(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            assertIsString(info[0]);
            assertIsBoolean(info[1]);
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto failed = toCppBoolean(info[1], __PRETTY_FUNCTION__);
            auto m = this->meta_group->members->get_or_construct(pubkeyHex);
            m.set_promoted(failed);

            this->meta_group->members->set(m);
            return this->meta_group->members->get_or_construct(m.session_id);
        });
    }

    Napi::Value memberSetProfilePicture(const Napi::CallbackInfo& info) {

        return wrapResult(info, [&] {
            assertInfoLength(info, 2);
            assertIsString(info[0]);
            assertIsObject(info[1]);

            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto profilePicture = profile_pic_from_object(info[1]);

            auto m = this->meta_group->members->get_or_construct(pubkeyHex);
            m.profile_picture = profilePicture;
            this->meta_group->members->set(m);
            return this->meta_group->members->get_or_construct(m.session_id);
        });
    }

    Napi::Value memberErase(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsString(info[0]);

            std::optional<ustring> result;
            auto pubkeyHex = toCppString(info[0], __PRETTY_FUNCTION__);
            auto erased = this->meta_group->members->erase(pubkeyHex);
            if (erased) {
                meta_group->keys->rekey(*(this->meta_group->info), *(this->meta_group->members));
            }

            return erased;
        });
    }

    /** Keys Actions */

    Napi::Value keysNeedsRekey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->keys->needs_rekey(); });
    }

    Napi::Value keyRekey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            return meta_group->keys->rekey(*(meta_group->info), *(meta_group->members));
        });
    }

    // TODO key_supplement, swarm_make_subaccount, ...

    Napi::Value loadKeyMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 3);
            assertIsString(info[0]);
            assertIsUInt8Array(info[1]);
            assertIsNumber(info[2]);

            auto hash = toCppString(info[0], __PRETTY_FUNCTION__);
            auto data = toCppBuffer(info[1], __PRETTY_FUNCTION__);
            auto timestamp_ms = toCppInteger(info[2], __PRETTY_FUNCTION__);

            return meta_group->keys->load_key_message(
                    hash,
                    data,
                    timestamp_ms,
                    *(this->meta_group->info),
                    *(this->meta_group->members));
        });
    }

    Napi::Value currentHashes(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] { return meta_group->keys->current_hashes(); });
    }

    Napi::Value encryptMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);

            auto plaintext = toCppBuffer(info[0], __PRETTY_FUNCTION__);
            return this->meta_group->keys->encrypt_message(plaintext);
        });
    }
    Napi::Value decryptMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsUInt8Array(info[0]);

            auto ciphertext = toCppBuffer(info[0], __PRETTY_FUNCTION__);
            auto decrypted = this->meta_group->keys->decrypt_message(ciphertext);

            return decrypt_result_to_JS(info.Env(), decrypted);
        });
    }
};

}  // namespace session::nodeapi
