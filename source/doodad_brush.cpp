#include "main.h"

#include "doodad_brush.h"
#include "basemap.h"

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
	// ...
}

DoodadBrush::~DoodadBrush() {
	for(std::vector<AlternativeBlock*>::iterator alt_iter = alternatives.begin();
			alt_iter != alternatives.end();
			++alt_iter)
	{
		delete *alt_iter;
	}
}

DoodadBrush::AlternativeBlock::AlternativeBlock() :
	composite_chance(0),
	single_chance(0)
{
	// ...
}

DoodadBrush::AlternativeBlock::~AlternativeBlock() {
	for(std::vector<CompositeBlock>::iterator composite_iter = composite_items.begin();
			composite_iter != composite_items.end();
			++composite_iter)
	{
		CompositeTileList& tv = composite_iter->items;

		for(CompositeTileList::iterator compt_iter = tv.begin();
				compt_iter != tv.end();
				++compt_iter)
		{
			ItemVector& items = compt_iter->second;
			for(ItemVector::iterator iiter = items.begin(); iiter != items.end(); ++iiter)
				delete *iiter;
		}
	}
	for(std::vector<SingleBlock>::iterator single_iter = single_items.begin();
			single_iter != single_items.end();
			++single_iter)
	{
		delete single_iter->item;
	}
}

bool DoodadBrush::loadAlternative(xmlNodePtr node, wxArrayString& warnings, AlternativeBlock* which) {
	xmlNodePtr child = node->children;
	AlternativeBlock* ab_ptr = NULL;

#define ab (which? which : (ab_ptr? ab_ptr : ab_ptr = newd AlternativeBlock()))
	while(child) {
		if(xmlStrcmp(child->name,(const xmlChar*)"item") == 0) {
			int chance;

			if(!readXMLValue(child, "chance", chance)) {
				wxString warning;
				warning = wxT("Can't read chance tag of doodad item node.");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}

			Item* item = Item::Create(child);
			if(item == NULL) {
				wxString warning;
				warning = wxT("Can't create item from doodad item node.");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}

			ItemType& it = item_db[item->getID()];
			if(it.id != 0) {
				it.doodad_brush = this;
			}

			SingleBlock sb;
			sb.chance = chance;
			sb.item = item;
			ab->single_items.push_back(sb);
			ab->single_chance += chance;
		} else if(xmlStrcmp(child->name,(const xmlChar*)"composite") == 0) {
			CompositeBlock cb;

			if(!readXMLValue(child, "chance", cb.chance)) {
				wxString warning;
				warning = wxT("Can't read chance tag of doodad composite node.");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}
			ab->composite_chance += cb.chance;
			cb.chance = ab->composite_chance;

			xmlNodePtr composite_child = child->children;
			while(composite_child) {
				if(xmlStrcmp(composite_child->name, (const xmlChar*)"tile") == 0) {
					int x = 0, y = 0, z = 0;

					if(!readXMLValue(composite_child, "x", x)){
						wxString warning;
						warning = wxT("Couldn't read positionX values of composite tile node.");
						warnings.push_back(warning);
						composite_child = composite_child->next;
						continue;
					}
					if(!readXMLValue(composite_child, "y", y)){
						wxString warning;
						warning = wxT("Couldn't read positionY values of composite tile node.");
						warnings.push_back(warning);
						composite_child = composite_child->next;
						continue;
					}
					readXMLValue(composite_child, "z", z); // Don't halt on error

					if(x <= -0x8000 || x >= +0x8000) {
						wxString warning;
						warning = wxT("Invalid range of x value on composite tile node.");
						warnings.push_back(warning);
						composite_child = composite_child->next;
						continue;
					}
					if(y <= -0x8000 || y >= +0x8000) {
						wxString warning;
						warning = wxT("Invalid range of y value on composite tile node.");
						warnings.push_back(warning);
						composite_child = composite_child->next;
						continue;
					}
					if(z <= -0x8 || z >= +0x8) {
						wxString warning;
						warning = wxT("Invalid range of z value on composite tile node.");
						warnings.push_back(warning);
						composite_child = composite_child->next;
						continue;
					}

					ItemVector items;
					xmlNodePtr itemNode = composite_child->children;
					while(itemNode) {
						if(xmlStrcmp(itemNode->name, (const xmlChar*)"item") == 0) {
							Item* item= Item::Create(itemNode);
							if(item) {
								items.push_back(item);

								ItemType& it = item_db[item->getID()];
								if(it.id != 0) {
									it.doodad_brush = this;
								}
							}
						}
						itemNode = itemNode->next;
					}
					if(items.size() > 0)
						cb.items.push_back(std::make_pair(Position(x, y, z), items));

				}
				composite_child = composite_child->next;
			}
			ab->composite_items.push_back(cb);
		}
		child = child->next;
	}
#undef ab

	if(ab_ptr) {
		alternatives.push_back(ab_ptr);
	}
	return true;
}

bool DoodadBrush::load(xmlNodePtr node, wxArrayString& warnings) {
	int intVal;
	std::string strVal;

	if(readXMLValue(node, "lookid", intVal)) {
		look_id = intVal;
	}
	if(readXMLValue(node, "server_lookid", intVal)) {
		look_id = item_db[intVal].clientID;
	}

	if(readXMLValue(node, "on_blocking", strVal)) {
		on_blocking = isTrueString(strVal);
	}

	if(readXMLValue(node, "on_duplicate", strVal)) {
		on_duplicate = isTrueString(strVal);
	}

	if(readXMLValue(node, "redo_borders", strVal) || readXMLValue(node, "reborder", strVal)) {
		do_new_borders = isTrueString(strVal);
	}

	if(readXMLValue(node, "one_size", strVal)) {
		one_size = isTrueString(strVal);
	}

	if(readXMLValue(node, "draggable", strVal)) {
		draggable = isTrueString(strVal);
	}

	if(readXMLValue(node, "remove_optional_border", strVal)) {
		if(do_new_borders == false) {
			wxString warning;
			warning << wxT("remove_optional_border will not work without redo_borders\n");
			warnings.push_back(warning);
		}
		clear_statflags |= TILESTATE_OP_BORDER;
	}

	if(readXMLValue(node, "thickness", strVal)) {
		size_t slash = strVal.find("/");
		if(slash != std::string::npos) {
			thickness = s2i(strVal.substr(0, max(0ul, slash)));
			thickness_ceiling = max(thickness, s2i(strVal.substr(slash+1)));
			//std::cout << "str:\"" << strVal << "\" -> " << thickness << " of " << thickness_ceiling << std::endl;
		}
	}

	xmlNodePtr child = node->children;
	while(child) {
		if(xmlStrcmp(child->name,(const xmlChar*)"alternate") == 0) {
			if(loadAlternative(child, warnings) == false) {
				return false;
			}
		}
		child = child->next;
	}
	loadAlternative(node, warnings, alternatives.size()? alternatives.back() : NULL);

	return true;
}

bool DoodadBrush::AlternativeBlock::ownsItem(uint16_t id) const {
	for(std::vector<SingleBlock>::const_iterator single_iter = single_items.begin();
			single_iter != single_items.end();
			++single_iter)
	{
		if(single_iter->item->getID() == id) {
			return true;
		}
	}

	for(std::vector<CompositeBlock>::const_iterator composite_iter = composite_items.begin();
			composite_iter != composite_items.end();
			++composite_iter)
	{
		const CompositeTileList& ctl = composite_iter->items;
		for(CompositeTileList::const_iterator comp_iter = ctl.begin();
				comp_iter != ctl.end();
				++comp_iter)
		{
			const ItemVector& items = comp_iter->second;

			for(ItemVector::const_iterator item_iter = items.begin(),
				item_end = items.end(); item_iter != item_end;
				++item_iter)
			{
				if((*item_iter)->getID() == id) {
					return true;
				}
			}
		}
	}
	return false;
}

bool DoodadBrush::ownsItem(Item* item) const {
	if(item->getDoodadBrush() == this) return true;
	uint16_t id = item->getID();

	for(std::vector<AlternativeBlock*>::const_iterator alt_iter = alternatives.begin();
			alt_iter != alternatives.end();
			++alt_iter)
	{
		if((*alt_iter)->ownsItem(id)) {
			return true;
		}
	}

	return false;
}

void DoodadBrush::undraw(BaseMap* map, Tile* tile) {
	// Remove all doodad-related
	for(ItemVector::iterator item_iter = tile->items.begin();
			item_iter != tile->items.end();)
	{
		Item* item = *item_iter;
		if(item->getDoodadBrush() != NULL) {
			if(item->isComplex() && settings.getInteger(Config::ERASER_LEAVE_UNIQUE)) {
				++item_iter;
			} else if (settings.getInteger(Config::DOODAD_BRUSH_ERASE_LIKE)) {
				// Only delete items of the same doodad brush
				if (ownsItem(item)) {
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
	if(tile->ground && tile->ground->getDoodadBrush() != NULL)
	{
		if (settings.getInteger(Config::DOODAD_BRUSH_ERASE_LIKE))
		{
			// Only delete items of the same doodad brush
			if (ownsItem(tile->ground)) {
				delete tile->ground;
				tile->ground = NULL;
			}
		}
		else
		{
			delete tile->ground;
			tile->ground = NULL;
		}
	}
}

void DoodadBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	int variation = 0;
	if(parameter) {
		variation = *reinterpret_cast<int*>(parameter);
	}

	if(alternatives.empty()) return;

	variation %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[variation];
	ASSERT(ab_ptr);

	int roll = random(1, ab_ptr->single_chance);
	for(std::vector<SingleBlock>::const_iterator block_iter = ab_ptr->single_items.begin();
			block_iter != ab_ptr->single_items.end();
			++block_iter)
	{
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

const CompositeTileList& DoodadBrush::getComposite(int variation) const {
	static CompositeTileList empty;

	if(alternatives.empty())
		return empty;

	variation %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[variation];
	ASSERT(ab_ptr);

	int roll = random(1, ab_ptr->composite_chance);
	for(std::vector<CompositeBlock>::const_iterator block_iter = ab_ptr->composite_items.begin();
			block_iter != ab_ptr->composite_items.end();
			++block_iter)
	{
		const CompositeBlock& cb = *block_iter;
		if(roll <= cb.chance) {
			return cb.items;
		}
	}
	return empty;
}

bool DoodadBrush::isEmpty(int variation) const {
	if(hasCompositeObjects(variation))
		return false;
	if(hasSingleObjects(variation))
		return false;
	if(thickness <= 0)
		return false;
	return true;
}

int DoodadBrush::getCompositeChance(int ab) const {
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance;
}

int DoodadBrush::getSingleChance(int ab) const {
	if(alternatives.empty()) return 0;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance;
}

int DoodadBrush::getTotalChance(int ab) const {
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	const AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance + ab_ptr->single_chance;
}

bool DoodadBrush::hasSingleObjects(int ab) const {
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->single_chance > 0;
}

bool DoodadBrush::hasCompositeObjects(int ab) const {
	if(alternatives.empty()) return false;
	ab %= alternatives.size();
	AlternativeBlock* ab_ptr = alternatives[ab];
	ASSERT(ab_ptr);
	return ab_ptr->composite_chance > 0;
}
