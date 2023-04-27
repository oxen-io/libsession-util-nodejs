#include "profile_pic.hpp"

#include "session/config/profile_pic.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

Napi::Value object_from_profile_pic(const Napi::Env& env, const config::profile_pic& pic) {
    if (!pic)
        return env.Null();

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

config::profile_pic profile_pic_from_object(Napi::Value val) {
    if (val.IsNull())
        return {};
    if (!val.IsObject())
        throw std::invalid_argument{"profilePicture must be null or Object"};

    auto obj = val.As<Napi::Object>();

    auto url = obj.Get("url");
    auto key = obj.Get("key");
    if (url.IsUndefined() || key.IsUndefined())
        throw std::invalid_argument{"profilePicture: url and key must both be present"};

    if (url.IsNull() || key.IsNull())
        return {};

    assertIsString(url);
    assertIsUInt8Array(key);

    auto url_str = toCppString(url, "profile_pic_from_object");
    if (url_str.size() > config::profile_pic::MAX_URL_LENGTH)
        throw std::invalid_argument{
                "profilePicture URL is too long (max length " +
                std::to_string(config::profile_pic::MAX_URL_LENGTH) + ")"};
    if (url_str.empty())
        return {};

    auto key_buf = toCppBuffer(key, "profile_pic_from_object");
    if (key_buf.empty())
        return {};

    return {std::move(url_str), std::move(key_buf)};
}

}  // namespace session::nodeapi
