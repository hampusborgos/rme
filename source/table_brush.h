#ifndef RME_TABLE_BRUSH_H
#define RME_TABLE_BRUSH_H

#include "brush.h"

//=============================================================================
// Tablebrush, for tables, and some things that behave like tables
// and with tables I really mean counters.

class TableBrush : public Brush
{
public:
	static void init();
	
	TableBrush();
	virtual ~TableBrush();

	virtual bool load(pugi::xml_node node, wxArrayString& warnings);

	virtual bool canDraw(BaseMap* map, Position pos) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	static void doTables(BaseMap* map, Tile* tile);

	virtual int getLookID() const;
	virtual void setName(std::string newname);
	virtual std::string getName() const;
	virtual bool needBorders() const {return true;}
protected:
	struct TableType
	{
		TableType() : chance(0), item_id(0) {}
		int chance;
		uint16_t item_id;
	};

	struct TableNode
	{
		TableNode() : total_chance(0) {}
		int total_chance;
		std::vector<TableType> items;
	};

	std::string name;
	uint16_t look_id;
	TableNode table_items[7];

	static uint32_t table_types[256];
};

#endif
