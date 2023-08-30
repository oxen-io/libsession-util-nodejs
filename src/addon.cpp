#include <napi.h>

#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "groups/group_info_config.hpp"
#include "groups/group_members_config.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    using namespace session::nodeapi;

    // User wrappers init
    UserConfigWrapper::Init(env, exports);
    ContactsConfigWrapper::Init(env, exports);
    UserGroupsWrapper::Init(env, exports);
    ConvoInfoVolatileWrapper::Init(env, exports);

    // Group wrappers init
    GroupInfoWrapper::Init(env, exports);
    GroupMembersWrapper::Init(env, exports);

    return exports;
}

NODE_API_MODULE(libsession_util_nodejs, InitAll);
