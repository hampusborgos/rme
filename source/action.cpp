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

#include "action.h"
#include "settings.h"
#include "map.h"
#include "editor.h"
#include "gui.h"

Change::Change() : type(CHANGE_NONE), data(nullptr)
{
	////
}

Change::Change(Tile* t) : type(CHANGE_TILE)
{
	ASSERT(t);
	data = t;
}

Change* Change::Create(House* house, const Position& where)
{
	Change* c = newd Change();
	c->type = CHANGE_MOVE_HOUSE_EXIT;
	std::pair<uint32_t, Position>* p = newd std::pair<uint32_t, Position>;
	p->first = house->id;
	p->second = where;
	c->data = p;
	return c;
}

Change* Change::Create(Waypoint* wp, const Position& where)
{
	Change* c = newd Change();
	c->type = CHANGE_MOVE_WAYPOINT;
	std::pair<std::string, Position>* p = newd std::pair<std::string, Position>;
	p->first = wp->name;
	p->second = where;
	c->data = p;
	return c;
}

Change::~Change()
{
	clear();
}

void Change::clear()
{
	switch(type) {
		case CHANGE_TILE:
			ASSERT(data);
			delete reinterpret_cast<Tile*>(data);
			break;
		case CHANGE_MOVE_HOUSE_EXIT:
			ASSERT(data);
			delete reinterpret_cast<std::pair<uint32_t, Position>* >(data);
			break;
		case CHANGE_MOVE_WAYPOINT:
			ASSERT(data);
			delete reinterpret_cast<std::pair<std::string, Position>* >(data);
			break;
		case CHANGE_NONE:
			break;
		default:
#ifdef __DEBUG_MODE__
			if(data)
				printf("UNHANDLED CHANGE TYPE! Leak!");
#endif
			break;
	}
	type = CHANGE_NONE;
	data = nullptr;
}

uint32_t Change::memsize() const
{
	uint32_t mem = sizeof(*this);
	switch(type) {
		case CHANGE_TILE:
			ASSERT(data);
			mem += reinterpret_cast<Tile*>(data)->memsize();
			break;
		default:
			break;
	}
	return mem;
}

Action::Action(Editor& editor, ActionIdentifier ident) :
	commited(false),
	editor(editor),
	type(ident)
{
}

Action::~Action()
{
	ChangeList::const_reverse_iterator it = changes.rbegin();
	while(it != changes.rend()) {
		delete *it;
		++it;
	}
}

size_t Action::approx_memsize() const
{
	uint32_t mem = sizeof(*this);
	mem += changes.size() * (sizeof(Change) + sizeof(Tile) + sizeof(Item) + 6/* approx overhead*/);
	return mem;
}

size_t Action::memsize() const
{
	uint32_t mem = sizeof(*this);
	mem += sizeof(Change*) * 3 * changes.size();
	ChangeList::const_iterator it = changes.begin();
	while(it != changes.end()) {
		Change* c = *it;
		switch(c->type) {
			case CHANGE_TILE:
			{
				ASSERT(c->data);
				mem += reinterpret_cast<Tile*>(c->data)->memsize();
				break;
			}

			default:
				break;
		}
		++it;
	}
	return mem;
}

void Action::commit(DirtyList* dirty_list)
{
	Selection& selection = editor.getSelection();
	selection.start(Selection::INTERNAL);
	ChangeList::const_iterator it = changes.begin();
	while(it != changes.end()) {
		Change* c = *it;
		switch(c->type) {
			case CHANGE_TILE: {
				void** data = &c->data;
				Tile* newtile = reinterpret_cast<Tile*>(*data);
				ASSERT(newtile);
				const Position& pos = newtile->getPosition();

				if(editor.IsLiveClient()) {
					QTreeNode* nd = editor.getMap().getLeaf(pos.x, pos.y);
					if(!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						++it;
						continue;
					}
				}

				Tile* oldtile = editor.getMap().swapTile(pos, newtile);
				TileLocation* location = newtile->getLocation();

				// Update other nodes in the network
				if(editor.IsLiveServer() && dirty_list)
					dirty_list->AddPosition(pos.x, pos.y, pos.z);


				newtile->update();

				//std::cout << "\tSwitched tile at " << pos.x << ";" << pos.y << ";" << pos.z << " from " << (void*)oldtile << " to " << *data <<  std::endl;
				if(newtile->isSelected())
					selection.addInternal(newtile);

				if(oldtile) {
					if(newtile->getHouseID() != oldtile->getHouseID()) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.getMap().houses.getHouse(oldtile->getHouseID());
						if(house)
							house->removeTile(oldtile);

						house = editor.getMap().houses.getHouse(newtile->getHouseID());
						if(house)
							house->addTile(newtile);
					}
					if(oldtile->spawn) {
						if(newtile->spawn) {
							if(*oldtile->spawn != *newtile->spawn) {
								editor.getMap().removeSpawn(oldtile);
								editor.getMap().addSpawn(newtile);
							}
						} else {
							// Spawn has been removed
							editor.getMap().removeSpawn(oldtile);
						}
					} else if(newtile->spawn) {
						editor.getMap().addSpawn(newtile);
					}

					//oldtile->update();
					if(oldtile->isSelected())
						selection.removeInternal(oldtile);

					*data = oldtile;
				} else {
					*data = editor.getMap().allocator(location);
					if(newtile->getHouseID() != 0) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.getMap().houses.getHouse(newtile->getHouseID());
						if(house) {
							house->addTile(newtile);
						}
					}

					if(newtile->spawn)
						editor.getMap().addSpawn(newtile);

				}
				// Mark the tile as modified
				newtile->modify();

				// Update client dirty list
				if(editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				std::pair<uint32_t, Position>* p = reinterpret_cast<std::pair<uint32_t, Position>* >(c->data);
				ASSERT(p);
				House* whathouse = editor.getMap().houses.getHouse(p->first);

				if(whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p->second);
					p->second = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				std::pair<std::string, Position>* p = reinterpret_cast<std::pair<std::string, Position>* >(c->data);
				ASSERT(p);
				Waypoint* wp = editor.getMap().waypoints.getWaypoint(p->first);

				if(wp) {
					// Change the tiles
					TileLocation* oldtile = editor.getMap().getTileL(wp->pos);
					TileLocation* newtile = editor.getMap().getTileL(p->second);

					// Only need to remove from old if it actually exists
					if(p->second.isValid() && oldtile && oldtile->getWaypointCount() > 0)
						oldtile->decreaseWaypointCount();

					newtile->increaseWaypointCount();

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p->second;
					p->second = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	selection.finish(Selection::INTERNAL);
	commited = true;
}

void Action::undo(DirtyList* dirty_list)
{
	if(changes.empty())
		return;

	Selection& selection = editor.getSelection();
	selection.start(Selection::INTERNAL);
	ChangeList::reverse_iterator it = changes.rbegin();

	while(it != changes.rend()) {
		Change* c = *it;
		switch(c->type) {
			case CHANGE_TILE: {
				void** data = &c->data;
				Tile* oldtile = reinterpret_cast<Tile*>(*data);
				ASSERT(oldtile);
				const Position& pos = oldtile->getPosition();

				if(editor.IsLiveClient()) {
					QTreeNode* nd = editor.getMap().getLeaf(pos.x, pos.y);
					if(!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						++it;
						continue;
					}
				}

				Tile* newtile = editor.getMap().swapTile(pos, oldtile);

				// Update server side change list (for broadcast)
				if(editor.IsLiveServer() && dirty_list)
					dirty_list->AddPosition(pos.x, pos.y, pos.z);


				if(oldtile->isSelected())
					selection.addInternal(oldtile);
				if(newtile->isSelected())
					selection.removeInternal(newtile);

				if(newtile->getHouseID() != oldtile->getHouseID()) {
					// oooooomggzzz we need to remove it from the appropriate house!
					House* house = editor.getMap().houses.getHouse(newtile->getHouseID());
					if(house) {
						house->removeTile(newtile);
					} else {
						// Set tile house to 0, house has been removed
						newtile->setHouse(nullptr);
					}

					house = editor.getMap().houses.getHouse(oldtile->getHouseID());
					if(house) {
						house->addTile(oldtile);
					}
				}

				if(oldtile->spawn) {
					if(newtile->spawn) {
						if(*oldtile->spawn != *newtile->spawn) {
							editor.getMap().removeSpawn(newtile);
							editor.getMap().addSpawn(oldtile);
						}
					} else {
						editor.getMap().addSpawn(oldtile);
					}
				} else if(newtile->spawn) {
					editor.getMap().removeSpawn(newtile);
				}
				*data = newtile;


				// Update client dirty list
				if(editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				std::pair<uint32_t, Position>* p = reinterpret_cast<std::pair<uint32_t, Position>* >(c->data);
				ASSERT(p);
				House* whathouse = editor.getMap().houses.getHouse(p->first);
				if(whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p->second);
					p->second = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				std::pair<std::string, Position>* p = reinterpret_cast<std::pair<std::string, Position>* >(c->data);
				ASSERT(p);
				Waypoint* wp = editor.getMap().waypoints.getWaypoint(p->first);

				if(wp) {
					// Change the tiles
					TileLocation* oldtile = editor.getMap().getTileL(wp->pos);
					TileLocation* newtile = editor.getMap().getTileL(p->second);

					// Only need to remove from old if it actually exists
					if(p->second.isValid() && oldtile && oldtile->getWaypointCount() > 0)
						oldtile->decreaseWaypointCount();

					newtile->increaseWaypointCount();

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p->second;
					p->second = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	selection.finish(Selection::INTERNAL);
	commited = false;
}

BatchAction::BatchAction(Editor& editor, ActionIdentifier ident) :
	editor(editor),
    timestamp(0),
    memory_size(0),
    type(ident)
{
    ////
}


BatchAction::~BatchAction()
{
	for(Action* action : batch) {
		delete action;
	}
	batch.clear();
}

size_t BatchAction::memsize(bool recalc) const
{
	// Expensive operation, only evaluate once (won't change anyways)
	if(!recalc && memory_size > 0) {
		return memory_size;
	}

	uint32_t mem = sizeof(*this);
	mem += sizeof(Action*) * 3 * batch.size();

	for(Action* action : batch) {
#ifdef __USE_EXACT_MEMSIZE__
		mem += action->memsize();
#else
		// Less exact but MUCH faster
		mem += action->approx_memsize();
#endif
	}

	const_cast<BatchAction*>(this)->memory_size = mem;
	return mem;
}

void BatchAction::addAction(Action* action)
{
	// If empty, do nothing.
	if(action->size() == 0) {
		delete action;
		return;
	}

	ASSERT(action->getType() == type);

	if(!editor.CanEdit()) {
		delete action;
		return;
	}

	// Add it!
	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::addAndCommitAction(Action* action)
{
	// If empty, do nothing.
	if(action->size() == 0) {
		delete action;
		return;
	}

	if(!editor.CanEdit()) {
		delete action;
		return;
	}

	// Add it!
	action->commit(nullptr);
	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::commit()
{
	for(Action* action : batch) {
		if(!action->isCommited()) {
			action->commit(nullptr);
		}
	}
}

void BatchAction::undo()
{
	for(Action* action : boost::adaptors::reverse(batch)) {
		action->undo(nullptr);
	}
}

void BatchAction::redo()
{
	for(Action* action : batch) {
		action->redo(nullptr);
	}
}

void BatchAction::merge(BatchAction* other)
{
	batch.insert(batch.end(), other->batch.begin(), other->batch.end());
	other->batch.clear();
}

ActionQueue::ActionQueue(Editor& editor) :
	current(0), memory_size(0), editor(editor)
{
	////
}

ActionQueue::~ActionQueue()
{
	for(auto it = actions.begin(); it != actions.end(); it = actions.erase(it)) {
		delete *it;
	}
}

Action* ActionQueue::createAction(ActionIdentifier ident) const
{
	return new Action(editor, ident);
}

Action* ActionQueue::createAction(BatchAction* batch) const
{
	return new Action(editor, batch->getType());
}

BatchAction* ActionQueue::createBatch(ActionIdentifier identifier) const
{
	return new BatchAction(editor, identifier);
}

void ActionQueue::resetTimer()
{
	if(!actions.empty())
		actions.back()->resetTimer();
}

void ActionQueue::addBatch(BatchAction* batch, int stacking_delay)
{
	ASSERT(batch);
	ASSERT(current <= actions.size());

	if(batch->size() == 0) {
		delete batch;
		return;
	}

	// Commit any uncommited actions...
	batch->commit();

	// Update title
	if(editor.getMap().doChange())
		g_gui.UpdateTitle();

	if(batch->type == ACTION_REMOTE) {
		delete batch;
		return;
	}

	while(current != actions.size()) {
		memory_size -= actions.back()->memsize();
		BatchAction* todelete = actions.back();
		actions.pop_back();
		delete todelete;
	}

	while(memory_size > size_t(1024 * 1024 * g_settings.getInteger(Config::UNDO_MEM_SIZE)) && !actions.empty()) {
		memory_size -= actions.front()->memsize();
		delete actions.front();
		actions.pop_front();
		current--;
	}

	if(actions.size() > size_t(g_settings.getInteger(Config::UNDO_SIZE)) && !actions.empty()) {
		memory_size -= actions.front()->memsize();
		BatchAction* todelete = actions.front();
		actions.pop_front();
		delete todelete;
		current--;
	}

	do {
		if(!actions.empty()) {
			BatchAction* lastAction = actions.back();
			if(lastAction->type == batch->type && g_settings.getInteger(Config::GROUP_ACTIONS) && time(nullptr) - stacking_delay < lastAction->timestamp) {
				lastAction->merge(batch);
				lastAction->timestamp = time(nullptr);
				memory_size -= lastAction->memsize();
				memory_size += lastAction->memsize(true);
				delete batch;
				break;
			}
		}
		memory_size += batch->memsize();
		actions.push_back(batch);
		batch->timestamp = time(nullptr);
		current++;
	} while(false);
}

void ActionQueue::addAction(Action* action, int stacking_delay)
{
	BatchAction* batch = createBatch(action->getType());
	batch->addAndCommitAction(action);
	if(batch->empty()) {
		delete batch;
		return;
	}

	addBatch(batch, stacking_delay);
}

const BatchAction* ActionQueue::getAction(size_t index) const
{
	if(index >= 0 && index < actions.size()) {
		return actions.at(index);
	}
	return nullptr;
}

void ActionQueue::generateLabels()
{
	for(BatchAction* batch : actions) {
		if(batch && batch->label.IsEmpty()) {
			batch->label = createLabel(batch->getType());
		}
	}
}

bool ActionQueue::undo()
{
	if(current > 0) {
		current--;
		BatchAction* batch = actions.at(current);
		if(batch) {
			batch->undo();
		}
		return true;
	}
	return false;
}

bool ActionQueue::redo()
{
	if(current < actions.size()) {
		BatchAction* batch = actions.at(current);
		if(batch) {
			batch->redo();
		}
		current++;
		return true;
	}
	return false;
}

void ActionQueue::clear()
{
	for(BatchAction* batch : actions) {
		delete batch;
	}
	actions.clear();
	current = 0;
}

wxString ActionQueue::createLabel(ActionIdentifier type)
{
	switch (type) {
		case ACTION_MOVE: return "Move";
		case ACTION_SELECT: return "Select";
		case ACTION_DELETE_TILES: return "Delete";
		case ACTION_CUT_TILES: return "Cut";
		case ACTION_PASTE_TILES: return "Paste";
		case ACTION_RANDOMIZE: return "Randomize";
		case ACTION_BORDERIZE: return "Borderize";
		case ACTION_DRAW: return "Draw";
		case ACTION_ERASE: return "Erase";
		case ACTION_SWITCHDOOR: return "Switch Door";
		case ACTION_ROTATE_ITEM: return "Rotate Item";
		case ACTION_REPLACE_ITEMS: return "Replace";
		case ACTION_CHANGE_PROPERTIES: return "Change Properties";
		default: return "Deselect";
	}
}

DirtyList::DirtyList() :
	owner(0)
{
	;
}

DirtyList::~DirtyList()
{
	;
}

void DirtyList::AddPosition(int x, int y, int z)
{
	uint32_t m = ((x >> 2) << 18) | ((y >> 2) << 4);
	ValueType fi = {m, 0};
	SetType::iterator s = iset.find(fi);
	if(s != iset.end()) {
		ValueType v = *s;
		iset.erase(s);
		v.floors = (1 << z) | v.floors;
		iset.insert(v);
	} else {
		ValueType v = {m, (uint32_t)(1 << z)};
		iset.insert(v);
	}
}

void DirtyList::AddChange(Change* c)
{
	ichanges.push_back(c);
}

DirtyList::SetType& DirtyList::GetPosList()
{
	return iset;
}

ChangeList& DirtyList::GetChanges()
{
	return ichanges;
}
