#include "user_groups_config.hpp"

#include <oxenc/hex.h>

#include <iostream>
#include <optional>

#include "base_config.hpp"
#include "community.hpp"
#include "session/config/user_groups.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

using config::community_info;
using config::group_info;
using config::legacy_group_info;
using config::UserGroups;

template <>
struct toJs_impl<community_info> : toJs_impl<config::community> {
    Napi::Object operator()(const Napi::Env& env, const community_info& info_comm) {
        auto obj = toJs_impl<config::community>::operator()(env, info_comm);
        obj["priority"] = toJs(env, info_comm.priority);

        return obj;
    }
};

static Napi::Array members_array(const Napi::Env& env, const std::map<std::string, bool>& members) {
    auto mems = Napi::Array::New(env, members.size());
    size_t i = 0;
    for (const auto& [session_id, is_admin] : members) {
        auto mem = Napi::Object::New(env);
        mem["pubkeyHex"] = toJs(env, session_id);
        mem["isAdmin"] = toJs(env, is_admin);
        mems[i++] = std::move(mem);
    }
    return mems;
}

template <>
struct toJs_impl<legacy_group_info> {
    Napi::Object operator()(const Napi::Env& env, const legacy_group_info& legacy_group) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, legacy_group.session_id);
        obj["name"] = toJs(env, legacy_group.name);
        obj["encPubkey"] = toJs(env, legacy_group.enc_pubkey);
        obj["encSeckey"] = toJs(env, legacy_group.enc_seckey);
        obj["disappearingTimerSeconds"] = toJs(env, legacy_group.disappearing_timer.count());
        obj["priority"] = toJs(env, legacy_group.priority);
        obj["joinedAtSeconds"] = toJs(env, legacy_group.joined_at);
        obj["members"] = members_array(env, legacy_group.members());

        return obj;
    }
};

template <>
struct toJs_impl<group_info> {
    Napi::Object operator()(const Napi::Env& env, const group_info& info) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info.id);
        obj["secretKey"] = toJs(env, info.secretkey);
        obj["priority"] = toJs(env, info.priority);
        obj["joinedAtSeconds"] = toJs(env, info.joined_at);
        obj["name"] = toJs(env, info.name);
        obj["authData"] = toJs(env, info.auth_data);
        obj["invitePending"] = toJs(env, info.invited);
        obj["kicked"] = toJs(env, info.kicked());

        return obj;
    }
};

void UserGroupsWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<UserGroupsWrapper>(
            env,
            exports,
            "UserGroupsWrapperNode",
            {
                    // Communities related methods
                    InstanceMethod(
                            "getCommunityByFullUrl", &UserGroupsWrapper::getCommunityByFullUrl),
                    InstanceMethod(
                            "setCommunityByFullUrl", &UserGroupsWrapper::setCommunityByFullUrl),
                    InstanceMethod("getAllCommunities", &UserGroupsWrapper::getAllCommunities),
                    InstanceMethod(
                            "eraseCommunityByFullUrl", &UserGroupsWrapper::eraseCommunityByFullUrl),
                    InstanceMethod(
                            "buildFullUrlFromDetails", &UserGroupsWrapper::buildFullUrlFromDetails),

                    // Legacy groups related methods
                    InstanceMethod("getLegacyGroup", &UserGroupsWrapper::getLegacyGroup),
                    InstanceMethod("getAllLegacyGroups", &UserGroupsWrapper::getAllLegacyGroups),
                    InstanceMethod("setLegacyGroup", &UserGroupsWrapper::setLegacyGroup),
                    InstanceMethod("eraseLegacyGroup", &UserGroupsWrapper::eraseLegacyGroup),

                    // Groups related methods
                    InstanceMethod("createGroup", &UserGroupsWrapper::createGroup),
                    InstanceMethod("getGroup", &UserGroupsWrapper::getGroup),
                    InstanceMethod("getAllGroups", &UserGroupsWrapper::getAllGroups),
                    InstanceMethod("setGroup", &UserGroupsWrapper::setGroup),
                    InstanceMethod("eraseGroup", &UserGroupsWrapper::eraseGroup),

            });
}

UserGroupsWrapper::UserGroupsWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<UserGroups>(info, "UserGroups")},
        Napi::ObjectWrap<UserGroupsWrapper>{info} {}

/**
 * =================================================
 * ================== COMMUNITIES ==================
 * =================================================
 */

Napi::Value UserGroupsWrapper::getCommunityByFullUrl(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_community(getStringArgs<1>(info)); });
}

void UserGroupsWrapper::setCommunityByFullUrl(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 2);
        auto first = info[0];
        assertIsString(first);
        auto createdOrFound = config.get_or_construct_community(
                toCppString(first, "group.SetCommunityByFullUrl"));

        auto second = info[1];
        assertIsNumber(second, "setCommunityByFullUrl");
        createdOrFound.priority = toPriority(second, createdOrFound.priority);

        config.set(createdOrFound);
    });
}

Napi::Value UserGroupsWrapper::getAllCommunities(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_communities(), config.begin_communities(), config.end());
}

Napi::Value UserGroupsWrapper::eraseCommunityByFullUrl(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto [base, room, pubkey] = config::community::parse_full_url(getStringArgs<1>(info));
        return config.erase_community(base, room);
    });
}

Napi::Value UserGroupsWrapper::buildFullUrlFromDetails(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    return wrapResult(env, [&]() {
        auto [baseUrl, roomId, pubkeyHex] = getStringArgs<3>(info);

        ustring pubkey_bytes;
        if (!oxenc::is_hex(pubkeyHex.begin(), pubkeyHex.end()))
            throw std::invalid_argument{"community pubkey is not hex!"};
        oxenc::from_hex(pubkeyHex.begin(), pubkeyHex.end(), std::back_inserter(pubkey_bytes));

        return toJs(env, config::community::full_url(baseUrl, roomId, pubkey_bytes));
    });
}

/**
 * =================================================
 * ================= LEGACY GROUPS =================
 * =================================================
 */

Napi::Value UserGroupsWrapper::getLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_legacy_group(getStringArgs<1>(info)); });
}

Napi::Value UserGroupsWrapper::getAllLegacyGroups(const Napi::CallbackInfo& info) {
    return get_all_impl(
            info, config.size_legacy_groups(), config.begin_legacy_groups(), config.end());
}

void UserGroupsWrapper::setLegacyGroup(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 1);

        auto legacyGroupValue = info[0];
        assertIsObject(legacyGroupValue);
        auto obj = legacyGroupValue.As<Napi::Object>();

        auto group = config.get_or_construct_legacy_group(
                toCppString(obj.Get("pubkeyHex"), "legacyGroup.set"));

        group.priority = toPriority(obj.Get("priority"), group.priority);
        group.joined_at = std::max<int64_t>(
                toCppInteger(obj.Get("joinedAtSeconds"), "legacyGroupInWrapper.joined_at"),
                group.joined_at);

        if (auto name = maybeNonemptyString(obj.Get("name"), "legacyGroup.set name"))
            group.name = std::move(*name);

        if (auto encPubkey = maybeNonemptyBuffer(obj.Get("encPubkey"), "legacyGroup.set encPubkey"))
            group.enc_pubkey = std::move(*encPubkey);
        if (auto encSeckey = maybeNonemptyBuffer(obj.Get("encSeckey"), "legacyGroup.set encSeckey"))
            group.enc_seckey = std::move(*encSeckey);

        group.disappearing_timer = std::chrono::seconds{toCppInteger(
                obj.Get("disappearingTimerSeconds"),
                "legacyGroup.set disappearingTimerSeconds",
                true)};

        auto membersJSValue = obj.Get("members");
        assertIsArray(membersJSValue);
        auto membersJS = membersJSValue.As<Napi::Array>();
        uint32_t arrayLength = membersJS.Length();
        std::vector<std::pair<std::string, bool>> membersToAddOrUpdate;
        membersToAddOrUpdate.reserve(arrayLength);

        /**
         * `inWrapperButNotInJsAnymore` holds the sessionId of the members currently
         * stored in the wrapper's group before we do any change. Then, while
         * iterating in the ones set from the JS, we also remove them from
         * inWrapperButNotInJsAnymore. After that step, the one still part of the
         * inWrapperButNotInJsAnymore, are the ones which are in the wrapper, but
         * not anymore in the JS side, hence those are the ones we have to remove.
         */
        std::unordered_set<std::string> inWrapperButNotInJsAnymore;
        for (auto& [sid, admin] : group.members())
            inWrapperButNotInJsAnymore.insert(sid);

        for (uint32_t i = 0; i < membersJS.Length(); i++) {
            auto itemValue = membersJS.Get(i);
            assertIsObject(itemValue);
            auto item = itemValue.As<Napi::Object>();

            auto pubkeyHex = item.Get("pubkeyHex");
            auto isAdmin = item.Get("isAdmin");
            assertIsString(pubkeyHex);
            assertIsBoolean(isAdmin);
            bool isAdminCpp = toCppBoolean(isAdmin, "setLegacyGroup");
            std::string pubkeyHexCpp = toCppString(pubkeyHex, "setLegacyGroup");
            membersToAddOrUpdate.emplace_back(pubkeyHexCpp, isAdminCpp);
        }

        for (const auto& [pubkey, admin] : membersToAddOrUpdate) {
            // This updates or add an entry for them, leaving them unchanged otherwise
            group.insert(pubkey, admin);
            inWrapperButNotInJsAnymore.erase(pubkey);
        }

        // at this point we updated members which should already be there or added
        // them into legacyGroupInWrapper from membersJSAsArray

        // now we need to iterate over all the members in legacyGroupInWrapper which
        // are not in membersToAddOrUpdate

        for (auto& sid : inWrapperButNotInJsAnymore) {
            group.erase(sid);
        }

        config.set(group);
    });
}

Napi::Value UserGroupsWrapper::eraseLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_legacy_group(getStringArgs<1>(info)); });
}

/**
 * =================================================
 * ===================== GROUPS ====================
 * =================================================
 */

Napi::Value UserGroupsWrapper::createGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.create_group(); });
}

Napi::Value UserGroupsWrapper::getGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_group(getStringArgs<1>(info)); });
}

Napi::Value UserGroupsWrapper::getAllGroups(const Napi::CallbackInfo& info) {

    return get_all_impl(info, config.size_groups(), config.begin_groups(), config.end());
}

Napi::Value UserGroupsWrapper::setGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        assertIsObject(info[0]);
        auto obj = info[0].As<Napi::Object>();

        if (obj.IsEmpty())
            throw std::invalid_argument("setGroup received empty");

        assertIsString(obj.Get("pubkeyHex"));
        auto groupPk = toCppString(obj.Get("pubkeyHex"), "legacyGroup.set");

        // we should get a `UserGroupsSet` object. If any fields are null, skip updating them.
        // Otherwise, use the corresponding value to update what we got from the
        // `get_or_construct_group` below

        auto group_info = config.get_or_construct_group(groupPk);

        if (auto priority =
                    maybeNonemptyInt(obj.Get("priority"), "UserGroupsWrapper::setGroup priority")) {
            group_info.priority = toPriority(obj.Get("priority"), group_info.priority);
        }

        if (auto joinedAtSeconds = maybeNonemptyInt(
                    obj.Get("joinedAtSeconds"), "UserGroupsWrapper::setGroup joinedAtSeconds")) {
            group_info.joined_at = *joinedAtSeconds;
        }

        if (auto invited = maybeNonemptyBoolean(
                    obj.Get("invitePending"), "UserGroupsWrapper::setGroup invitePending")) {
            group_info.invited = *invited;
        }

        if (auto kicked =
                    maybeNonemptyBoolean(obj.Get("kicked"), "UserGroupsWrapper::setGroup kicked")) {
            if (*kicked) {
                group_info.setKicked();
            }
        }

        if (auto secretKey = maybeNonemptyBuffer(
                    obj.Get("secretKey"), "UserGroupsWrapper::setGroup secretKey")) {
            group_info.secretkey = *secretKey;
        }

        if (auto authData = maybeNonemptyBuffer(
                    obj.Get("authData"), "UserGroupsWrapper::setGroup authData")) {
            group_info.auth_data = *authData;
        }

        if (auto name = maybeNonemptyString(obj.Get("name"), "UserGroupsWrapper::setGroup name")) {
            group_info.name = *name;
        }

        config.set(group_info);

        return config.get_or_construct_group(groupPk);
    });
}

Napi::Value UserGroupsWrapper::eraseGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_group(getStringArgs<1>(info)); });
}

}  // namespace session::nodeapi
