#include "main.h"

#include "ground_brush.h"
#include "items.h"
#include "basemap.h"
#include "pugicast.h"

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

bool AutoBorder::load(pugi::xml_node node, wxArrayString& warnings, GroundBrush* owner, uint16_t ground_equivalent)
{
	ASSERT(ground ? ground_equivalent != 0 : true);

	pugi::xml_attribute attribute;

	bool optionalBorder = false;
	if ((attribute = node.attribute("type"))) {
		if (std::string(attribute.as_string()) == "optional") {
			optionalBorder = true;
		}
	}

	if ((attribute = node.attribute("group"))) {
		group = pugi::cast<uint16_t>(attribute.value());
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (!(attribute = childNode.attribute("item"))) {
			continue;
		}

		int32_t itemid = pugi::cast<int32_t>(attribute.value());
		if (!(attribute = childNode.attribute("edge"))) {
			continue;
		}

		const std::string& orientation = attribute.as_string();

		ItemType& it = item_db[itemid];
		if (it.id == 0) {
			warnings.push_back(wxT("Invalid item ID ") + std::to_string(itemid) + wxT(" for border ") + std::to_string(id));
			continue;
		}

		if (ground) { // We are a ground border
			it.group = ITEM_GROUP_NONE;
			it.ground_equivalent = ground_equivalent;
			it.brush = owner;

			ItemType& it2 = item_db[ground_equivalent];
			it2.has_equivalent = it2.id != 0;
		}

		it.alwaysOnBottom = true; // Never-ever place other items under this, will confuse the user something awful.
		it.isBorder = true;
		it.isOptionalBorder = it.isOptionalBorder ? true : optionalBorder;
		if (group && !it.border_group) {
			it.border_group = group;
		}

		int32_t edge_id = edgeNameToID(orientation);
		if (edge_id != BORDER_NONE) {
			tiles[edge_id] = itemid;
			if (it.border_alignment == BORDER_NONE) {
				it.border_alignment = ::BorderType(edge_id);
			}
		}
	}
	return true;
}

GroundBrush::GroundBrush() :
	z_order(0),
	has_zilch_outer_border(false),
	has_zilch_inner_border(false),
	has_outer_border(false),
	has_inner_border(false),
	optional_border(nullptr),
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

bool GroundBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if ((attribute = node.attribute("lookid"))) {
		look_id = pugi::cast<uint16_t>(attribute.value());
	}

	if ((attribute = node.attribute("server_lookid"))) {
		look_id = item_db[pugi::cast<uint16_t>(attribute.value())].clientID;
	}

	if ((attribute = node.attribute("z-order"))) {
		z_order = pugi::cast<int32_t>(attribute.value());
	}

	if ((attribute = node.attribute("solo_optional"))) {
		use_only_optional = attribute.as_bool();
	}

	if ((attribute = node.attribute("randomize"))) {
		randomize = attribute.as_bool();
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if (childName == "item") {
			uint16_t itemId = pugi::cast<uint16_t>(childNode.attribute("id").value());
			int32_t chance = pugi::cast<int32_t>(childNode.attribute("chance").value());

			ItemType& it = item_db[itemId];
			if (it.id == 0) {
				warnings.push_back(wxT("\nInvalid item id ") + std::to_string(itemId));
				return false;
			}

			if (!it.isGroundTile()) {
				warnings.push_back(wxT("\nItem ") + std::to_string(itemId) + wxT(" is not ground item."));
				return false;
			}

			if (it.brush && it.brush != this) {
				warnings.push_back(wxT("\nItem ") + std::to_string(itemId) + wxT(" can not be member of two brushes"));
				return false;
			}

			it.brush = this;
			total_chance += chance;

			ItemChanceBlock ci;
			ci.id = itemId;
			ci.chance = total_chance;
			border_items.push_back(ci);
		} else if (childName == "optional") {
			// Mountain border!
			if (optional_border) {
				warnings.push_back(wxT("\nDuplicate optional borders!"));
				continue;
			}

			if ((attribute = childNode.attribute("ground_equivalent"))) {
				uint16_t ground_equivalent = pugi::cast<uint16_t>(attribute.value());

				// Load from inline definition
				ItemType& it = item_db[ground_equivalent];
				if (it.id == 0) {
					warnings.push_back(wxT("Invalid id of ground dependency equivalent item.\n"));
					continue;
				} else if (!it.isGroundTile()) {
					warnings.push_back(wxT("Ground dependency equivalent is not a ground item.\n"));
					continue;
				} else if (it.brush && it.brush != this) {
					warnings.push_back(wxT("Ground dependency equivalent does not use the same brush as ground border.\n"));
					continue;
				}

				AutoBorder* autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, warnings, this, ground_equivalent);
				optional_border = autoBorder;
			} else {
				// Load from ID
				if (!(attribute = childNode.attribute("id"))) {
					warnings.push_back(wxT("\nMissing tag id for border node"));
					continue;
				}

				uint16_t id = pugi::cast<uint16_t>(attribute.value());
				auto it = brushes.borders.find(id);
				if (it == brushes.borders.end() || !it->second) {
					warnings.push_back(wxT("\nCould not find border id ") + std::to_string(id));
					continue;
				}

				optional_border = it->second;
			}
		} else if (childName == "border") {
			AutoBorder* autoBorder;
			if (!(attribute = childNode.attribute("id"))) {
				if (!(attribute = childNode.attribute("ground_equivalent"))) {
					continue;
				}

				uint16_t ground_equivalent = pugi::cast<uint16_t>(attribute.value());
				ItemType& it = item_db[ground_equivalent];
				if (it.id == 0) {
					warnings.push_back(wxT("Invalid id of ground dependency equivalent item.\n"));
				}

				if (!it.isGroundTile()) {
					warnings.push_back(wxT("Ground dependency equivalent is not a ground item.\n"));
				}

				if (it.brush && it.brush != this) {
					warnings.push_back(wxT("Ground dependency equivalent does not use the same brush as ground border.\n"));
				}

				autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, warnings, this, ground_equivalent);
			} else {
				int32_t id = pugi::cast<int32_t>(attribute.value());
				if (id == 0) {
					autoBorder = nullptr;
				} else {
					auto it = brushes.borders.find(id);
					if (it == brushes.borders.end() || !it->second) {
						warnings.push_back(wxT("\nCould not find border id ") + std::to_string(id));
						continue;
					}
					autoBorder = it->second;
				}
			}

			BorderBlock* borderBlock = newd BorderBlock;
			borderBlock->super = false;
			borderBlock->autoborder = autoBorder;

			if ((attribute = childNode.attribute("to"))) {
				const std::string& value = attribute.as_string();
				if (value == "all") {
					borderBlock->to = 0xFFFFFFFF;
				} else if (value == "none") {
					borderBlock->to = 0;
				} else {
					Brush* tobrush = brushes.getBrush(value);
					if (!tobrush) {
						warnings.push_back(wxT("To brush ") + wxstr(value) + wxT(" doesn't exist."));
						continue;
					}
					borderBlock->to = tobrush->getID();
				}
			} else {
				borderBlock->to = 0xFFFFFFFF;
			}

			if ((attribute = childNode.attribute("super")) && attribute.as_bool()) {
				borderBlock->super = true;
			}

			if ((attribute = childNode.attribute("align"))) {
				const std::string& value = attribute.as_string();
				if (value == "outer") {
					borderBlock->outer = true;
				} else if (value == "inner") {
					borderBlock->outer = false;
				} else {
					borderBlock->outer = true;
				}
			}

			if (borderBlock->outer) {
				if (borderBlock->to == 0) {
					has_zilch_outer_border = true;
				} else {
					has_outer_border = true;
				}
			} else {
				if (borderBlock->to == 0) {
					has_zilch_inner_border = true;
				} else {
					has_inner_border = true;
				}
			}

			for (pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
				if (as_lower_str(subChildNode.name()) != "specific") {
					continue;
				}

				SpecificCaseBlock* specificCaseBlock = nullptr;
				for (pugi::xml_node superChildNode = subChildNode.first_child(); superChildNode; superChildNode = superChildNode.next_sibling()) {
					const std::string& superChildName = as_lower_str(superChildNode.name());
					if (superChildName == "conditions") {
						for (pugi::xml_node conditionChild = superChildNode.first_child(); conditionChild; conditionChild = conditionChild.next_sibling()) {
							const std::string& conditionName = as_lower_str(conditionChild.name());
							if (conditionName == "match_border") {
								if (!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = pugi::cast<int32_t>(attribute.value());
								if (!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								auto it = brushes.borders.find(border_id);
								if (it == brushes.borders.end()) {
									warnings.push_back(wxT("Unknown border id in specific case match block ") + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = it->second;
								ASSERT(autoBorder != nullptr);

								uint32_t match_itemid = autoBorder->tiles[edge_id];
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->items_to_match.push_back(match_itemid);
							} else if (conditionName == "match_group") {
								if (!(attribute = conditionChild.attribute("group"))) {
									continue;
								}

								uint16_t group = pugi::cast<uint16_t>(attribute.value());
								if (!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = group;
								specificCaseBlock->group_match_alignment = ::BorderType(edge_id);
								specificCaseBlock->items_to_match.push_back(group);
							} else if (conditionName == "match_item") {
								if (!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t match_itemid = pugi::cast<int32_t>(attribute.value());
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = 0;
								specificCaseBlock->items_to_match.push_back(match_itemid);
							}
						}
					} else if (superChildName == "actions") {
						for (pugi::xml_node actionChild = superChildNode.first_child(); actionChild; actionChild = actionChild.next_sibling()) {
							const std::string& actionName = as_lower_str(actionChild.name());
							if (actionName == "replace_border") {
								if (!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = pugi::cast<int32_t>(attribute.value());
								if (!(attribute = actionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if (!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = pugi::cast<int32_t>(attribute.value());
								auto itt = brushes.borders.find(border_id);
								if (itt == brushes.borders.end()) {
									warnings.push_back(wxT("Unknown border id in specific case match block ") + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = itt->second;
								ASSERT(autoBorder != nullptr);

								ItemType& it = item_db[with_id];
								if (it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = autoBorder->tiles[edge_id];
								specificCaseBlock->with_id = with_id;
							} else if (actionName == "replace_item") {
								if (!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t to_replace_id = pugi::cast<int32_t>(attribute.value());
								if (!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = pugi::cast<int32_t>(attribute.value());
								ItemType& it = item_db[with_id];
								if (it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = to_replace_id;
								specificCaseBlock->with_id = with_id;
							} else if (actionName == "delete_borders") {
								if (!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->delete_all = true;
							}
						}
					}
				}
			}
			borders.push_back(borderBlock);
		} else if (childName == "friend") {
			const std::string& name = childNode.attribute("name").as_string();
			if (!name.empty()) {
				if (name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = brushes.getBrush(name);
					if (brush) {
						friends.push_back(brush->getID());
					} else {
						warnings.push_back(wxT("Brush '") + wxstr(name) + wxT("' is not defined."));
					}
				}
			}
			hate_friends = false;
		} else if (childName == "enemy") {
			const std::string& name = childNode.attribute("name").as_string();
			if (!name.empty()) {
				if (name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = brushes.getBrush(name);
					if (brush) {
						friends.push_back(brush->getID());
					} else {
						warnings.push_back(wxT("Brush '") + wxstr(name) + wxT("' is not defined."));
					}
				}
			}
			hate_friends = true;
		} else if (childName == "clear_borders") {
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
		} else if (childName == "clear_friends") {
			friends.clear();
			hate_friends = false;
		}
	}

	if (total_chance == 0) {
		randomize = false;
	}
	
	return true;
}

void GroundBrush::undraw(BaseMap* map, Tile* tile) {
	ASSERT(tile);
	if(tile->hasGround() && tile->ground->getGroundBrush() == this) {
		delete tile->ground;
		tile->ground = nullptr;
	}
}

void GroundBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	if(border_items.empty()) return;

	if(parameter != nullptr) {
		std::pair<bool, GroundBrush*>& param = *reinterpret_cast<std::pair<bool, GroundBrush*>* >(parameter);
		GroundBrush* other = tile->getGroundBrush();
		if(param.first) { // Volatile? :)
			if(other != nullptr) {
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
	return nullptr;
}

inline GroundBrush* extractGroundBrushFromTile(BaseMap* map, uint32_t x, uint32_t y, uint32_t z) {
	Tile* t = map->getTile(x, y, z);
	return t? t->getGroundBrush() : nullptr;
}

void GroundBrush::doBorders(BaseMap* map, Tile* tile) {
	ASSERT(tile);

	GroundBrush* border_brush = (tile->ground? tile->ground->getGroundBrush() : nullptr);

	unsigned int x = tile->getPosition().x;
	unsigned int y = tile->getPosition().y;
	unsigned int z = tile->getPosition().z;
	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8];

	if(x == 0) {
		if(y == 0) {
			neighbours[0] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[1] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[2] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[3] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
			neighbours[5] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
			neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
		} else {
			neighbours[0] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[1] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y - 1, z));
			neighbours[2] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y - 1, z));
			neighbours[3] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[4] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y, z));
			neighbours[5] = std::make_pair<bool, GroundBrush*>(false, nullptr);
			neighbours[6] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x,     y + 1, z));
			neighbours[7] = std::make_pair<bool, GroundBrush*>(false, extractGroundBrushFromTile(map, x + 1, y + 1, z));
		}
	} else if(y == 0) {
		neighbours[0] = std::make_pair<bool, GroundBrush*>(false, nullptr);
		neighbours[1] = std::make_pair<bool, GroundBrush*>(false, nullptr);
		neighbours[2] = std::make_pair<bool, GroundBrush*>(false, nullptr);
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
					const BorderBlock* bb = getBrushTo(border_brush, nullptr);
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
				const BorderBlock* bb = getBrushTo(nullptr, other);
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
		if(cluster.border == nullptr) {
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

			uint32_t matches = 0;
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
