//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
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
	}
}

bool MaterialsExtension::isForVersion(uint16_t versionId)
{
	if(for_all_versions) {
		return true;
	}

	return false;
}

std::string MaterialsExtension::getVersionString()
{
	if(for_all_versions) {
		return "All";
	}

	std::string versions = Assets::getVersionName();
	if(versions.empty()) {
		return "None";
	}

	return versions;
}
