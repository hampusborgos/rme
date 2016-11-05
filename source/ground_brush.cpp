#include "main.h"

#include "ground_brush.h"
#include "items.h"
#include "basemap.h"
#include "pugicast.h"

uint32_t GroundBrush::border_types[256];

int AutoBorder::edgeNameToID(const std::string& edgename)
{
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
	if((attribute = node.attribute("type"))) {
		if(std::string(attribute.as_string()) == "optional") {
			optionalBorder = true;
		}
	}

	if((attribute = node.attribute("group"))) {
		group = pugi::cast<uint16_t>(attribute.value());
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(!(attribute = childNode.attribute("item"))) {
			continue;
		}

		int32_t itemid = pugi::cast<int32_t>(attribute.value());
		if(!(attribute = childNode.attribute("edge"))) {
			continue;
		}

		const std::string& orientation = attribute.as_string();

		ItemType& it = g_items[itemid];
		if(it.id == 0) {
			warnings.push_back("Invalid item ID " + std::to_string(itemid) + " for border " + std::to_string(id));
			continue;
		}

		if(ground) { // We are a ground border
			it.group = ITEM_GROUP_NONE;
			it.ground_equivalent = ground_equivalent;
			it.brush = owner;

			ItemType& it2 = g_items[ground_equivalent];
			it2.has_equivalent = it2.id != 0;
		}

		it.alwaysOnBottom = true; // Never-ever place other items under this, will confuse the user something awful.
		it.isBorder = true;
		it.isOptionalBorder = it.isOptionalBorder ? true : optionalBorder;
		if(group && !it.border_group) {
			it.border_group = group;
		}

		int32_t edge_id = edgeNameToID(orientation);
		if(edge_id != BORDER_NONE) {
			tiles[edge_id] = itemid;
			if(it.border_alignment == BORDER_NONE) {
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
	////
}

GroundBrush::~GroundBrush()
{
	for(BorderBlock* borderBlock : borders) {
		if(borderBlock->autoborder) {
			for(SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
				delete specificCaseBlock;
			}

			if(borderBlock->autoborder->ground) {
				delete borderBlock->autoborder;
			}
		}
		delete borderBlock;
	}
	borders.clear();
}

bool GroundBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = pugi::cast<uint16_t>(attribute.value());
	}

	if((attribute = node.attribute("server_lookid"))) {
		look_id = g_items[pugi::cast<uint16_t>(attribute.value())].clientID;
	}

	if((attribute = node.attribute("z-order"))) {
		z_order = pugi::cast<int32_t>(attribute.value());
	}

	if((attribute = node.attribute("solo_optional"))) {
		use_only_optional = attribute.as_bool();
	}

	if((attribute = node.attribute("randomize"))) {
		randomize = attribute.as_bool();
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "item") {
			uint16_t itemId = pugi::cast<uint16_t>(childNode.attribute("id").value());
			int32_t chance = pugi::cast<int32_t>(childNode.attribute("chance").value());

			ItemType& it = g_items[itemId];
			if(it.id == 0) {
				warnings.push_back("\nInvalid item id " + std::to_string(itemId));
				return false;
			}

			if(!it.isGroundTile()) {
				warnings.push_back("\nItem " + std::to_string(itemId) + " is not ground item.");
				return false;
			}

			if(it.brush && it.brush != this) {
				warnings.push_back("\nItem " + std::to_string(itemId) + " can not be member of two brushes");
				return false;
			}

			it.brush = this;
			total_chance += chance;

			ItemChanceBlock ci;
			ci.id = itemId;
			ci.chance = total_chance;
			border_items.push_back(ci);
		} else if(childName == "optional") {
			// Mountain border!
			if(optional_border) {
				warnings.push_back("\nDuplicate optional borders!");
				continue;
			}

			if((attribute = childNode.attribute("ground_equivalent"))) {
				uint16_t ground_equivalent = pugi::cast<uint16_t>(attribute.value());

				// Load from inline definition
				ItemType& it = g_items[ground_equivalent];
				if(it.id == 0) {
					warnings.push_back("Invalid id of ground dependency equivalent item.\n");
					continue;
				} else if(!it.isGroundTile()) {
					warnings.push_back("Ground dependency equivalent is not a ground item.\n");
					continue;
				} else if(it.brush && it.brush != this) {
					warnings.push_back("Ground dependency equivalent does not use the same brush as ground border.\n");
					continue;
				}

				AutoBorder* autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, warnings, this, ground_equivalent);
				optional_border = autoBorder;
			} else {
				// Load from ID
				if(!(attribute = childNode.attribute("id"))) {
					warnings.push_back("\nMissing tag id for border node");
					continue;
				}

				uint16_t id = pugi::cast<uint16_t>(attribute.value());
				auto it = g_brushes.borders.find(id);
				if(it == g_brushes.borders.end() || !it->second) {
					warnings.push_back("\nCould not find border id " + std::to_string(id));
					continue;
				}

				optional_border = it->second;
			}
		} else if(childName == "border") {
			AutoBorder* autoBorder;
			if(!(attribute = childNode.attribute("id"))) {
				if(!(attribute = childNode.attribute("ground_equivalent"))) {
					continue;
				}

				uint16_t ground_equivalent = pugi::cast<uint16_t>(attribute.value());
				ItemType& it = g_items[ground_equivalent];
				if(it.id == 0) {
					warnings.push_back("Invalid id of ground dependency equivalent item.\n");
				}

				if(!it.isGroundTile()) {
					warnings.push_back("Ground dependency equivalent is not a ground item.\n");
				}

				if(it.brush && it.brush != this) {
					warnings.push_back("Ground dependency equivalent does not use the same brush as ground border.\n");
				}

				autoBorder = newd AutoBorder(0); // Empty id basically
				autoBorder->load(childNode, warnings, this, ground_equivalent);
			} else {
				int32_t id = pugi::cast<int32_t>(attribute.value());
				if(id == 0) {
					autoBorder = nullptr;
				} else {
					auto it = g_brushes.borders.find(id);
					if(it == g_brushes.borders.end() || !it->second) {
						warnings.push_back("\nCould not find border id " + std::to_string(id));
						continue;
					}
					autoBorder = it->second;
				}
			}

			BorderBlock* borderBlock = newd BorderBlock;
			borderBlock->super = false;
			borderBlock->autoborder = autoBorder;

			if((attribute = childNode.attribute("to"))) {
				const std::string& value = attribute.as_string();
				if(value == "all") {
					borderBlock->to = 0xFFFFFFFF;
				} else if(value == "none") {
					borderBlock->to = 0;
				} else {
					Brush* tobrush = g_brushes.getBrush(value);
					if(!tobrush) {
						warnings.push_back("To brush " + wxstr(value) + " doesn't exist.");
						continue;
					}
					borderBlock->to = tobrush->getID();
				}
			} else {
				borderBlock->to = 0xFFFFFFFF;
			}

			if((attribute = childNode.attribute("super")) && attribute.as_bool()) {
				borderBlock->super = true;
			}

			if((attribute = childNode.attribute("align"))) {
				const std::string& value = attribute.as_string();
				if(value == "outer") {
					borderBlock->outer = true;
				} else if(value == "inner") {
					borderBlock->outer = false;
				} else {
					borderBlock->outer = true;
				}
			}

			if(borderBlock->outer) {
				if(borderBlock->to == 0) {
					has_zilch_outer_border = true;
				} else {
					has_outer_border = true;
				}
			} else {
				if(borderBlock->to == 0) {
					has_zilch_inner_border = true;
				} else {
					has_inner_border = true;
				}
			}

			for(pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
				if(as_lower_str(subChildNode.name()) != "specific") {
					continue;
				}

				SpecificCaseBlock* specificCaseBlock = nullptr;
				for(pugi::xml_node superChildNode = subChildNode.first_child(); superChildNode; superChildNode = superChildNode.next_sibling()) {
					const std::string& superChildName = as_lower_str(superChildNode.name());
					if(superChildName == "conditions") {
						for(pugi::xml_node conditionChild = superChildNode.first_child(); conditionChild; conditionChild = conditionChild.next_sibling()) {
							const std::string& conditionName = as_lower_str(conditionChild.name());
							if(conditionName == "match_border") {
								if(!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = pugi::cast<int32_t>(attribute.value());
								if(!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								auto it = g_brushes.borders.find(border_id);
								if(it == g_brushes.borders.end()) {
									warnings.push_back("Unknown border id in specific case match block " + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = it->second;
								ASSERT(autoBorder != nullptr);

								uint32_t match_itemid = autoBorder->tiles[edge_id];
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->items_to_match.push_back(match_itemid);
							} else if(conditionName == "match_group") {
								if(!(attribute = conditionChild.attribute("group"))) {
									continue;
								}

								uint16_t group = pugi::cast<uint16_t>(attribute.value());
								if(!(attribute = conditionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = group;
								specificCaseBlock->group_match_alignment = ::BorderType(edge_id);
								specificCaseBlock->items_to_match.push_back(group);
							} else if(conditionName == "match_item") {
								if(!(attribute = conditionChild.attribute("id"))) {
									continue;
								}

								int32_t match_itemid = pugi::cast<int32_t>(attribute.value());
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->match_group = 0;
								specificCaseBlock->items_to_match.push_back(match_itemid);
							}
						}
					} else if(superChildName == "actions") {
						for(pugi::xml_node actionChild = superChildNode.first_child(); actionChild; actionChild = actionChild.next_sibling()) {
							const std::string& actionName = as_lower_str(actionChild.name());
							if(actionName == "replace_border") {
								if(!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t border_id = pugi::cast<int32_t>(attribute.value());
								if(!(attribute = actionChild.attribute("edge"))) {
									continue;
								}

								int32_t edge_id = AutoBorder::edgeNameToID(attribute.as_string());
								if(!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = pugi::cast<int32_t>(attribute.value());
								auto itt = g_brushes.borders.find(border_id);
								if(itt == g_brushes.borders.end()) {
									warnings.push_back("Unknown border id in specific case match block " + std::to_string(border_id));
									continue;
								}

								AutoBorder* autoBorder = itt->second;
								ASSERT(autoBorder != nullptr);

								ItemType& it = g_items[with_id];
								if(it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = autoBorder->tiles[edge_id];
								specificCaseBlock->with_id = with_id;
							} else if(actionName == "replace_item") {
								if(!(attribute = actionChild.attribute("id"))) {
									continue;
								}

								int32_t to_replace_id = pugi::cast<int32_t>(attribute.value());
								if(!(attribute = actionChild.attribute("with"))) {
									continue;
								}

								int32_t with_id = pugi::cast<int32_t>(attribute.value());
								ItemType& it = g_items[with_id];
								if(it.id == 0) {
									return false;
								}

								it.isBorder = true;
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}

								specificCaseBlock->to_replace_id = to_replace_id;
								specificCaseBlock->with_id = with_id;
							} else if(actionName == "delete_borders") {
								if(!specificCaseBlock) {
									specificCaseBlock = newd SpecificCaseBlock();
								}
								specificCaseBlock->delete_all = true;
							}
						}
					}
				}
			}
			borders.push_back(borderBlock);
		} else if(childName == "friend") {
			const std::string& name = childNode.attribute("name").as_string();
			if(!name.empty()) {
				if(name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = g_brushes.getBrush(name);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						warnings.push_back("Brush '" + wxstr(name) + "' is not defined.");
					}
				}
			}
			hate_friends = false;
		} else if(childName == "enemy") {
			const std::string& name = childNode.attribute("name").as_string();
			if(!name.empty()) {
				if(name == "all") {
					friends.push_back(0xFFFFFFFF);
				} else {
					Brush* brush = g_brushes.getBrush(name);
					if(brush) {
						friends.push_back(brush->getID());
					} else {
						warnings.push_back("Brush '" + wxstr(name) + "' is not defined.");
					}
				}
			}
			hate_friends = true;
		} else if(childName == "clear_borders") {
			for(std::vector<BorderBlock*>::iterator it = borders.begin();
					it != borders.end();
					++it)
			{
				BorderBlock* bb = *it;
				if(bb->autoborder) {
					for(std::vector<SpecificCaseBlock*>::iterator specific_iter = bb->specific_cases.begin(); specific_iter != bb->specific_cases.end(); ++specific_iter) {
						delete *specific_iter;
					}
					if(bb->autoborder->ground) {
						delete bb->autoborder;
					}
				}
				delete bb;
			}
			borders.clear();
		} else if(childName == "clear_friends") {
			friends.clear();
			hate_friends = false;
		}
	}

	if(total_chance == 0) {
		randomize = false;
	}

	return true;
}

void GroundBrush::undraw(BaseMap* map, Tile* tile)
{
	ASSERT(tile);
	if(tile->hasGround() && tile->ground->getGroundBrush() == this) {
		delete tile->ground;
		tile->ground = nullptr;
	}
}

void GroundBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
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
						if(bb->outer) {
							continue;
						} else if(bb->to == second->getID() || bb->to == 0xFFFFFFFF) {
							//printf("%d\n", bb->autoborder);
							return bb;
						}
					}
				}
				for(std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if(!bb->outer) {
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
					if(bb->outer) {
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
				if(bb->outer) {
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
			if(!bb->outer) {
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

void GroundBrush::doBorders(BaseMap* map, Tile* tile)
{
	static const auto extractGroundBrushFromTile = [](BaseMap* map, uint32_t x, uint32_t y, uint32_t z) -> GroundBrush* {
		Tile* tile = map->getTile(x, y, z);
		if(tile) {
			return tile->getGroundBrush();
		}
		return nullptr;
	};

	ASSERT(tile);

	GroundBrush* borderBrush;
	if(tile->ground) {
		borderBrush = tile->ground->getGroundBrush();
	} else {
		borderBrush = nullptr;
	}

	const Position& position = tile->getPosition();

	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;

	// Pair of visited / what border type
	std::pair<bool, GroundBrush*> neighbours[8];
	if(x == 0) {
		if(y == 0) {
			neighbours[0] = { false, nullptr };
			neighbours[1] = { false, nullptr };
			neighbours[2] = { false, nullptr };
			neighbours[3] = { false, nullptr };
			neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y,     z) };
			neighbours[5] = { false, nullptr };
			neighbours[6] = { false, extractGroundBrushFromTile(map, x,     y + 1, z) };
			neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
		} else {
			neighbours[0] = { false, nullptr };
			neighbours[1] = { false, extractGroundBrushFromTile(map, x,     y - 1, z) };
			neighbours[2] = { false, extractGroundBrushFromTile(map, x + 1, y - 1, z) };
			neighbours[3] = { false, nullptr };
			neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y,     z) };
			neighbours[5] = { false, nullptr };
			neighbours[6] = { false, extractGroundBrushFromTile(map, x,     y + 1, z) };
			neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
		}
	} else if(y == 0) {
		neighbours[0] = { false, nullptr };
		neighbours[1] = { false, nullptr };
		neighbours[2] = { false, nullptr };
		neighbours[3] = { false, extractGroundBrushFromTile(map, x - 1, y,     z) };
		neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y,     z) };
		neighbours[5] = { false, extractGroundBrushFromTile(map, x - 1, y + 1, z) };
		neighbours[6] = { false, extractGroundBrushFromTile(map, x,     y + 1, z) };
		neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
	} else {
		neighbours[0] = { false, extractGroundBrushFromTile(map, x - 1, y - 1, z) };
		neighbours[1] = { false, extractGroundBrushFromTile(map, x,     y - 1, z) };
		neighbours[2] = { false, extractGroundBrushFromTile(map, x + 1, y - 1, z) };
		neighbours[3] = { false, extractGroundBrushFromTile(map, x - 1, y,     z) };
		neighbours[4] = { false, extractGroundBrushFromTile(map, x + 1, y,     z) };
		neighbours[5] = { false, extractGroundBrushFromTile(map, x - 1, y + 1, z) };
		neighbours[6] = { false, extractGroundBrushFromTile(map, x,     y + 1, z) };
		neighbours[7] = { false, extractGroundBrushFromTile(map, x + 1, y + 1, z) };
	}

	static std::vector<const BorderBlock*> specificList;
	specificList.clear();

	std::vector<BorderCluster> borderList;
	for(int32_t i = 0; i < 8; ++i) {
		auto& neighbourPair = neighbours[i];
		if(neighbourPair.first) {
			continue;
		}

		//printf("Checking neighbour #%d\n", i);
		//printf("\tNeighbour not checked before\n");

		GroundBrush* other = neighbourPair.second;
		if(borderBrush) {
			if(other) {
				//printf("\tNeighbour has brush\n");
				if(other->getID() == borderBrush->getID()) {
					//printf("\tNeighbour has same brush as we\n");
					continue;
				}

				if(other->hasOuterBorder() || borderBrush->hasInnerBorder()) {
					bool only_mountain = false;
					if(/*!borderBrush->hasInnerBorder() && */(other->friendOf(borderBrush) || borderBrush->friendOf(other))) {
						if(!other->hasOptionalBorder()) {
							continue;
						}
						only_mountain = true;
					}

					uint32_t tiledata = 0;
					for(int32_t j = i; j < 8; ++j) {
						auto& otherPair = neighbours[j];
						if(!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
							otherPair.first = true;
							tiledata |= 1 << j;
						}
					}

					if(tiledata != 0) {
						// Add mountain if appropriate!
						if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
							BorderCluster borderCluster;
							borderCluster.alignment = tiledata;
							borderCluster.z = 0x7FFFFFFF; // Above all other borders
							borderCluster.border = other->optional_border;

							borderList.push_back(borderCluster);
							if(other->useSoloOptionalBorder()) {
								only_mountain = true;
							}
						}

						if(!only_mountain) {
							const BorderBlock* borderBlock = getBrushTo(borderBrush, other);
							if(borderBlock) {
								bool found = false;
								for(BorderCluster& borderCluster : borderList) {
									if(borderCluster.border == borderBlock->autoborder) {
										borderCluster.alignment |= tiledata;
										if(borderCluster.z < other->getZ()) {
											borderCluster.z = other->getZ();
										}

										if(!borderBlock->specific_cases.empty()) {
											if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
												specificList.push_back(borderBlock);
											}
										}

										found = true;
										break;
									}
								}

								if(!found) {
									BorderCluster borderCluster;
									borderCluster.alignment = tiledata;
									borderCluster.z = other->getZ();
									borderCluster.border = borderBlock->autoborder;

									borderList.push_back(borderCluster);
									if(!borderBlock->specific_cases.empty()) {
										if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
											specificList.push_back(borderBlock);
										}
									}
								}
							}
						}
					}
				}
			} else if(borderBrush->hasInnerZilchBorder()) {
				// Border against nothing (or undefined tile)
				uint32_t tiledata = 0;
				for(int32_t j = i; j < 8; ++j) {
					auto& otherPair = neighbours[j];
					if(!otherPair.first && !otherPair.second) {
						otherPair.first = true;
						tiledata |= 1 << j;
					}
				}

				if(tiledata != 0) {
					const BorderBlock* borderBlock = getBrushTo(borderBrush, nullptr);
					if(!borderBlock) {
						continue;
					}

					if(borderBlock->autoborder) {
						BorderCluster borderCluster;
						borderCluster.alignment = tiledata;
						borderCluster.z = 5000;
						borderCluster.border = borderBlock->autoborder;

						borderList.push_back(borderCluster);
					}

					if(!borderBlock->specific_cases.empty()) {
						if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
							specificList.push_back(borderBlock);
						}
					}
				}
				continue;
			}
		} else if(other && other->hasOuterZilchBorder()) {
			// Border against nothing (or undefined tile)
			uint32_t tiledata = 0;
			for(int32_t j = i; j < 8; ++j) {
				auto& otherPair = neighbours[j];
				if(!otherPair.first && otherPair.second && otherPair.second->getID() == other->getID()) {
					otherPair.first = true;
					tiledata |= 1 << j;
				}
			}

			if(tiledata != 0) {
				const BorderBlock* borderBlock = getBrushTo(nullptr, other);
				if(borderBlock) {
					if(borderBlock->autoborder) {
						BorderCluster borderCluster;
						borderCluster.alignment = tiledata;
						borderCluster.z = other->getZ();
						borderCluster.border = borderBlock->autoborder;

						borderList.push_back(borderCluster);
					}

					if(!borderBlock->specific_cases.empty()) {
						if(std::find(specificList.begin(), specificList.end(), borderBlock) == specificList.end()) {
							specificList.push_back(borderBlock);
						}
					}
				}

				// Add mountain if appropriate!
				if(other->hasOptionalBorder() && tile->hasOptionalBorder()) {
					BorderCluster borderCluster;
					borderCluster.alignment = tiledata;
					borderCluster.z = 0x7FFFFFFF; // Above other zilch borders
					borderCluster.border = other->optional_border;

					borderList.push_back(borderCluster);
				} else {
					tile->setOptionalBorder(false);
				}
			}
		}
		// Check tile as done
		neighbourPair.first = true;
	}

	std::sort(borderList.begin(), borderList.end());
	tile->cleanBorders();

	while(!borderList.empty()) {
		BorderCluster& borderCluster = borderList.back();
		if(!borderCluster.border) {
			borderList.pop_back();
			continue;
		}

		BorderType directions[4] = {
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x000000FF) >> 0),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x0000FF00) >> 8),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0x00FF0000) >> 16),
			static_cast<BorderType>((border_types[borderCluster.alignment] & 0xFF000000) >> 24)
		};

		for(int32_t i = 0; i < 4; ++i) {
			BorderType direction = directions[i];
			if(direction == BORDER_NONE) {
				break;
			}

			if(borderCluster.border->tiles[direction]) {
				tile->addBorderItem(Item::Create(borderCluster.border->tiles[direction]));
			} else {
				if(direction == NORTHWEST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if(direction == NORTHEAST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[NORTH_HORIZONTAL]));
				} else if(direction == SOUTHWEST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[WEST_HORIZONTAL]));
				} else if(direction == SOUTHEAST_DIAGONAL) {
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[SOUTH_HORIZONTAL]));
					tile->addBorderItem(Item::Create(borderCluster.border->tiles[EAST_HORIZONTAL]));
				}
			}
		}

		borderList.pop_back();
	}

	for(const BorderBlock* borderBlock : specificList) {
		for(const SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
			/*
			printf("New round\n");
			if(specificCaseBlock->to_replace_id == 0) {
				continue;
			}

			if(specificCaseBlock->with_id == 0) {
				continue;
			}
			*/
			uint32_t matches = 0;
			for(Item* item : tile->items) {
				if(!item->isBorder()) {
					break;
				}

				if(specificCaseBlock->match_group > 0) {
					//printf("Matching %d == %d : %d == %d\n", item->getBorderGroup(), specificCaseBlock->match_group, item->getBorderAlignment(), specificCaseBlock->group_match_alignment);
					if(item->getBorderGroup() == specificCaseBlock->match_group && item->getBorderAlignment() == specificCaseBlock->group_match_alignment) {
						//printf("Successfully matched %d == %d : %d == %d\n", item->getBorderGroup(), specificCaseBlock->match_group, item->getBorderAlignment(), specificCaseBlock->group_match_alignment);
						++matches;
						continue;
					}
				}

				//printf("\tInvestigating first item id:%d\n", item->getID());
				for(uint16_t matchId : specificCaseBlock->items_to_match) {
					if(item->getID() == matchId) {
						//printf("\t\tMatched item id %d\n", item->getID());
						++matches;
					}
				}
			}

			//printf("\t\t%d matches of %d\n", matches, scb->items_to_match.size());
			if(matches == specificCaseBlock->items_to_match.size()) {
				auto& tileItems = tile->items;
				auto it = tileItems.begin();
				if(specificCaseBlock->delete_all) {
					// Delete all matching borders
					while(it != tileItems.end()) {
						Item* item = *it;
						if(!item->isBorder()) {
							break;
						}

						bool inc = true;
						for(uint16_t matchId : specificCaseBlock->items_to_match) {
							if(item->getID() == matchId) {
								delete item;
								it = tileItems.erase(it);
								inc = false;
								break;
							}
						}

						if(inc) {
							++it;
						}
					}
				} else {
					// All matched, replace!
					while(it != tileItems.end()) {
						Item* item = *it;
						if(!item->isBorder()) {
							return;
						}

						if(item->getID() == specificCaseBlock->to_replace_id) {
							item->setID(specificCaseBlock->with_id);
							return;
						}
						++it;
					}
				}
			}
		}
	}
}
