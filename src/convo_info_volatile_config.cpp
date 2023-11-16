#include "convo_info_volatile_config.hpp"

#include <optional>

#include "base_config.hpp"
#include "community.hpp"
#include "session/config/convo_info_volatile.hpp"
#include "session/types.hpp"

namespace session::nodeapi {

namespace convo = config::convo;

using config::ConvoInfoVolatile;

template <>
struct toJs_impl<convo::one_to_one> {
    Napi::Object operator()(const Napi::Env& env, const convo::one_to_one& info_1o1) {

        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info_1o1.session_id);
        obj["unread"] = toJs(env, info_1o1.unread);
        obj["lastRead"] = toJs(env, info_1o1.last_read);

        return obj;
    }
};

template <>
struct toJs_impl<convo::legacy_group> {
    Napi::Object operator()(const Napi::Env& env, const convo::legacy_group info_legacy) {
        auto obj = Napi::Object::New(env);

        obj["pubkeyHex"] = toJs(env, info_legacy.id);
        obj["unread"] = toJs(env, info_legacy.unread);
        obj["lastRead"] = toJs(env, info_legacy.last_read);

        return obj;
    }
};

template <>
struct toJs_impl<convo::community> : toJs_impl<config::community> {
    Napi::Object operator()(const Napi::Env& env, const convo::community info_comm) {
        auto obj = toJs_impl<config::community>::operator()(env, info_comm);
        obj["unread"] = toJs(env, info_comm.unread);
        obj["lastRead"] = toJs(env, info_comm.last_read);
        return obj;
    }
};

void ConvoInfoVolatileWrapper::Init(Napi::Env env, Napi::Object exports) {
    InitHelper<ConvoInfoVolatileWrapper>(
            env,
            exports,
            "ConvoInfoVolatileWrapperNode",
            {
                    // 1o1 related methods
                    InstanceMethod("get1o1", &ConvoInfoVolatileWrapper::get1o1),
                    InstanceMethod("getAll1o1", &ConvoInfoVolatileWrapper::getAll1o1),
                    InstanceMethod("set1o1", &ConvoInfoVolatileWrapper::set1o1),
                    InstanceMethod("erase1o1", &ConvoInfoVolatileWrapper::erase1o1),

                    // legacy group related methods
                    InstanceMethod("getLegacyGroup", &ConvoInfoVolatileWrapper::getLegacyGroup),
                    InstanceMethod(
                            "getAllLegacyGroups", &ConvoInfoVolatileWrapper::getAllLegacyGroups),
                    InstanceMethod("setLegacyGroup", &ConvoInfoVolatileWrapper::setLegacyGroup),
                    InstanceMethod("eraseLegacyGroup", &ConvoInfoVolatileWrapper::eraseLegacyGroup),

                    // communities related methods
                    InstanceMethod("getCommunity", &ConvoInfoVolatileWrapper::getCommunity),
                    InstanceMethod(
                            "getAllCommunities", &ConvoInfoVolatileWrapper::getAllCommunities),
                    InstanceMethod(
                            "setCommunityByFullUrl",
                            &ConvoInfoVolatileWrapper::setCommunityByFullUrl),
                    InstanceMethod(
                            "eraseCommunityByFullUrl",
                            &ConvoInfoVolatileWrapper::eraseCommunityByFullUrl),
            });
}

ConvoInfoVolatileWrapper::ConvoInfoVolatileWrapper(const Napi::CallbackInfo& info) :
        ConfigBaseImpl{construct<ConvoInfoVolatile>(info, "ConvoInfoVolatile")},
        Napi::ObjectWrap<ConvoInfoVolatileWrapper>{info} {}

/**
 * =================================================
 * ====================== 1o1 ======================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::get1o1(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_1to1(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAll1o1(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_1to1(), config.begin_1to1(), config.end());
}

void ConvoInfoVolatileWrapper::set1o1(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 3);
        auto first = info[0];
        assertIsString(first);

        auto second = info[1];
        assertIsNumber(second, "set1o1");

        auto third = info[2];
        assertIsBoolean(third);

        auto convo = config.get_or_construct_1to1(toCppString(first, "convoInfo.set1o1"));

        if (auto last_read = toCppInteger(second, "convoInfo.set1o1_2");
            last_read > convo.last_read)
            convo.last_read = last_read;
        convo.unread = toCppBoolean(third, "convoInfo.set1o1_3");

        config.set(convo);
    });
}

/**
 * =================================================
 * ================= Legacy groups =================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::getLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_legacy_group(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAllLegacyGroups(const Napi::CallbackInfo& info) {
    return get_all_impl(
            info, config.size_legacy_groups(), config.begin_legacy_groups(), config.end());
}

void ConvoInfoVolatileWrapper::setLegacyGroup(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 3);
        auto first = info[0];
        assertIsString(first);
        auto second = info[1];
        assertIsNumber(second, "setLegacyGroup");

        auto third = info[2];
        assertIsBoolean(third);

        auto convo = config.get_or_construct_legacy_group(
                toCppString(first, "convoInfo.SetLegacyGroup1"));

        if (auto last_read = toCppInteger(second, "convoInfo.SetLegacyGroup2");
            last_read > convo.last_read)
            convo.last_read = last_read;

        convo.unread = toCppBoolean(third, "convoInfo.SetLegacyGroup3");

        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::eraseLegacyGroup(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_legacy_group(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::erase1o1(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.erase_1to1(getStringArgs<1>(info)); });
}

/**
 * =================================================
 * ================== Communities ==================
 * =================================================
 */

Napi::Value ConvoInfoVolatileWrapper::getCommunity(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return config.get_community(getStringArgs<1>(info)); });
}

Napi::Value ConvoInfoVolatileWrapper::getAllCommunities(const Napi::CallbackInfo& info) {
    return get_all_impl(info, config.size_communities(), config.begin_communities(), config.end());
}

// TODO maybe make the setXXX   return the update value so we avoid having to
// fetch again updated values from the renderer

void ConvoInfoVolatileWrapper::setCommunityByFullUrl(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&] {
        assertInfoLength(info, 3);
        auto first = info[0];
        assertIsString(first);

        auto second = info[1];
        assertIsNumber(second, "setCommunityByFullUrl");

        auto third = info[2];
        assertIsBoolean(third);

        auto convo = config.get_or_construct_community(
                toCppString(first, "convoInfo.SetCommunityByFullUrl1"));

        if (auto last_read = toCppInteger(second, "convoInfo.SetCommunityByFullUrl2");
            last_read > convo.last_read)
            convo.last_read = last_read;

        convo.unread = toCppBoolean(third, "convoInfo.SetCommunityByFullUrl3");

        // Note: we only keep the messages read when their timestamp is not older
        // than 30 days or so (see libsession util PRUNE constant). so this `set()`
        // here might actually not create an entry
        config.set(convo);
    });
}

Napi::Value ConvoInfoVolatileWrapper::eraseCommunityByFullUrl(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto [base, room, pubkey] = config::community::parse_full_url(getStringArgs<1>(info));
        return config.erase_community(base, room);
    });
}

}  // namespace session::nodeapi
