#pragma once

#include <napi.h>

#include "../meta/meta_base.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/groups/keys.hpp"
#include "session/config/groups/members.hpp"

namespace session::nodeapi {

using config::groups::Info;
using config::groups::Keys;
using config::groups::Members;
using session::config::profile_pic;
using std::pair;
using std::string;
using std::tuple;
using std::vector;

class MetaGroup : public MetaBase {
  public:
    explicit MetaGroup(
            std::shared_ptr<Info> info,
            std::shared_ptr<Members> members,
            std::shared_ptr<Keys> keys);

    explicit MetaGroup(const Napi::CallbackInfo& info);

    //
    //
    //
    //
    //
    // shared actions

    auto needs_push() { return this->members->needs_push() || this->info->needs_push(); }

    auto push() {
        vector<pair<string, tuple<seqno_t, ustring, vector<string>>>> to_push;
        if (this->members->needs_push()) {
            to_push.push_back(std::make_pair("GroupMember"s, this->members->push()));
        }

        if (this->info->needs_push()) {
            to_push.push_back(std::make_pair("GroupInfo"s, this->info->push()));
        }

        if (this->keys->needs_rekey()) {
            to_push.push_back(std::make_pair("GroupKeys"s, this->info->push()));
        }
        return to_push;
    }

    //
    //
    //
    //
    //
    // info wrapper actions

    auto info_subaccount_mask() { return this->info->subaccount_mask(); }
    auto info_get_name() { return this->info->get_name(); }
    auto info_set_name(std::string_view new_name) { return this->info->set_name(new_name); }

    auto info_get_profile_pic() { return this->info->get_profile_pic(); }
    auto info_set_profile_pic(profile_pic pic) { return this->info->set_profile_pic(pic); }

    auto info_set_expiry_timer(std::chrono::seconds expiration_timer) {
        return this->info->set_expiry_timer(expiration_timer);
    }
    auto info_get_expiry_timer() { return this->info->get_expiry_timer(); }

    auto info_set_created(int64_t timestamp) { return this->info->set_created(timestamp); }
    auto info_get_created() { return this->info->get_created(); }

    auto info_set_delete_before(int64_t timestamp) {
        return this->info->set_delete_before(timestamp);
    }
    auto info_get_delete_before() { return this->info->get_delete_before(); }

    auto info_set_delete_attach_before(int64_t timestamp) {
        return this->info->set_delete_attach_before(timestamp);
    }
    auto info_get_delete_attach_before() { return this->info->get_delete_attach_before(); }

    auto info_destroy_group() { return this->info->destroy_group(); }
    auto info_is_destroyed() { return this->info->is_destroyed(); }

    //
    //
    //
    //
    // member wrapper actions
    auto member_get(std::string_view pubkey_hex) { return this->members->get(pubkey_hex); }
    auto member_get_or_construct(std::string_view pubkey_hex) {
        return this->members->get_or_construct(pubkey_hex);
    }

    auto member_get_all() {
        std::vector<session::config::groups::member> allMembers;
        for (auto& member : *this->members) {
            allMembers.push_back(member);
        }
        return allMembers;
    }

    auto member_set(const session::config::groups::member& member) {
        this->members->set(member);
        return this->members->get_or_construct(member.session_id);
    }

    auto member_erase(std::string_view session_id) { return this->members->erase(session_id); }

    //
    //
    //
    //
    // keys wrapper actions
    auto keys_admin() { return this->keys->admin(); }
    auto keys_key_supplement(std::vector<std::string> sids) const;
    auto keys_rekey() { return this->keys->rekey(*info, *members); }
    auto keys_needs_rekey() { return this->keys->needs_rekey(); }
    auto keys_needs_dump() { return this->keys->needs_dump(); }
    auto keys_pending_config() { return this->keys->pending_config(); }
    auto keys_encrypt_message(ustring_view plaintext, bool compress = true) const {
        return this->keys->encrypt_message(plaintext, compress);
    }
    auto keys_decrypt_message(ustring_view ciphertext) const {
        return this->keys->decrypt_message(ciphertext);
    }

  private:
    std::shared_ptr<config::groups::Info> info;
    std::shared_ptr<config::groups::Members> members;
    std::shared_ptr<config::groups::Keys> keys;
};

}  // namespace session::nodeapi
