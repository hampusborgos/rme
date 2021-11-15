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

#ifndef RME_OUTFIT_H_
#define RME_OUTFIT_H_

struct Outfit {
	Outfit() : lookType(0), lookItem(0), lookMount(0), lookAddon(0), lookHead(0), lookBody(0), lookLegs(0), lookFeet(0), lookMountHead(0), lookMountBody(0), lookMountLegs(0), lookMountFeet(0) {}
	~Outfit() {}
	int lookType;
	int lookItem;
	int lookMount;
	int lookAddon;
	int lookHead;
	int lookBody;
	int lookLegs;
	int lookFeet;
	int lookMountHead;
	int lookMountBody;
	int lookMountLegs;
	int lookMountFeet;

	uint32_t getColorHash() const {
		return lookHead << 24 | lookBody << 16 | lookLegs << 8 | lookFeet;
	}
};

#endif
