
#include "Plugin.h"

namespace plugin { namespace Trapmine_AMQPWriter { Plugin plugin; } }

using namespace plugin::Trapmine_AMQPWriter;

zeek::plugin::Configuration Plugin::Configure()
	{
	zeek::plugin::Configuration config;
	config.name = "Trapmine::AMQPWriter";
	config.description = "<Insert description>";
	config.version.major = 0;
	config.version.minor = 1;
	config.version.patch = 0;
	return config;
	}
