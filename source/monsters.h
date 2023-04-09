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

#ifndef RME_MONSTERS_H_
#define RME_MONSTERS_H_

#include "outfit.h"

class MonsterType;
class MonsterBrush;

typedef std::map<std::string, MonsterType*> MonsterMap;

class MonsterDatabase
{
protected:
	MonsterMap monster_map;

public:
	typedef MonsterMap::iterator iterator;
	typedef MonsterMap::const_iterator const_iterator;

	MonsterDatabase();
	~MonsterDatabase();

	void clear();

	MonsterType* operator[](const std::string& name);
	MonsterType* addMissingMonsterType(const std::string& name);
	MonsterType* addMonsterType(const std::string& name, const Outfit& outfit);

	bool hasMissing() const;
	iterator begin() {return monster_map.begin();}
	iterator end() {return monster_map.end();}

	bool loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings);
	bool importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings);

	bool saveToXML(const FileName& filename);
};

class MonsterType {
public:
	MonsterType();
	MonsterType(const MonsterType& ct);
	MonsterType& operator=(const MonsterType& ct);
	~MonsterType();

	bool missing;
	bool in_other_tileset;
	bool standard;
	std::string name;
	Outfit outfit;
	MonsterBrush* brush;

	static MonsterType* loadFromXML(pugi::xml_node node, wxArrayString& warnings);
	static MonsterType* loadFromOTXML(const FileName& filename, pugi::xml_document& node, wxArrayString& warnings);
};

extern MonsterDatabase g_monsters;

#endif
