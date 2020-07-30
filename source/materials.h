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

#ifndef RME_MATERIALS_H_
#define RME_MATERIALS_H_

#include "extension.h"

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
	bool unserializeMaterials(const FileName& filename, pugi::xml_node node, wxString& error, wxArrayString& warnings);
	bool unserializeTileset(pugi::xml_node node, wxArrayString& warnings);

	MaterialsExtensionList extensions;

private:
	Materials(const Materials&);
	Materials& operator=(const Materials&);
};

extern Materials g_materials;

#endif
