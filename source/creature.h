//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////


#ifndef RME_CREATURE_H_
#define RME_CREATURE_H_


#include "creatures.h"

class Creature {
public:
	Creature(CreatureType* ctype);
	Creature(std::string type_name);
	~Creature();

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

protected:
	std::string type_name;
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
	CreatureType* type = creature_db[type_name];
	if(type) {
		return type->isNpc;
	}
	return false;
}

inline std::string Creature::getName() const {
	CreatureType* type = creature_db[type_name];
	if(type) {
		return type->name;
	}
	return "";
}
inline CreatureBrush* Creature::getBrush() const {
	CreatureType* type = creature_db[type_name];
	if(type) {
		return type->brush;
	}
	return nullptr;
}

typedef std::vector<Creature*> CreatureVector;
typedef std::list<Creature*> CreatureList;

#endif
