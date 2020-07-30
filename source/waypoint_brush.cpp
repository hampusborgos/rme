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

#include "waypoint_brush.h"
#include "waypoints.h"
#include "basemap.h"

//=============================================================================
// Waypoint Brush

WaypointBrush::WaypointBrush() :
	Brush()
{
	////
}

WaypointBrush::~WaypointBrush()
{
	////
}

void WaypointBrush::setWaypoint(Waypoint* wp)
{
	if(wp) {
		waypoint_name = wp->name;
	} else {
		waypoint_name = "";
	}
}

std::string WaypointBrush::getWaypoint() const
{
	return waypoint_name;
}

bool WaypointBrush::canDraw(BaseMap* map, const Position& position) const
{
	return map->getTile(position) != nullptr;
}

void WaypointBrush::undraw(BaseMap* map, Tile* tile)
{
	// Never called
	ASSERT(false);
}

void WaypointBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	// Never called
	ASSERT(false);
}
