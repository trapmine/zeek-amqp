#include "AMQP.h"

namespace zeek::logging::writer {
	AMQP::AMQP(WriterFrontend* frontend) : WriterBackend(frontend) {}

	AMQP::~AMQP() {}

    bool AMQP::DoInit(const WriterInfo& info, int num_fields, const zeek::threading::Field* const* fields) { return true; }

	bool AMQP::DoWrite(int num_fields, const zeek::threading::Field* const* fields, zeek::threading::Value** vals) { return true; }

	bool AMQP::DoSetBuf(bool enabled) { return true; }

	bool AMQP::DoFlush(double network_time) { return true; }

	bool AMQP::DoRotate(const char* rotated_path, double open, double close, bool terminating) { return true; }

	bool AMQP::DoHeartbeat(double network_time, double current_time) { return true; }

	bool AMQP::DoFinish(double network_time) { return true; }
