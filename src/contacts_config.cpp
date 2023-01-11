#include "contacts_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/types.hpp"

#include <iostream>
#include <optional>

using Nan::MaybeLocal;
using v8::Array;
using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

using session::ustring;
using session::config::contact_info;
// using session::config::contact_info_hard;
using session::config::profile_pic;

using session::config::Contacts;
using std::cerr;
using std::endl;
using std::make_optional;
using std::optional;
using std::string;

struct profile_pic_hard {
  std::string url;
  ustring key;

  profile_pic_hard(std::string url, ustring key) : url{url}, key{key} {}
};

struct contact_info_hard {
  std::string session_id; // in hex
  std::optional<std::string> name;
  std::optional<std::string> nickname;
  std::optional<profile_pic_hard> profile_picture;
  bool approved = false;
  bool approved_me = false;
  bool blocked = false;

  contact_info_hard(std::string sid) : session_id(sid) {}
};

contact_info to_contact_info(contact_info_hard info) {
  auto ret = contact_info(info.session_id);

  ret.approved = info.approved;
  ret.approved_me = info.approved_me;
  ret.blocked = info.blocked;
  ret.name = info.name;
  ret.nickname = info.nickname;

  if (info.profile_picture) {
    auto pic =
        profile_pic(info.profile_picture->url, info.profile_picture->key);
    ret.profile_picture = pic;
  }

  return ret;
};

/**
 * TODO
 * - pull latest lib changes and use .size() for the getAll method
 *
 */

Local<Object> toJSContact(const contact_info contact) {
  Local<Object> obj = Nan::New<Object>();
  auto context = Nan::GetCurrentContext();

  auto result = obj->Set(context, toJsString("id"),
                         toJsString(contact.session_id)); // in hex

  if (contact.name) {
    result = obj->Set(context, toJsString("name"), toJsString(*(contact.name)));

  } else {
    result = obj->Set(context, toJsString("name"), Nan::Null());
  }

  if (contact.nickname) {
    result = obj->Set(context, toJsString("nickname"),
                      toJsString(*contact.nickname));
  } else {
    result = obj->Set(context, toJsString("nickname"), Nan::Null());
  }

  result = obj->Set(context, toJsString("approved"),
                    Nan::New<Boolean>(contact.approved));
  result = obj->Set(context, toJsString("approvedMe"),
                    Nan::New<Boolean>(contact.approved_me));
  result = obj->Set(context, toJsString("blocked"),
                    Nan::New<Boolean>(contact.blocked));

  if (contact.profile_picture) {
    Local<Object> profilePic = Nan::New<Object>();
    result = profilePic->Set(context, toJsString("url"),
                             toJsString(contact.profile_picture->url));
    result = profilePic->Set(context, toJsString("key"),
                             toJsBuffer(contact.profile_picture->key));

    result = obj->Set(context, toJsString("profilePicture"), profilePic);
  }

  return obj;
}

contact_info_hard toCppContact(MaybeLocal<Value> contactMaybe) {

  if (contactMaybe.IsEmpty()) {
    throw std::invalid_argument("cppContact received empty");
  }
  Local<Value> contactValue = contactMaybe.ToLocalChecked();

  if (!contactValue->IsObject()) {
    throw std::invalid_argument("cppContact received not object");
  }
  Local<Object> contact = Nan::To<Object>(contactValue).ToLocalChecked();

  MaybeLocal<Value> sessionIdMaybe = Nan::Get(contact, toJsString("id"));

  if (sessionIdMaybe.IsEmpty() ||
      sessionIdMaybe.ToLocalChecked()->IsNullOrUndefined()) {
    throw std::invalid_argument("SessionID is empty");
  }

  Local<Value> sessionId = sessionIdMaybe.ToLocalChecked();
  assertIsString(sessionId);
  std::string sessionIdStr = toCppString(sessionId);

  contact_info_hard contactCpp(sessionIdStr);

  contactCpp.approved = toCppBoolean(
      (Nan::Get(contact, toJsString("approved"))).ToLocalChecked());
  contactCpp.approved_me = toCppBoolean(
      (Nan::Get(contact, toJsString("approvedMe"))).ToLocalChecked());
  contactCpp.blocked =
      toCppBoolean((Nan::Get(contact, toJsString("blocked"))).ToLocalChecked());

  auto name = Nan::Get(contact, toJsString("name"));
  if (!name.IsEmpty() && !name.ToLocalChecked()->IsNullOrUndefined()) {
    // We need to store it as a string and not directly the  string_view
    // otherwise it gets garbage collected
    auto nameStr = toCppString(name.ToLocalChecked());
    contactCpp.name = nameStr;
  }

  auto nickname = Nan::Get(contact, toJsString("nickname"));
  if (!nickname.IsEmpty() && !nickname.ToLocalChecked()->IsNullOrUndefined()) {
    // We need to store it as a string and not directly the  string_view
    // otherwise it gets garbage collected
    auto nicknameStr = toCppString(nickname.ToLocalChecked());

    contactCpp.nickname = nicknameStr;
  }

  auto picMaybe = Nan::Get(contact, toJsString("profilePicture"));
  if (!picMaybe.IsEmpty() & !picMaybe.ToLocalChecked()->IsNullOrUndefined()) {
    auto pic = picMaybe.ToLocalChecked();

    assertIsObject(pic);

    auto picObject = Nan::To<Object>(pic).ToLocalChecked();
    auto urlMaybe = Nan::Get(picObject, toJsString("url"));
    auto keyMaybe = Nan::Get(picObject, toJsString("key"));

    if (!urlMaybe.IsEmpty() && !keyMaybe.IsEmpty()) {
      std::string url = toCppString(urlMaybe.ToLocalChecked());
      session::ustring key = toCppBuffer(keyMaybe.ToLocalChecked());
      profile_pic_hard img = profile_pic_hard(url, key);
      contactCpp.profile_picture = img;
    }
  }

  return contactCpp;
}

NAN_MODULE_INIT(ContactsConfigWrapper::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ContactsConfigWrapper").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "get", Get);
  RegisterNANMethods(tpl, "getOrCreate", GetOrCreate);
  RegisterNANMethods(tpl, "getAll", GetAll);
  RegisterNANMethods(tpl, "set", Set);
  RegisterNANMethods(tpl, "setName", SetName);
  RegisterNANMethods(tpl, "setNickname", SetNickname);
  RegisterNANMethods(tpl, "setApproved", SetApproved);
  RegisterNANMethods(tpl, "setApprovedMe", SetApprovedMe);
  RegisterNANMethods(tpl, "setProfilePicture", SetProfilePicture);
  RegisterNANMethods(tpl, "setBlocked", SetBlocked);
  RegisterNANMethods(tpl, "erase", Erase);

  Nan::Set(target, Nan::New("ContactsConfigWrapper").ToLocalChecked(),
           Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(ContactsConfigWrapper::New) {
  tryOrWrapStdException([&]() {
    if (info.IsConstructCall()) {
      assertInfoLength(info, 2);
      auto first = info[0];
      auto second = info[1];

      // we should get secret key as first arg and optional dumped as second
      // argument
      assertIsUInt8Array(first);
      assertIsUInt8ArrayOrNull(second);

      ustring secretKey = toCppBuffer(first);
      bool dumpIsSet = !second.IsEmpty() && !second->IsNullOrUndefined();
      if (dumpIsSet) {
        ustring dumped = toCppBuffer(second);
        ContactsConfigWrapper *obj =
            new ContactsConfigWrapper(secretKey, dumped);
        obj->Wrap(info.This());
      } else {

        ContactsConfigWrapper *obj =
            new ContactsConfigWrapper(secretKey, std::nullopt);
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

NAN_METHOD(ContactsConfigWrapper::Get) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);
    auto contacts = to<session::config::Contacts>(info);

    if (!contacts) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    optional<contact_info> contact = contacts->get(sessionIdHexStr);

    if (contact) {
      info.GetReturnValue().Set(toJSContact(*contact));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }

    return;
  });
}

NAN_METHOD(ContactsConfigWrapper::GetOrCreate) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto contacts = to<session::config::Contacts>(info);

    if (!contacts) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    auto contact = contacts->get_or_create(sessionIdHexStr);

    info.GetReturnValue().Set(toJSContact(contact));

    return;
  });
}

NAN_METHOD(ContactsConfigWrapper::GetAll) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 0);
    auto contacts = to<session::config::Contacts>(info);

    if (!contacts) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    int length = 0;
    // get the length // FIXME
    for (auto &contact : *contacts) {
      length++;
    }

    Local<Array> allContacts = Nan::New<Array>(length);
    int index = 0;
    for (auto &contact : *contacts) {
      allContacts->Set(Nan::GetCurrentContext(), index, toJSContact((contact)));
      index++;
    }

    info.GetReturnValue().Set(allContacts);

    return;
  });
}

/** ==============================
 *             SETTERS
 * ============================== */

NAN_METHOD(ContactsConfigWrapper::Set) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsObject(first);
    contact_info_hard firstAsContact = toCppContact(first);

    Contacts *contacts = to<Contacts>(info);
    if (!contacts) {
      return;
    }

    contacts->set(to_contact_info(firstAsContact));
  });
}

NAN_METHOD(ContactsConfigWrapper::SetName) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    auto assertIsString(second);
    Nan::Utf8String name(second);
    std::string nameStr(*name);

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->set_name(sessionIdHexStr, nameStr);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetNickname) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    auto assertIsString(second);
    Nan::Utf8String nickname(second);
    std::string nicknameStr(*nickname);

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->set_nickname(sessionIdHexStr, nicknameStr);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetApproved) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    assertIsBoolean(second);
    bool approved = *(Nan::To<Boolean>(second).ToLocalChecked());

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->set_approved(sessionIdHexStr, approved);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetApprovedMe) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    assertIsBoolean(second);
    bool approvedMe = *(Nan::To<Boolean>(second).ToLocalChecked());

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->set_approved_me(sessionIdHexStr, approvedMe);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetBlocked) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    assertIsBoolean(second);
    bool blocked = *(Nan::To<Boolean>(second).ToLocalChecked());

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->set_blocked(sessionIdHexStr, blocked);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetProfilePicture) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 3);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto second = info[1];
    assertIsStringOrNull(second);

    auto third = info[2];
    assertIsUInt8ArrayOrNull(third);

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }
    if (!second.IsEmpty() && !third.IsEmpty() && !second->IsNullOrUndefined() &&
        !third->IsNullOrUndefined()) {

      auto url = toCppString(second);
      auto key = toCppBuffer(third);
      profile_pic picDetails(url, key);
      contacts->set_profile_pic(sessionIdHexStr, picDetails);
    } else {
      // profile_pic picDetails(nullptr, nullptr);
      // contacts->set_profile_pic(sessionIdHexStr, picDetails);
    }
  });
}

/** ==============================
 *             ERASERS
 * ============================== */

NAN_METHOD(ContactsConfigWrapper::Erase) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsString(first);
    std::string sessionIdHexStr = toCppString(first);

    auto contacts = to<session::config::Contacts>(info);
    if (!contacts) {
      throw std::invalid_argument("Contacts is null");
      return;
    }

    contacts->erase(sessionIdHexStr);
  });
}