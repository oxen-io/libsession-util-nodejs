#include <napi.h>

#include "session/config/profile_pic.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

// Returns {"url": "...", "key": buffer} object; both values will be Null if the pic is not set.

template <>
struct toJs_impl<config::profile_pic> {
    Napi::Object operator()(const Napi::Env& env, const config::profile_pic& pic) {
        auto obj = Napi::Object::New(env);
        if (pic) {
            obj["url"] = toJs(env, pic.url);
            obj["key"] = toJs(env, pic.key);
        } else {
            obj["url"] = env.Null();
            obj["key"] = env.Null();
        }
        return obj;
    }
};

// Constructs a profile_pic from a Napi::Value which must be either Null or an Object; if an
// Object then it *must* contain "url" (string or null) and "key" (uint8array of size 32 or
// null) keys; if either is empty or Null then you get back an empty (i.e. clearing)
// profile_pic.  Throws on invalid input.
config::profile_pic profile_pic_from_object(Napi::Value val);

}  // namespace session::nodeapi
