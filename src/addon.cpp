#include <napi.h>

#include "contacts_config.hpp"
#include "convo_info_volatile_config.hpp"
#include "groups/meta_group_wrapper.hpp"
#include "multi_encrypt/multi_encrypt.hpp"
#include "user_config.hpp"
#include "user_groups_config.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    using namespace session::nodeapi;
    // Group wrappers init
    MetaGroupWrapper::Init(env, exports);

    // User wrappers init
    UserConfigWrapper::Init(env, exports);
    ContactsConfigWrapper::Init(env, exports);
    UserGroupsWrapper::Init(env, exports);
    ConvoInfoVolatileWrapper::Init(env, exports);

    // Fully static wrappers init
    MultiEncryptWrapper::Init(env, exports);

    return exports;
}

NODE_API_MODULE(libsession_util_nodejs, InitAll);
