// See the file "COPYING" in the main distribution directory for copyright.

#include "Plugin.h"
#include "PktDagSrc.h"
#include "iosource/Component.h"

namespace plugin { namespace Endace_DAG { Plugin plugin; } }

using namespace plugin::Endace_DAG;

plugin::Configuration Plugin::Configure()
	{
	AddComponent(new ::iosource::PktSrcComponent("DAGReader", "endace", ::iosource::PktSrcComponent::LIVE, ::iosource::pktsrc::PktDagSrc::InstantiatePktDagSrc));

	plugin::Configuration config;
	config.name = "Endace::DAG";
	config.description = "Packet acquisition via Endace DAG capture cards";
	config.version.major = 0;
	config.version.minor = 3;
	return config;
	}
