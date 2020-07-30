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

#ifndef RME_WAYPOINT_BRUSH_H
#define RME_WAYPOINT_BRUSH_H

#include "brush.h"

//=============================================================================
// WaypointBrush, draws waypoints simply
// This doesn't actually draw anything, and the draw/undraw functions will ASSERT if
// you try to call them, so I strongly advice against it

class WaypointBrush : public Brush
{
public:
	WaypointBrush();
	virtual ~WaypointBrush();

	bool isWaypoint() const { return true; }
	WaypointBrush* asWaypoint() { return static_cast<WaypointBrush*>(this); }

	// Not used
	virtual bool load(pugi::xml_node node, wxArrayString& warnings) { return true; }

	virtual bool canDraw(BaseMap* map, const Position& position) const;
	// Will ASSERT
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const { return false; }
	virtual bool canSmear() const { return false; }
	virtual bool oneSizeFitsAll() const { return true; }

	std::string getWaypoint() const;
	void setWaypoint(Waypoint* wp);
	virtual int getLookID() const { return 0; } // We don't have a graphic
	virtual std::string getName() const { return "Waypoint Brush"; }

protected:
	std::string waypoint_name;
};

#endif
