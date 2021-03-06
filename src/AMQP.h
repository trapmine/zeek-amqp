#ifndef ZEEK_AMQP_WRITER_H
#define ZEEK_AMQP_WRITER_H

#include <cstring>
#include <zeek/logging/WriterBackend.h>
#include <zeek/Desc.h>
#include <zeek/threading/formatters/JSON.h>
#include <zeek/util.h>
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

#include "amqpwriter.bif.h"
#include "TaggedJSON.h"

namespace zeek::plugin::Trapmine_AMQPWriter {
	class Plugin;
}

namespace zeek::logging::writer {
	class AMQP : public WriterBackend {
		public:
			AMQP(WriterFrontend* frontend);
			~AMQP();

			static WriterBackend* Instantiate(WriterFrontend* frontend) {
				return new AMQP(frontend);
			}

		protected:
			virtual bool DoInit(const WriterBackend::WriterInfo& info, int num_fields, const zeek::threading::Field* const* fields);
			virtual bool DoWrite(int num_fields, const zeek::threading::Field* const* fields, zeek::threading::Value** vals);
			virtual bool DoSetBuf(bool enabled);
			virtual bool DoRotate(const char* rotated_path, double open, double close, bool terminating);
			virtual bool DoFlush(double network_time);
			virtual bool DoFinish(double network_time);
			virtual bool DoHeartbeat(double network_time, double current_time);

		private:
			friend class zeek::plugin::Trapmine_AMQPWriter::Plugin;
			amqp_connection_state_t amqp_conn;
			zeek::threading::formatter::TaggedJSON* tagged_json_formatter;
			ODesc desc;

			std::string hostname;
			int port;
			std::string vhost;
			std::string username;
			std::string password;
			std::string queue_name;
			std::string exchange;
			std::string routing_key;

			bool init_options;

			void InitConfigOptions();
			bool InitFilterOptions();
			bool CheckAllOptions();
			bool handle_amqp_error(amqp_rpc_reply_t x, char const *context);
	};
}

#endif
