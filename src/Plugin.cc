// See the file "COPYING" in the main distribution directory for copyright.

#include "Plugin.h"
#include "PktDagSrc.h"
#include "iosource/Component.h"

#if ZEEK_VERSION_NUMBER >= 40100
using namespace zeek;
#endif

namespace Endace_DAG { Plugin plugin; }

using namespace Endace_DAG;

plugin::Configuration Endace_DAG::Plugin::Configure()
	{
	AddComponent(new iosource::PktSrcComponent("DAGReader", "endace", iosource::PktSrcComponent::LIVE, PktDagSrc::InstantiatePktDagSrc));

	plugin::Configuration config;
	config.name = "Endace::DAG";
	config.description = "Packet acquisition via Endace DAG capture cards";
	config.version.major = 0;
	config.version.minor = 6;
	return config;
	}
