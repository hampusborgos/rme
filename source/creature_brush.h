#ifndef RME_CREATURE_BRUSH_H
#define RME_CREATURE_BRUSH_H

#include "brush.h"

//=============================================================================
// CreatureBrush, place creatures

class CreatureBrush : public Brush {
public:
	CreatureBrush(CreatureType* type); // Create a RAWBrush of the specified type
	virtual ~CreatureBrush();

	virtual bool canDraw(BaseMap* map, Position pos) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	CreatureType* getType() const {return creature_type;}

	virtual int getLookID() const; // We don't have a look type, this will always return 0
	virtual std::string getName() const;
	virtual bool canDrag() const {return false;}
	virtual bool canSmear() const {return true;}
	virtual bool oneSizeFitsAll() const {return true;}
protected:
	CreatureType* creature_type;
};

#endif
