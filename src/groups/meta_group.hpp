#pragma once

#include <napi.h>

#include "../meta/meta_base.hpp"
#include "session/config/groups/info.hpp"
#include "session/config/groups/keys.hpp"
#include "session/config/groups/members.hpp"

namespace session::nodeapi {

class MetaGroup : public MetaBase {
  public:
    explicit MetaGroup(
            std::shared_ptr<config::groups::Info> info,
            std::shared_ptr<config::groups::Members> members,
            std::shared_ptr<config::groups::Keys> keys);

    explicit MetaGroup(const Napi::CallbackInfo& info);

    bool info_needs_push() { return this->info->needs_push(); }
    bool members_needs_push() { return this->members->needs_push(); }
    bool keys_needs_rekey() { return this->keys->needs_rekey(); }
    session::ustring_view keys_rekey() { return this->keys->rekey(*info, *members); }

  private:
    std::shared_ptr<config::groups::Info> info;
    std::shared_ptr<config::groups::Members> members;
    std::shared_ptr<config::groups::Keys> keys;
};

}  // namespace session::nodeapi
