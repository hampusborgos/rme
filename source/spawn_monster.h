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

#ifndef RME_SPAWN_MONSTER_H_
#define RME_SPAWN_MONSTER_H_

class Tile;

class SpawnMonster {
public:
	SpawnMonster(int size = 3) : size(0), selected(false) {setSize(size);}
	~SpawnMonster() {}

	SpawnMonster* deepCopy() {
		SpawnMonster* copy = newd SpawnMonster(size);
		copy->selected = selected;
		return copy;
	}

	bool isSelected() const {return selected;}
	void select() {selected = true;}
	void deselect() {selected = false;}

	// Does not compare selection!
	bool operator==(const SpawnMonster& other) {return size == other.size;}
	bool operator!=(const SpawnMonster& other) {return size != other.size;}

	void setSize(int newsize) {
		ASSERT(size < 100);
		size = newsize;
	}
	int getSize() const {return size;}
protected:
	int size;
	bool selected;
};

typedef std::set<Position> SpawnMonsterPositionList;
typedef std::list<SpawnMonster*> SpawnMonsterList;

class SpawnsMonster {
public:
	SpawnsMonster();
	~SpawnsMonster();

	void addSpawnMonster(Tile* tile);
	void removeSpawnMonster(Tile* tile);

	SpawnMonsterPositionList::iterator begin() {return spawnsMonster.begin();}
	SpawnMonsterPositionList::const_iterator begin() const {return spawnsMonster.begin();}
	SpawnMonsterPositionList::iterator end() {return spawnsMonster.end();}
	SpawnMonsterPositionList::const_iterator end() const {return spawnsMonster.end();}
	void erase(SpawnMonsterPositionList::iterator iter) {spawnsMonster.erase(iter);}
	SpawnMonsterPositionList::iterator find(Position& pos) {return spawnsMonster.find(pos);}
private:
	SpawnMonsterPositionList spawnsMonster;
};


#endif
