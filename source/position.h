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

#ifndef __POSITION_HPP__
#define __POSITION_HPP__

#include <ostream>
#include <cstdint>
#include <vector>
#include <list>

class SmallPosition;

class Position {
public:
	// We use int since it's the native machine type and can be several times faster than
	// the other integer types in most cases, also, the position may be negative in some
	// cases
	int x, y, z;

	Position() : x(0), y(0), z(0) {}
	Position(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

	bool operator<(const Position& p) const {
		if(z < p.z)
			return true;
		if(z > p.z)
			return false;

		if(y < p.y)
			return true;
		if(y > p.y)
			return false;

		if(x < p.x)
			return true;
		//if(x > p.x)
		//	return false;


		return false;
	}

	bool operator>(const Position& p) const {
		return !(*this < p);
	}

	Position operator-(const Position& p) const {
		Position newpos;
		newpos.x = x - p.x;
		newpos.y = y - p.y;
		newpos.z = z - p.z;
		return newpos;
	}

	Position operator+(const Position& p) const {
		Position newpos;
		newpos.x = x + p.x;
		newpos.y = y + p.y;
		newpos.z = z + p.z;
		return newpos;
	}

	Position& operator+=(const Position& p) {
		*this = *this + p;
		return *this;
	}

	bool operator==(const Position& p) const {
		return p.x == x && p.y == y && p.z == z;
	}

	bool operator!=(const Position& p) const {
		return ! (*this == p);
	}

	bool isValid() const;
};

inline std::ostream& operator<<(std::ostream& os, const Position& pos) {
	os << pos.x << ':' << pos.y << ':' << pos.z;
	return os;
}

inline std::istream& operator>>(std::istream& is, Position& pos) {
	char a, b;
	int x, y, z;
	is >> x;
	if(!is) return is;
	is >> a;
	if(!is || a != ':') return is;
	is >> y;
	if(!is) return is;
	is >> b;
	if(!is || b != ':') return is;
	is >> z;
	if(!is) return is;

	pos.x = x;
	pos.y = y;
	pos.z = z;

	return is;
}

inline bool Position::isValid() const {
	return x >= 0 && x <= MAP_MAX_WIDTH && y >= 0 && y <= MAP_MAX_HEIGHT && z >= 0 && z <= MAP_MAX_LAYER;
}

inline Position abs(const Position& position) {
	return Position(
		std::abs(position.x),
		std::abs(position.y),
		std::abs(position.z)
	);
}

typedef std::vector<Position> PositionVector;
typedef std::list<Position> PositionList;

#endif
