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
	////
}

MaterialsExtension::~MaterialsExtension()
{
	////
}

void MaterialsExtension::addVersion(const std::string& versionString)
{
	if(versionString == "all") {
		for_all_versions = true;
	} else {
		ClientVersion* client = ClientVersion::get(versionString);
		if(client) {
			ClientVersionList supported_versions = ClientVersion::getAllVersionsSupportedForClientVersion(client);
			version_list.insert(version_list.end(), supported_versions.begin(), supported_versions.end());
		}
	}
}

bool MaterialsExtension::isForVersion(uint16_t versionId)
{
	if(for_all_versions) {
		return true;
	}

	for(ClientVersion* version : version_list) {
		if(version->getID() == versionId) {
			return true;
		}
	}
	return false;
}

std::string MaterialsExtension::getVersionString()
{
	if(for_all_versions) {
		return "All";
	}

	std::string versions;
	std::string last;
	for(ClientVersion* version : version_list) {
		if(!last.empty()) {
			if(!versions.empty()) {
				versions += ", " + last;
			} else {
				versions = last;
			}
		}
		last = version->getName();
	}

	if(!last.empty()) {
		if(!versions.empty()) {
			versions += " and " + last;
		} else {
			versions = last;
		}
	} else {
		return "None";
	}
	return versions;
}
