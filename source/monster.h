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

#ifndef RME_MONSTER_H_
#define RME_MONSTER_H_

#include "monsters.h"
#include "enums.h"

class Monster {
public:
	Monster(MonsterType* ctype);
	Monster(std::string type_name);
	~Monster();

	// Static conversions
	static std::string DirID2Name(uint16_t id);
	static uint16_t DirName2ID(std::string id);

	Monster* deepCopy() const;

	const Outfit& getLookType() const;

	bool isSaved();
	void save();
	void reset();

	bool isSelected() const {return selected;}
	void deselect() {selected = false;}
	void select() {selected = true;}

	std::string getName() const;
	MonsterBrush* getBrush() const;

	int getSpawnMonsterTime() const {return spawntime;}
	void setSpawnMonsterTime(int spawntime) {this->spawntime = spawntime;}

	Direction getDirection() const { return direction; }
	void setDirection(Direction direction) { this->direction = direction; }

protected:
	std::string type_name;
	Direction direction;
	int spawntime;
	bool saved;
	bool selected;
};

inline void Monster::save() {
	saved = true;
}

inline void Monster::reset() {
	saved = false;
}

inline bool Monster::isSaved() {
	return saved;
}

inline std::string Monster::getName() const {
	MonsterType* type = g_monsters[type_name];
	if(type) {
		return type->name;
	}
	return "";
}
inline MonsterBrush* Monster::getBrush() const {
	MonsterType* type = g_monsters[type_name];
	if(type) {
		return type->brush;
	}
	return nullptr;
}

typedef std::vector<Monster*> MonsterVector;
typedef std::list<Monster*> MonsterList;

#endif
