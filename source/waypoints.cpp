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

#include "main.h"

#include "waypoints.h"
#include "map.h"

void Waypoints::addWaypoint(Waypoint* wp)
{
	removeWaypoint(wp->name);
	if(wp->pos != Position()) {
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

Waypoint* Waypoints::getWaypoint(TileLocation* location)
{
	if(!location)
		return nullptr;
	// TODO find waypoint by position hash.
	for(WaypointMap::iterator it = waypoints.begin(); it != waypoints.end(); it++) {
		Waypoint* waypoint = it->second;
		if(waypoint && waypoint->pos == location->position)
			return waypoint;
	}
	return nullptr;
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
