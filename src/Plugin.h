
#pragma once

#include <zeek/plugin/Plugin.h>

namespace plugin {
namespace Trapmine_AMQPWriter {

class Plugin : public zeek::plugin::Plugin
{
protected:
	// Overridden from zeek::plugin::Plugin.
	zeek::plugin::Configuration Configure() override;
};

extern Plugin plugin;

}
}
