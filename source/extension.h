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

#ifndef RME_EXTENSION_H_
#define RME_EXTENSION_H_

#include "tileset.h"
#include "client_version.h"

class MaterialsExtension
{
public:
	MaterialsExtension(std::string name, std::string author, std::string description);
	~MaterialsExtension();

	void addVersion(const std::string& versionString);
	bool isForVersion(uint16_t versionId);
	std::string getVersionString();

	std::string name;
	std::string url;
	std::string author;
	std::string author_url;
	std::string description;
	bool for_all_versions;
	ClientVersionList version_list;
private:
	MaterialsExtension(const MaterialsExtension&);
	MaterialsExtension& operator=(const MaterialsExtension&);
};

typedef std::vector<MaterialsExtension*> MaterialsExtensionList;

#endif
