//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "doodad_brush.h"
#include "basemap.h"

#include <boost/lexical_cast.hpp>

//=============================================================================
// Doodad brush

DoodadBrush::DoodadBrush() :
	look_id(0),
	thickness(0),
	thickness_ceiling(0),
	draggable(false),
	on_blocking(false),
	one_size(false),
	do_new_borders(false),
	on_duplicate(false),
	clear_mapflags(0),
	clear_statflags(0)
{
	////
}

DoodadBrush::~DoodadBrush()
{
	for(std::vector<AlternativeBlock*>::iterator alt_iter = alternatives.begin(); alt_iter != alternatives.end(); ++alt_iter) {
		delete *alt_iter;
	}
}

DoodadBrush::AlternativeBlock::AlternativeBlock() :
	composite_chance(0),
	single_chance(0)
{
	////
}

DoodadBrush::AlternativeBlock::~AlternativeBlock()
{
	for(std::vector<CompositeBlock>::iterator composite_iter = composite_items.begin(); composite_iter != composite_items.end(); ++composite_iter) {
		CompositeTileList& tv = composite_iter->items;

		for(CompositeTileList::iterator compt_iter = tv.begin(); compt_iter != tv.end(); ++compt_iter) {
			ItemVector& items = compt_iter->second;
			for(ItemVector::iterator iiter = items.begin(); iiter != items.end(); ++iiter)
				delete *iiter;
		}
	}

	for(std::vector<SingleBlock>::iterator single_iter = single_items.begin(); single_iter != single_items.end(); ++single_iter) {
		delete single_iter->item;
	}
}

bool DoodadBrush::loadAlternative(pugi::xml_node node, wxArrayString& warnings, AlternativeBlock* which)
{
	AlternativeBlock* alternativeBlock;
	if(which) {
		alternativeBlock = which;
	} else {
		alternativeBlock = newd AlternativeBlock();
	}

	pugi::xml_attribute attribute;
	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "item") {
			if(!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			Item* item = Item::Create(childNode);
			if(!item) {
				warnings.push_back("Can't create item from doodad item node.");
				continue;
			}

			ItemType& it = g_items[item->getID()];
			if(it.id != 0) {
				it.doodad_brush = this;
			}

			SingleBlock sb;
			sb.item = item;
			sb.chance = attribute.as_int();

			alternativeBlock->single_items.push_back(sb);
			alternativeBlock->single_chance += sb.chance;
		} else if(childName == "composite") {
			if(!(attribute = childNode.attribute("chance"))) {
				warnings.push_back("Can't read chance tag of doodad item node.");
				continue;
			}

			alternativeBlock->composite_chance += attribute.as_int();

			CompositeBlock cb;
			cb.chance = alternativeBlock->composite_chance;

			for(pugi::xml_node compositeNode = childNode.first_child(); compositeNode; compositeNode = compositeNode.next_sibling()) {
				if(as_lower_str(compositeNode.name()) != "tile") {
					continue;
				}

				if(!(attribute = compositeNode.attribute("x"))) {
					warnings.push_back("Couldn't read positionX values of composite tile node.");
					continue;
				}

				int32_t x = attribute.as_int();
				if(!(attribute = compositeNode.attribute("y"))) {
					warnings.push_back("Couldn't read positionY values of composite tile node.");
					continue;
				}

				int32_t y = attribute.as_int();
				int32_t z = compositeNode.attribute("z").as_int();
				if(x < -0x7FFF || x > 0x7FFF) {
					warnings.push_back("Invalid range of x value on composite tile node.");
					continue;
				} else if(y < -0x7FFF || y > 0x7FFF) {
					warnings.push_back("Invalid range of y value on composite tile node.");
					continue;
				} else if(z < -0x7 || z > 0x7) {
					warnings.push_back("Invalid range of z value on composite tile node.");
					continue;
				}

				ItemVector items;
				for(pugi::xml_node itemNode = compositeNode.first_child(); itemNode; itemNode = itemNode.next_sibling()) {
					if(as_lower_str(itemNode.name()) != "item") {
						continue;
					}

					Item* item = Item::Create(itemNode);
					if(item) {
						items.push_back(item);

						ItemType& it = g_items[item->getID()];
						if(it.id != 0) {
							it.doodad_brush = this;
						}
					}
				}

				if(!items.empty()) {
					cb.items.push_back(std::make_pair(Position(x, y, z), items));
				}
			}
			alternativeBlock->composite_items.push_back(cb);
		}
	}

	if(!which) {
		alternatives.push_back(alternativeBlock);
	}
	return true;
}

bool DoodadBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = attribute.as_ushort();
	}

	if((attribute = node.attribute("server_lookid"))) {
		look_id = g_items[attribute.as_ushort()].clientID;
	}

	if((attribute = node.attribute("on_blocking"))) {
		on_blocking = attribute.as_bool();
	}

	if((attribute = node.attribute("on_duplicate"))) {
		on_duplicate = attribute.as_bool();
	}

	if((attribute = node.attribute("redo_borders")) || (attribute = node.attribute("reborder"))) {
		do_new_borders = attribute.as_bool();
	}

	if((attribute = node.attribute("one_size"))) {
		one_size = attribute.as_bool();
	}

	if((attribute = node.attribute("draggable"))) {
		draggable = attribute.as_bool();
	}

	if(node.attribute("remove_optional_border").as_bool()) {
		if(!do_new_borders) {
			warnings.push_back("remove_optional_border will not work without redo_borders\n");
		}
		clear_statflags |= TILESTATE_OP_BORDER;
	}

	const std::string& thicknessString = node.attribute("thickness").as_string();
	if(!thicknessString.empty()) {
		size_t slash = thicknessString.find('/');
		if(slash != std::string::npos) {
			thickness = boost::lexical_cast<int32_t>(thicknessString.substr(0, slash));
			thickness_ceiling = std::max<int32_t>(thickness, boost::lexical_cast<int32_t>(thicknessString.substr(slash + 1)));
		}
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(as_lower_str(childNode.name()) != "alternate") {
			continue;
		}
		if(!loadAlternative(childNode, warnings)) {
			return false;
		}
	}
	loadAlternative(node, warnings, alternatives.empty() ? nullptr : alternatives.back());
	return true;
}

bool DoodadBrush::AlternativeBlock::ownsItem(uint16_t id) const
{
	for(std::vector<SingleBlock>::const_iterator single_iter = single_items.begin(); single_iter != single_items.end(); ++single_iter) {
		if(single_iter->item->getID() == id) {
			return true;
		}
	}

	for(std::vector<CompositeBlock>::const_iterator composite_iter = composite_items.begin(); composite_iter != composite_items.end(); ++composite_iter) {
		const CompositeTileList& ctl = composite_iter->items;
		for(CompositeTileList::const_iterator comp_iter = ctl.begin(); comp_iter != ctl.end(); ++comp_iter) {
			const ItemVector& items = comp_iter->second;

			for(ItemVector::const_iterator item_iter = items.begin(), item_end = items.end(); item_iter != item_end; ++item_iter) {
				if((*item_iter)->getID() == id) {
					return true;
				}
			}
		}
	}
	return false;
}

bool DoodadBrush::ownsItem(Item* item) const
{
	if(item->getDoodadBrush() == this) return true;
	uint16_t id = item->getID();

	for(std::vector<AlternativeBlock*>::const_iterator alt_iter = alternatives.begin(); alt_iter != alternatives.end(); ++alt_iter) {
		if((*alt_iter)->ownsItem(id)) {
			return true;
		}
	}
	return false;
}

void DoodadBrush::undraw(BaseMap* map, Tile* tile)
{
	// Remove all doodad-related
	for(ItemVector::iterator item_iter = tile->items.begin(); item_iter != tile->items.end();) {
		Item* item = *item_iter;
		if(item->getDoodadBrush() != nullptr) {
			if(item->isComplex() && g_settings.getInteger(Config::ERASER_LEAVE_UNIQUE)) {
				++item_iter;
			} else if(g_settings.getInteger(Config::DOODAD_BRUSH_ERASE_LIKE)) {
				// Only delete items of the same doodad brush
				if(ownsItem(item)) {
					delete item;
					item_iter = tile->items.erase(item_iter);
				} else {
					++item_iter;
				}
			} else {
				delete item;
				item_iter = tile->items.erase(item_iter);
			}
		} else {
			++item_iter;
		}
	}

	if(tile->ground && tile->ground->getDoodadBrush() != nullptr) {
		if(g_settings.getInteger(Config::DOODAD_BRUSH_ERASE_LIKE)) {
			// Only delete items of the same doodad brush
			if(ownsItem(tile->ground)) {
				delete tile->ground;
				tile->ground = nullptr;
			}
		} else {
			delete tile->ground;
			tile->ground = nullptr;
		}
	}
}

void DoodadBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	int variation = 0;
	if(parameter) {
		variation = *reinterpret_cast<int*>(parameter);
	}

	if(alternatives.empty()) return;

	variation %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[variation];
	ASSERT(ab_ptr);

	int roll = random(1, ab_ptr->single_chance);
	for(std::vector<SingleBlock>::const_iterator block_iter = ab_ptr->single_items.begin(); block_iter != ab_ptr->single_items.end(); ++block_iter) {
		const SingleBlock& sb = *block_iter;
		if(roll <= sb.chance) {
			// Use this!
			tile->addItem(sb.item->deepCopy());
			break;
		}
		roll -= sb.chance;
	}
	if(clear_mapflags || clear_statflags) {
		tile->setMapFlags(tile->getMapFlags() & (~clear_mapflags));
		tile->setMapFlags(tile->getStatFlags() & (~clear_statflags));
	}
}

const CompositeTileList& DoodadBrush::getComposite(int variation) const
{
	static CompositeTileList empty;

	if(alternatives.empty())
		return empty;

	variation %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[variation];
	ASSERT(ab_ptr);

	int roll = random(1, ab_ptr->composite_chance);
	for(std::vector<CompositeBlock>::const_iterator block_iter = ab_ptr->composite_items.begin(); block_iter != ab_ptr->composite_items.end(); ++block_iter) {
		const CompositeBlock& cb = *block_iter;
		if(roll <= cb.chance) {
			return cb.items;
		}
	}
	return empty;
}

bool DoodadBrush::isEmpty(int variation) const
{
	if(hasCompositeObjects(variation))
		return false;
	if(hasSingleObjects(variation))
		return false;
	if(thickness <= 0)
		return false;
	return true;
}

int DoodadBrush::getCompositeChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance;
}

int DoodadBrush::getSingleChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance;
}

int DoodadBrush::getTotalChance(int ab) const
{
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance + ab_ptr->single_chance;
}

bool DoodadBrush::hasSingleObjects(int ab) const
{
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance > 0;
}

bool DoodadBrush::hasCompositeObjects(int ab) const
{
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance > 0;
}
