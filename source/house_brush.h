#ifndef RME_HOUSE_BRUSH_H
#define RME_HOUSE_BRUSH_H

#include "brush.h"

//=============================================================================
// HouseBrush, draw house tiles
// This brush is created on demand and NOT loaded, as such, the load() method is empty
// Should be deleted by the owning palette

// Forward declaration
class HouseBrush : public Brush {
public:
	HouseBrush();
	virtual ~HouseBrush();
	// Not used
	virtual bool load(pugi::xml_node node, wxArrayString& warnings) {return true;}

	// You can always draw house tiles!
	virtual bool canDraw(BaseMap* map, const Position& position) const {return true;}
	// Draw the shit!
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	// Undraw the shit!
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const {return true;}

	void setHouse(House* house);
	
	uint32_t getHouseID() const;
	virtual int getLookID() const {return 0;} // We don't have a graphic
	virtual std::string getName() const {return "House Brush";}
protected:
	House* draw_house;
};

#endif
