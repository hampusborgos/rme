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

class Creature
{
public:
	Creature(CreatureType* type);
	Creature(const std::string& type_name);

	Creature* deepCopy() const;

	const Outfit& getLookType() const;

	bool isSaved() const noexcept { return saved; }
	void save() noexcept { saved = true; }
	void reset() noexcept { saved = false; }

	bool isSelected() const noexcept { return selected; }
	void deselect() noexcept { selected = false; }
	void select() noexcept { selected = true; }

	bool isNpc() const;

	std::string getName() const;
	CreatureBrush* getBrush() const;

	int getSpawnTime() const noexcept { return spawntime; }
	void setSpawnTime(int time) noexcept { spawntime = time; }

	Direction getDirection() const noexcept { return direction; }
	void setDirection(Direction _direction) noexcept { direction = _direction; }

	// Static conversions
	static std::string DirID2Name(uint16_t id);
	static uint16_t DirName2ID(std::string id);

protected:
	std::string type_name;
	Direction direction;
	int spawntime;
	bool saved;
	bool selected;
};

typedef std::vector<Creature*> CreatureVector;
typedef std::list<Creature*> CreatureList;

#endif
