#ifndef RME_RAW_BRUSH_H
#define RME_RAW_BRUSH_H

#include "brush.h"

//=============================================================================
// RAWBrush, draw items like SimOne's editor

class RAWBrush : public Brush
{
public:
	RAWBrush(uint16_t itemid); // Create a RAWBrush of the specified type
	virtual ~RAWBrush();

	bool isRaw() const { return true; }
	RAWBrush* asRaw() { return static_cast<RAWBrush*>(this); }

	virtual bool canDraw(BaseMap* map, const Position& position) const { return true; }
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const { return true; }
	virtual int getLookID() const;
	virtual std::string getName() const;
	ItemType* getItemType() const { return itemtype; }
	uint16_t getItemID() const;

protected:
	ItemType* itemtype;
};

#endif
