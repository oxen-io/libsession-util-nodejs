#include "contacts_config.hpp"

#include <optional>

#include "profile_pic.hpp"
#include "session/config/expiring.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

using config::contact_info;

using config::Contacts;
using config::expiration_mode;

using namespace std::literals;

inline constexpr std::string_view expiration_mode_string(expiration_mode mode) {
    switch (mode) {
        case expiration_mode::none: return "off"sv;
        case expiration_mode::after_read: return "disappearAfterRead"sv;
        case expiration_mode::after_send: return "disappearAfterSend"sv;
    }
    // Don't do this via a default case so that the above will start warning about unhandled cases
    // if a newer libsession-util adds a new expiration mode value.
    throw std::logic_error{"Internal error: unhandled expiration mode!"};
}

inline constexpr expiration_mode expiration_mode_from_string(std::string_view mode) {
    if (mode == "disappearAfterRead"sv)
        return expiration_mode::after_read;
    if (mode == "disappearAfterSend"sv)
        return expiration_mode::after_send;
    return expiration_mode::none;
}

template <>
struct toJs_impl<contact_info> {
    Napi::Object operator()(const Napi::Env& env, const contact_info& contact) {
        auto obj = Napi::Object::New(env);

        obj["id"] = toJs(env, contact.session_id);
        obj["name"] = toJs(env, maybe_string(contact.name));
        obj["nickname"] = toJs(env, maybe_string(contact.nickname));
        obj["approved"] = toJs(env, contact.approved);
        obj["approvedMe"] = toJs(env, contact.approved_me);
        obj["blocked"] = toJs(env, contact.blocked);
        obj["priority"] = toJs(env, contact.priority);
        obj["createdAtSeconds"] = toJs(env, contact.created);
        // obj["expirationMode"] = toJs(env, expiration_mode_string(contact.exp_mode));
        // obj["expirationTimerSeconds"] = toJs(env, contact.exp_timer.count());
        obj["profilePicture"] = object_from_profile_pic(env, contact.profile_picture);

        return obj;
    }
};

void ContactsConfigWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<ContactsConfigWrapper>(
            env,
            exports,
            "ContactsConfigWrapperNode",
            {
                    InstanceMethod("get", &ContactsConfigWrapper::get),
                    InstanceMethod("getAll", &ContactsConfigWrapper::getAll),
                    InstanceMethod("set", &ContactsConfigWrapper::set),
                    InstanceMethod("erase", &ContactsConfigWrapper::erase),
            });
}

ContactsConfigWrapper::ContactsConfigWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<Contacts>(info, "ContactsConfig")},
        Napi::ObjectWrap<ContactsConfigWrapper>{info} {}

/** ==============================
 *             GETTERS
 * ============================== */

Napi::Value ContactsConfigWrapper::get(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    return wrapResult(env, [&] { return config.get(getStringArgs<1>(info)); });
}

Napi::Value ContactsConfigWrapper::getAll(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    return wrapExceptions(env, [&] {
        assertInfoLength(info, 0);

        auto contacts = Napi::Array::New(env, config.size());
        size_t i = 0;
        for (const auto& contact : config)
            contacts[i++] = toJs(env, contact);
        return contacts;
    });
}

/** ==============================
 *             SETTERS
 * ============================== */

void ContactsConfigWrapper::set(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        if (obj.IsEmpty())
            throw std::invalid_argument("cppContact received empty");

        auto contact = config.get_or_construct(toCppString(obj.Get("id"), "contacts.set, id"));

        if (contact.created == 0)
            contact.created = unix_timestamp_now();

        if (auto name = maybeNonemptyString(obj.Get("name"), "contacts.set name"))
            contact.set_name(std::move(*name));
        if (auto nickname = maybeNonemptyString(obj.Get("nickname"), "contacts.set nickname"))
            contact.set_nickname(std::move(*nickname));
        else
            contact.set_nickname("");
        // if no nickname are passed from the JS side, reset the nickname

        contact.approved = toCppBoolean(obj.Get("approved"), "contacts.set approved");
        contact.approved_me = toCppBoolean(obj.Get("approvedMe"), "contacts.set approvedMe");
        contact.blocked = toCppBoolean(obj.Get("blocked"), "contacts.set blocked");
        contact.priority = toPriority(obj.Get("priority"), contact.priority);

        // contact.exp_mode = expiration_mode_from_string(
        //         toCppString(obj.Get("expirationMode"), "contacts.set expirationMode"));
        // contact.exp_timer = std::chrono::seconds{toCppInteger(
        //         obj.Get("expirationTimerSeconds"), "contacts.set expirationTimerSeconds")};
        if (auto pic = obj.Get("profilePicture"); !pic.IsUndefined())
            contact.profile_picture = profile_pic_from_object(pic);
        else
            contact.profile_picture.clear();
        // if no profile picture are given from the JS side,
        // reset that user profile picture

        config.set(contact);
    });
}

/** ==============================
 *             ERASERS
 * ============================== */

Napi::Value ContactsConfigWrapper::erase(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase(getStringArgs<1>(info)); });
}

}  // namespace session::nodeapi
