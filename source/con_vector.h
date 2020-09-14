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

#ifndef RME_CON_VECTOR_H_
#define RME_CON_VECTOR_H_

#define REALLOC_INCREASE 600

template <class T> // This only really works with pointers.. hrhr "T" might be abit misleading.. :o
class contigous_vector {
	T __pointer_check(T t) {return *t;} // If this fails, you have tried using this class with a non-pointer type, DONT
public:
	contigous_vector(size_t start_size = 7) {
		start = reinterpret_cast<T*>(malloc(sizeof(T) * start_size));
		memset(start, 0, sizeof(T) * start_size);
		sz = start_size;
	}
	~contigous_vector() {
		free(start);
	}

	void resize(size_t new_size) {
		size_t old_size = sz;
		start = reinterpret_cast<T*>(realloc(start, sizeof(T) * new_size));
		memset(start + old_size, 0, sizeof(T) * (new_size - old_size));
		sz = new_size;
	}
	size_t size() {
		return sz;
	}

	T& locate(size_t index) {
		// Masterly inefficient!
		while(index >= sz) {
			resize(sz+REALLOC_INCREASE);
		}
		return start[index];
	}

	T at(size_t index) const {
		if(index >= sz) {
			return nullptr;
		}
		return start[index];
	}

	void set(size_t index, T value) {
		locate(index) = value;
	}

	T operator[](size_t index) {return at(index);}
	const T operator[](size_t index) const {return at(index);}
private:
	T* start;
	size_t sz;
};

#endif
