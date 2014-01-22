#ifndef RME_CARPET_BRUSH_H
#define RME_CARPET_BRUSH_H

#include "brush.h"

//=============================================================================
// Carpetbrush, for tables, and some things that behave like tables
// and with tables I really mean counters.

class CarpetBrush : public Brush {
public:
	static void init();
	
	CarpetBrush();
	virtual ~CarpetBrush();

	virtual bool load(pugi::xml_node node, wxArrayString& warnings);

	virtual bool canDraw(BaseMap* map, Position pos) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	static void doCarpets(BaseMap* map, Tile* tile);

	virtual bool canDrag() const {return true;}
	virtual int getLookID() const;
	virtual void setName(std::string newname);
	virtual std::string getName() const;
	virtual bool needBorders() const {return true;}
protected:
	uint16_t getRandomCarpet(BorderType alignment);

	struct CarpetType {
		uint16_t id;
		int chance;
	};

	struct CarpetNode {
		CarpetNode() : total_chance(0) {}
		int total_chance;
		std::vector<CarpetType> items;
	};

	std::string name;
	uint16_t look_id;
	CarpetNode carpet_items[14];
	static uint32_t carpet_types[256];
};

#endif
