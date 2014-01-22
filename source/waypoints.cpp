//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/waypoints.hpp $
// $Id: waypoints.hpp 298 2010-02-23 17:09:13Z admin $

#include "main.h"

#include "waypoints.h"
#include "map.h"

void Waypoints::addWaypoint(Waypoint* wp)
{
	removeWaypoint(wp->name);
	if(wp->pos != Position())
	{
		Tile* t = map.getTile(wp->pos);
		if(!t)
			map.setTile(wp->pos, t = map.allocator(map.createTileL(wp->pos)));
		t->getLocation()->increaseWaypointCount();
	}
	waypoints.insert(std::make_pair(as_lower_str(wp->name), wp));
}

Waypoint* Waypoints::getWaypoint(std::string name)
{
	to_lower_str(name);
	WaypointMap::iterator iter = waypoints.find(name);
	if(iter == waypoints.end())
		return nullptr;
	return iter->second;
}

void Waypoints::removeWaypoint(std::string name)
{
	to_lower_str(name);
	WaypointMap::iterator iter = waypoints.find(name);
	if(iter == waypoints.end())
		return;
	delete iter->second;
	waypoints.erase(iter);
}
