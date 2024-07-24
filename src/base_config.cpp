#include "base_config.hpp"

#include "session/config/base.hpp"
#include "session/config/encrypt.hpp"

namespace session::nodeapi {

using config::ConfigBase;

Napi::Value ConfigBaseImpl::needsDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return get_config<ConfigBase>().needs_dump(); });
}

Napi::Value ConfigBaseImpl::needsPush(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return get_config<ConfigBase>().needs_push(); });
}

Napi::Value ConfigBaseImpl::currentHashes(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return (get_config<ConfigBase>().current_hashes()); });
}

Napi::Value ConfigBaseImpl::push(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        auto& conf = get_config<ConfigBase>();
        auto to_push = conf.push();

        return push_result_to_JS(info.Env(), to_push, conf.storage_namespace());
    });
}

Napi::Value ConfigBaseImpl::dump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        return get_config<ConfigBase>().dump();
    });
}

Napi::Value ConfigBaseImpl::makeDump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        return get_config<ConfigBase>().make_dump();
    });
}

void ConfigBaseImpl::confirmPushed(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 2);
        assertIsNumber(info[0], "confirmPushed");
        assertIsString(info[1]);

        get_config<ConfigBase>().confirm_pushed(
                toCppInteger(info[0], "confirmPushed", false),
                toCppString(info[1], "confirmPushed"));
    });
}

Napi::Value ConfigBaseImpl::merge(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 1);
        assertIsArray(info[0]);
        Napi::Array asArray = info[0].As<Napi::Array>();

        std::vector<std::pair<std::string, ustring_view>> conf_strs;
        conf_strs.reserve(asArray.Length());

        for (uint32_t i = 0; i < asArray.Length(); i++) {
            Napi::Value item = asArray[i];
            assertIsObject(item);
            if (item.IsEmpty())
                throw std::invalid_argument("Merge.item received empty");

            Napi::Object itemObject = item.As<Napi::Object>();
            conf_strs.emplace_back(
                    toCppString(itemObject.Get("hash"), "base.merge"),
                    toCppBufferView(itemObject.Get("data"), "base.merge"));
        }

        return get_config<ConfigBase>().merge(conf_strs);
    });
}

}  // namespace session::nodeapi
