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

Napi::Value ConfigBaseImpl::storageNamespace(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] {
        return static_cast<uint16_t>(get_config<ConfigBase>().storage_namespace());
    });
}

Napi::Value ConfigBaseImpl::currentHashes(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&] { return (get_config<ConfigBase>().current_hashes()); });
}

Napi::Value ConfigBaseImpl::push(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        auto [seqno, to_push, hashes] = get_config<ConfigBase>().push();

        auto env = info.Env();
        Napi::Object result = Napi::Object::New(env);
        result["data"] = toJs(env, to_push);
        result["seqno"] = toJs(env, seqno);
        result["hashes"] = toJs(env, hashes);

        return result;
    });
}

Napi::Value ConfigBaseImpl::dump(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 0);
        return get_config<ConfigBase>().dump();
    });
}

void ConfigBaseImpl::confirmPushed(const Napi::CallbackInfo& info) {
    return wrapResult(info, [&]() {
        assertInfoLength(info, 2);
        assertIsNumber(info[0]);
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

        std::vector<std::pair<std::string, ustring_view>> conf_strs(asArray.Length());

        for (uint32_t i = 0; i < asArray.Length(); i++) {
            Napi::Value item = asArray[i];
            assertIsObject(item);
            if (item.IsEmpty())
                throw std::invalid_argument("Merge.item received empty");

            Napi::Object itemObject = item.As<Napi::Object>();

            auto hash = itemObject.Get("hash");
            auto data = itemObject.Get("data");
            assertIsString(hash);
            assertIsUInt8Array(data);

            conf_strs[i].first = toCppString(hash, "base.merge");
            auto u8_data = data.As<Napi::Uint8Array>();
            conf_strs[i].second = {u8_data.Data(), u8_data.ByteLength()};
        }

        return get_config<ConfigBase>().merge(conf_strs);
    });
}

}  // namespace session::nodeapi
