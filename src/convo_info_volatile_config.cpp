#include "convo_info_volatile_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/config/convo_info_volatile.hpp"
#include "session/types.hpp"

#include <iostream>
#include <optional>

using namespace Nan;
using namespace v8;

using namespace std;
using namespace session;

session::config::ConvoInfoVolatile *getConvoInfoVolatileWrapperOrThrow(
    const Nan::FunctionCallbackInfo<v8::Value> &info) {
  auto convoInfoVolatile =
      ConfigBaseWrapperInsideWorker::to<session::config::ConvoInfoVolatile>(
          info);

  if (!convoInfoVolatile) {
    throw std::invalid_argument("convoInfoVolatile wrapper is empty");
  }
  return convoInfoVolatile;
}

Local<Object> toJSConvoVolatile1o1(const convo::one_to_one info_1o1) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("pubkeyHex"),
                         toJsString(info_1o1.session_id)); // in hex

  result =
      obj->Set(context, toJsString("unread"), toJsBoolean(info_1o1.unread));

  result =
      obj->Set(context, toJsString("lastRead"), toJsNumber(info_1o1.last_read));

  return obj;
}

Local<Object>
toJSConvoVolatileLegacyGroup(const convo::legacy_group info_legacy) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("pubkeyHex"),
                         toJsString(info_legacy.id)); // in hex

  result = obj->Set(context, toJsString("."), toJsBoolean(info_legacy.unread));

  result = obj->Set(context, toJsString("lastRead"),
                    toJsNumber(info_legacy.last_read));

  return obj;
}

Local<Object> toJSConvoVolatileCommunity(const convo::community info_comm) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("fullUrlWithPubkey"),
                         toJsString(info_comm.full_url()));

  result = obj->Set(context, toJsString("baseUrl"),
                    toJsString(info_comm.base_url()));

  result = obj->Set(context, toJsString("roomCasePreserved"),
                    toJsString(info_comm.room()));

  result =
      obj->Set(context, toJsString("unread"), toJsBoolean(info_comm.unread));

  result = obj->Set(context, toJsString("lastRead"),
                    toJsNumber(info_comm.last_read));

  return obj;
}

NAN_MODULE_INIT(ConvoInfoVolatileWrapperInsideWorker::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(
      Nan::New("ConvoInfoVolatileWrapperInsideWorker").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // 1o1 related methods
  RegisterNANMethods(tpl, "get1o1", Get1o1);
  RegisterNANMethods(tpl, "getAll1o1", GetAll1o1);
  RegisterNANMethods(tpl, "set1o1", Set1o1);

  // legacy group related methods
  RegisterNANMethods(tpl, "getLegacyGroup", GetLegacyGroup);
  RegisterNANMethods(tpl, "getAllLegacyGroups", GetAllLegacyGroups);
  RegisterNANMethods(tpl, "setLegacyGroup", SetLegacyGroup);
  RegisterNANMethods(tpl, "eraseLegacyGroup", EraseLegacyGroup);

  // communities related methods
  RegisterNANMethods(tpl, "getCommunity", GetCommunity);
  RegisterNANMethods(tpl, "getAllCommunities", GetAllCommunities);
  RegisterNANMethods(tpl, "setCommunityByFullUrl", SetCommunityByFullUrl);
  RegisterNANMethods(tpl, "eraseCommunityByFullUrl", EraseCommunityByFullUrl);

  Nan::Set(target,
           Nan::New("ConvoInfoVolatileWrapperInsideWorker").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::New) {
  tryOrWrapStdException([&]() {
    if (!info.IsConstructCall()) {
      throw std::invalid_argument(
          "You need to call the constructor with the `new` syntax");
    }
    assertInfoLength(info, 2);
    auto first = info[0];
    auto second = info[1];

    // we should get secret key as first arg and optional dumped as second
    // argument
    assertIsUInt8Array(first);
    assertIsUInt8ArrayOrNull(second);

    ustring secretKey = toCppBuffer(first, "convoInfoVolatile.new1");
    bool dumpIsSet = !second.IsEmpty() && !second->IsNullOrUndefined();
    if (dumpIsSet) {
      ustring dumped = toCppBuffer(second, "convoInfoVolatile.new2");
      ConvoInfoVolatileWrapperInsideWorker *obj =
          new ConvoInfoVolatileWrapperInsideWorker(secretKey, dumped);
      obj->Wrap(info.This());
    } else {

      ConvoInfoVolatileWrapperInsideWorker *obj =
          new ConvoInfoVolatileWrapperInsideWorker(secretKey, std::nullopt);
      obj->Wrap(info.This());
    }

    info.GetReturnValue().Set(info.This());
    return;
  });
}

/**
 * =================================================
 * ====================== 1o1 ======================
 * =================================================
 */

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::Get1o1) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sesionIdHex = toCppString(first, "convoInfoVolatile.Get1o1");

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto found = convoVolatileInfo->get_1to1(sesionIdHex);

    if (found) {
      info.GetReturnValue().Set(toJSConvoVolatile1o1(*found));
      return;
    }
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::GetAll1o1) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto length = convoVolatileInfo->size_1to1();

    Local<Array> toReturn = Nan::New<Array>(length);
    int index = 0;

    for (auto &convo : *convoVolatileInfo) {
      if (auto *convo1o1 = std::get_if<convo::one_to_one>(&convo)) {
        ignore_result(toReturn->Set(Nan::GetCurrentContext(), index,
                                    toJSConvoVolatile1o1((*convo1o1))));
        index++;
      }
    }

    info.GetReturnValue().Set(toReturn);

    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::Set1o1) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 3);
    auto first = info[0];
    assertIsString(first);

    auto second = info[1];
    assertIsNumber(second);

    auto third = info[2];
    assertIsBoolean(third);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);

    auto createdOrFound = convoVolatileInfo->get_or_construct_1to1(
        toCppString(first, "convoInfo.Set1o1"));

    auto lastReadTimestampFromJs =
        toCppInteger(second, "convoInfo.Set1o1_2", false);

    createdOrFound.last_read =
        std::max(createdOrFound.last_read, lastReadTimestampFromJs);

    createdOrFound.unread = toCppBoolean(third, "convoInfo.Set1o1_3");

    convoVolatileInfo->set(createdOrFound);

    info.GetReturnValue().Set(Nan::Null());

    return;
  });
}

/**
 * =================================================
 * ================= Legacy groups =================
 * =================================================
 */

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::GetLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHex =
        toCppString(first, "convoInfoVolatile.GetLegacyGroup");

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto found = convoVolatileInfo->get_legacy_group(sessionIdHex);

    if (found) {
      info.GetReturnValue().Set(toJSConvoVolatileLegacyGroup(*found));
      return;
    }
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::GetAllLegacyGroups) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto length = convoVolatileInfo->size_legacy_groups();

    Local<Array> toReturn = Nan::New<Array>(length);
    int index = 0;

    for (auto &convo : *convoVolatileInfo) {
      if (auto *convo_legacy_group = std::get_if<convo::legacy_group>(&convo)) {
        ignore_result(
            toReturn->Set(Nan::GetCurrentContext(), index,
                          toJSConvoVolatileLegacyGroup((*convo_legacy_group))));
        index++;
      }
    }

    info.GetReturnValue().Set(toReturn);

    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::SetLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 3);
    auto first = info[0];
    assertIsString(first);
    // TODO notification
    auto second = info[1];
    assertIsNumber(second);

    auto third = info[2];
    assertIsBoolean(third);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);

    auto createdOrFound = convoVolatileInfo->get_or_construct_legacy_group(
        toCppString(first, "convoInfo.SetLegacyGroup1"));

    auto lastReadTimestampFromJs =
        toCppInteger(second, "convoInfo.SetLegacyGroup2", false);

    createdOrFound.last_read =
        std::max(createdOrFound.last_read, lastReadTimestampFromJs);
    createdOrFound.unread = toCppBoolean(third, "convoInfo.SetLegacyGroup3");

    convoVolatileInfo->set(createdOrFound);

    info.GetReturnValue().Set(Nan::Null());

    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::EraseLegacyGroup) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    assertIsString(info[0]);

    std::string toRemove = toCppString(info[0], __FUNCTION__);

    session::config::ConvoInfoVolatile *convoInfoVolatile =
        getConvoInfoVolatileWrapperOrThrow(info);
    bool removed = convoInfoVolatile->erase_legacy_group(toRemove);

    info.GetReturnValue().Set(toJsBoolean(removed));

    return;
  });
}

/**
 * =================================================
 * ================== Communities ==================
 * =================================================
 */

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::GetCommunity) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string communityFullUrl =
        toCppString(first, "convoInfoVolatile.GetCommunity");

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto found = convoVolatileInfo->get_community(communityFullUrl);

    if (found) {
      info.GetReturnValue().Set(toJSConvoVolatileCommunity(*found));
      return;
    }
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::GetAllCommunities) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    auto length = convoVolatileInfo->size_communities();

    Local<Array> toReturn = Nan::New<Array>(length);
    int index = 0;

    for (auto &convo : *convoVolatileInfo) {
      if (auto *convo_comm = std::get_if<convo::community>(&convo)) {
        ignore_result(toReturn->Set(Nan::GetCurrentContext(), index,
                                    toJSConvoVolatileCommunity((*convo_comm))));
        index++;
      }
    }

    info.GetReturnValue().Set(toReturn);

    return;
  });
}

// TODO maybe make the setXXX   return the update value so we avoid having to
// fetch again updated values from the renderer

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::SetCommunityByFullUrl) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 3);
    Local<Value> first = info[0];
    assertIsString(first);

    Local<Value> second = info[1];
    assertIsNumber(second);

    Local<Value> third = info[2];
    assertIsBoolean(third);

    session::config::ConvoInfoVolatile *convos =
        getConvoInfoVolatileWrapperOrThrow(info);

    std::string communityFullUrl =
        toCppString(first, "convoInfo.SetCommunityByFullUrl1");

    session::config::convo::community createdOrFound =
        convos->get_or_construct_community(communityFullUrl);

    auto lastReadTimestampFromJs =
        toCppInteger(second, "convoInfo.SetCommunityByFullUrl2", false);

    createdOrFound.last_read =
        std::max(createdOrFound.last_read, lastReadTimestampFromJs);

    createdOrFound.unread =
        toCppBoolean(third, "convoInfo.SetCommunityByFullUrl3");

    // Note: we only keep the messages read when their timestamp is not older
    // than 30 days or so (see libsession util PRUNE constant). so this `set()`
    // here might actually not create an entry
    convos->set(createdOrFound);
    info.GetReturnValue().Set(Nan::Null());
    return;
  });
}

NAN_METHOD(ConvoInfoVolatileWrapperInsideWorker::EraseCommunityByFullUrl) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsString(first);
    std::string fullUrl =
        toCppString(first, "infoVolatile.EraseCommunityByFullUrl");
    auto [base, room, pubkey] = community::parse_full_url(fullUrl);

    session::config::ConvoInfoVolatile *convoVolatileInfo =
        getConvoInfoVolatileWrapperOrThrow(info);
    convoVolatileInfo->erase_community(base, room);
  });
}