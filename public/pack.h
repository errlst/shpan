#pragma once
#include <cstdint>
#include <memory>
#include <string.h>
#include <utility>

enum class Api : uint8_t {
    REGIST,      // 注册
    LOGIN,       // 登录
    LS_ENTRY,    // 查看条目
    MK_DIR,      // 创建目录
    UPLOAD_META, // 文件元数据

    MAX_INVALID
};

static auto api_to_string(Api api) -> std::string {
    switch (api) {
    case Api::REGIST:
        return "regist";
    case Api::LOGIN:
        return "login";
    case Api::LS_ENTRY:
        return "ls_entry";
    case Api::MK_DIR:
        return "mkdir";
    case Api::UPLOAD_META:
        return "upload_meta";
    default:
        return "invalid";
    }
    std::unreachable();
}

struct Pack {
    struct { // 包元数据
        Api api;
        uint8_t state; // 该数据响应有效
        uint16_t data_size;
    };
    char data[0];
};

static auto create_pack_with_meta(Pack meta) -> std::shared_ptr<Pack> {
    auto p = (Pack *)malloc(meta.data_size + sizeof(Pack));
    p->api = meta.api;
    p->state = meta.state;
    p->data_size = meta.data_size;
    return std::shared_ptr<Pack>{p, [](Pack *p) { free(p); }};
}

static auto create_pack_with_size(Api api, bool state, uint16_t data_size) -> std::shared_ptr<Pack> {
    if (api > Api::MAX_INVALID) {
        return nullptr;
    }
    auto p = (Pack *)malloc(data_size + sizeof(Pack));
    p->api = api;
    p->state = state;
    p->data_size = data_size;
    return std::shared_ptr<Pack>{p, [](Pack *p) { free(p); }};
}

static auto create_pack_with_str_msg(Api api, bool state, const std::string &msg) -> std::shared_ptr<Pack> {
    auto p = (Pack *)malloc(msg.size() + sizeof(Pack));
    p->api = api;
    p->state = state;
    p->data_size = msg.size();
    memcpy(p->data, msg.data(), msg.size());
    return std::shared_ptr<Pack>{p, [](Pack *p) { free(p); }};
}

static auto create_pack_with_num_msg(Api api, bool state, uint64_t msg) -> std::shared_ptr<Pack> {
    auto p = (Pack *)malloc(sizeof(msg) + sizeof(Pack));
    p->api = api;
    p->state = state;
    p->data_size = sizeof(msg);
    memcpy(p->data, &msg, sizeof(msg));
    return std::shared_ptr<Pack>{p, [](Pack *p) { free(p); }};
}