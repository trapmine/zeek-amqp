
#include "Plugin.h"

namespace plugin { namespace Trapmine_AMQPWriter { Plugin plugin; } }

using namespace plugin::Trapmine_AMQPWriter;

zeek::plugin::Configuration Plugin::Configure()
	{
	AddComponent(new zeek::logging::Component("AMQP",zeek::logging::writer::AMQP::Instantiate));

	zeek::plugin::Configuration config;
	config.name = "Trapmine::AMQPWriter";
	config.description = "Sends logs to AMQP Exchange";
	config.version.major = 0;
	config.version.minor = 1;
	config.version.patch = 0;
	return config;
	}
