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

using push_entry_t = std::pair<
        std::string,
        std::tuple<
                session::config::seqno_t,
                session::ustring,
                std::vector<std::string, std::allocator<std::string>>>>;

using dump_entry_t = std::pair<std::string, session::ustring>;

Napi::Object push_entry_to_JS(const Napi::Env& env, const push_entry_t& push_entry) {
    auto obj = Napi::Object::New(env);

    obj["type"] = toJs(env, push_entry.first);
    obj["seqno"] = toJs(env, std::get<0>(push_entry.second));
    obj["data"] = toJs(env, std::get<1>(push_entry.second));
    obj["seqno"] = toJs(env, std::get<2>(push_entry.second));

    return obj;
};

Napi::Object dump_entry_to_JS(const Napi::Env& env, const dump_entry_t& dump_entry) {
    auto obj = Napi::Object::New(env);

    obj["type"] = toJs(env, dump_entry.first);
    obj["data"] = toJs(env, dump_entry.second);

    return obj;
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
                        InstanceMethod("infoGet", &MetaGroupWrapper::infoGet),
                        InstanceMethod("infoSet", &MetaGroupWrapper::infoSet),
                        InstanceMethod("infoDestroy", &MetaGroupWrapper::infoDestroy),

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
                        InstanceMethod("keyRekey", &MetaGroupWrapper::keyRekey),

                        InstanceMethod("currentHashes", &MetaGroupWrapper::currentHashes),
                        InstanceMethod("loadKeyMessage", &MetaGroupWrapper::loadKeyMessage),

                        InstanceMethod("encryptMessage", &MetaGroupWrapper::encryptMessage),
                        InstanceMethod("decryptMessage", &MetaGroupWrapper::decryptMessage),

                });
    }

    explicit MetaGroupWrapper(const Napi::CallbackInfo& info);

  private:
    std::shared_ptr<MetaGroup> meta_group;

    /* Shared Actions */

    Napi::Value needsPush(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            return this->meta_group->members->needs_push() ||
                   this->meta_group->info
                           ->needs_push();  // || this->meta_group->keys->needs_rekey() // TODO see
                                            // what to do with this and needs_rekey below
        });
    }

    Napi::Value push(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();

            vector<Napi::Object> to_push;
            if (this->meta_group->members->needs_push()) {
                auto memberPush = push_entry_to_JS(
                        env, std::make_pair("GroupMember"s, this->meta_group->members->push()));
                to_push.push_back(memberPush);
            }

            if (this->meta_group->info->needs_push()) {
                auto infosPush = push_entry_to_JS(
                        env, std::make_pair("GroupInfo"s, this->meta_group->info->push()));
                to_push.push_back(infosPush);
            }
            // TODO see what to do with this and needs_rekey below
            // if (this->meta_group->keys->needs_rekey()) {
            //     to_push.push_back(std::make_pair("GroupKeys"s, this->meta_group->keys_rekey()));
            // }
            return to_push;
        });
    }

    Napi::Value needsDump(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            return this->meta_group->members->needs_dump() ||
                   this->meta_group->info->needs_dump() || this->meta_group->keys->needs_dump();
        });
    }

    Napi::Value dump(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            auto env = info.Env();

            vector<Napi::Object> to_dump;
            if (this->meta_group->members->needs_dump()) {
                auto memberDump = dump_entry_to_JS(
                        env, std::make_pair("GroupMember"s, this->meta_group->members->dump()));
                to_dump.push_back(memberDump);
            }

            if (this->meta_group->info->needs_dump()) {
                auto infoDump = dump_entry_to_JS(
                        env, std::make_pair("GroupInfo"s, this->meta_group->members->dump()));
                to_dump.push_back(infoDump);
            }

            if (this->meta_group->keys->needs_dump()) {
                auto keysDump = dump_entry_to_JS(
                        env, std::make_pair("GroupKeys"s, this->meta_group->members->dump()));
                to_dump.push_back(keysDump);
            }

            return to_dump;
        });
    }

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
            assertInfoLength(info, 2);
            assertIsUInt8Array(info[0]);
            assertIsBoolean(info[1]);

            auto plaintext = toCppBuffer(info[0], __PRETTY_FUNCTION__);
            auto compress = toCppBoolean(info[1], __PRETTY_FUNCTION__);
            return this->meta_group->keys->encrypt_message(plaintext, compress);
        });
    }
    Napi::Value decryptMessage(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsUInt8Array(info[0]);

            auto ciphertext = toCppBuffer(info[0], __PRETTY_FUNCTION__);
            return this->meta_group->keys->decrypt_message(ciphertext);
        });
    }
};

}  // namespace session::nodeapi
