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

NAN_MODULE_INIT(UserGroupsWrapperInsideWorker::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("UserGroupsWrapperInsideWorker").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "getCommunityByFullUrl", GetCommunityByFullUrl);
  RegisterNANMethods(tpl, "setCommunityByFullUrl", SetCommunityByFullUrl);
  RegisterNANMethods(tpl, "getAllCommunities", GetAllCommunities);
  RegisterNANMethods(tpl, "eraseCommunityByFullUrl", EraseCommunityByFullUrl);
  RegisterNANMethods(tpl, "buildFullUrlFromDetails", BuildFullUrlFromDetails);

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

/** ==============================
 *             GETTERS
 * ============================== */

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
    int64_t priority =
        toCppInteger(second, "group.SetCommunityByFullUrl", false);

    session::config::UserGroups *userGroups = getUserGroupsWrapperOrThrow(info);

    auto createdOrFound = userGroups->get_or_construct_community(fullUrl);
    createdOrFound.priority = priority;

    userGroups->set(createdOrFound);

    info.GetReturnValue().Set(toJSCommunity(createdOrFound));

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

/** ==============================
 *             ERASERS
 * ============================== */

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