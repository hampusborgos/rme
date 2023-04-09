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

#ifndef RME_FORWARD_H
#define RME_FORWARD_H

class Map;
class Tile;
class TileLocation;
class Item;
class container;
class SpawnMonster;
class SpawnNpc;
struct Outfit;
class House;
class Monster;
class Npc;
class BaseMap;
class Waypoint;
class Waypoints;
class Tileset;
class Town;
class Position;
class Editor;
class GUI;
class Selection;
class CopyBuffer;
class ItemType;
class Monsters;
class Npcs;
class ItemDatabase;
class QTreeNode;
class Floor;
class Action;

class Brush;

typedef std::vector<uint32_t> HouseExitList;
typedef std::vector<Tile*> TileVector;
typedef std::unordered_set<Tile*> TileSet;
typedef std::vector<Item*> ItemVector;
typedef std::vector<Brush*> BrushVector;

#endif
