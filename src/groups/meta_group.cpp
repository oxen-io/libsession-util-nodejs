#include "meta_group.hpp"

#include <optional>

#include "session/config/groups/info.hpp"
#include "session/config/groups/keys.hpp"
#include "session/config/groups/members.hpp"

namespace session::nodeapi {

using config::groups::Info;
using config::groups::Keys;
using config::groups::Members;

using std::make_shared;

MetaGroup::MetaGroup(
        std::shared_ptr<config::groups::Info> info,
        std::shared_ptr<config::groups::Members> members,
        std::shared_ptr<config::groups::Keys> ke) :
        MetaBase(), info{info}, members{members}, keys{keys} {}

explicit MetaGroup::MetaGroup(const Napi::CallbackInfo& info) {}

}  // namespace session::nodeapi
