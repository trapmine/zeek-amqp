#include "AMQP.h"

namespace zeek::logging::writer {
	AMQP::AMQP(WriterFrontend* frontend) : WriterBackend(frontend) {}

	AMQP::~AMQP() {
		delete this->tagged_json_formatter;
	}

    bool AMQP::DoInit(const WriterInfo& info, int num_fields, const zeek::threading::Field* const* fields) {
		//initialize log formatter
		this->tagged_json_formatter = new zeek::threading::formatter::TaggedJSON(info.path,
            this,
            zeek::threading::formatter::JSON::TS_EPOCH);

		//create connection state and a new socket associated with this connection
		this->amqp_conn = amqp_new_connection();
		amqp_socket_t *socket = amqp_tcp_socket_new(this->amqp_conn);

		if(!socket) {
			Error(Fmt("Cannot create TCP socket"));
			return false;
		}

		//open the socket connection
		int status = amqp_socket_open(socket,
			"localhost", //hostname
			5672 //port
		);

		if(status != 0) {
			Error(Fmt("Cannot open TCP socket"));
			return false;
		}

		//login
        if(!handle_amqp_error(amqp_login(this->amqp_conn,
            "/", //vhost
            0, //channel_max
            131072, //frame_max
            0, //heartbeat
            AMQP_SASL_METHOD_PLAIN, //sasl_method
            "guest", //username
            "guest"), //password
            "Logging in")) {
            return false;
        }

		//open a channel for communication
        amqp_channel_open(this->amqp_conn, 1);
        if(!handle_amqp_error(amqp_get_rpc_reply(this->amqp_conn),"Opening channel")) {
            return false;
        }

		//declare a queue
        amqp_queue_declare(this->amqp_conn, 1, amqp_cstring_bytes("test_queue"), 0, 1, 0, 0, amqp_empty_table);
        if(!handle_amqp_error(amqp_get_rpc_reply(this->amqp_conn),"Declaring queue")) {
            return false;
        }

		return true;
	}

	bool AMQP::DoWrite(int num_fields, const zeek::threading::Field* const* fields, zeek::threading::Value** vals) { return true; }

	bool AMQP::DoSetBuf(bool enabled) { return true; }

	bool AMQP::DoFlush(double network_time) { return true; }

	bool AMQP::DoRotate(const char* rotated_path, double open, double close, bool terminating) { return true; }

	bool AMQP::DoHeartbeat(double network_time, double current_time) { return true; }

	bool AMQP::DoFinish(double network_time) {
		//close channel
        if(!handle_amqp_error(amqp_channel_close(this->amqp_conn,
            1,
            AMQP_REPLY_SUCCESS),
            "Closing Channel")) {
            return false;
        }

		//close connection
        if(!handle_amqp_error(amqp_connection_close(this->amqp_conn,
            AMQP_REPLY_SUCCESS),
            "Closing connection")) {
            return false;
        }

		//destroy connection state
        int destroy_result = amqp_destroy_connection(this->amqp_conn);
        if(destroy_result != 0) {
            Error(Fmt("%s: %s\n", "Destroying connection", amqp_error_string2(destroy_result)));
            return false;
        }
        return true;
	}

	bool AMQP::handle_amqp_error(amqp_rpc_reply_t x, char const *context) {
        switch (x.reply_type) {
            case AMQP_RESPONSE_NORMAL:
                return true;

            case AMQP_RESPONSE_NONE:
                Error(Fmt("%s: missing RPC reply type!\n", context));
                break;

            case AMQP_RESPONSE_LIBRARY_EXCEPTION:
                Error(Fmt("%s: %s\n", context, amqp_error_string2(x.library_error)));
                break;

            case AMQP_RESPONSE_SERVER_EXCEPTION:
                switch (x.reply.id) {
                    case AMQP_CONNECTION_CLOSE_METHOD: {
                        amqp_connection_close_t *m =
                            (amqp_connection_close_t *)x.reply.decoded;
                        Error(Fmt("%s: server connection error %uh, message: %.*s\n",
                                context, m->reply_code, (int)m->reply_text.len,
                                (char *)m->reply_text.bytes));
                        break;
                    }
                    case AMQP_CHANNEL_CLOSE_METHOD: {
                        amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
                        Error(Fmt("%s: server channel error %uh, message: %.*s\n",
                                context, m->reply_code, (int)m->reply_text.len,
                                (char *)m->reply_text.bytes));
                        break;
                    }
                    default:
                        Error(Fmt("%s: unknown server error, method id 0x%08X\n",
                                context, x.reply.id));
                        break;
                }
                break;
        }
        return false;
    }
}
