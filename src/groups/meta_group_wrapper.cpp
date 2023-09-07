#include "meta_group_wrapper.hpp"

#include <napi.h>

#include <memory>

namespace session::nodeapi {

Napi::Object push_entry_to_JS(const Napi::Env& env, const push_entry_t& push_entry) {
    auto obj = Napi::Object::New(env);

    obj["seqno"] = toJs(env, std::get<0>(push_entry));
    obj["data"] = toJs(env, std::get<1>(push_entry));
    obj["hashes"] = toJs(env, std::get<2>(push_entry));

    return obj;
};

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
        return this->meta_group->members->needs_push() ||
               this->meta_group->info
                       ->needs_push();  // || this->meta_group->keys->needs_rekey() // TODO
        // see what to do with this and needs_rekey below
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
