//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_NPCS_H_
#define RME_NPCS_H_

#include "outfit.h"

class NpcType;
class NpcBrush;

typedef std::map<std::string, NpcType*> NpcMap;

class NpcDatabase
{
protected:
	NpcMap npcMap;

public:
	typedef NpcMap::iterator iterator;
	typedef NpcMap::const_iterator const_iterator;

	NpcDatabase();
	~NpcDatabase();

	void clear();

	NpcType* operator[](const std::string& name);
	NpcType* addMissingNpcType(const std::string& name);
	NpcType* addNpcType(const std::string& name, const Outfit& outfit);

	bool hasMissing() const;
	iterator begin() {return npcMap.begin();}
	iterator end() {return npcMap.end();}

	bool loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings);
	bool importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings);

	bool saveToXML(const FileName& filename);

	wxArrayString getMissingNpcNames() const;
};

class NpcType {
public:
	NpcType();
	NpcType(const NpcType& ct);
	NpcType& operator=(const NpcType& ct);
	~NpcType();

	bool missing;
	bool in_other_tileset;
	bool standard;
	std::string name;
	Outfit outfit;
	NpcBrush* brush;

	static NpcType* loadFromXML(pugi::xml_node node, wxArrayString& warnings);
	static NpcType* loadFromOTXML(const FileName& filename, pugi::xml_document& node, wxArrayString& warnings);
};

extern NpcDatabase g_npcs;

#endif
