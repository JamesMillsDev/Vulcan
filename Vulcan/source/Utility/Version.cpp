#include "Utility/Version.h"

#include "Utility/Config.h"

Version::Version(const string& name, Config* config)
{
	Json versionArray = config->Get<Json>(name);

	major = versionArray[0].get<int>();
	minor = versionArray[1].get<int>();
	patch = versionArray[2].get<int>();
}
