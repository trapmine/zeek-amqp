#include "AMQP.h"

namespace zeek::logging::writer {
	AMQP::AMQP(WriterFrontend* frontend) : WriterBackend(frontend) {
		InitConfigOptions();
		bool filter_options = InitFilterOptions();
		bool check_options = CheckAllOptions();
		this->init_options = filter_options && check_options;
	}

	AMQP::~AMQP() {
		delete this->tagged_json_formatter;
	}

	void AMQP::InitConfigOptions() {
		this->hostname.assign((const char *)BifConst::LogAMQP::hostname->Bytes(),BifConst::LogAMQP::hostname->Len());
		this->port = BifConst::LogAMQP::amqp_port;
		this->vhost.assign((const char *)BifConst::LogAMQP::vhost->Bytes(),BifConst::LogAMQP::vhost->Len());
		this->username.assign((const char *)BifConst::LogAMQP::username->Bytes(),BifConst::LogAMQP::username->Len());
		this->password.assign((const char *)BifConst::LogAMQP::password->Bytes(),BifConst::LogAMQP::password->Len());
		this->queue_name.assign((const char *)BifConst::LogAMQP::queue_name->Bytes(),BifConst::LogAMQP::queue_name->Len());
		this->exchange.assign((const char *)BifConst::LogAMQP::exchange->Bytes(),BifConst::LogAMQP::exchange->Len());
		this->routing_key.assign((const char *)BifConst::LogAMQP::routing_key->Bytes(),BifConst::LogAMQP::routing_key->Len());
	}

	bool AMQP::InitFilterOptions() {
		const WriterInfo& info = Info();
		for (WriterInfo::config_map::const_iterator i = info.config.begin(); i != info.config.end(); i++) {
			if(strcmp(i->first, "hostname") == 0) {
				this->hostname = i->second;
			}
			if(strcmp(i->first, "amqp_port") == 0) {
				int errorno = 0;
				char *endptr;
				this->port = strtol(i->second, &endptr, 10);
				if(errorno != 0 || endptr == i->second) {
					Error(Fmt("Invalid port"));
					return false;
				}
			}
			if(strcmp(i->first, "vhost") == 0) {
				this->vhost = i->second;
			}
			if(strcmp(i->first, "username") == 0) {
				this->username = i->second;
			}
			if(strcmp(i->first, "password") == 0) {
				this->password = i->second;
			}
			if(strcmp(i->first, "queue_name") == 0) {
				this->queue_name = i->second;
			}
			if(strcmp(i->first, "exchange") == 0) {
				this->exchange = i->second;
			}
			if(strcmp(i->first, "routing_key") == 0) {
				this->routing_key = i->second;
			}
		}
		return true;
	}

	bool AMQP::CheckAllOptions() {
		if(this->hostname.empty()) {
			Error(Fmt("AMQP Hostname is not set"));
			return false;
		}
		if(this->port <= 0 || this->port > 65535) {
			Error(Fmt("Invalid AMQP port"));
			return false;
		}
		if(this->vhost.empty()) {
			Error(Fmt("AMQP vhost is not set"));
			return false;
		}
		if(this->username.empty()) {
			Error(Fmt("AMQP username is not set"));
			return false;
		}
		if(this->password.empty()) {
			Error(Fmt("AMQP password is not set"));
			return false;
		}
		if(this->queue_name.empty()) {
			Error(Fmt("AMQP queue name is not set"));
			return false;
		}
		if(this->routing_key.empty()) {
			Error(Fmt("AMQP routing key is not set"));
			return false;
		}

		return true;
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
		int status = amqp_socket_open(socket, this->hostname.c_str(), this->port);

		if(status != 0) {
			Error(Fmt("Cannot open TCP socket"));
			return false;
		}

		//login
		if(!handle_amqp_error(amqp_login(this->amqp_conn,
			this->vhost.c_str(), //vhost
			0, //channel_max
			131072, //frame_max
			0, //heartbeat
			AMQP_SASL_METHOD_PLAIN, //sasl_method
			this->username.c_str(), //username
			this->password.c_str()), //password
			"Logging in")) {
			return false;
		}

		//open a channel for communication
		amqp_channel_open(this->amqp_conn, 1);
		if(!handle_amqp_error(amqp_get_rpc_reply(this->amqp_conn),"Opening channel")) {
			return false;
		}

		//declare a queue
		amqp_queue_declare(this->amqp_conn, 1, amqp_cstring_bytes(this->queue_name.c_str()), 0, 1, 0, 0, amqp_empty_table);
		if(!handle_amqp_error(amqp_get_rpc_reply(this->amqp_conn),"Declaring queue")) {
			return false;
		}

		return true;
	}

	bool AMQP::DoWrite(int num_fields, const zeek::threading::Field* const* fields, zeek::threading::Value** vals) {
		//clear the buffer and write the log entry in the buffer
		ODesc buff;
		buff.Clear();
		this->tagged_json_formatter->Describe(&buff, num_fields, fields, vals);

		//create amqp properties for publishing message
		amqp_basic_properties_t props;
		props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
		props.content_type = amqp_cstring_bytes("text/plain");
		props.delivery_mode = AMQP_DELIVERY_NONPERSISTENT;

		//publish amqp message
		int publish_result = amqp_basic_publish(
			this->amqp_conn,
			1, //channel
			amqp_cstring_bytes(this->exchange.c_str()), //exchange
			amqp_cstring_bytes(this->routing_key.c_str()), //routing key
			0, // mandatory
			0, // immediate
			&props, // properties
			amqp_cstring_bytes((const char *)buff.Bytes()) // log entry
		);

		if(publish_result != 0) {
			Error(Fmt("%s: %s\n", "Publishing log entry", amqp_error_string2(publish_result)));
			return false;
		}

		return true;
	}

	bool AMQP::DoSetBuf(bool enabled) {
		return true;
	}

	bool AMQP::DoFlush(double network_time) {
		return true;
	}

	bool AMQP::DoRotate(const char* rotated_path, double open, double close, bool terminating) {
		return FinishedRotation();
	}

	bool AMQP::DoHeartbeat(double network_time, double current_time) {
		return true;
	}

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
