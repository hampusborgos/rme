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

#ifndef RME_TILESET_H_
#define RME_TILESET_H_

class Brushes;

enum TilesetCategoryType {
	TILESET_UNKNOWN,
	TILESET_TERRAIN,
	TILESET_CREATURE,
	TILESET_DOODAD,
	TILESET_ITEM,
	TILESET_RAW,
	TILESET_HOUSE,
	TILESET_WAYPOINT,
};

class TilesetCategory {
public:
	TilesetCategory(Tileset& parent, TilesetCategoryType type);
	~TilesetCategory();

	bool isTrivial() const;
	TilesetCategoryType getType() const {return type;}
	size_t size() const {return brushlist.size();}

	void loadBrush(pugi::xml_node node, wxArrayString& warnings);
	void clear();

	bool containsBrush(Brush* brush) const;

protected:
	TilesetCategoryType type;
public:
	std::vector<Brush*> brushlist;
	Tileset& tileset;

private:
	TilesetCategory(const TilesetCategory&);
	TilesetCategory operator=(const TilesetCategory&);
};

typedef std::vector<TilesetCategory*> TilesetCategoryArray;

class Tileset {
public:
	Tileset(Brushes& brushes, const std::string& name);
	~Tileset();

	TilesetCategory* getCategory(TilesetCategoryType type);
	const TilesetCategory* getCategory(TilesetCategoryType type) const;

	void loadCategory(pugi::xml_node node, wxArrayString& warnings);
	void clear();

	bool containsBrush(Brush* brush) const;

public:
	std::string name;
	TilesetCategoryArray categories;

protected:
	Brushes& brushes;

protected:
	Tileset(const Tileset&);
	Tileset operator=(const Tileset&);

	friend class TilesetCategory;
};

typedef std::map<std::string, Tileset*> TilesetContainer;

#endif
