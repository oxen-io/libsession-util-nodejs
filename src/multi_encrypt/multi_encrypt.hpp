#pragma once

#include <napi.h>

#include <algorithm>

#include "session/config/user_profile.hpp"
#include "session/multi_encrypt.hpp"
#include "session/random.hpp"
#include "utilities.hpp"

namespace session::nodeapi {

class MultiEncryptWrapper : public Napi::ObjectWrap<MultiEncryptWrapper> {

  public:
    MultiEncryptWrapper(const Napi::CallbackInfo& info) :
            Napi::ObjectWrap<MultiEncryptWrapper>{info} {
        throw std::invalid_argument(
                "MultiEncryptWrapper is all static and don't need to be constructed");
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        MetaBaseWrapper::NoBaseClassInitHelper<MultiEncryptWrapper>(
                env,
                exports,
                "MultiEncryptWrapperNode",
                {
                        StaticMethod<&MultiEncryptWrapper::multiEncrypt>(
                                "multiEncrypt",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&MultiEncryptWrapper::multiDecryptEd25519>(
                                "multiDecryptEd25519",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                });
    }

  private:
    static Napi::Value multiEncrypt(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("multiEncrypt received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");
            auto ed25519SecretKey =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "multiEncrypt.ed25519SecretKey");

            assertIsString(obj.Get("domain"));
            auto domain = toCppString(obj.Get("domain"), "multiEncrypt.domain");

            // handle the messages conversion
            auto messagesJSValue = obj.Get("messages");
            assertIsArray(messagesJSValue);
            auto messagesJS = messagesJSValue.As<Napi::Array>();
            std::vector<ustring> messages;
            messages.reserve(messagesJS.Length());
            for (uint32_t i = 0; i < messagesJS.Length(); i++) {
                auto itemValue = messagesJS.Get(i);
                assertIsUInt8Array(itemValue, "multiEncrypt.itemValue.message");
                auto item = toCppBuffer(itemValue, "multiEncrypt.itemValue.message");
                messages.push_back(item);
            }

            // handle the recipients conversion
            auto recipientsJSValue = obj.Get("recipients");
            assertIsArray(recipientsJSValue);
            auto recipientsJS = recipientsJSValue.As<Napi::Array>();
            std::vector<ustring> recipients;
            recipients.reserve(recipientsJS.Length());
            for (uint32_t i = 0; i < recipientsJS.Length(); i++) {
                auto itemValue = recipientsJS.Get(i);
                assertIsUInt8Array(itemValue, "multiEncrypt.itemValue.recipient");
                auto item = toCppBuffer(itemValue, "multiEncrypt.itemValue.recipient");
                recipients.push_back(item);
            }
            ustring random_nonce = session::random::random(24);

            std::vector<ustring_view> messages_sv(messages.begin(), messages.end());
            std::vector<ustring_view> recipients_sv(recipients.begin(), recipients.end());

            // Note: this function needs the first 2 args to be vector of sv explicitely
            return session::encrypt_for_multiple_simple(
                    messages_sv, recipients_sv, ed25519SecretKey, domain, random_nonce);
        });
    };

    static Napi::Value multiDecryptEd25519(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("multiDecryptEd25519 received empty");

            assertIsUInt8Array(obj.Get("encoded"), "multiDecryptEd25519.encoded");
            auto encoded = toCppBuffer(obj.Get("encoded"), "multiDecryptEd25519.encoded");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"), "multiDecryptEd25519.ed25519SecretKey");
            auto ed25519_secret_key = toCppBuffer(
                    obj.Get("ed25519SecretKey"), "multiDecryptEd25519.ed25519SecretKey");

            assertIsUInt8Array(
                    obj.Get("senderEd25519Pubkey"), "multiDecryptEd25519.senderEd25519Pubkey");
            auto sender_ed25519_pubkey = toCppBuffer(
                    obj.Get("senderEd25519Pubkey"), "multiDecryptEd25519.senderEd25519Pubkey");

            assertIsString(obj.Get("domain"));
            auto domain = toCppString(obj.Get("domain"), "multiDecryptEd25519.domain");

            return session::decrypt_for_multiple_simple_ed25519(
                    encoded, ed25519_secret_key, sender_ed25519_pubkey, domain);
        });
    };
};

}  // namespace session::nodeapi
