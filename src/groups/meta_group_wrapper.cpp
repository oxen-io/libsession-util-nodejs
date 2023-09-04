#include "meta_group_wrapper.hpp"

#include <napi.h>

#include <memory>

namespace session::nodeapi {

MetaGroupWrapper::MetaGroupWrapper(const Napi::CallbackInfo& info) :
        MetaBaseWrapper{},
        Napi::ObjectWrap<MetaGroupWrapper>{info},
        meta_group{constructGroupWrapper(info, "MetaGroupWrapper")} {}

// Napi::Value MetaGroupWrapper::infoGet(const Napi::CallbackInfo& info) {
//     return wrapResult(info, [&] {
//         auto env = info.Env();
//         auto obj = Napi::Object::New(env);

//         obj["name"] = toJs(env, config.get_name());
//         obj["createdAtSeconds"] = toJs(env, config.get_created());
//         obj["deleteAttachBeforeSeconds"] = toJs(env, config.get_delete_attach_before());
//         obj["deleteBeforeSeconds"] = toJs(env, config.get_delete_before());

//         if (auto expiry = config.get_expiry_timer(); expiry)
//             obj["expirySeconds"] = toJs(env, expiry->count());
//         obj["isDestroyed"] = toJs(env, config.is_destroyed());
//         obj["profilePicture"] = toJs(env, config.get_profile_pic());

//         return obj;
//     });
// }

// Napi::Value MetaGroupWrapper::infoSet(const Napi::CallbackInfo& info) {
//     return wrapResult(info, [&] {
//         assertInfoLength(info, 1);
//         auto arg = info[0];
//         assertIsObject(arg);
//         auto obj = arg.As<Napi::Object>();

//         if (auto name = maybeNonemptyString(obj.Get("name"), "MetaGroupWrapper::setInfo name"))
//             config.set_name(*name);

//         if (auto created = maybeNonemptyInt(
//                     obj.Get("createdAtSeconds"), "MetaGroupWrapper::setInfo set_created"))
//             config.set_created(std::move(*created));

//         if (auto expiry = maybeNonemptyInt(
//                     obj.Get("expirySeconds"), "MetaGroupWrapper::setInfo set_expiry_timer"))
//             config.set_expiry_timer(std::chrono::seconds{*expiry});

//         if (auto deleteBefore = maybeNonemptyInt(
//                     obj.Get("deleteBeforeSeconds"), "MetaGroupWrapper::setInfo
//                     set_delete_before"))
//             config.set_delete_before(std::move(*deleteBefore));

//         if (auto deleteAttachBefore = maybeNonemptyInt(
//                     obj.Get("deleteAttachBeforeSeconds"),
//                     "MetaGroupWrapper::setInfo set_delete_attach_before"))
//             config.set_delete_attach_before(std::move(*deleteAttachBefore));

//         if (auto profilePicture = obj.Get("profilePicture")) {
//             auto profilePic = profile_pic_from_object(profilePicture);
//             config.set_profile_pic(profilePic);
//         }

//         return this->getInfo(info);
//     });
// }

}  // namespace session::nodeapi
