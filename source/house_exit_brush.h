#ifndef RME_HOUSE_EXIT_BRUSH_H
#define RME_HOUSE_EXIT_BRUSH_H

#include "brush.h"

//=============================================================================
// HouseExitBrush, draw house exit tiles
// This doesn't actually draw anything, and the draw/undraw functions will ASSERT if
// you try to call them, so I strongly advice against it

class HouseExitBrush : public Brush
{
public:
	HouseExitBrush();
	virtual ~HouseExitBrush();

	bool isHouseExit() const { return true; }
	HouseExitBrush* asHouseExit() { return static_cast<HouseExitBrush*>(this); }

	// Not used
	virtual bool load(pugi::xml_node node, wxArrayString& warnings) { return true; }

	virtual bool canDraw(BaseMap* map, const Position& position) const;
	// Will ASSERT
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const { return false; }
	virtual bool canSmear() const { return false; }
	virtual bool oneSizeFitsAll() const { return true; }

	void setHouse(House* house);

	uint32_t getHouseID() const;
	virtual int getLookID() const { return 0; } // We don't have a graphic
	virtual std::string getName() const { return "House Exit Brush"; } // We don't have a name

protected:
	uint32_t draw_house;
};

#endif
