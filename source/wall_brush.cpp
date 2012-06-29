//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "wall_brush.h"
#include "items.h"
#include "basemap.h"

uint32_t WallBrush::full_border_types[16];
uint32_t WallBrush::half_border_types[16];

WallBrush::WallBrush() :
	redirect_to(NULL)
{

}

WallBrush::~WallBrush() {

}

bool WallBrush::load(xmlNodePtr node, wxArrayString& warnings) {
	std::string strVal;
	int intVal;

	if(readXMLValue(node, "lookid", intVal)) {
		look_id = intVal;
	}
	if(readXMLValue(node, "server_lookid", intVal)) {
		look_id = item_db[intVal].clientID;
	}

	xmlNodePtr child = node->children;
	while(child) {
		if(xmlStrcmp(child->name,(const xmlChar*)"wall") == 0) {
			uint alignment;
			if(readXMLValue(child, "type", strVal)) {
				if(strVal == "vertical") {
					alignment = WALL_VERTICAL;
				} else if(strVal == "horizontal") {
					alignment = WALL_HORIZONTAL;
				} else if(strVal == "corner") {
					alignment = WALL_NORTHWEST_DIAGONAL;
				} else if(strVal == "pole") {
					alignment = WALL_POLE;
				} else if(strVal == "south end") {
					alignment = WALL_SOUTH_END;
				} else if(strVal == "east end") {
					alignment = WALL_EAST_END;
				} else if(strVal == "north end") {
					alignment = WALL_NORTH_END;
				} else if(strVal == "west end") {
					alignment = WALL_WEST_END;
				} else if(strVal == "south T") {
					alignment = WALL_SOUTH_T;
				} else if(strVal == "east T") {
					alignment = WALL_EAST_T;
				} else if(strVal == "west T") {
					alignment = WALL_WEST_T;
				} else if(strVal == "north T") {
					alignment = WALL_NORTH_T;
				} else if(strVal == "northwest diagonal") {
					alignment = WALL_NORTHWEST_DIAGONAL;
				} else if(strVal == "northeast diagonal") {
					alignment = WALL_NORTHEAST_DIAGONAL;
				} else if(strVal == "southwest diagonal") {
					alignment = WALL_SOUTHWEST_DIAGONAL;
				} else if(strVal == "southeast diagonal") {
					alignment = WALL_SOUTHEAST_DIAGONAL;
				} else if(strVal == "intersection") {
					alignment = WALL_INTERSECTION;
				} else if(strVal == "untouchable") {
					alignment = WALL_UNTOUCHABLE;
				} else {
					wxString warning;
					warning << wxT("Unknown wall alignment '") + wxstr(strVal) + wxT("'\n");
					warnings.push_back(warning);
					child = child->next;
					continue;
				}
			} else {
				wxString warning;
				warning << wxT("Could not read type tag of wall node\n");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}

			xmlNodePtr subchild = child->children;
			while(subchild) {
				do {
					if(xmlStrcmp(subchild->name,(const xmlChar*)"item") == 0) {
						int id, chance = 0;
						if(!readXMLValue(subchild, "id", id)) {
							wxString warning;
							warning << wxT("Could not read id tag of item node\n");
							warnings.push_back(warning);
							break;
						}
						readXMLValue(subchild, "chance", chance);

						ItemType& it = item_db[id];
						if(it.id == 0) {
							wxString warning;
							warning << wxT("There is no itemtype with id ") << id;
							warnings.push_back(warning);
							return false;
						}
						if(it.brush != NULL && it.brush != this) {
							wxString warning;
							warning << wxT("Itemtype id ") << id << wxT(" already has a brush");
							warnings.push_back(warning);
							return false;
						}
						it.isWall = true;
						it.brush = this;
						it.border_alignment = ::BorderType(alignment);

						WallType wt;
						wall_items[alignment].total_chance += chance;
						wt.chance = wall_items[alignment].total_chance;

						wt.id  = uint16_t(id);
						wall_items[alignment].items.push_back(wt);
					} else if(xmlStrcmp(subchild->name,(const xmlChar*)"door") == 0) {
						std::string type;
						int id;
						int chance = 0;
						bool isOpen;
						bool hate = false;

						if(!readXMLValue(subchild, "id", id)) {
							wxString warning;
							warning << wxT("Could not read id tag of item node\n");
							warnings.push_back(warning);
							break;
						}
						readXMLValue(subchild, "id", chance);
						if(readXMLValue(subchild, "type", strVal)) {
							type = strVal;
							if(readXMLValue(subchild, "open", strVal)) {
								isOpen = isTrueString(strVal);
							} else {
								isOpen = true;
								if(type != "window" && type != "any window" && type != "hatch window") {
									wxString warning;
									warning << wxT("Could not read open tag of item node\n");
									warnings.push_back(warning);
									break;
								}
							}
						} else {
							wxString warning;
							warning << wxT("Could not read type tag of item node\n");
							warnings.push_back(warning);
							break;
						}

						if(readXMLValue(subchild, "hate", strVal)) {
							hate = isTrueString(strVal);
						}

						ItemType& it = item_db[id];
						if(it.id == 0) {
							wxString warning;
							warning << wxT("There is no itemtype with id ") << id;
							warnings.push_back(warning);
							return false;
						}
						if(it.brush != NULL && it.brush != this) {
							wxString warning;
							warning << wxT("Itemtype id ") << id << wxT(" already has a brush");
							warnings.push_back(warning);
							return false;
						}
						it.isWall = true;
						it.brush = this;
						it.isBrushDoor = true;
						it.wall_hate_me = hate;
						it.isOpen = isOpen;
						it.border_alignment = ::BorderType(alignment);

						DoorType dt;
						bool all_windows = false;
						bool all_doors = false;
						if(type == "normal") {
							dt.type = WALL_DOOR_NORMAL;
						} else if(type == "locked") {
							dt.type = WALL_DOOR_LOCKED;
						} else if(type == "quest") {
							dt.type = WALL_DOOR_QUEST;
						} else if(type == "magic") {
							dt.type = WALL_DOOR_MAGIC;
						} else if(type == "archway") {
							dt.type = WALL_ARCHWAY;
						} else if(type == "window") {
							dt.type = WALL_WINDOW;
						} else if(type == "hatch_window" || type == "hatch window") {
							dt.type = WALL_HATCH_WINDOW;
						} else if(type == "any door") {
							all_doors = true;
						} else if(type == "any window") {
							all_windows = true;
						} else if(type == "any") {
							all_windows = true;
							all_doors = true;
						} else {
							wxString warning;
							warning << wxT("Unknown door type '") << wxstr(type) << wxT("'\n");
							warnings.push_back(warning);
							break;
						}
						dt.id = id;
						if(all_windows) {
							dt.type = WALL_WINDOW;       door_items[alignment].push_back(dt);
							dt.type = WALL_HATCH_WINDOW; door_items[alignment].push_back(dt);
						}
						if(all_doors) {
							dt.type = WALL_ARCHWAY;     door_items[alignment].push_back(dt);
							dt.type = WALL_DOOR_NORMAL; door_items[alignment].push_back(dt);
							dt.type = WALL_DOOR_LOCKED; door_items[alignment].push_back(dt);
							dt.type = WALL_DOOR_QUEST;  door_items[alignment].push_back(dt);
							dt.type = WALL_DOOR_MAGIC;  door_items[alignment].push_back(dt);
						}

						if(!all_doors && !all_windows) {
							door_items[alignment].push_back(dt);
						}
					}
				} while(false);
				subchild = subchild->next;
			}
		} else if(xmlStrcmp(child->name,(const xmlChar*)"friend") == 0) {
			if(readXMLValue(child, "name", strVal)) {
				if(strVal == "all") {
					//friends.push_back(-1);
				} else {
					Brush* brush = brushes.getBrush(strVal);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						wxString warning;
						warning << wxT("Brush '") + wxstr(strVal) + wxT("' is not defined.");
						warnings.push_back(warning);
					}
					if(readXMLValue(child, "redirect", strVal) && isTrueString(strVal)) {
						WallBrush* rd = dynamic_cast<WallBrush*>(brush);
						if(rd == NULL) {
							wxString warning;
							warning << wxT("Wall brush redirect link: '") + wxstr(strVal) + wxT("' is not a wall brush.");
							warnings.push_back(warning);
						} else if(redirect_to == NULL) {
							redirect_to = rd;
						} else {
							wxString warning;
							warning << wxT("Wall brush '") + wxstr(getName()) + wxT("' has more than one redirect link.");
							warnings.push_back(warning);
						}
					}
				}
			}
		}
		child = child->next;
	}

	return true;
}

void WallBrush::undraw(BaseMap* map, Tile* tile) {
	tile->cleanWalls(this);
}

void WallBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	bool b = (parameter? *reinterpret_cast<bool*>(parameter) : false);
	if(b) {
		// Find a matching wall item on this tile, and shift the id
		for(ItemVector::iterator item_iter = tile->items.begin();
				item_iter != tile->items.end();
				++item_iter)
		{
			Item* item = *item_iter;
			if(item->isWall()) {
				WallBrush* wb = item->getWallBrush();
				if(wb == this) {
					// Ok, shift alignment
					BorderType alignment = item->getWallAlignment();
					uint16_t id = 0;
					WallBrush* try_brush = this;
					while(true) {
						if(id != 0) break;
						if(try_brush == NULL) return;

						for(int i = alignment + 1; i != alignment; ++i) {
							if(i == 16) i = 0;
							WallNode& wn = try_brush->wall_items[i];
							if(wn.total_chance <= 0) {
								continue;
							}
							int chance = random(1, wn.total_chance);
							for(std::vector<WallType>::const_iterator it = wn.items.begin(); it != wn.items.end(); ++it) {
								if(chance <= it->chance) {
									id = it->id;
									break;
								}
							}
							if(id != 0) {
								break;
							}
						}

						try_brush = try_brush->redirect_to;
						if(try_brush == this) {
							break;
						}
					}
					if(id != 0) {
						item->setID(id);
					}
					return;
				}
			}
		}
	}

	tile->cleanWalls(this);

	// Just find a valid item and place it, the bordering algorithm will change it to the proper shape.
	uint16_t id = 0;
	WallBrush* try_brush = this;

	while(true) {
		if(id != 0) break;
		if(try_brush == NULL) return;

		for(int i = 0; i < 16; ++i) {
			WallNode& wn = try_brush->wall_items[i];
			if(wn.total_chance <= 0) {
				continue;
			}
			int chance = random(1, wn.total_chance);
			for(std::vector<WallType>::const_iterator it = wn.items.begin(); it != wn.items.end(); ++it) {
				if(chance <= it->chance) {
					id = it->id;
					break;
				}
			}
			if(id != 0) {
				break;
			}
		}

		try_brush = try_brush->redirect_to;
		if(try_brush == this) {
			break;
		}
	}

	tile->addWallItem(Item::Create(id));
}

bool hasMatchingWallBrushAtTile(BaseMap* map, WallBrush* wall_brush, uint x, uint y, uint z) {
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;

	ItemVector::const_iterator it = t->items.begin();
	for(; it != t->items.end(); ++it) {
		Item* item = *it;
		if(item->isWall()) {
			WallBrush* wb = item->getWallBrush();
			if(wb == wall_brush) {
				return !item_db[item->getID()].wall_hate_me;
			} else if(wall_brush->friendOf(wb) || wb->friendOf(wall_brush)) {
				return !item_db[item->getID()].wall_hate_me;
			}
		}
	}

	return false;
}

void WallBrush::doWalls(BaseMap* map, Tile* tile) {
	ASSERT(tile);

	// For quicker reference
	unsigned int x = tile->getPosition().x;
	unsigned int y = tile->getPosition().y;
	unsigned int z = tile->getPosition().z;

	// Advance the vector to the beginning of the walls
	ItemVector::iterator it = tile->items.begin();
	for(; it != tile->items.end() && (*it)->isBorder(); ++it);

	ItemVector items_to_add;

	while(it != tile->items.end()) {
		Item* wall = *it;
		if(!wall->isWall()) {
			++it;
			continue;
		}
		WallBrush* wall_brush = wall->getWallBrush();
		// Skip if either the wall has no brush
		if(!wall_brush) {
			++it;
			continue;
		}
		// or if it's a decoration brush.
		if(dynamic_cast<WallDecorationBrush*>(wall_brush)) {
			items_to_add.push_back(wall);
			it = tile->items.erase(it);
			continue;
		}
		bool neighbours[4];

		if(x == 0) {
			if(y == 0) {
				neighbours[0] = false;
				neighbours[1] = false;
				neighbours[2] = hasMatchingWallBrushAtTile(map, wall_brush, x + 1, y, z);
				neighbours[3] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y + 1, z);
			} else {
				neighbours[0] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y - 1, z);
				neighbours[1] = false;
				neighbours[2] = hasMatchingWallBrushAtTile(map, wall_brush, x + 1, y, z);
				neighbours[3] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y + 1, z);
			}
		} else if(y == 0) {
			neighbours[0] = false;
			neighbours[1] = hasMatchingWallBrushAtTile(map, wall_brush, x - 1, y, z);
			neighbours[2] = hasMatchingWallBrushAtTile(map, wall_brush, x + 1, y, z);
			neighbours[3] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y + 1, z);
		} else {
			neighbours[0] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y - 1, z);
			neighbours[1] = hasMatchingWallBrushAtTile(map, wall_brush, x - 1, y, z);
			neighbours[2] = hasMatchingWallBrushAtTile(map, wall_brush, x + 1, y, z);
			neighbours[3] = hasMatchingWallBrushAtTile(map, wall_brush, x,     y + 1, z);
		}

		uint32_t tiledata = 0;
		for(int i = 0; i < 4; i++) {
			if(neighbours[i]) {
				// Same wall as this one, calculate what border
				tiledata |= 1 << i;
			}
		}

		bool exit = false;
		for(int i = 0; i < 2; ++i) { // Repeat twice
			if(exit) {
				break;
			}
			::BorderType bt;
			if(i == 0) {
				bt = ::BorderType(full_border_types[tiledata]);
			} else {
				bt = ::BorderType(half_border_types[tiledata]);
			}

			if(wall->getWallAlignment() == WALL_UNTOUCHABLE) {
				items_to_add.push_back(wall);
				it = tile->items.erase(it);
				exit = true;
			} else if(wall->getWallAlignment() == bt) {
				// Do nothing, the tile already has a wall like this
				// However, wall decorations associated with this wall might need to change...
				items_to_add.push_back(wall);
				it = tile->items.erase(it);
				exit = true;

				while(it != tile->items.end()) {
					// If we have a decoration ontop of us, we need to change it's alignment aswell!

					Item* wall_decoration = *it;
					ASSERT(wall_decoration);
					if(WallDecorationBrush* wdb = dynamic_cast<WallDecorationBrush*>(wall_decoration->getWallBrush())) {
						// We don't know if we have changed alignment
						if(wall_decoration->getWallAlignment() == bt) {
							// Same, no need to change...
							items_to_add.push_back(wall_decoration);
							it = tile->items.erase(it);
							continue;
						}
						// Not the same alignment, create newd item with correct alignment
						uint16_t id = 0;
						WallNode& wn = wdb->wall_items[int(bt)];
						if(wn.total_chance <= 0) {
							if(wn.items.size() == 0) {
								++it;
								continue;
							} else {
								id = wn.items.front().id;
							}
						} else {
							int chance = random(1, wn.total_chance);
							for(std::vector<WallType>::const_iterator witer = wn.items.begin();
									witer != wn.items.end();
									++witer)
							{
								if(chance <= witer->chance) {
									id = witer->id;
									break;
								}
							}
						}
						if(id != 0) {
							Item* new_wall = Item::Create(id);
							if(wall_decoration->isSelected()) {
								new_wall->select();
							}
							items_to_add.push_back(new_wall);
						}
						++it;
					} else {
						break;
					}
				}
			} else {
				// Randomize a newd wall of the proper alignment
				uint16_t id = 0;
				WallBrush* try_brush = wall_brush;

				while(true) {
					if(try_brush == NULL) break;
					if(id != 0) break;

					WallNode& wn = try_brush->wall_items[int(bt)];
					if(wn.total_chance <= 0) {
						if(wn.items.size() == 0) {
							try_brush = try_brush->redirect_to;
							if(try_brush == wall_brush) break; // To prevent infinite loop
							continue;
						} else {
							id = wn.items.front().id;
						}
					} else {
						int chance = random(1, wn.total_chance);
						for(std::vector<WallType>::const_iterator node_iter = wn.items.begin();
								node_iter != wn.items.end();
								++node_iter)
						{
							if(chance <= node_iter->chance) {
								id = node_iter->id;
								break;
							}
						}
					}
					// Propagate down the chain
					try_brush = try_brush->redirect_to;
					if(try_brush == wall_brush) break; // To prevent infinite loop
				}
				if(try_brush == NULL && id == 0) {
					if(i == 1) {
						++it;
					}
					continue;
				} else {
					// If there is such an item, add it to the tile
					Item* new_wall = Item::Create(id);
					if(wall->isSelected()) {
						new_wall->select();
					}
					items_to_add.push_back(new_wall);
					exit = true;
					++it;
				}

				// Increment and check for end
				while(it != tile->items.end()) {
					// If we have a decoration ontop of us, we need to change it's alignment aswell!
					Item* wall_decoration = *it;
					if(WallDecorationBrush* wdb = dynamic_cast<WallDecorationBrush*>(wall_decoration->getWallBrush())) {
						// We know we have changed alignment, so no need to check for it again.
						uint16_t id = 0;
						WallNode& wn = wdb->wall_items[int(bt)];
						if(wn.total_chance <= 0) {
							if(wn.items.size() == 0) {
								++it;
								continue;
							} else {
								id = wn.items.front().id;
							}
						} else {
							int chance = random(1, wn.total_chance);
							for(std::vector<WallType>::const_iterator node_iter = wn.items.begin();
									node_iter != wn.items.end();
									++node_iter)
							{
								if(chance <= node_iter->chance) {
									id = node_iter->id;
									break;
								}
							}
						}
						if(id != 0) {
							Item* new_wall = Item::Create(id);
							if(wall_decoration->isSelected()) {
								new_wall->select();
							}
							items_to_add.push_back(new_wall);
						}
						++it;
					} else {
						++it;
						break;
					}
				}
			}
		}
	}
	tile->cleanWalls();
	for(ItemVector::const_iterator it = items_to_add.begin(); it != items_to_add.end(); ++it) {
		tile->addWallItem(*it);
	}
}

bool WallBrush::hasWall(Item* item) {
	ASSERT(item->isWall());
	::BorderType bt = item->getWallAlignment();

	WallBrush* test_wall = this;

	while(test_wall != NULL) {
		for(std::vector<WallType>::const_iterator it = test_wall->wall_items[int(bt)].items.begin();
				it != test_wall->wall_items[int(bt)].items.end();
				++it)
		{
			if(it->id == item->getID()) {
				return true;
			}
		}
		for(std::vector<DoorType>::const_iterator it = test_wall->door_items[int(bt)].begin();
				it != test_wall->door_items[int(bt)].end();
				++it)
		{
			if(it->id == item->getID()) {
				return true;
			}
		}

		test_wall = test_wall->redirect_to;
		if(test_wall == this) return false; // Prevent infinite loop
	}
	return false;
}

::DoorType WallBrush::getDoorTypeFromID(uint16_t id) {
	for(int index = 0; index < 16; ++index) {
		for(std::vector<DoorType>::const_iterator iter = door_items[index].begin();
				iter != door_items[index].end();
				++iter)
		{
			if(iter->id == id) {
				return iter->type;
			}
		}
	}
	return WALL_UNDEFINED;
}

//=============================================================================
// Wall Decoration brush

WallDecorationBrush::WallDecorationBrush() {
	// ...
}

WallDecorationBrush::~WallDecorationBrush() {
	// ...
}

void WallDecorationBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);

	ItemVector::iterator iter = tile->items.begin();

	tile->cleanWalls(this);
	while(iter != tile->items.end()) {
		Item* item = *iter;
		if(item->isBorder()) {
			++iter;
			continue;
		}

		if(item->isWall()) {
			// Now we found something interesting.

			// Is it just a decoration, like what we're trying to add?
			WallBrush* wb = item->getWallBrush();
			if(dynamic_cast<WallDecorationBrush*>(wb)) {
				// It is, discard and advance!
				++iter;
				continue;
			}

			// We first need to figure out the alignment of this item (wall)
			BorderType wall_alignment = item->getWallAlignment();

			// Now we need to figure out if we got an item that mights suffice to place on this tile..

			int id = 0;
			if(item->isBrushDoor()) {
				// If it's a door
				::DoorType doortype = wb->getDoorTypeFromID(item->getID());
				uint16_t discarded_id = 0;
				bool close_match = false;
				bool open = item->isOpen();

				for(std::vector<WallBrush::DoorType>::iterator door_iter = door_items[wall_alignment].begin();
						door_iter!= door_items[wall_alignment].end();
						++door_iter)
				{
					WallBrush::DoorType& dt = *door_iter;
					if(dt.type == doortype) {
						ASSERT(dt.id);
						ItemType& it = item_db[dt.id];
						ASSERT(it.id != 0);

						if(it.isOpen == open) {
							id = dt.id;
							break;
						} else {
							discarded_id = dt.id;
							close_match = true;
						}
						if(!close_match && discarded_id == 0) {
							discarded_id = dt.id;
						}
					}
				}
				if(id == 0) {
					id = discarded_id;
					if(id == 0) {
						++iter;
						continue;
					}
				}
			} else {
				// If it's a normal wall...
				if(wall_items[wall_alignment].total_chance <= 0) {
					// No fitting item, exit
					++iter;
					continue;
				}
				int chance = random(1, wall_items[wall_alignment].total_chance);
				for(std::vector<WallBrush::WallType>::const_iterator witer = wall_items[wall_alignment].items.begin();
					witer != wall_items[wall_alignment].items.end();
					++witer)
				{
					if(chance <= witer->chance) {
						id = witer->id;
						break;
					}
				}
			}
			// If we found an invalid id we should've already exited the loop
			ASSERT(id);

			// Add a matching item above this item.
			Item* item = Item::Create(id);
			++iter;
			iter = tile->items.insert(iter, item);
		}
		++iter;
	}
}

