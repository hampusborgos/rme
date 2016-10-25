//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "wall_brush.h"
#include "items.h"
#include "basemap.h"
#include "pugicast.h"

uint32_t WallBrush::full_border_types[16];
uint32_t WallBrush::half_border_types[16];

WallBrush::WallBrush() :
	redirect_to(nullptr)
{
	////
}

WallBrush::~WallBrush()
{
	////
}

bool WallBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = pugi::cast<uint16_t>(attribute.value());
	}

	if((attribute = node.attribute("server_lookid"))) {
		look_id = g_items[pugi::cast<uint16_t>(attribute.value())].clientID;
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "wall") {
			const std::string& typeString = childNode.attribute("type").as_string();
			if(typeString.empty()) {
				warnings.push_back(wxT("Could not read type tag of wall node\n"));
				continue;
			}

			uint32_t alignment;
			if(typeString == "vertical") {
				alignment = WALL_VERTICAL;
			} else if(typeString == "horizontal") {
				alignment = WALL_HORIZONTAL;
			} else if(typeString == "corner") {
				alignment = WALL_NORTHWEST_DIAGONAL;
			} else if(typeString == "pole") {
				alignment = WALL_POLE;
			} else if(typeString == "south end") {
				alignment = WALL_SOUTH_END;
			} else if(typeString == "east end") {
				alignment = WALL_EAST_END;
			} else if(typeString == "north end") {
				alignment = WALL_NORTH_END;
			} else if(typeString == "west end") {
				alignment = WALL_WEST_END;
			} else if(typeString == "south T") {
				alignment = WALL_SOUTH_T;
			} else if(typeString == "east T") {
				alignment = WALL_EAST_T;
			} else if(typeString == "west T") {
				alignment = WALL_WEST_T;
			} else if(typeString == "north T") {
				alignment = WALL_NORTH_T;
			} else if(typeString == "northwest diagonal") {
				alignment = WALL_NORTHWEST_DIAGONAL;
			} else if(typeString == "northeast diagonal") {
				alignment = WALL_NORTHEAST_DIAGONAL;
			} else if(typeString == "southwest diagonal") {
				alignment = WALL_SOUTHWEST_DIAGONAL;
			} else if(typeString == "southeast diagonal") {
				alignment = WALL_SOUTHEAST_DIAGONAL;
			} else if(typeString == "intersection") {
				alignment = WALL_INTERSECTION;
			} else if(typeString == "untouchable") {
				alignment = WALL_UNTOUCHABLE;
			} else {
				warnings.push_back(wxT("Unknown wall alignment '") + wxstr(typeString) + wxT("'\n"));
				continue;
			}

			for(pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
				const std::string& subChildName = as_lower_str(subChildNode.name());
				if(subChildName == "item") {
					uint16_t id = pugi::cast<uint16_t>(subChildNode.attribute("id").value());
					if(id == 0) {
						warnings.push_back(wxT("Could not read id tag of item node\n"));
						break;
					}

					ItemType& it = g_items[id];
					if(it.id == 0) {
						warnings.push_back(wxT("There is no itemtype with id ") + std::to_string(id));
						return false;
					} else if(it.brush && it.brush != this) {
						warnings.push_back(wxT("Itemtype id ") + std::to_string(id) + wxT(" already has a brush"));
						return false;
					}

					it.isWall = true;
					it.brush = this;
					it.border_alignment = ::BorderType(alignment);

					WallType wt;
					wt.id = id;

					wall_items[alignment].total_chance += pugi::cast<int32_t>(subChildNode.attribute("chance").value());
					wt.chance = wall_items[alignment].total_chance;

					wall_items[alignment].items.push_back(wt);
				} else if(subChildName == "door") {
					uint16_t id = pugi::cast<uint16_t>(subChildNode.attribute("id").value());
					if(id == 0) {
						warnings.push_back(wxT("Could not read id tag of door node\n"));
						break;
					}

					const std::string& type = subChildNode.attribute("type").as_string();
					if(type.empty()) {
						warnings.push_back(wxT("Could not read type tag of door node\n"));
						continue;
					}

					bool isOpen;
					pugi::xml_attribute openAttribute = subChildNode.attribute("open");
					if(openAttribute) {
						isOpen = openAttribute.as_bool();
					} else {
						isOpen = true;
						if(type != "window" && type != "any window" && type != "hatch window") {
							warnings.push_back(wxT("Could not read open tag of door node\n"));
							break;
						}
					}

					ItemType& it = g_items[id];
					if(it.id == 0) {
						warnings.push_back(wxT("There is no itemtype with id ") + std::to_string(id));
						return false;
					} else if(it.brush && it.brush != this) {
						warnings.push_back(wxT("Itemtype id ") + std::to_string(id) + wxT(" already has a brush"));
						return false;
					}

					it.isWall = true;
					it.brush = this;
					it.isBrushDoor = true;
					it.wall_hate_me = subChildNode.attribute("hate").as_bool();
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
						warnings.push_back(wxT("Unknown door type '") + wxstr(type) + wxT("'\n"));
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
			}
		} else if(childName == "friend") {
			const std::string& name = childNode.attribute("name").as_string();
			if(name.empty()) {
				continue;
			}

			if(name == "all") {
				//friends.push_back(-1);
			} else {
				Brush* brush = brushes.getBrush(name);
				if(brush) {
					friends.push_back(brush->getID());
				} else {
					warnings.push_back(wxT("Brush '") + wxstr(name) + wxT("' is not defined."));
				}

				if(childNode.attribute("redirect").as_bool()) {
					WallBrush* rd = dynamic_cast<WallBrush*>(brush);
					if(!rd) {
						warnings.push_back(wxT("Wall brush redirect link: '") + wxstr(name) + wxT("' is not a wall brush."));
					} else if(!redirect_to) {
						redirect_to = rd;
					} else {
						warnings.push_back( wxT("Wall brush '") + wxstr(getName()) + wxT("' has more than one redirect link."));
					}
				}
			}
		}
	}
	return true;
}

void WallBrush::undraw(BaseMap* map, Tile* tile)
{
	tile->cleanWalls(this);
}

void WallBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	bool b = (parameter? *reinterpret_cast<bool*>(parameter) : false);
	if(b) {
		// Find a matching wall item on this tile, and shift the id
		for(ItemVector::iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ++item_iter) {
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
						if(try_brush == nullptr) return;

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
		if(try_brush == nullptr) return;

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

bool hasMatchingWallBrushAtTile(BaseMap* map, WallBrush* wall_brush, uint32_t x, uint32_t y, uint32_t z)
{
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;

	ItemVector::const_iterator it = t->items.begin();
	for(; it != t->items.end(); ++it) {
		Item* item = *it;
		if(item->isWall()) {
			WallBrush* wb = item->getWallBrush();
			if(wb == wall_brush) {
				return !g_items[item->getID()].wall_hate_me;
			} else if(wall_brush->friendOf(wb) || wb->friendOf(wall_brush)) {
				return !g_items[item->getID()].wall_hate_me;
			}
		}
	}

	return false;
}

void WallBrush::doWalls(BaseMap* map, Tile* tile)
{
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
					if(try_brush == nullptr) break;
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
								++node_iter) {
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
				if(try_brush == nullptr && id == 0) {
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

bool WallBrush::hasWall(Item* item)
{
	ASSERT(item->isWall());
	::BorderType bt = item->getWallAlignment();

	WallBrush* test_wall = this;

	while(test_wall != nullptr) {
		for(std::vector<WallType>::const_iterator it = test_wall->wall_items[int(bt)].items.begin(); it != test_wall->wall_items[int(bt)].items.end(); ++it) {
			if(it->id == item->getID()) {
				return true;
			}
		}
		for(std::vector<DoorType>::const_iterator it = test_wall->door_items[int(bt)].begin(); it != test_wall->door_items[int(bt)].end(); ++it) {
			if(it->id == item->getID()) {
				return true;
			}
		}

		test_wall = test_wall->redirect_to;
		if(test_wall == this) return false; // Prevent infinite loop
	}
	return false;
}

::DoorType WallBrush::getDoorTypeFromID(uint16_t id)
{
	for(int index = 0; index < 16; ++index) {
		for(std::vector<DoorType>::const_iterator iter = door_items[index].begin(); iter != door_items[index].end(); ++iter) {
			if(iter->id == id) {
				return iter->type;
			}
		}
	}
	return WALL_UNDEFINED;
}

//=============================================================================
// Wall Decoration brush

WallDecorationBrush::WallDecorationBrush()
{
	////
}

WallDecorationBrush::~WallDecorationBrush()
{
	////
}

void WallDecorationBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
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
						ItemType& it = g_items[dt.id];
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

