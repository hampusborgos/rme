//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/materials.h $
// $Id: materials.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_MATERIALS_H_
#define RME_MATERIALS_H_

#include "tileset.h"

class Brush;
class ItemType;
class TerrainBrush;

class MaterialsExtension {
public:
	MaterialsExtension(std::string name, std::string author, std::string description);
	~MaterialsExtension();

	void addVersion(std::string str);
	bool isForVersion(uint16_t ver_id);
	std::string getVersionString();

	std::string name;
	std::string author;
	std::string description;
	bool for_all_versions;
	std::vector<uint16_t> versionList;
private:
	MaterialsExtension(const MaterialsExtension&);
	MaterialsExtension& operator=(const MaterialsExtension&);
};

typedef std::vector<MaterialsExtension*> MaterialsExtensionList;

class Materials {
public:
	Materials();
	~Materials();

	void clear();
	
	const MaterialsExtensionList& getExtensions();
	MaterialsExtensionList getExtensionsByVersion(uint16_t version_id);

	TilesetContainer tilesets;

	bool loadMaterials(const FileName& identifier, wxString& error, wxArrayString& warnings);
	bool loadExtensions(FileName identifier, wxString& error, wxArrayString& warnings);
	void createOtherTileset();

	bool isInTileset(Item* item, std::string tileset) const;
	bool isInTileset(Brush* brush, std::string tileset) const;

protected:
	bool unserializeMaterials(const FileName& filename, xmlNodePtr node, wxString& error, wxArrayString& warnings);
	bool unserializeTileset(xmlNodePtr node, wxArrayString& warnings);
	
	MaterialsExtensionList extensions;

private:
	Materials(const Materials&);
	Materials& operator=(const Materials&);
};

extern Materials materials;

#endif
