#include "TaggedJSON.h"

namespace zeek::threading::formatter {
    TaggedJSON::TaggedJSON(std::string log_name, MsgThread *thread, JSON::TimeFormat tf) : JSON(thread, tf) {
        this->log_name = log_name;
    }

    TaggedJSON::~TaggedJSON() {

    }

    bool TaggedJSON::Describe(ODesc *desc, int num_fields, const Field *const *fields, Value **vals) const {
		//create outer JSON object with a key of logname and
        desc->AddRaw("{\"");
        desc->AddRaw(this->log_name);
        desc->AddRaw("\": ");

		//use parent JSON class to get JSON representation of log entry
        if(!JSON::Describe(desc, num_fields, fields, vals)) {
            return false;
        }

		//finish the object
        desc->AddRaw("}");
        return true;
    }
}
