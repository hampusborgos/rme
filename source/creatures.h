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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/creatures.h $
// $Id: creatures.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_CREATURES_H_
#define RME_CREATURES_H_

#include "outfit.h"

#include <string>
#include <map>

class CreatureType;
class CreatureBrush;

typedef std::map<std::string, CreatureType*> CreatureMap;

class CreatureDatabase {
public:
	CreatureDatabase();
	~CreatureDatabase();

	void clear();

	CreatureType* operator[](std::string name);
	CreatureType* addMissingCreatureType(std::string name, bool isNpc);

	bool hasMissing() const;
	CreatureMap::iterator begin() {return creature_map.begin();}
	CreatureMap::iterator end() {return creature_map.end();}

	bool loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings);
	bool importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings);

	bool saveToXML(const FileName& filename);
protected:
	CreatureMap creature_map;
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
	
	static CreatureType* loadFromXML(xmlNodePtr node, wxArrayString& warnings);
	static CreatureType* loadFromOTXML(const FileName& filename, xmlDocPtr node, wxArrayString& warnings);
	xmlNodePtr saveToXML();
};

extern CreatureDatabase creature_db;

#endif
