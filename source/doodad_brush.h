#ifndef RME_DOODAD_BRUSH_H
#define RME_DOODAD_BRUSH_H

#include "brush.h"

//=============================================================================
// Doodadbrush, add doodads!

typedef std::vector<std::pair<Position, ItemVector> > CompositeTileList;

class DoodadBrush : public Brush {
public:
	DoodadBrush();
	virtual ~DoodadBrush();

protected:
	struct AlternativeBlock;
public:

	bool loadAlternative(xmlNodePtr node, wxArrayString& warnings, AlternativeBlock* which = NULL);
	virtual bool load(xmlNodePtr node, wxArrayString& warnings);

	virtual bool canDraw(BaseMap* map, Position pos) const {return true;}
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	const CompositeTileList& getComposite(int variation) const;
	virtual void undraw(BaseMap* map, Tile* tile);

	bool isEmpty(int variation) const;

	int getThickness() const {return thickness;}
	int getThicknessCeiling() const {return thickness_ceiling;}
	
	int getCompositeChance(int variation) const;
	int getSingleChance(int variation) const;
	int getTotalChance(int variation) const;

	bool hasSingleObjects(int variation) const;
	bool hasCompositeObjects(int variation) const;

	bool placeOnBlocking() const {return on_blocking;}
	bool placeOnDuplicate() const {return on_duplicate;}
	bool doNewBorders() const {return do_new_borders;}
	bool ownsItem(Item* item) const;
	
	virtual bool canSmear() const {return draggable;}
	virtual bool canDrag() const {return false;}
	virtual bool oneSizeFitsAll() const {return one_size;}
	virtual int getLookID() const {return look_id;}
	virtual int getMaxVariation() const {return alternatives.size();}
	virtual std::string getName() const {return name;}
	virtual void setName(std::string newname) {name = newname;}

protected:
	std::string name;
	uint16_t look_id;

	int thickness;
	int thickness_ceiling;

	bool draggable;
	bool on_blocking;
	bool one_size;
	bool do_new_borders;
	bool on_duplicate;
	uint16_t clear_mapflags;
	uint16_t clear_statflags;
	
	struct SingleBlock {
		int chance;
		Item* item;
	};

	struct CompositeBlock {
		int chance;
		CompositeTileList items;
	};

	struct AlternativeBlock {
		AlternativeBlock();
		~AlternativeBlock();
		bool ownsItem(uint16_t id) const;
		std::vector<SingleBlock> single_items;
		std::vector<CompositeBlock> composite_items;
		
		int composite_chance; // Total chance of a composite
		int single_chance; // Total chance of a single object
	};

	std::vector<AlternativeBlock*> alternatives;
};

#endif
