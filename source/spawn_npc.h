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

#ifndef RME_SPAWN_NPC_H_
#define RME_SPAWN_NPC_H_

class Tile;

class SpawnNpc {
public:
	SpawnNpc(int size = 3) : size(0), selected(false) {setSize(size);}
	~SpawnNpc() {}

	SpawnNpc* deepCopy() {
		SpawnNpc* copy = newd SpawnNpc(size);
		copy->selected = selected;
		return copy;
	}

	bool isSelected() const {return selected;}
	void select() {selected = true;}
	void deselect() {selected = false;}

	// Does not compare selection!
	bool operator==(const SpawnNpc& other) {return size == other.size;}
	bool operator!=(const SpawnNpc& other) {return size != other.size;}

	void setSize(int newsize) {
		ASSERT(size < 100);
		size = newsize;
	}
	int getSize() const {return size;}
protected:
	int size;
	bool selected;
};

typedef std::set<Position> SpawnNpcPositionList;
typedef std::list<SpawnNpc*> SpawnNpcList;

class SpawnsNpc {
public:
	SpawnsNpc();
	~SpawnsNpc();

	void addSpawnNpc(Tile* tile);
	void removeSpawnNpc(Tile* tile);

	SpawnNpcPositionList::iterator begin() {return spawnsNpc.begin();}
	SpawnNpcPositionList::const_iterator begin() const {return spawnsNpc.begin();}
	SpawnNpcPositionList::iterator end() {return spawnsNpc.end();}
	SpawnNpcPositionList::const_iterator end() const {return spawnsNpc.end();}
	void erase(SpawnNpcPositionList::iterator iter) {spawnsNpc.erase(iter);}
	SpawnNpcPositionList::iterator find(Position& pos) {return spawnsNpc.find(pos);}
private:
	SpawnNpcPositionList spawnsNpc;
};

#endif
