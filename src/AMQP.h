#ifndef ZEEK_AMQP_WRITER_H
#define ZEEK_AMQP_WRITER_H

#include <zeek/logging/WriterBackend.h>
#include "amqpwriter.bif.h"

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
		};
}

#endif
