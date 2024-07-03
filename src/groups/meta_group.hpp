#pragma once

#include <napi.h>

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

using std::make_shared;
using std::shared_ptr;

class MetaGroup {
  public:
    shared_ptr<config::groups::Info> info;
    shared_ptr<config::groups::Members> members;
    shared_ptr<config::groups::Keys> keys;
    string edGroupPubKey;
    std::optional<string> edGroupSecKey;

    MetaGroup(
            shared_ptr<config::groups::Info> info,
            shared_ptr<config::groups::Members> members,
            shared_ptr<config::groups::Keys> keys,
            session::ustring edGroupPubKey,
            std::optional<session::ustring> edGroupSecKey) :
            info{info}, members{members}, keys{keys} {

        this->edGroupPubKey = oxenc::to_hex(edGroupPubKey);
        this->edGroupSecKey = edGroupSecKey ? oxenc::to_hex(*edGroupSecKey) : nullptr;
    };

    explicit MetaGroup(const Napi::CallbackInfo& info) {}
};

}  // namespace session::nodeapi
