#include "meta_group_wrapper.hpp"

#include <napi.h>

#include <memory>

namespace session::nodeapi {

MetaGroupWrapper::MetaGroupWrapper(const Napi::CallbackInfo& info) :
        meta_group{MetaBaseWrapper::constructGroupWrapper(info, "MetaGroupWrapper")},
        Napi::ObjectWrap<MetaGroupWrapper>{info} {}

void MetaGroupWrapper::Init(Napi::Env env, Napi::Object exports) {
    MetaBaseWrapper::NoBaseClassInitHelper<MetaGroupWrapper>(
            env,
            exports,
            "MetaGroupWrapperNode",
            {
                    // shared exposed functions

                    InstanceMethod("needsPush", &MetaGroupWrapper::needsPush),
                    InstanceMethod("push", &MetaGroupWrapper::push),
                    InstanceMethod("needsDump", &MetaGroupWrapper::needsDump),
                    InstanceMethod("metaDump", &MetaGroupWrapper::metaDump),
                    InstanceMethod("metaConfirmPushed", &MetaGroupWrapper::metaConfirmPushed),
                    InstanceMethod("metaMerge", &MetaGroupWrapper::metaMerge),

                    // infos exposed functions
                    InstanceMethod("infoGet", &MetaGroupWrapper::infoGet),
                    InstanceMethod("infoSet", &MetaGroupWrapper::infoSet),
                    InstanceMethod("infoDestroy", &MetaGroupWrapper::infoDestroy),

                    // members exposed functions
                    InstanceMethod("memberGet", &MetaGroupWrapper::memberGet),
                    InstanceMethod("memberGetOrConstruct", &MetaGroupWrapper::memberGetOrConstruct),
                    InstanceMethod("memberGetAll", &MetaGroupWrapper::memberGetAll),
                    InstanceMethod("memberSetName", &MetaGroupWrapper::memberSetName),
                    InstanceMethod("memberSetInvited", &MetaGroupWrapper::memberSetInvited),
                    InstanceMethod("memberSetAccepted", &MetaGroupWrapper::memberSetAccepted),
                    InstanceMethod("memberSetPromoted", &MetaGroupWrapper::memberSetPromoted),
                    InstanceMethod(
                            "memberSetProfilePicture", &MetaGroupWrapper::memberSetProfilePicture),
                    InstanceMethod("memberErase", &MetaGroupWrapper::memberErase),

                    // keys exposed functions

                    InstanceMethod("keysNeedsRekey", &MetaGroupWrapper::keysNeedsRekey),
                    InstanceMethod("keyRekey", &MetaGroupWrapper::keyRekey),

                    InstanceMethod("currentHashes", &MetaGroupWrapper::currentHashes),
                    InstanceMethod("loadKeyMessage", &MetaGroupWrapper::loadKeyMessage),

                    InstanceMethod("encryptMessage", &MetaGroupWrapper::encryptMessage),
                    InstanceMethod("decryptMessage", &MetaGroupWrapper::decryptMessage),

            });
}

Napi::Value MetaGroupWrapper::needsPush(const Napi::CallbackInfo& info) {

    return wrapResult(info, [&] {
        return this->meta_group->members->needs_push() || this->meta_group->info->needs_push() ||
               this->meta_group->keys->pending_config();
    });
}

void MetaGroupWrapper::metaConfirmPushed(const Napi::CallbackInfo& info) {
    wrapExceptions(info, [&]() {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");
        auto groupKeys = obj.Get("groupKeys");

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsArray(groupInfo);
            auto groupInfoArr = groupInfo.As<Napi::Array>();
            if (groupInfoArr.Length() != 2) {
                throw std::invalid_argument("groupInfoArr length was not 2");
            }

            auto seqno = maybeNonemptyInt(
                    groupInfoArr.Get("0"), "MetaGroupWrapper::metaConfirmPushed groupInfo seqno");
            auto hash = maybeNonemptyString(
                    groupInfoArr.Get("1"), "MetaGroupWrapper::metaConfirmPushed groupInfo hash");
            if (seqno && hash)
                this->meta_group->info->confirm_pushed(*seqno, *hash);
        }

        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsArray(groupMember);
            auto groupMemberArr = groupMember.As<Napi::Array>();
            if (groupMemberArr.Length() != 2) {
                throw std::invalid_argument("groupMemberArr length was not 2");
            }

            auto seqno = maybeNonemptyInt(
                    groupMemberArr.Get("0"),
                    "MetaGroupWrapper::metaConfirmPushed groupMemberArr seqno");
            auto hash = maybeNonemptyString(
                    groupMemberArr.Get("1"),
                    "MetaGroupWrapper::metaConfirmPushed groupMemberArr hash");
            if (seqno && hash)
                this->meta_group->members->confirm_pushed(*seqno, *hash);
        }
    });
};

Napi::Value MetaGroupWrapper::metaMerge(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        auto groupInfo = obj.Get("groupInfo");
        auto groupMember = obj.Get("groupMember");
        auto groupKeys = obj.Get("groupKeys");

        auto count_merged = 0;

        // Note: we need to process keys first as they might allow us the incoming info+members
        // details
        if (!groupKeys.IsNull() && !groupKeys.IsUndefined()) {
            assertIsArray(groupKeys);
            auto asArr = groupKeys.As<Napi::Array>();

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupKeys received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"));
                assertIsNumber(itemObject.Get("timestampMs"));

                auto hash = toCppString(itemObject.Get("hash"), "meta.merge keys hash");
                auto data = toCppBuffer(itemObject.Get("data"), "meta.merge keys data");
                auto timestamp_ms = toCppInteger(
                        itemObject.Get("timestampMs"), "meta.merge keys timestampMs", false);

                this->meta_group->keys->load_key_message(
                        hash,
                        data,
                        timestamp_ms,
                        *(this->meta_group->info),
                        *(this->meta_group->members));
                count_merged++;  // load_key_message doesn't necessarely merge something as not
                                 // all keys are for us.
            }
        }

        if (!groupInfo.IsNull() && !groupInfo.IsUndefined()) {
            assertIsArray(groupInfo);
            auto asArr = groupInfo.As<Napi::Array>();

            std::vector<std::pair<std::string, ustring_view>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupInfo received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"));
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBufferView(itemObject.Get("data"), "meta.merge"));
            }

            auto info_count_merged = this->meta_group->info->merge(conf_strs);
            count_merged += info_count_merged;
        }

        if (!groupMember.IsNull() && !groupMember.IsUndefined()) {
            assertIsArray(groupMember);
            auto asArr = groupMember.As<Napi::Array>();

            std::vector<std::pair<std::string, ustring_view>> conf_strs;
            conf_strs.reserve(asArr.Length());

            for (uint32_t i = 0; i < asArr.Length(); i++) {
                Napi::Value item = asArr[i];
                assertIsObject(item);
                if (item.IsEmpty())
                    throw std::invalid_argument("MetaMerge.item groupMember received empty");

                Napi::Object itemObject = item.As<Napi::Object>();
                assertIsString(itemObject.Get("hash"));
                assertIsUInt8Array(itemObject.Get("data"));
                conf_strs.emplace_back(
                        toCppString(itemObject.Get("hash"), "meta.merge"),
                        toCppBufferView(itemObject.Get("data"), "meta.merge"));
            }

            auto member_count_merged = this->meta_group->members->merge(conf_strs);
            count_merged += member_count_merged;
        }
        return count_merged;
    });
}

Napi::Value MetaGroupWrapper::infoGet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        auto env = info.Env();
        auto obj = Napi::Object::New(env);

        obj["name"] = toJs(env, this->meta_group->info->get_name());
        obj["createdAtSeconds"] = toJs(env, this->meta_group->info->get_created());
        obj["deleteAttachBeforeSeconds"] =
                toJs(env, this->meta_group->info->get_delete_attach_before());
        obj["deleteBeforeSeconds"] = toJs(env, this->meta_group->info->get_delete_before());

        if (auto expiry = this->meta_group->info->get_expiry_timer(); expiry)
            obj["expirySeconds"] = toJs(env, expiry->count());
        else
            obj["expirySeconds"] = env.Null();

        obj["isDestroyed"] = toJs(env, this->meta_group->info->is_destroyed());
        obj["profilePicture"] = toJs(env, this->meta_group->info->get_profile_pic());

        return obj;
    });
}

Napi::Value MetaGroupWrapper::infoSet(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        assertInfoLength(info, 1);
        auto arg = info[0];
        assertIsObject(arg);
        auto obj = arg.As<Napi::Object>();

        if (auto name = maybeNonemptyString(obj.Get("name"), "MetaGroupWrapper::setInfo name"))
            this->meta_group->info->set_name(*name);

        if (auto created = maybeNonemptyInt(
                    obj.Get("createdAtSeconds"), "MetaGroupWrapper::setInfo set_created"))
            this->meta_group->info->set_created(std::move(*created));

        if (auto expiry = maybeNonemptyInt(
                    obj.Get("expirySeconds"), "MetaGroupWrapper::setInfo set_expiry_timer"))
            this->meta_group->info->set_expiry_timer(std::chrono::seconds{*expiry});

        if (auto deleteBefore = maybeNonemptyInt(
                    obj.Get("deleteBeforeSeconds"), "MetaGroupWrapper::setInfo set_delete_before"))
            this->meta_group->info->set_delete_before(std::move(*deleteBefore));

        if (auto deleteAttachBefore = maybeNonemptyInt(
                    obj.Get("deleteAttachBeforeSeconds"),
                    "MetaGroupWrapper::setInfo set_delete_attach_before"))
            this->meta_group->info->set_delete_attach_before(std::move(*deleteAttachBefore));

        if (auto profilePicture = obj.Get("profilePicture")) {
            auto profilePic = profile_pic_from_object(profilePicture);
            this->meta_group->info->set_profile_pic(profilePic);
        }

        return this->infoGet(info);
    });
}

}  // namespace session::nodeapi
