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

class Position
{
public:
	// We use int since it's the native machine type and can be several times faster than
	// the other integer types in most cases, also, the position may be negative in some
	// cases
	int x, y, z;

	Position() : x(0), y(0), z(0) {}
	Position(int x, int y, int z) : x(x), y(y), z(z) {}

	bool operator<(const Position& other) const noexcept {
		if(z < other.z)
			return true;
		if(z > other.z)
			return false;

		if(y < other.y)
			return true;
		if(y > other.y)
			return false;

		if(x < other.x)
			return true;
		//if(x > p.x)
		//	return false;

		return false;
	}

	bool operator>(const Position& other) const noexcept {
		return !(*this < other);
	}

	Position operator-(const Position& other) const noexcept {
		return Position(x - other.x, y - other.y, z - other.z);
	}

	Position operator+(const Position& other) const noexcept {
		return Position(x + other.x, y + other.y, z + other.z);
	}

	Position& operator+=(const Position& other) {
		*this = *this + other;
		return *this;
	}

	bool operator==(const Position& other) const noexcept {
		return other.z == z && other.x == x && other.y == y;
	}

	bool operator!=(const Position& other) const noexcept {
		return !(*this == other);
	}

	bool isValid() const noexcept {
		if(x == 0 && y == 0 && z == 0)
			return false;
		return (z >= rme::MapMinLayer && z <= rme::MapMaxLayer)
			&& (x >= 0 && x <= rme::MapMaxWidth)
			&& (y >= 0 && y <= rme::MapMaxHeight);
	}
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
