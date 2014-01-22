#ifndef RME_WAYPOINT_BRUSH_H
#define RME_WAYPOINT_BRUSH_H

#include "brush.h"

//=============================================================================
// WaypointBrush, draws waypoints simply
// This doesn't actually draw anything, and the draw/undraw functions will ASSERT if
// you try to call them, so I strongly advice against it

class WaypointBrush : public Brush {
public:
	WaypointBrush();
	virtual ~WaypointBrush();
	// Not used
	virtual bool load(pugi::xml_node node, wxArrayString& warnings) {return true;}

	virtual bool canDraw(BaseMap* map, Position pos) const;
	// Will ASSERT
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const {return false;}
	virtual bool canSmear() const {return false;}
	virtual bool oneSizeFitsAll() const {return true;}
	
	std::string getWaypoint() const;
	void setWaypoint(Waypoint* wp);
	virtual int getLookID() const {return 0;} // We don't have a graphic
	virtual std::string getName() const {return "Waypoint Brush";}
protected:
	std::string waypoint_name;
};

#endif
