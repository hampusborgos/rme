//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
	bool unserializeMaterials(const FileName& filename, xmlNodePtr node, wxString& error, wxArrayString& warnings);
	bool unserializeTileset(xmlNodePtr node, wxArrayString& warnings);
	
	MaterialsExtensionList extensions;

private:
	Materials(const Materials&);
	Materials& operator=(const Materials&);
};

extern Materials materials;

#endif
