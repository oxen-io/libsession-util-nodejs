#include "contacts_config.hpp"
#include "base_config.hpp"
#include "oxenc/hex.h"
#include "session/types.hpp"

#include <iostream>
#include <optional>

using Nan::MaybeLocal;
using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Uint8Array;
using v8::Value;

using session::ustring;
using session::ustring_view;
using session::config::contact_info;
using session::config::profile_pic;

using session::config::Contacts;
using std::cerr;
using std::endl;
using std::make_optional;
using std::optional;
using std::string;

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

  return obj;
}

contact_info toCppContact(MaybeLocal<Value> contactMaybe) {

  if (contactMaybe.IsEmpty()) {
    throw std::invalid_argument("cppContact received empty");
  }
  Local<Value> contactValue = contactMaybe.ToLocalChecked();

  if (!contactValue->IsObject()) {
    throw std::invalid_argument("cppContact received not object");
  }
  Local<Object> contact = Nan::To<Object>(contactValue).ToLocalChecked();

  MaybeLocal<Value> sessionIdMaybe = Nan::Get(contact, toJsString("id"));

  if (sessionIdMaybe.IsEmpty()) {
    throw std::invalid_argument("SessionID is empty");
  }

  Local<Value> sessionId = sessionIdMaybe.ToLocalChecked();
  assertIsString(sessionId);
  std::string sessionIdStr = toCppString(sessionId);

  contact_info contactCpp(sessionIdStr);

  auto name = Nan::Get(contact, toJsString("name"));
  if (!name.IsEmpty()) {
    contactCpp.name = toCppString(name.ToLocalChecked());
  }

  auto nickname = Nan::Get(contact, toJsString("nickname"));
  if (!nickname.IsEmpty()) {
    contactCpp.nickname = toCppString(nickname.ToLocalChecked());
  }
  auto picMaybe = Nan::Get(contact, toJsString("profilePicture"));
  // if (!picMaybe.IsEmpty()) {
  //   auto pic = picMaybe.ToLocalChecked();

  //   assertIsObject(pic);

  //   auto picObject = Nan::To<Object>(pic).ToLocalChecked();
  //   auto urlMaybe = Nan::Get(picObject, toJsString("url"));
  //   auto keyMaybe = Nan::Get(picObject, toJsString("key"));

  //   if (!urlMaybe.IsEmpty() && !keyMaybe.IsEmpty()) {
  //     // std::string url = toCppString(urlMaybe.ToLocalChecked());
  //     // session::ustring_view key = toCppBuffer(keyMaybe.ToLocalChecked());
  //     // profile_pic img = profile_pic(url, key);
  //     // contactCpp.profile_picture = img;
  //   }
  // }

  if (contactCpp.name) {
    cerr << "inside toCppContact name: " << *(contactCpp.name) << "\n";
  }

  return contactCpp;
}

NAN_MODULE_INIT(ContactsConfigWrapper::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ContactsConfigWrapper").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  RegisterNANMethods(tpl, "get", Get);
  RegisterNANMethods(tpl, "getOrCreate", GetOrCreate);
  RegisterNANMethods(tpl, "set", Set);
  RegisterNANMethods(tpl, "setName", SetName);
  RegisterNANMethods(tpl, "setNickname", SetNickname);
  RegisterNANMethods(tpl, "setApproved", SetApproved);
  RegisterNANMethods(tpl, "setApprovedMe", SetApprovedMe);

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

NAN_METHOD(ContactsConfigWrapper::Get) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);
    auto first = info[0];
    assertIsString(first);
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);
    auto contacts = to<session::config::Contacts>(info);

    if (!contacts) {
      info.GetReturnValue().Set(Nan::Null());
      return;
    }

    optional<contact_info> contact = contacts->get(sessionIdHexStr);

    if (contact) {
      // if (contact->name) {
      //   cerr << "contact name :" << *(contact->name) << ":  \n";
      // }
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
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);
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

NAN_METHOD(ContactsConfigWrapper::Set) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 1);

    auto first = info[0];
    assertIsObject(first);
    contact_info firstAsContact = toCppContact(first);

    Contacts *contacts = to<Contacts>(info);
    if (!contacts) {
      return;
    }
    cerr << "outside id" << firstAsContact.session_id << "\n";

    if (firstAsContact.name) {
      cerr << "outside " << *(firstAsContact.name) << "\n";
    }
    contacts->set(firstAsContact);
  });
}

NAN_METHOD(ContactsConfigWrapper::SetName) {
  tryOrWrapStdException([&]() {
    assertInfoLength(info, 2);

    auto first = info[0];
    assertIsString(first);
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);

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
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);

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
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);

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
    Nan::Utf8String sessionIdHex(first);
    std::string sessionIdHexStr(*sessionIdHex);

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
