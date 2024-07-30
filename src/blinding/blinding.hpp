#pragma once

#include <napi.h>

#include <algorithm>

#include "../meta/meta_base_wrapper.hpp"
#include "../utilities.hpp"
#include "oxenc/hex.h"
#include "session/blinding.hpp"
#include "session/config/user_profile.hpp"
#include "session/platform.hpp"
#include "session/random.hpp"

namespace session::nodeapi {

class BlindingWrapper : public Napi::ObjectWrap<BlindingWrapper> {

  public:
    BlindingWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BlindingWrapper>{info} {
        throw std::invalid_argument(
                "BlindingWrapper is all static and don't need to be constructed");
    }

    static void Init(Napi::Env env, Napi::Object exports) {
        MetaBaseWrapper::NoBaseClassInitHelper<BlindingWrapper>(
                env,
                exports,
                "BlindingWrapperNode",
                {
                        StaticMethod<&BlindingWrapper::blindVersionPubkey>(
                                "blindVersionPubkey",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),
                        StaticMethod<&BlindingWrapper::blindVersionSign>(
                                "blindVersionSign",
                                static_cast<napi_property_attributes>(
                                        napi_writable | napi_configurable)),

                });
    }

  private:
    static Napi::Value blindVersionPubkey(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("blindVersionPubkey received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"));
            auto ed25519_secret_key =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "blindVersionPubkey.ed25519SecretKey");

            auto keypair = session::blind_version_key_pair(ed25519_secret_key);
            session::uc32 pk_arr = std::get<0>(keypair);
            ustring blinded_pk = session::ustring(
                    session::to_unsigned_sv(std::string(pk_arr.begin(), pk_arr.end())));
            std::string blinded_pk_hex;
            blinded_pk_hex.reserve(66);
            blinded_pk_hex += "07";
            oxenc::to_hex(blinded_pk.begin(), blinded_pk.end(), std::back_inserter(blinded_pk_hex));

            return blinded_pk_hex;
        });
    };

    static Napi::Value blindVersionSign(const Napi::CallbackInfo& info) {
        return wrapResult(info, [&] {
            assertInfoLength(info, 1);
            assertIsObject(info[0]);
            auto obj = info[0].As<Napi::Object>();

            if (obj.IsEmpty())
                throw std::invalid_argument("blindVersionSign received empty");

            assertIsUInt8Array(obj.Get("ed25519SecretKey"));
            auto ed25519_secret_key =
                    toCppBuffer(obj.Get("ed25519SecretKey"), "blindVersionSign.ed25519SecretKey");

            assertIsNumber(obj.Get("sigTimestampSeconds"));
            auto sig_timestamp = toCppInteger(
                    obj.Get("sigTimestampSeconds"), "blindVersionSign.sigTimestampSeconds", false);

            return session::blind_version_sign(
                    ed25519_secret_key, Platform::desktop, sig_timestamp);
        });
    };
};

}  // namespace session::nodeapi
