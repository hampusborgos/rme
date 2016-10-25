//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////


#ifndef RME_CREATURE_H_
#define RME_CREATURE_H_

#include "creatures.h"

enum Direction
{
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,

	DIRECTION_FIRST = NORTH,
	DIRECTION_LAST = WEST
};

IMPLEMENT_INCREMENT_OP(Direction)

class Creature {
public:
	Creature(CreatureType* ctype);
	Creature(std::string type_name);
	~Creature();

	// Static conversions
	static std::string DirID2Name(uint16_t id);
	static uint16_t DirName2ID(std::string id);

	Creature* deepCopy() const;

	const Outfit& getLookType() const;

	bool isSaved();
	void save();
	void reset();

	bool isSelected() const {return selected;}
	void deselect() {selected = false;}
	void select() {selected = true;}

	bool isNpc() const;

	std::string getName() const;
	CreatureBrush* getBrush() const;

	int getSpawnTime() const {return spawntime;}
	void setSpawnTime(int spawntime) {this->spawntime = spawntime;}

	Direction getDirection() const { return direction; }
	void setDirection(Direction direction) { this->direction = direction; }

protected:
	std::string type_name;
	Direction direction;
	int spawntime;
	bool saved;
	bool selected;
};

inline void Creature::save() {
	saved = true;
}

inline void Creature::reset() {
	saved = false;
}

inline bool Creature::isSaved() {
	return saved;
}

inline bool Creature::isNpc() const {
	CreatureType* type = g_creatures[type_name];
	if(type) {
		return type->isNpc;
	}
	return false;
}

inline std::string Creature::getName() const {
	CreatureType* type = g_creatures[type_name];
	if(type) {
		return type->name;
	}
	return "";
}
inline CreatureBrush* Creature::getBrush() const {
	CreatureType* type = g_creatures[type_name];
	if(type) {
		return type->brush;
	}
	return nullptr;
}

typedef std::vector<Creature*> CreatureVector;
typedef std::list<Creature*> CreatureList;

#endif
