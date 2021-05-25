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

#ifndef RME_NPC_H_
#define RME_NPC_H_

#include "npcs.h"
#include "enums.h"

class Npc {
public:
	Npc(NpcType* type);
	Npc(std::string type_name);
	~Npc();

	// Static conversions
	static std::string DirID2Name(uint16_t id);
	static uint16_t DirName2ID(std::string id);

	Npc* deepCopy() const;

	const Outfit& getLookType() const;

	bool isSaved();
	void save();
	void reset();

	bool isSelected() const {return selected;}
	void deselect() {selected = false;}
	void select() {selected = true;}

	std::string getName() const;
	NpcBrush* getBrush() const;

	int getSpawnNpcTime() const {return spawnNpcTime;}
	void setSpawnNpcTime(int spawnNpcTime) {this->spawnNpcTime = spawnNpcTime;}

	Direction getDirection() const { return direction; }
	void setDirection(Direction direction) { this->direction = direction; }

protected:
	std::string type_name;
	Direction direction;
	int spawnNpcTime;
	bool saved;
	bool selected;
};

inline void Npc::save() {
	saved = true;
}

inline void Npc::reset() {
	saved = false;
}

inline bool Npc::isSaved() {
	return saved;
}

inline std::string Npc::getName() const {
	NpcType* npcType = g_npcs[type_name];
	if(npcType) {
		return npcType->name;
	}
	return "";
}
inline NpcBrush* Npc::getBrush() const {
	NpcType* npcType = g_npcs[type_name];
	if(npcType) {
		return npcType->brush;
	}
	return nullptr;
}

typedef std::vector<Npc*> NpcVector;
typedef std::list<Npc*> NpcList;

#endif
