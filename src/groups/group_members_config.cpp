#include "group_members_config.hpp"

#include <optional>

#include "../base_config.hpp"
#include "../profile_pic.hpp"
#include "session/config/groups/members.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

using config::groups::Members;
using session::config::groups::member;

template <>
struct toJs_impl<member> {
    Napi::Object operator()(const Napi::Env& env, const member& info) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info.session_id);
        obj["name"] = toJs(env, info.name);
        obj["profilePicture"] = toJs(env, info.profile_picture);
        obj["invitePending"] = toJs(env, info.invite_pending());
        obj["inviteFailed"] = toJs(env, info.invite_failed());
        obj["promotionPending"] = toJs(env, info.promotion_pending());
        obj["promotionFailed"] = toJs(env, info.promotion_failed());
        obj["promoted"] = toJs(env, info.promoted());

        return obj;
    }
};

void GroupMembersWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<GroupMembersWrapper>(
            env,
            exports,
            "GroupMembersWrapperNode",
            {
                    // group related methods
                    InstanceMethod("get", &GroupMembersWrapper::get),
                    InstanceMethod("getOrConstruct", &GroupMembersWrapper::getOrConstruct),
                    InstanceMethod("getAll", &GroupMembersWrapper::getAll),
                    InstanceMethod("setName", &GroupMembersWrapper::setName),
                    InstanceMethod("setInvited", &GroupMembersWrapper::setInvited),
                    InstanceMethod("setAccepted", &GroupMembersWrapper::setAccepted),
                    InstanceMethod("setPromoted", &GroupMembersWrapper::setPromoted),
                    InstanceMethod("setProfilePicture", &GroupMembersWrapper::setProfilePicture),
                    InstanceMethod("erase", &GroupMembersWrapper::erase),

            });
}

GroupMembersWrapper::GroupMembersWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct3Args<session::config::groups::Members>(info, "GroupsMembers")},
        Napi::ObjectWrap<GroupMembersWrapper>{info} {}

/**
 * =================================================
 * ==================== GETTERS ====================
 * =================================================
 */

Napi::Value GroupMembersWrapper::get(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::get");

        return config.get(pubkeyHex);
    });
}

Napi::Value GroupMembersWrapper::getOrConstruct(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::getOrConstruct");

        return config.get_or_construct(pubkeyHex);
    });
}

Napi::Value GroupMembersWrapper::getAll(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        assertInfoLength(info, 0);

        std::vector<member> allMembers;
        for (auto& member : config) {
            allMembers.push_back(member);
        }

        return allMembers;
    });
}

Napi::Value GroupMembersWrapper::setName(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsString(info[1]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::setName1");
        auto newName = toCppString(info[1], "GroupMembersWrapper::setName2");
        auto m = config.get_or_construct(pubkeyHex);
        m.set_name(newName);

        return m;
    });
}

Napi::Value GroupMembersWrapper::setInvited(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsBoolean(info[1]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::setInvited1");
        auto failed = toCppBoolean(info[1], "GroupMembersWrapper::setInvited2");
        auto m = config.get_or_construct(pubkeyHex);
        m.set_invited(failed);

        return m;
    });
}

Napi::Value GroupMembersWrapper::setAccepted(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::setAccepted");
        auto m = config.get_or_construct(pubkeyHex);
        m.set_accepted();

        return m;
    });
}

Napi::Value GroupMembersWrapper::setPromoted(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsBoolean(info[1]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::setPromoted1");
        auto failed = toCppBoolean(info[1], "GroupMembersWrapper::setPromoted2");
        auto m = config.get_or_construct(pubkeyHex);
        m.set_promoted(failed);

        return m;
    });
}

Napi::Value GroupMembersWrapper::setProfilePicture(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 2);
        assertIsString(info[0]);
        assertIsObject(info[1]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::setAccepted");
        auto profilePicture = profile_pic_from_object(info[1]);

        auto m = config.get_or_construct(pubkeyHex);
        m.profile_picture = profilePicture;
        config.set(m);

        return m;
    });
}

Napi::Value GroupMembersWrapper::erase(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsString(info[0]);

        auto pubkeyHex = toCppString(info[0], "GroupMembersWrapper::erase");
        return config.erase(pubkeyHex);
    });
}

}  // namespace session::nodeapi
