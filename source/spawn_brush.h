#ifndef RME_SPAWN_BRUSH_H
#define RME_SPAWN_BRUSH_H

#include "brush.h"

//=============================================================================
// SpawnBrush, place spawns

class SpawnBrush : public Brush {
public:
	SpawnBrush(); // Create a RAWBrush of the specified type
	virtual ~SpawnBrush();

	virtual bool canDraw(BaseMap* map, Position pos) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter); // parameter is brush size
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual int getLookID() const; // We don't have a look, sorry!
	virtual std::string getName() const;
	virtual bool canDrag() const {return true;}
	virtual bool canSmear() const {return false;}
	virtual bool oneSizeFitsAll() const {return true;}
};

#endif
