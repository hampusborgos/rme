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

#ifndef RME_CREATURES_H_
#define RME_CREATURES_H_

#include "outfit.h"

#include <string>
#include <map>

class CreatureType;
class CreatureBrush;

typedef std::map<std::string, CreatureType*> CreatureMap;

class CreatureDatabase
{
protected:
	CreatureMap creature_map;

public:
	typedef CreatureMap::iterator iterator;
	typedef CreatureMap::const_iterator const_iterator;

	CreatureDatabase();
	~CreatureDatabase();

	void clear();

	CreatureType* operator[](const std::string& name);
	CreatureType* addMissingCreatureType(const std::string& name, bool isNpc);
	CreatureType* addCreatureType(const std::string& name, bool isNpc, const Outfit& outfit);

	bool hasMissing() const;
	iterator begin() {return creature_map.begin();}
	iterator end() {return creature_map.end();}

	bool loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings);
	bool importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings);

	bool saveToXML(const FileName& filename);
};

class CreatureType {
public:
	CreatureType();
	CreatureType(const CreatureType& ct);
	CreatureType& operator=(const CreatureType& ct);
	~CreatureType();

	bool isNpc;
	bool missing;
	bool in_other_tileset;
	bool standard;
	std::string name;
	Outfit outfit;
	CreatureBrush* brush;

	static CreatureType* loadFromXML(pugi::xml_node node, wxArrayString& warnings);
	static CreatureType* loadFromOTXML(const FileName& filename, pugi::xml_document& node, wxArrayString& warnings);
};

extern CreatureDatabase g_creatures;

#endif
