#include "session/config/community.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

template <>
struct toJs_impl<config::community> {
    Napi::Object operator()(const Napi::Env& env, const config::community info_comm) {
        auto obj = Napi::Object::New(env);

        obj["fullUrlWithPubkey"] = toJs(env, info_comm.full_url());
        obj["baseUrl"] = toJs(env, info_comm.base_url());
        obj["roomCasePreserved"] = toJs(env, info_comm.room());

        return obj;
    }
};

}  // namespace session::nodeapi
