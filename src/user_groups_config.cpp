#include "user_groups_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/types.hpp"

#include <iostream>
#include <optional>

using namespace Nan;
using namespace v8;

using namespace std;
using namespace session;

session::config::UserGroups *
getUserGroupsWrapperOrThrow(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  auto userGroups =
      ConfigBaseWrapperInsideWorker::to<session::config::UserGroups>(info);

  if (!userGroups) {
    throw std::invalid_argument("userGroups wrapper is empty");
  }
  return userGroups;
}

Local<Object> toJSCommunity(const community_info community) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("pubkeyHex"),
                         toJsString(community.pubkey_hex())); // in hex

  result = obj->Set(context, toJsString("baseUrl"),
                    toJsString(community.base_url()));

  result = obj->Set(context, toJsString("roomCasePreserved"),
                    toJsString(community.room()));

  result = obj->Set(context, toJsString("fullUrl"),
                    toJsString(community.full_url()));
  result =
      obj->Set(context, toJsString("priority"), toJsNumber(community.priority));

  return obj;
}

Local<Object> toJSLegacyGroupMember(const std::pair<std::string, bool> member) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result =
      obj->Set(context, toJsString("pubkeyHex"), toJsString(member.first));

  result = obj->Set(context, toJsString("isAdmin"), toJsBoolean(member.second));

  return obj;
}

Local<Object> toJSLegacyGroup(const legacy_group_info legacy_group) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("pubkeyHex"),
                         toJsString(legacy_group.session_id));

  result = obj->Set(context, toJsString("name"), toJsString(legacy_group.name));
  result = obj->Set(context, toJsString("encPubkey"),
                    toJsBuffer(legacy_group.enc_pubkey));
  result = obj->Set(context, toJsString("encSeckey"),
                    toJsBuffer(legacy_group.enc_seckey));
  result = obj->Set(context, toJsString("disappearingTimerSeconds"),
                    toJsNumber(legacy_group.disappearing_timer.count()));
  result = obj->Set(context, toJsString("priority"),
                    toJsNumber(legacy_group.priority));

  auto length = legacy_group.members().size();

  Local<Array> allMembers = Nan::New<Array>(length);
  int index = 0;

  for (auto member : legacy_group.members()) {
    ignore_result(allMembers->Set(Nan::GetCurrentContext(), index,
                                  toJSLegacyGroupMember(member)));
    index++;
  }
  result = obj->Set(context, toJsString("members"), allMembers);

  return obj;
}

NAN_MODULE_INIT(UserGroupsWrapperInsideWorker::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("UserGroupsWrapperInsideWorker").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // communities related methods
  RegisterNANMethods(tpl, "getCommunityByFullUrl", GetCommunityByFullUrl);
  RegisterNANMethods(tpl, "setCommunityByFullUrl", SetCommunityByFullUrl);
  RegisterNANMethods(tpl, "getAllCommunities", GetAllCommunities);
  RegisterNANMethods(tpl, "eraseCommunityByFullUrl", EraseCommunityByFullUrl);
  RegisterNANMethods(tpl, "buildFullUrlFromDetails", BuildFullUrlFromDetails);

  // legacy-groups related methods
  RegisterNANMethods(tpl, "getLegacyGroup", GetLegacyGroup);
  RegisterNANMethods(tpl, "getAllLegacyGroups", GetAllLegacyGroups);
  RegisterNANMethods(tpl, "setLegacyGroup", SetLegacyGroup);
  RegisterNANMethods(tpl, "eraseLegacyGroup", EraseLegacyGroup);

  Nan::Set(target, Nan::New("UserGroupsWrapperInsideWorker").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(UserGroupsWrapperInsideWorker::New) {
  tryOrWrapStdException([&]() {
    if (info.IsConstructCall()) {
      assertInfoLength(info, 2);
      auto first = info[0];
      auto second = info[1];

      // we should get secret key as first arg and optional dumped as second
      // argument
      assertIsUInt8Array(first);
      assertIsUInt8ArrayOrNull(second);

      ustring secretKey = toCppBuffer(first, "groups.new");
      bool dumpIsSet = !second.IsEmpty() && !second->IsNullOrUndefined();
      if (dumpIsSet) {
        ustring dumped = toCppBuffer(second, "groups.new");
        UserGroupsWrapperInsideWorker *obj =
            new UserGroupsWrapperInsideWorker(secretKey, dumped);
        obj->Wrap(info.This());
      } else {

        UserGroupsWrapperInsideWorker *obj =
            new UserGroupsWrapperInsideWorker(secretKey, std::nullopt);
        obj->Wrap(info.This());
      }

      info.GetReturnValue().Set(info.This());
      return;
    } else {
      throw std::invalid_argument(
          "You need to call the constructor with the `new` syntax");
    }
  });
}

/**
 * =================================================
 * ================== COMMUNITIES ==================
 * =================================================
 */

NAN_METHOD(UserGroupsWrapperInsideWorker::GetCommunityByFullUrl) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string fullUrl = toCppString(first, "group.GetCommunityByFullUrl");

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);
    // this will fail if the pubkey is not in the string but is not
    auto found = userGroups->get_community(fullUrl);

    if (found) {
      info.GetReturnValue().Set(toJSCommunity(*found));
      return;
    }
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::SetCommunityByFullUrl) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);
    auto first = info[0];
    assertIsString(first);
    std::string fullUrl = toCppString(first, "group.SetCommunityByFullUrl");

    auto second = info[1];
    assertIsNumber(second);
    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);

    auto createdOrFound = userGroups->get_or_construct_community(fullUrl);
    createdOrFound.priority = toPriority(second, createdOrFound.priority);

    userGroups->set(createdOrFound);

    info.GetReturnValue().Set(Nan::Null());

    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::GetAllCommunities) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);
    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);

    auto length = userGroups->size_communities();

    Local<Array> allCommunities = Nan::New<Array>(length);
    int index = 0;

    for (auto &group : *userGroups) {
      if (auto *comm = std::get_if<community_info>(&group)) {
        ignore_result(allCommunities->Set(Nan::GetCurrentContext(), index,
                                          toJSCommunity((*comm))));
        index++;
      }
    }

    info.GetReturnValue().Set(allCommunities);

    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::EraseCommunityByFullUrl) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsString(first);
    std::string fullUrl = toCppString(first, "group.EraseCommunityByFullUrl");
    auto [base, room, pubkey] = community::parse_full_url(fullUrl);

    auto userGroups = getUserGroupsWrapperOrThrow(info);
    userGroups->erase_community(base, room);
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::BuildFullUrlFromDetails) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 3);

    auto first = info[0];
    assertIsString(first);
    std::string baseUrl = toCppString(first, "group.BuildFullUrlFromDetails");

    auto second = info[1];
    assertIsString(second);
    std::string roomId = toCppString(second, "group.BuildFullUrlFromDetails2");

    auto third = info[2];
    assertIsString(third);
    std::string pubkeyHex =
        toCppString(third, "group.BuildFullUrlFromDetails3");
    ustring pubkey_bytes;
    oxenc::from_hex(pubkeyHex.begin(), pubkeyHex.end(),
                    std::back_inserter(pubkey_bytes));

    std::string full_url = community::full_url(baseUrl, roomId, pubkey_bytes);
    info.GetReturnValue().Set(toJsString(full_url));
    return;
  });
}

/**
 * =================================================
 * ================= LEGACY GROUPS =================
 * =================================================
 */

NAN_METHOD(UserGroupsWrapperInsideWorker::GetLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string pubkeyHex = toCppString(first, "group.GetLegacyGroup");

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);
    auto found = userGroups->get_legacy_group(pubkeyHex);

    if (found) {
      info.GetReturnValue().Set(toJSLegacyGroup(*found));
      return;
    }
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::GetAllLegacyGroups) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);
    auto length = userGroups->size_legacy_groups();

    Local<Array> allLegacyGroups = Nan::New<Array>(length);
    int index = 0;

    for (auto &group : *userGroups) {
      if (auto *legacyGroup = std::get_if<legacy_group_info>(&group)) {
        ignore_result(allLegacyGroups->Set(Nan::GetCurrentContext(), index,
                                           toJSLegacyGroup((*legacyGroup))));
        index++;
      }
    }

    info.GetReturnValue().Set(allLegacyGroups);

    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::SetLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);

    auto legacyGroupValue = info[0];
    assertIsObject(legacyGroupValue);

    if (legacyGroupValue.IsEmpty()) {
      throw std::invalid_argument("SetLegacyGroup received empty group object");
    }

    Local<Object> legacyGroup =
        Nan::To<Object>(legacyGroupValue).ToLocalChecked();

    Nan::MaybeLocal<Value> sessionIdMaybe =
        Nan::Get(legacyGroup, toJsString("pubkeyHex"));

    if (sessionIdMaybe.IsEmpty() ||
        sessionIdMaybe.ToLocalChecked()->IsNullOrUndefined()) {
      throw std::invalid_argument("SessionID is empty");
    }

    Local<Value> sessionId = sessionIdMaybe.ToLocalChecked();
    assertIsString(sessionId);
    std::string sessionIdStr = toCppString(sessionId, "legacyGroup.set");

    legacy_group_info legacyGroupInWrapper =
        userGroups->get_or_construct_legacy_group(sessionIdStr);

    // TODO desktop does not understand what is a `hidden` legacy group
    // currently. So better not override whatever anyone else is setting here

    auto priority = toPriority(
        (Nan::Get(legacyGroup, toJsString("priority"))).ToLocalChecked(),
        legacyGroupInWrapper.priority);
    legacyGroupInWrapper.priority = priority;

    auto name = Nan::Get(legacyGroup, toJsString("name"));
    if (!name.IsEmpty() && !name.ToLocalChecked()->IsNullOrUndefined()) {
      // We need to store it as a string and not directly the  string_view
      // otherwise it gets garbage collected
      auto nameStr = toCppString(name.ToLocalChecked(), "legacyGroup.set1");
      legacyGroupInWrapper.name = nameStr;
    }

    auto encPubkey = Nan::Get(legacyGroup, toJsString("encPubkey"));
    if (!encPubkey.IsEmpty() &&
        !encPubkey.ToLocalChecked()->IsNullOrUndefined()) {

      auto enc_pubkey =
          toCppBuffer(encPubkey.ToLocalChecked(), "legacyGroup.set1");
      legacyGroupInWrapper.enc_pubkey = enc_pubkey;
    }

    auto encSeckey = Nan::Get(legacyGroup, toJsString("encSeckey"));
    if (!encSeckey.IsEmpty() &&
        !encSeckey.ToLocalChecked()->IsNullOrUndefined()) {

      auto enc_seckey =
          toCppBuffer(encSeckey.ToLocalChecked(), "legacyGroup.set2");
      legacyGroupInWrapper.enc_seckey = enc_seckey;
    }

    auto disappearingTimerSeconds =
        Nan::Get(legacyGroup, toJsString("disappearingTimerSeconds"));
    if (!disappearingTimerSeconds.IsEmpty() &&
        !disappearingTimerSeconds.ToLocalChecked()->IsNullOrUndefined()) {

      legacyGroupInWrapper.disappearing_timer = std::chrono::seconds(
          toCppInteger(disappearingTimerSeconds.ToLocalChecked(),
                       "legacyGroup.set3", true));
    } else {
      legacyGroupInWrapper.disappearing_timer = std::chrono::seconds(0);
    }

    auto membersJS = Nan::Get(legacyGroup, toJsString("members"));

    if (membersJS.IsEmpty() ||
        membersJS.ToLocalChecked()->IsNullOrUndefined()) {
      throw std::invalid_argument(
          "set legacy group members must be a defined array");
    }
    assertIsArray(membersJS.ToLocalChecked());

    Local<Array> membersJSAsArray = membersJS.ToLocalChecked().As<Array>();
    uint32_t arrayLength = membersJSAsArray->Length();
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
    for (auto &[sid, admin] : legacyGroupInWrapper.members())
      inWrapperButNotInJsAnymore.insert(sid);

    for (uint32_t i = 0; i < membersJSAsArray->Length(); i++) {
      Local<Value> item =
          membersJSAsArray->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      assertIsObject(item);
      if (item.IsEmpty()) {
        throw std::invalid_argument("setLegacyGroup.item received empty");
      }

      Local<Object> itemObject = Nan::To<Object>(item).ToLocalChecked();

      auto pubkeyHex =
          Nan::Get(itemObject, toJsString("pubkeyHex")).ToLocalChecked();
      auto isAdmin =
          Nan::Get(itemObject, toJsString("isAdmin")).ToLocalChecked();
      assertIsString(pubkeyHex);
      assertIsBoolean(isAdmin);

      std::string pubkeyHexCpp = toCppString(pubkeyHex, "setLegacyGroup");
      bool isAdminCpp = toCppBoolean(isAdmin, "setLegacyGroup");
      std::pair<std::string, bool> memberPair =
          std::make_pair(pubkeyHexCpp, isAdminCpp);

      membersToAddOrUpdate.push_back(memberPair);
    }

    for (auto &member : membersToAddOrUpdate) {
      // This updates or add an entry for them, leaving them unchanged otherwise
      legacyGroupInWrapper.insert(member.first, member.second);
      inWrapperButNotInJsAnymore.erase(member.first);
    }

    // at this point we updated members which should already be there or added
    // them into legacyGroupInWrapper from membersJSAsArray

    // now we need to iterate over all the members in legacyGroupInWrapper which
    // are not in membersToAddOrUpdate

    std::vector<std::string> membersToRemove;
    for (auto &sid : inWrapperButNotInJsAnymore) {
      legacyGroupInWrapper.erase(sid);
    }

    userGroups->set(legacyGroupInWrapper);
    info.GetReturnValue().Set(Nan::Null());

    return;
  });
}

NAN_METHOD(UserGroupsWrapperInsideWorker::EraseLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    assertIsString(info[0]);

    std::string toRemove = toCppString(info[0], __FUNCTION__);

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);
    bool removed = userGroups->erase_legacy_group(toRemove);

    info.GetReturnValue().Set(toJsBoolean(removed));

    return;
  });
}