
#include "Plugin.h"

namespace plugin { namespace Endace_DAG { Plugin plugin; } }

using namespace plugin::Endace_DAG;

plugin::Configuration Plugin::Configure()
	{
	plugin::Configuration config;
	config.name = "Endace::DAG";
	config.description = "<Insert description>";
	config.version.major = 0;
	config.version.minor = 1;
	return config;
	}
