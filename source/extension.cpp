//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "extension.h"

MaterialsExtension::MaterialsExtension(std::string name, std::string author, std::string description) :
	name(name),
	author(author),
	description(description),
	for_all_versions(false)
{
	// ...
}

MaterialsExtension::~MaterialsExtension()
{
	// ...
}

void MaterialsExtension::addVersion(std::string verStr)
{
	if(verStr == "all")
	{
		for_all_versions = true;
	}
	else
	{
		ClientVersion* client = ClientVersion::get(verStr);
		if(client)
		{
			ClientVersionList supported_versions = ClientVersion::getAllVersionsSupportedForClientVersion(client);
			version_list.insert(version_list.end(), supported_versions.begin(), supported_versions.end());
		}
	}
}

bool MaterialsExtension::isForVersion(uint16_t ver_id)
{
	if (for_all_versions)
		return true;

	for(ClientVersionList::iterator iter = version_list.begin();
		iter != version_list.end();
		++iter)
	{
		if((*iter)->getID() == ver_id)
			return true;
	}
	return false;
}

std::string MaterialsExtension::getVersionString()
{
	if (for_all_versions)
		return "All";

	std::string versions;
	std::string last;
	for(ClientVersionList::iterator iter = version_list.begin();
		iter != version_list.end();
		++iter)
	{
		if(last.size())
		{
			if(versions.size())
				versions += ", " + last;
			else
				versions = last;
		}

		last = (*iter)->getName();
	}

	if(last.size())
	{
		if(versions.size())
			versions += " and " + last;
		else
			versions = last;
	}
	else
		return "None";

	return versions;
}
