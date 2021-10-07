#ifndef ZEEK_AMQP_TAGGED_JSON_H
#define ZEEK_AMQP_TAGGED_JSON_H

#include <string>
#include <zeek/Desc.h>
#include <zeek/threading/Formatter.h>
#include <zeek/threading/formatters/JSON.h>

using zeek::threading::Field;
using zeek::threading::MsgThread;
using zeek::threading::Value;
using zeek::threading::formatter::JSON;

namespace zeek::threading::formatter {
    class TaggedJSON : public JSON {
    public:
        TaggedJSON(std::string log_name, MsgThread *thread, JSON::TimeFormat tf);
        virtual ~TaggedJSON();
        virtual bool Describe(ODesc *desc, int num_fields, const Field *const *fields, Value **vals) const;

    private:
        std::string log_name;
    };
}
