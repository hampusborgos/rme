#include "main.h"

#include "ground_brush.h"
#include "items.h"
#include "basemap.h"

uint32_t GroundBrush::border_types[256];

int AutoBorder::edgeNameToID(std::string edgename) {
	if(edgename == "n") {
		return NORTH_HORIZONTAL;
	} else if(edgename == "w") {
		return WEST_HORIZONTAL;
	} else if(edgename == "s") {
		return SOUTH_HORIZONTAL;
	} else if(edgename == "e") {
		return EAST_HORIZONTAL;
	} else if(edgename == "cnw") {
		return NORTHWEST_CORNER;
	} else if(edgename == "cne") {
		return NORTHEAST_CORNER;
	} else if(edgename == "csw") {
		return SOUTHWEST_CORNER;
	} else if(edgename == "cse") {
		return SOUTHEAST_CORNER;
	} else if(edgename == "dnw") {
		return NORTHWEST_DIAGONAL;
	} else if(edgename == "dne") {
		return NORTHEAST_DIAGONAL;
	} else if(edgename == "dsw") {
		return SOUTHWEST_DIAGONAL;
	} else if(edgename == "dse") {
		return SOUTHEAST_DIAGONAL;
	}
	return BORDER_NONE;
}

bool AutoBorder::load(xmlNodePtr node, wxArrayString& warnings, GroundBrush* owner, uint16_t ground_equivalent) {
	std::string orientation;
	bool op_border = false;
	int itemid;
	ground = bool(owner);

	ASSERT(ground? ground_equivalent != 0 : true);

	std::string strVal;
	int intVal;

	if(readXMLValue(node, "type", strVal)) {
		if(strVal == "optional") {
			op_border = true;
		}
	}

	if(readXMLValue(node, "group", intVal)) {
		group = intVal;
	}

	xmlNodePtr child = node->children;
	while(child) {
		if(readXMLValue(child, "item", itemid) && readXMLValue(child, "edge", orientation)) {
			ItemType& it = item_db[itemid];
			if(it.id == 0) {
				child = child->next;
				warnings.push_back(wxString(wxT("Invalid item ID ")) << itemid << wxT(" for border ") << id);
				continue;
			}

			if(ground) { // We are a ground border
				it.group = ITEM_GROUP_NONE;
				it.ground_equivalent = ground_equivalent;
				it.brush = owner;

				ItemType& it2 = item_db[ground_equivalent];
				if(it2.id == 0) {
					// Do nothing
				} else {
					it2.has_equivalent = true;
				}
			}
			it.alwaysOnBottom = true; // Never-ever place other items under this, will confuse the user something awful.
			it.isBorder = true;
			it.isOptionalBorder = it.isOptionalBorder? true : op_border;
			if(group && !it.border_group) {
				it.border_group = group;
			}

			int edge_id = edgeNameToID(orientation);
			if(edge_id != BORDER_NONE) {
				tiles[edge_id] = itemid;
				if(it.border_alignment == BORDER_NONE) {
					it.border_alignment = ::BorderType(edge_id);
				}
			}
		}
		child = child->next;
	}
	return true;
}

GroundBrush::GroundBrush() :
	z_order(0),
	has_zilch_outer_border(false),
	has_zilch_inner_border(false),
	has_outer_border(false),
	has_inner_border(false),
	optional_border(NULL),
	use_only_optional(false),
	randomize(true),
	total_chance(0)
{

}

GroundBrush::~GroundBrush() {
	for(std::vector<BorderBlock*>::iterator it = borders.begin();
			it != borders.end();
			++it)
	{
		BorderBlock* bb = *it;
		if(bb->autoborder) {
			for(std::vector<SpecificCaseBlock*>::iterator specific_iter = bb->specific_cases.begin();
					specific_iter != bb->specific_cases.end();
					++specific_iter)
			{
				delete *specific_iter;
			}
			if(bb->autoborder->ground == true) {
				delete bb->autoborder;
			}
		}
		delete bb;
	}
}

bool GroundBrush::load(xmlNodePtr node, wxArrayString& warnings) {
	std::string strVal;
	int intVal;

	if(readXMLValue(node, "lookid", intVal)) {
		look_id = intVal;
	}
	if(readXMLValue(node, "server_lookid", intVal)) {
		look_id = item_db[intVal].clientID;
	}
	if(readXMLValue(node, "z-order", intVal)) {
		z_order = intVal;
	}
	if(readXMLValue(node, "solo_optional", strVal)) {
		use_only_optional = isTrueString(strVal);
	}
	if(readXMLValue(node, "randomize", strVal)) {
		randomize = isTrueString(strVal);
	}

	xmlNodePtr child = node->children;
	while(child) {
		if(xmlStrcmp(child->name,(const xmlChar*)"item") == 0) {
			uint16_t itemid = 0;
			int chance = 0;
			if(readXMLValue(child, "id", intVal)) {
				itemid = intVal;
			}
			if(readXMLValue(child, "chance", intVal)) {
				chance = intVal;
			}

			ItemType& it = item_db[itemid];
			if(it.id == 0) {
				wxString warning;
				warning << wxT("\nInvalid item id") << itemid;
				warnings.push_back(warning);
				return false;
			}
			if(it.isGroundTile() == false) {
				wxString warning;
				warning << wxT("\nItem ") << itemid << wxT(" is not ground item.");
				warnings.push_back(warning);
				return false;
			}
			if(it.brush != NULL && it.brush != this) {
				wxString warning;
				warning << wxT("\nItem ") << itemid << wxT(" can not be member of two brushes");
				warnings.push_back(warning);
				return false;
			}
			it.brush = this;

			ItemChanceBlock ci;
			ci.id = itemid;
			ci.chance = total_chance + chance;
			border_items.push_back(ci);
			total_chance += chance;
		} else if(xmlStrcmp(child->name,(const xmlChar*)"optional") == 0) {
			// Mountain border!
			if(optional_border) {
				wxString warning;
				warning << wxT("\nDuplicate optional borders!");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}
			
			if(readXMLValue(child, "ground_equivalent", intVal)) {
				// Load from inline definition
				ItemType& it = item_db[intVal];
				if(it.id == 0)  {
					wxString warning;
					warning = wxT("Invalid id of ground dependency equivalent item.\n");
					warnings.push_back(warning);
				}
				if(it.isGroundTile() == false)  {
					wxString warning;
					warning = wxT("Ground dependency equivalent is not a ground item.\n");
					warnings.push_back(warning);
				}
				if(it.brush != this) {
					wxString warning;
					warning = wxT("Ground dependency equivalent does not use the same brush as ground border.\n");
					warnings.push_back(warning);
				}

				AutoBorder* ab = newd AutoBorder(0); // Empty id basically
				ab->load(child, warnings, this, intVal);
				optional_border = ab;
			} else {
				// Load from ID
				int id;
				if(!readXMLValue(child, "id", id)) {
					wxString warning;
					warning << wxT("\nMissing tag id for border node");
					warnings.push_back(warning);
					child = child->next;
					continue;
				}

				Brushes::BorderMap::iterator it = brushes.borders.find(id);
				if(it == brushes.borders.end() || it->second == NULL) {
					wxString warning;
					warning << wxT("\nCould not find border id ") << id;
					warnings.push_back(warning);
					child = child->next;
					continue;
				}
				optional_border = it->second;
			}
		} else if(xmlStrcmp(child->name,(const xmlChar*)"border") == 0) {
			AutoBorder* ab;
			int id;

			if(!readXMLValue(child, "id", id)) {
				if(readXMLValue(child, "ground_equivalent", intVal)) {
					ItemType& it = item_db[intVal];
					if(it.id == 0)  {
						wxString warning;
						warning = wxT("Invalid id of ground dependency equivalent item.\n");
						warnings.push_back(warning);
					}
					if(it.isGroundTile() == false)  {
						wxString warning;
						warning = wxT("Ground dependency equivalent is not a ground item.\n");
						warnings.push_back(warning);
					}
					if(it.brush != this) {
						wxString warning;
						warning = wxT("Ground dependency equivalent does not use the same brush as ground border.\n");
						warnings.push_back(warning);
					}

					ab = newd AutoBorder(0); // Empty id basically
					ab->load(child, warnings, this, intVal);
				} else {
					child = child->next;
					continue;
				}
			} else if(id == 0) {
				ab = NULL;
			} else { // id != 0
				Brushes::BorderMap::iterator it = brushes.borders.find(id);
				if(it == brushes.borders.end() || it->second == NULL) {
					wxString warning;
					warning << wxT("\nCould not find border id ") << id;
					warnings.push_back(warning);
					child = child->next;
					continue;
				}
				ab = it->second;
			}

			BorderBlock* bb = newd BorderBlock;
			bb->super = false;
			bb->autoborder = ab;

			if(readXMLValue(child, "to", strVal)) {
				if(strVal == "all") {
					bb->to = 0xFFFFFFFF;
				} else if(strVal == "none") {
					bb->to = 0;
				} else {
					Brush* tobrush = brushes.getBrush(strVal);
					if(tobrush) {
						bb->to = tobrush->getID();
					} else {
						wxString warning;
						warning = wxT("To brush ") + wxstr(strVal) + wxT(" doesn't exist.");
						warnings.push_back(warning);
						child = child->next;
						continue;
					}
				}
			} else {
				bb->to = 0xFFFFFFFF;
			}

			if(readXMLValue(child, "super", strVal)) {
				if(isTrueString(strVal)) {
					bb->super = true;
				}
			}

			if(readXMLValue(child, "align", strVal)) {
				if(strVal == "outer") {
					bb->outer = true;
				} else if(strVal == "inner") {
					bb->outer = false;
				} else {
					bb->outer = true;
				}
			}
			if(bb->outer == true) {
				if(bb->to == 0) {
					has_zilch_outer_border = true;
				} else {
					has_outer_border = true;
				}
			} else {
				if(bb->to == 0) {
					has_zilch_inner_border = true;
				} else {
					has_inner_border = true;
				}
			}

			xmlNodePtr subChild = child->children;
			while(subChild) {
				if(xmlStrcmp(subChild->name,(const xmlChar*)"specific") == 0) {
					xmlNodePtr superChild = subChild->children;
					SpecificCaseBlock* scb = NULL;

					while(superChild) {
						if(xmlStrcmp(superChild->name,(const xmlChar*)"conditions") == 0) {
							xmlNodePtr conditionChild = superChild->children;
							while(conditionChild) {
								if(xmlStrcmp(conditionChild->name,(const xmlChar*)"match_border") == 0) {
									int border_id = 0;
									std::string edge;

									if(!readXMLValue(conditionChild, "id", border_id) ||
											!readXMLValue(conditionChild, "edge", edge)) {
										conditionChild = conditionChild->next;
										continue;
									}
									int edge_id = AutoBorder::edgeNameToID(edge);

									Brushes::BorderMap::iterator bit = brushes.borders.find(border_id);
									if(bit == brushes.borders.end()) {
										wxString warning;
										warning = wxT("Unknown border id in specific case match block "); warning << border_id	;
										warnings.push_back(warning);
										conditionChild = conditionChild->next;
										continue;
									}
									AutoBorder* ab = bit->second;
									ASSERT(ab != NULL);

									uint32_t match_itemid = ab->tiles[edge_id];

									if(!scb) scb = newd SpecificCaseBlock();
									scb->items_to_match.push_back(match_itemid);
								} else if(xmlStrcmp(conditionChild->name,(const xmlChar*)"match_group") == 0) {
									int group = 0;
									std::string edge;

									if(!readXMLValue(conditionChild, "group", group) ||
											!readXMLValue(conditionChild, "edge", edge)) {
										conditionChild = conditionChild->next;
										continue;
									}
									int edge_id = AutoBorder::edgeNameToID(edge);

									if(!scb) scb = newd SpecificCaseBlock();
									scb->match_group = group;
									scb->group_match_alignment = ::BorderType(edge_id);
									scb->items_to_match.push_back(uint16_t(group));
								} else if(xmlStrcmp(conditionChild->name,(const xmlChar*)"match_item") == 0) {
									int match_itemid = 0;

									if(!readXMLValue(conditionChild, "id", match_itemid)) {
										conditionChild = conditionChild->next;
										continue;
									}

									if(!scb) scb = newd SpecificCaseBlock();
									scb->match_group = 0;
									scb->items_to_match.push_back(match_itemid);
								}
								conditionChild = conditionChild->next;
							}
						} else if(xmlStrcmp(superChild->name,(const xmlChar*)"actions") == 0) {
							xmlNodePtr actionChild = superChild->children;
							while(actionChild) {
								if(xmlStrcmp(actionChild->name,(const xmlChar*)"replace_border") == 0) {
									int border_id = 0;
									std::string edge;
									int with_id = 0;

									if(!readXMLValue(actionChild, "id", border_id) ||
											!readXMLValue(actionChild, "edge", edge) ||
											!readXMLValue(actionChild, "with", with_id)) {
										actionChild = actionChild->next;
										continue;
									}
									int edge_id = AutoBorder::edgeNameToID(edge);

									Brushes::BorderMap::iterator bit = brushes.borders.find(border_id);
									if(bit == brushes.borders.end()) {
										wxString warning;
										warning = wxT("Unknown border id in specific case match block "); warning << border_id;
										warnings.push_back(warning);
										actionChild = actionChild->next;
										continue;
									}
									AutoBorder* ab = bit->second;
									ASSERT(ab != NULL);

									ItemType& it = item_db[with_id];
									if(it.id == 0) {return false;}
									it.isBorder = true;

									if(!scb) scb = newd SpecificCaseBlock();
									scb->to_replace_id = ab->tiles[edge_id];
									scb->with_id = with_id;
								} else if(xmlStrcmp(actionChild->name,(const xmlChar*)"replace_item") == 0) {
									int to_replace_id = 0;
									int with_id = 0;

									if(!readXMLValue(actionChild, "id", to_replace_id) ||
											!readXMLValue(actionChild, "with", with_id)) {
										actionChild = actionChild->next;
										continue;
									}

									ItemType& it = item_db[with_id];
									if(it.id == 0) {return false;}
									it.isBorder = true;

									if(!scb) scb = newd SpecificCaseBlock();
									scb->to_replace_id = to_replace_id;
									scb->with_id = with_id;
								} else if(xmlStrcmp(actionChild->name,(const xmlChar*)"delete_borders") == 0) {
									if(!scb) scb = newd SpecificCaseBlock();
									scb->delete_all = true;
								}
								actionChild = actionChild->next;
							}
						}
						superChild = superChild->next;
					}
					if(scb) {
						bb->specific_cases.push_back(scb);
					}
				}
				subChild = subChild->next;
			}

			borders.push_back(bb);
		} else if(xmlStrcmp(child->name,(const xmlChar*)"friend") == 0) {
			if(readXMLValue(child, "name", strVal)) {
				if(strVal == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = brushes.getBrush(strVal);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						wxString warning;
						warning << wxT("Brush '") + wxstr(strVal) + wxT("' is not defined.");
						warnings.push_back(warning);
					}
				}
			}
			hate_friends = false;
		} else if(xmlStrcmp(child->name,(const xmlChar*)"enemy") == 0) {
			if(readXMLValue(child, "name", strVal)) {
				if(strVal == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = brushes.getBrush(strVal);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						wxString warning;
						warning << wxT("Brush '") + wxstr(strVal) + wxT("' is not defined.");
						warnings.push_back(warning);
					}
				}
			}
			hate_friends = true;
		} else if(xmlStrcmp(child->name,(const xmlChar*)"clear_borders") == 0) {
			for(std::vector<BorderBlock*>::iterator it = borders.begin();
					it != borders.end();
					++it)
			{
				BorderBlock* bb = *it;
				if(bb->autoborder) {
					for(std::vector<SpecificCaseBlock*>::iterator specific_iter = bb->specific_cases.begin();
							specific_iter != bb->specific_cases.end();
							++specific_iter)
					{
						delete *specific_iter;
					}
					if(bb->autoborder->ground == true) {
						delete bb->autoborder;
					}
				}
				delete bb;
			}
			borders.clear();
		} else if(xmlStrcmp(child->name,(const xmlChar*)"clear_friends") == 0) {
			friends.clear();
			hate_friends = false;
		}

		child = child->next;
	}

	if(total_chance == 0) {
		randomize = false;
	}
	
	return true;
}

void GroundBrush::undraw(BaseMap* map, Tile* tile) {
	ASSERT(tile);
	if(tile->hasGround() && tile->ground->getGroundBrush() == this) {
		delete tile->ground;
		tile->ground = NULL;
	}
}

void GroundBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	if(border_items.empty()) return;

	if(parameter != NULL) {
		std::pair<bool, GroundBrush*>& param = *reinterpret_cast<std::pair<bool, GroundBrush*>* >(parameter);
		GroundBrush* other = tile->getGroundBrush();
		if(param.first) { // Volatile? :)
			if(other != NULL) {
				return;
			}
		} else if(other != param.second) {
			return;
		}
	}
	int chance = random(1, total_chance);
	uint16_t id = 0;
	for(std::vector<ItemChanceBlock>::const_iterator it = border_items.begin(); it != border_items.end(); ++it) {
		if(chance < it->chance) {
			id = it->id;
			break;
		}
	}
	if(id == 0) {
		id = border_items.front().id;
	}

	tile->addItem(Item::Create(id));
}

const GroundBrush::BorderBlock* GroundBrush::getBrushTo(GroundBrush* first, GroundBrush* second) {
	//printf("Border from %s to %s : ", first->getName().c_str(), second->getName().c_str());
	if(first) {
		if(second) {
			if(first->getZ() < second->getZ() && second->hasOuterBorder()) {
				if(first->hasInnerBorder()) {
					for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
						BorderBlock* bb = *it;
						if(bb->outer == true) {
							continue;
						} else if(bb->to == second->getID() || bb->to == 0xFFFFFFFF) {
							//printf("%d\n", bb->autoborder);
							return bb;
						}
					}
				}
				for(std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if(bb->outer != true) {
						continue;
					} else if(bb->to == first->getID()) {
						//printf("%d\n", bb->autoborder);
						return bb;
					} else if(bb->to == 0xFFFFFFFF) {
						//printf("%d\n", bb->autoborder);
						return bb;
					}
				}
			} else if(first->hasInnerBorder()) {
				for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if(bb->outer == true) {
						continue;
					} else if(bb->to == second->getID()) {
						//printf("%d\n", bb->autoborder);
						return bb;
					} else if(bb->to == 0xFFFFFFFF) {
						//printf("%d\n", bb->autoborder);
						return bb;
					}
				}
			}
		} else if(first->hasInnerZilchBorder()) {
			for(std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
				BorderBlock* bb = *it;
				if(bb->outer == true) {
					continue;
				} else if(bb->to == 0) {
					//printf("%d\n", bb->autoborder);
					return bb;
				}
			}
		}
	} else if(second && second->hasOuterZilchBorder()) {
		for(std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
			BorderBlock* bb = *it;
			if(bb->outer == false) {
				continue;
			} else if(bb->to == 0) {
				//printf("%d\n", bb->autoborder);
				return bb;
			}
		}
	}
	//printf("None\n");
	return NULL;
}

inline GroundBrush* extractGroundBrushFromTile(BaseMap* map, uint x, uint y, uint z) {
	Tile* t = map->getTile(x, y, z);
	return t? t->getGroundBrush() : NULL;
}

void GroundBrush::doBorders(BaseMap* map, Tile* tile) {
	ASSERT(tile);

	GroundBrush* border_brush = (tile->ground? tile->ground->getGroundBrush() : NULL);

	unsigned int x = tile->getPosition().x;
	unsigned int y = tile->getPosition().y;
	unsigned int z = tile->getPosition().z;
	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8];

	if(x == 0) {
		if(y == 0) {
			neighbours[0] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[1] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[2] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[3] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
			neighbours[5] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
			neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
		} else {
			neighbours[0] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[1] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y - 1, z));
			neighbours[2] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y - 1, z));
			neighbours[3] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
			neighbours[5] = std::make_pair<bool, GroundBrush*>(false, NULL);
			neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
			neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
		}
	} else if(y == 0) {
		neighbours[0] = std::make_pair<bool, GroundBrush*>(false, NULL);
		neighbours[1] = std::make_pair<bool, GroundBrush*>(false, NULL);
		neighbours[2] = std::make_pair<bool, GroundBrush*>(false, NULL);
		neighbours[3] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x - 1, y, z));
		neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
		neighbours[5] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x - 1, y + 1, z));
		neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
		neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
	} else {
		neighbours[0] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x - 1, y - 1, z));
		neighbours[1] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y - 1, z));
		neighbours[2] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y - 1, z));
		neighbours[3] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x - 1, y, z));
		neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
		neighbours[5] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x - 1, y + 1, z));
		neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
		neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
	}

	std::vector<BorderCluster> border_list;
	static std::vector<const BorderBlock*> specific_list;
	specific_list.clear();

	for(int i = 0; i < 8; i++) {
		//printf("Checking neighbour #%d\n", i);
		if(neighbours[i].first) continue;
		//printf("\tNeighbour not checked before\n");
		GroundBrush* other = neighbours[i].second;

		if(border_brush) {
			if(other) {
				//printf("\tNeighbour has brush\n");
				if(other->getID() == border_brush->getID()) {
					//printf("\tNeighbour has same brush as we\n");
					continue;
				}

				if(other->hasOuterBorder() || border_brush->hasInnerBorder()) {
					bool only_mountain = false;
					if(/*border_brush->hasInnerBorder() == false && */(other->friendOf(border_brush) || border_brush->friendOf(other))) {
						if(other->hasOptionalBorder()) {
							only_mountain = true;
						} else {
							continue;
						}
					}

					uint32_t tiledata = 0;
					for(int j = i; j < 8; j++) {
						if(neighbours[j].first == false && neighbours[j].second && neighbours[j].second->getID() == other->getID()) {
							neighbours[j].first = true;
							tiledata |= 1 << j;
						}
					}

					if(tiledata != 0) {
						// Add mountain if appropriate!
						if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
							BorderCluster cluster;
							cluster.alignment = tiledata;
							cluster.z = 0x7FFFFFFF; // Above all other borders
							cluster.border = other->optional_border;
							border_list.push_back(cluster);
							if(other->useSoloOptionalBorder()) {
								only_mountain = true;
							}
						}
						if(only_mountain == false) {
							const BorderBlock* bb = getBrushTo(border_brush, other);

							if(bb) {
								bool found = false;
								for(std::vector<BorderCluster>::iterator iter = border_list.begin();
										iter != border_list.end();
										++iter)
								{
									BorderCluster& bc = *iter;
									if(bc.border == bb->autoborder) {
										// z0MG! Same borderzzz, Group it!
										bc.alignment |= tiledata;
										if(bc.z < other->getZ()) {
											bc.z = other->getZ(); // Z always has top value!
										}
										if(bb->specific_cases.size() > 0) {
											if(std::find(specific_list.begin(), specific_list.end(), bb) == specific_list.end()) {
												specific_list.push_back(bb);
											}
										}
										found = true;
										break;
									}
								}

								if(found == false) {
									BorderCluster bc;
									bc.alignment = tiledata;
									bc.z = other->getZ();
									bc.border = bb->autoborder;
									border_list.push_back(bc);
									if(bb->specific_cases.size() > 0) {
										if(std::find(specific_list.begin(), specific_list.end(), bb) == specific_list.end()) {
											specific_list.push_back(bb);
										}
									}
								}
							}
						}
					}
				}
			} else if(border_brush->hasInnerZilchBorder()) {
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for(int j = i; j < 8; j++) {
					if(neighbours[j].first == false && !neighbours[j].second) {
						neighbours[j].first = true;
						tiledata |= 1 << j;
					}
				}

				if(tiledata != 0) {
					BorderCluster cluster;
					cluster.alignment = tiledata;
					cluster.z = 5000;
					const BorderBlock* bb = getBrushTo(border_brush, NULL);
					if(!bb) {
						continue;
					}
					cluster.border = bb->autoborder;
					if(cluster.border) {
						border_list.push_back(cluster);
					}
					if(bb->specific_cases.size() > 0) {
						if(std::find(specific_list.begin(), specific_list.end(), bb) == specific_list.end()) {
							specific_list.push_back(bb);
						}
					}
				}
				continue;
			}
		} else if(other && other->hasOuterZilchBorder()) { // outer zilch border
			// Border against nothing (or undefined tile)
			uint32_t tiledata = 0;
			for(int j = i; j < 8; j++) {
				if(neighbours[j].first == false && neighbours[j].second && neighbours[j].second->getID() == other->getID()) {
					neighbours[j].first = true;
					tiledata |= 1 << j;
				}
			}

			if(tiledata != 0) {
				BorderCluster cluster;
				cluster.alignment = tiledata;
				cluster.z = other->getZ();
				const BorderBlock* bb = getBrushTo(NULL, other);
				if(bb) {
					cluster.border = bb->autoborder;
					if(cluster.border) {
						border_list.push_back(cluster);
					}
					if(bb->specific_cases.size() > 0) {
						if(std::find(specific_list.begin(), specific_list.end(), bb) == specific_list.end()) {
							specific_list.push_back(bb);
						}
					}
				}

				// Add mountain if appropriate!
				if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
					BorderCluster cluster;
					cluster.alignment = tiledata;
					cluster.z = 0x7FFFFFFF; // Above other zilch borders
					cluster.border = other->optional_border;
					border_list.push_back(cluster);
				} else {
					tile->setOptionalBorder(false);
				}
			}
		}
		// Check tile as done
		neighbours[i].first = true;
	}

	sort(border_list.begin(), border_list.end());

	tile->cleanBorders();
	while(border_list.empty() == false) {
		BorderCluster& cluster = border_list.back();
		if(cluster.border == NULL) {
			border_list.pop_back();
			continue;
		}
		::BorderType i[4];

		i[0] = ::BorderType((border_types[cluster.alignment] & 0x000000FF) >> 0);
		i[1] = ::BorderType((border_types[cluster.alignment] & 0x0000FF00) >> 8);
		i[2] = ::BorderType((border_types[cluster.alignment] & 0x00FF0000) >> 16);
		i[3] = ::BorderType((border_types[cluster.alignment] & 0xFF000000) >> 24);

		for(int iter = 0; iter < 4; ++iter) {
			if(i[iter]) {
				if(cluster.border->tiles[i[iter]]) {
					tile->addBorderItem(Item::Create(cluster.border->tiles[i[iter]]));
				} else {
					if(i[iter] == NORTHWEST_DIAGONAL) {
						tile->addBorderItem(Item::Create(cluster.border->tiles[WEST_HORIZONTAL]));
						tile->addBorderItem(Item::Create(cluster.border->tiles[NORTH_HORIZONTAL]));
					} else if(i[iter] == NORTHEAST_DIAGONAL) {
						tile->addBorderItem(Item::Create(cluster.border->tiles[EAST_HORIZONTAL]));
						tile->addBorderItem(Item::Create(cluster.border->tiles[NORTH_HORIZONTAL]));
					} else if(i[iter] == SOUTHWEST_DIAGONAL) {
						tile->addBorderItem(Item::Create(cluster.border->tiles[SOUTH_HORIZONTAL]));
						tile->addBorderItem(Item::Create(cluster.border->tiles[WEST_HORIZONTAL]));
					} else if(i[iter] == SOUTHEAST_DIAGONAL) {
						tile->addBorderItem(Item::Create(cluster.border->tiles[SOUTH_HORIZONTAL]));
						tile->addBorderItem(Item::Create(cluster.border->tiles[EAST_HORIZONTAL]));
					}
				}
			} else {
				break;
			}
		}

		border_list.pop_back();
	}

	for(std::vector<const BorderBlock*>::const_iterator specific_outer_iter = specific_list.begin();
			specific_outer_iter != specific_list.end();
			++specific_outer_iter)
	{
		const BorderBlock* specific = *specific_outer_iter;
		//printf("Specific block\n");
		for(std::vector<SpecificCaseBlock*>::const_iterator specific_it = specific->specific_cases.begin();
				specific_it!= specific->specific_cases.end();
				++specific_it)
		{
			//printf("New round\n");
			SpecificCaseBlock* const scb = *specific_it;
			//if(scb->to_replace_id == 0) continue;
			//if(scb->with_id == 0) continue;
			ItemVector::iterator it = tile->items.begin();

			uint matches = 0;
			while(it != tile->items.end()) {
				Item* item = *it;
				if(item->isBorder() == false) {
					break;
				}

				if(scb->match_group > 0) {
					//printf("Matching %d == %d : %d == %d\n", item->getBorderGroup(), scb->match_group, item->getBorderAlignment(), scb->group_match_alignment);
					if(item->getBorderGroup() == scb->match_group && item->getBorderAlignment() == scb->group_match_alignment) {
						//printf("Successfully matched %d == %d : %d == %d\n", item->getBorderGroup(), scb->match_group, item->getBorderAlignment(), scb->group_match_alignment);
						++matches;
						++it;
						continue;
					}
				}

				//printf("\tInvestigating first item id:%d\n", item->getID());
				for(std::vector<uint16_t>::iterator case_it = scb->items_to_match.begin();
						case_it != scb->items_to_match.end();
						++case_it)
				{
					if(item->getID() == (*case_it)) {
						//printf("\t\tMatched item id %d\n", item->getID());
						++matches;
					}
				}
				++it;
			}

			//printf("\t\t%d matches of %d\n", matches, scb->items_to_match.size());
			if(matches == scb->items_to_match.size()) {
				if(scb->delete_all) {
					// Delete all matching borders
					it = tile->items.begin();

					while(it != tile->items.end()) {
						bool inc = true;
						Item* item = *it;
						if(item->isBorder() == false) {
							break;
						}
						for(std::vector<uint16_t>::iterator case_it = scb->items_to_match.begin();
								case_it != scb->items_to_match.end();
								++case_it) {
							if(item->getID() == (*case_it)) {
								delete *it;
								it = tile->items.erase(it);
								inc = false;
								break;
							}
						}
						if(inc) ++it;
					}
				} else {
					// All matched, replace!
					it = tile->items.begin();
					while(true) {
						if(it == tile->items.end()) {
							return;
						}
						if((*it)->isBorder() == false) {
							return;
						}
						if((*it)->getID() == scb->to_replace_id) {
							(*it)->setID(scb->with_id);
							return;
						}
						++it;
					}
				}
			}
		}
	}
}
