#include "contacts_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/types.hpp"

#include <iostream>
#include <optional>

using namespace Nan;
using namespace v8;

using session::ustring;
using session::config::contact_info;
using session::config::profile_pic;

using session::config::Contacts;
using namespace std;

session::config::Contacts *
getContactWrapperOrThrow(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  auto contacts =
      ConfigBaseWrapperInsideWorker::to<session::config::Contacts>(info);

  if (!contacts) {
    throw std::invalid_argument("contacts wrapper is empty");
  }
  return contacts;
}

Local<Object> toJSContact(const contact_info contact) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("id"),
                         toJsString(contact.session_id)); // in hex

  if (!contact.name.empty()) {
    result = obj->Set(context, toJsString("name"), toJsString(contact.name));

  } else {
    result = obj->Set(context, toJsString("name"), Nan::Null());
  }

  if (!contact.nickname.empty()) {
    result =
        obj->Set(context, toJsString("nickname"), toJsString(contact.nickname));
  } else {
    result = obj->Set(context, toJsString("nickname"), Nan::Null());
  }

  result = obj->Set(context, toJsString("approved"),
                    Nan::New<Boolean>(contact.approved));
  result = obj->Set(context, toJsString("approvedMe"),
                    Nan::New<Boolean>(contact.approved_me));
  result = obj->Set(context, toJsString("blocked"),
                    Nan::New<Boolean>(contact.blocked));
  result = obj->Set(context, toJsString("hidden"),
                    Nan::New<Boolean>(contact.hidden));
  result = obj->Set(context, toJsString("priority"),
                    Nan::New<Number>(contact.priority));

  if (contact.profile_picture) {
    Local<Object> profilePic = Nan::New<Object>();
    result = profilePic->Set(context, toJsString("url"),
                             toJsString(contact.profile_picture.url));
    result = profilePic->Set(context, toJsString("key"),
                             toJsBuffer(contact.profile_picture.key));

    result = obj->Set(context, toJsString("profilePicture"), profilePic);
  }

  return obj;
}

NAN_MODULE_INIT(ContactsConfigWrapperInsideWorker::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(
      Nan::New("ContactsConfigWrapperInsideWorker").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "get", Get);
  RegisterNANMethods(tpl, "getOrConstruct", GetOrConstruct);
  RegisterNANMethods(tpl, "getAll", GetAll);
  RegisterNANMethods(tpl, "set", Set);
  RegisterNANMethods(tpl, "erase", Erase);

  Nan::Set(target,
           Nan::New("ContactsConfigWrapperInsideWorker").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ContactsConfigWrapperInsideWorker::New) {
  tryOrWrapStdException([&]() {
    if (info.IsConstructCall()) {
      assertInfoLength(info, 2);
      auto first = info[0];
      auto second = info[1];

      // we should get secret key as first arg and optional dumped as second
      // argument
      assertIsUInt8Array(first);
      assertIsUInt8ArrayOrNull(second);

      ustring secretKey = toCppBuffer(first, "contacts.new");
      bool dumpIsSet = !second.IsEmpty() && !second->IsNullOrUndefined();
      if (dumpIsSet) {
        ustring dumped = toCppBuffer(second, "contacts.new");
        ContactsConfigWrapperInsideWorker *obj =
            new ContactsConfigWrapperInsideWorker(secretKey, dumped);
        obj->Wrap(info.This());
      } else {

        ContactsConfigWrapperInsideWorker *obj =
            new ContactsConfigWrapperInsideWorker(secretKey, std::nullopt);
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

NAN_METHOD(ContactsConfigWrapperInsideWorker::Get) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first, "contacts.get");
    auto contacts = getContactWrapperOrThrow(info);

    optional<contact_info> contact = contacts->get(sessionIdHexStr);

    if (contact) {
      info.GetReturnValue().Set(toJSContact(*contact));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }

    return;
  });
}

NAN_METHOD(ContactsConfigWrapperInsideWorker::GetOrConstruct) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first, "contacts.getOrConstruct");

    auto contacts = getContactWrapperOrThrow(info);

    auto contact = contacts->get_or_construct(sessionIdHexStr);

    info.GetReturnValue().Set(toJSContact(contact));

    return;
  });
}

NAN_METHOD(ContactsConfigWrapperInsideWorker::GetAll) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);
    auto contacts = to<session::config::Contacts>(info);

    if (!contacts) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto length = contacts->size();

    Local<Array> allContacts = Nan::New<Array>(length);
    int index = 0;
    for (auto &contact : *contacts) {
      ignore_result(allContacts->Set(Nan::GetCurrentContext(), index,
                                     toJSContact((contact))));
      index++;
    }

    info.GetReturnValue().Set(allContacts);

    return;
  });
}

/** ==============================
 *             SETTERS
 * ============================== */

NAN_METHOD(ContactsConfigWrapperInsideWorker::Set) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto contactValue = info[0];
    assertIsObject(contactValue);

    if (contactValue.IsEmpty()) {
      throw std::invalid_argument("cppContact received empty");
    }

    Contacts *contacts = to<Contacts>(info);
    if (!contacts) {
      return;
    }

    Local<Object> contact = Nan::To<Object>(contactValue).ToLocalChecked();

    Nan::MaybeLocal<Value> sessionIdMaybe = Nan::Get(contact, toJsString("id"));

    if (sessionIdMaybe.IsEmpty() ||
        sessionIdMaybe.ToLocalChecked()->IsNullOrUndefined()) {
      throw std::invalid_argument("SessionID is empty");
    }

    Local<Value> sessionId = sessionIdMaybe.ToLocalChecked();
    assertIsString(sessionId);
    std::string sessionIdStr = toCppString(sessionId, "contacts.set");

    contact_info contactCpp = contacts->get_or_construct(sessionIdStr);

    contactCpp.approved = toCppBoolean(
        (Nan::Get(contact, toJsString("approved"))).ToLocalChecked(),
        "set.approved");

    contactCpp.approved_me = toCppBoolean(
        (Nan::Get(contact, toJsString("approvedMe"))).ToLocalChecked(),
        "set.approvedMe");

    contactCpp.blocked = toCppBoolean(
        (Nan::Get(contact, toJsString("blocked"))).ToLocalChecked(),
        "set.blocked");

    contactCpp.hidden =
        toCppBoolean((Nan::Get(contact, toJsString("hidden"))).ToLocalChecked(),
                     "set.hidden");

    contactCpp.priority =
        toPriority((Nan::Get(contact, toJsString("priority"))).ToLocalChecked(),
                   contactCpp.priority);

    auto name = Nan::Get(contact, toJsString("name"));
    if (!name.IsEmpty() && !name.ToLocalChecked()->IsNullOrUndefined()) {
      // We need to store it as a string and not directly the  string_view
      // otherwise it gets garbage collected
      auto nameStr = toCppString(name.ToLocalChecked(), "contacts.set1");
      contactCpp.set_name(nameStr);
    }
    auto nickname = Nan::Get(contact, toJsString("nickname"));
    if (!nickname.IsEmpty() &&
        !nickname.ToLocalChecked()->IsNullOrUndefined()) {
      // We need to store it as a string and not directly the  string_view
      // otherwise it gets garbage collected
      auto nicknameStr =
          toCppString(nickname.ToLocalChecked(), "contacts.set2");
      contactCpp.set_nickname(nicknameStr);
    }

    auto picMaybe = Nan::Get(contact, toJsString("profilePicture"));

    if (!picMaybe.IsEmpty() &&
        !picMaybe.ToLocalChecked()->IsNullOrUndefined()) {
      auto pic = picMaybe.ToLocalChecked();

      assertIsObject(pic);

      auto picObject = Nan::To<Object>(pic).ToLocalChecked();
      auto urlMaybe = Nan::Get(picObject, toJsString("url"));
      auto keyMaybe = Nan::Get(picObject, toJsString("key"));

      if (!urlMaybe.IsEmpty() && !keyMaybe.IsEmpty() &&
          !urlMaybe.ToLocalChecked()->IsNullOrUndefined() &&
          !keyMaybe.ToLocalChecked()->IsNullOrUndefined()) {
        std::string url =
            toCppString(urlMaybe.ToLocalChecked(), "contacts.set3");
        session::ustring key =
            toCppBuffer(keyMaybe.ToLocalChecked(), "contacts.set4");

        contactCpp.profile_picture = profile_pic(url, key);
      }
    }

    contacts->set(contactCpp);
  });
}

/** ==============================
 *             ERASERS
 * ============================== */

NAN_METHOD(ContactsConfigWrapperInsideWorker::Erase) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first, "contacts.erase");

    auto contacts = getContactWrapperOrThrow(info);

    contacts->erase(sessionIdHexStr);
  });
}