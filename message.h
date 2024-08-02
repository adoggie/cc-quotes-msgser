//
// Created by bin zhang on 2019/1/6.
//

#ifndef CC_QUOTES_MESSAGE_H
#define CC_QUOTES_MESSAGE_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <algorithm>
#include <iterator>
#include <arpa/inet.h>
#include "crc16.h"

namespace  cc_quotes {

    typedef std::map<std::string, std::string> PropertyMap;
    typedef unsigned char *BytePtr;
    typedef std::vector<unsigned char> ByteArray;
    typedef std::shared_ptr<ByteArray> ByteArrayPtr;

//#define MESSAGE_TRAVERSE_DOWN     	"traverse_down"       //下行消息
//#define MESSAGE_TRAVERSE_UP     	    "traverse_up"       //上行消息
//#define MESSAGE_LOGIN     			"login"       // 设备登录
//#define MESSAGE_LOGIN_RESP     		"login_resp"       // 设备登录反馈
//#define MESSAGE_HEARTBEAT     		"heartbeat"       // 设备登录反馈
//#define MESSAGE_STATUS_QUERY     	"data_query"       // 设备登录反馈
//#define MESSAGE_STATUS     			"data_report"       	// 设备状态上报
//#define MESSAGE_VALUE_SET     		"data_set"       // 设备参数设置
//


    struct Message {
        std::uint32_t   ver;
        std::string     id;
        std::uint64_t   ts; // timestam;
        std::string     name;
        PropertyMap     attrs;
        ByteArray       data;
    public:
        Message() {
            ver = 0 ;
            ts = 0;
        }

    public:
        typedef std::shared_ptr<Message> Ptr;

        void serStr(const std::string &s, ByteArray &bytes) {
            serUint32(s.size(), bytes);
            std::copy_n(s.c_str(), s.size(), std::back_inserter(bytes));
        }

        void serUint32(const std::uint32_t s, ByteArray &bytes) {
            std::uint32_t size = htonl(s);
            std::copy_n((unsigned char *) &size, 4, std::back_inserter(bytes));
        }

        void serUint64(const std::uint64_t s, ByteArray &bytes) {
            std::uint64_t size = htonll(s);
            std::copy_n((unsigned char *) &size, 8, std::back_inserter(bytes));
        }

        static bool deserStr(const unsigned char *&data, std::uint32_t &size, std::string &value) {
            if (data == nullptr || size == 0) {
                return false;
            }
            std::uint32_t vv;
            if (deserUint32(data, size, vv)) {
                if (vv > size) {
                    return false;
                }
                value.assign((char *) data, vv);
                data += vv;
                size -= vv;
            }
            return true;
        }

        static bool deserUint32(const unsigned char *&data, std::uint32_t &size, std::uint32_t &value) {
            if (data == nullptr || size < sizeof(std::uint32_t)) {
                return false;
            }
            value = ntohl(*(uint32_t *) data);
            data += 4;
            size -= 4;
            return true;
        }

        static bool deserUint64(const unsigned char *&data, std::uint32_t &size, std::uint64_t &value) {
            if (data == nullptr || size < sizeof(std::uint64_t)) {
                return false;
            }
            value = ntohll(*(uint64_t *) data);
            data += 8;
            size -= 8;
            return true;
        }

        static bool deserStrMap(const unsigned char *&data, std::uint32_t &size, PropertyMap &value) {

            std::uint32_t v32;
            if (!deserUint32(data, size, v32)) {
                return false;
            }
            for (auto n = 0; n < v32; n++) {
                std::string k, v;
                if (deserStr(data, size, k) && deserStr(data, size, v)) {
                    value[k] = v;
                }
            }
            return true;
        }

        void serStrMap(const PropertyMap &attrs, ByteArray &bytes) {
            serUint32(attrs.size(), bytes);
            for (auto itr = attrs.begin(); itr != attrs.end(); itr++) {
                serStr(itr->first, bytes);
                serStr(itr->second, bytes);
            }
        }

        virtual ByteArrayPtr marshall() {
            auto bytes = std::make_shared<ByteArray>();
            {
                serUint32(this->ver, *bytes);
                serStr(this->id, *bytes);
                serUint64(this->ts, *bytes);
                serStr(this->name, *bytes);
                serStrMap(this->attrs, *bytes);
                serUint32(this->data.size(), *bytes);
                std::copy_n((unsigned char *) this->data.data(), this->data.size(), std::back_inserter(*bytes));
                std::uint32_t crc = crc16((const char *) bytes->data(), bytes->size());
                serUint32(crc, *bytes);
            }
            return bytes;
        }

        static std::shared_ptr<Message> parse(const unsigned char *data, std::uint32_t size) {
            std::shared_ptr<Message> nil, message = std::make_shared<Message>();
            if (data == nullptr || size <= 4) {
                return nil;
            }
            {
//                std::uint32_t v  =*((uint32_t *) (data + size - 4) );
                std::uint32_t fr = ntohl(*((uint32_t *) (data + size - 4)));
                std::uint32_t crc = crc16((const char *) data, size - 4);

                if (crc != fr) {
                    return nil;
                }
            }


            if (!deserUint32(data, size, message->ver)) {
                return nil;
            }
            if (!deserStr(data, size, message->id)) {
                return nil;
            }
            if (!deserUint64(data, size, message->ts)) {
                return nil;
            }
            if (!deserStr(data, size, message->name)) {
                return nil;
            }
            if (!deserStrMap(data, size, message->attrs)) {
                return nil;
            }
            std::uint32_t sz;
            if (!deserUint32(data, size, sz)) {
                return nil;
            }
            if (sz > 0) {
                std::copy_n(data, sz, std::back_inserter(message->data));
            }
            return message;
        }

    };
}

#endif
