//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "waypoint_brush.h"
#include "waypoints.h"

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
	//if(tile == nullptr) {
	//	return false;
	//}
	return true;
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
