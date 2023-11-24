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

#include "action.h"

#include "editor.h"
#include "gui.h"
#include "main.h"
#include "map.h"
#include "settings.h"

Change::Change() : type(CHANGE_NONE), data(nullptr)
{
	////
}

Change::Change(Tile* tile) : type(CHANGE_TILE)
{
	ASSERT(tile);
	data = tile;
}

Change* Change::Create(House* house, const Position& position)
{
	Change* change = new Change();
	change->type = CHANGE_MOVE_HOUSE_EXIT;
	change->data = new HouseData{house->id, position};
	return change;
}

Change* Change::Create(Waypoint* waypoint, const Position& position)
{
	Change* change = new Change();
	change->type = CHANGE_MOVE_WAYPOINT;
	change->data = new WaypointData{waypoint->name, position};
	return change;
}

Change::~Change() { clear(); }

void Change::clear()
{
	switch (type) {
		case CHANGE_TILE:
			ASSERT(data);
			delete reinterpret_cast<Tile*>(data);
			break;
		case CHANGE_MOVE_HOUSE_EXIT:
			ASSERT(data);
			delete reinterpret_cast<HouseData*>(data);
			break;
		case CHANGE_MOVE_WAYPOINT:
			ASSERT(data);
			delete reinterpret_cast<WaypointData*>(data);
			break;
		case CHANGE_NONE:
			break;
		default:
#ifdef __DEBUG_MODE__
			if (data) printf("UNHANDLED CHANGE TYPE! Leak!");
#endif
			break;
	}

	type = CHANGE_NONE;
	data = nullptr;
}

uint32_t Change::memsize() const
{
	uint32_t mem = sizeof(*this);
	if (type == CHANGE_TILE) {
		mem += reinterpret_cast<Tile*>(data)->memsize();
	}
	return mem;
}

Action::Action(Editor& editor, ActionIdentifier ident) : commited(false), editor(editor), type(ident) {}

Action::~Action()
{
	for (Change* change : changes) {
		delete change;
	}
	changes.clear();
}

size_t Action::approx_memsize() const
{
	uint32_t mem = sizeof(*this);
	mem += changes.size() * (sizeof(Change) + sizeof(Tile) + sizeof(Item) + 6 /* approx overhead*/);
	return mem;
}

size_t Action::memsize() const
{
	uint32_t mem = sizeof(*this);
	mem += sizeof(Change*) * 3 * changes.size();

	for (const Change* change : changes) {
		if (change && change->getType() == CHANGE_TILE) {
			mem += reinterpret_cast<Tile*>(change->getData())->memsize();
		}
	}

	return mem;
}

void Action::commit(DirtyList* dirty_list)
{
	Map& map = editor.getMap();
	Selection& selection = editor.getSelection();
	selection.start(Selection::INTERNAL);

	for (Change* change : changes) {
		switch (change->getType()) {
			case CHANGE_TILE: {
				void** data = &change->data;
				Tile* new_tile = reinterpret_cast<Tile*>(*data);
				ASSERT(new_tile);

				const Position& pos = new_tile->getPosition();

				if (editor.IsLiveClient()) {
					QTreeNode* node = map.getLeaf(pos.x, pos.y);
					if (!node || !node->isVisible(pos.z > rme::MapGroundLayer)) {
						change->clear();
						continue;
					}
				}

				Tile* old_tile = map.swapTile(pos, new_tile);
				TileLocation* location = new_tile->getLocation();

				// Update other nodes in the network
				if (editor.IsLiveServer() && dirty_list) dirty_list->AddPosition(pos.x, pos.y, pos.z);

				new_tile->update();

				// std::cout << "\tSwitched tile at " << pos.x << ";" << pos.y << ";" << pos.z << " from " <<
				// (void*)oldtile << " to " << *data <<  std::endl;
				if (new_tile->isSelected()) selection.addInternal(new_tile);

				if (old_tile) {
					if (new_tile->getHouseID() != old_tile->getHouseID()) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = map.houses.getHouse(old_tile->getHouseID());
						if (house) house->removeTile(old_tile);

						house = map.houses.getHouse(new_tile->getHouseID());
						if (house) house->addTile(new_tile);
					}
					if (old_tile->spawn) {
						if (new_tile->spawn) {
							if (*old_tile->spawn != *new_tile->spawn) {
								map.removeSpawn(old_tile);
								map.addSpawn(new_tile);
							}
						} else {
							map.removeSpawn(old_tile);
						}
					} else if (new_tile->spawn) {
						map.addSpawn(new_tile);
					}

					// oldtile->update();
					if (old_tile->isSelected()) selection.removeInternal(old_tile);

					*data = old_tile;
				} else {
					*data = map.allocator(location);
					if (new_tile->getHouseID() != 0) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = map.houses.getHouse(new_tile->getHouseID());
						if (house) {
							house->addTile(new_tile);
						}
					}

					if (new_tile->spawn) map.addSpawn(new_tile);
				}
				new_tile->modify();

				// Update client dirty list
				if (editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					dirty_list->AddChange(change);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				HouseData* data = reinterpret_cast<HouseData*>(change->data);
				ASSERT(data);

				House* house = map.houses.getHouse(data->id);
				if (house) {
					const Position& old_pos = house->getExit();
					house->setExit(data->position);
					data->position = old_pos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				WaypointData* data = reinterpret_cast<WaypointData*>(change->data);
				ASSERT(data);

				Waypoint* waypoint = map.waypoints.getWaypoint(data->id);
				if (waypoint) {
					TileLocation* old_tile = map.getTileL(waypoint->pos);
					TileLocation* new_tile = map.getTileL(data->position);

					if (data->position.isValid() && old_tile && old_tile->getWaypointCount() > 0)
						old_tile->decreaseWaypointCount();

					new_tile->increaseWaypointCount();

					Position old_pos = waypoint->pos;
					waypoint->pos = data->position;
					data->position = old_pos;
				}
				break;
			}

			default:
				break;
		}
	}
	selection.finish(Selection::INTERNAL);
	commited = true;
}

void Action::undo(DirtyList* dirty_list)
{
	if (changes.empty()) return;

	Map& map = editor.getMap();
	Selection& selection = editor.getSelection();
	selection.start(Selection::INTERNAL);

	for (Change* change : changes) {
		switch (change->getType()) {
			case CHANGE_TILE: {
				void** data = &change->data;
				Tile* old_tile = reinterpret_cast<Tile*>(*data);
				ASSERT(old_tile);
				const Position& pos = old_tile->getPosition();

				if (editor.IsLiveClient()) {
					QTreeNode* node = map.getLeaf(pos.x, pos.y);
					if (!node || !node->isVisible(pos.z > rme::MapGroundLayer)) {
						// Delete all changes that affect tiles outside our view
						change->clear();
						continue;
					}
				}

				Tile* new_tile = map.swapTile(pos, old_tile);

				// Update server side change list (for broadcast)
				if (editor.IsLiveServer() && dirty_list) dirty_list->AddPosition(pos.x, pos.y, pos.z);

				if (old_tile->isSelected()) selection.addInternal(old_tile);
				if (new_tile->isSelected()) selection.removeInternal(new_tile);

				if (new_tile->getHouseID() != old_tile->getHouseID()) {
					// oooooomggzzz we need to remove it from the appropriate house!
					House* house = map.houses.getHouse(new_tile->getHouseID());
					if (house) {
						house->removeTile(new_tile);
					} else {
						new_tile->setHouse(nullptr);
					}

					house = map.houses.getHouse(old_tile->getHouseID());
					if (house) {
						house->addTile(old_tile);
					}
				}

				if (old_tile->spawn) {
					if (new_tile->spawn) {
						if (*old_tile->spawn != *new_tile->spawn) {
							map.removeSpawn(new_tile);
							map.addSpawn(old_tile);
						}
					} else {
						map.addSpawn(old_tile);
					}
				} else if (new_tile->spawn) {
					map.removeSpawn(new_tile);
				}
				*data = new_tile;

				// Update client dirty list
				if (editor.IsLiveClient() && dirty_list && type != ACTION_REMOTE) {
					dirty_list->AddChange(change);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				HouseData* data = reinterpret_cast<HouseData*>(change->data);
				ASSERT(data);

				House* house = map.houses.getHouse(data->id);
				if (house) {
					const Position& oldpos = house->getExit();
					house->setExit(data->position);
					data->position = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				WaypointData* data = reinterpret_cast<WaypointData*>(change->data);
				ASSERT(data);

				Waypoint* waypoint = map.waypoints.getWaypoint(data->id);
				if (waypoint) {
					TileLocation* old_tile = map.getTileL(waypoint->pos);
					TileLocation* new_tile = map.getTileL(data->position);

					if (data->position.isValid() && old_tile && old_tile->getWaypointCount() > 0)
						old_tile->decreaseWaypointCount();

					new_tile->increaseWaypointCount();

					Position old_pos = waypoint->pos;
					waypoint->pos = data->position;
					data->position = old_pos;
				}
				break;
			}

			default:
				break;
		}
	}

	selection.finish(Selection::INTERNAL);
	commited = false;
}

BatchAction::BatchAction(Editor& editor, ActionIdentifier ident) :
    editor(editor), timestamp(0), memory_size(0), type(ident)
{
	////
}

BatchAction::~BatchAction()
{
	for (Action* action : batch) {
		delete action;
	}
	batch.clear();
}

size_t BatchAction::memsize(bool recalc) const
{
	// Expensive operation, only evaluate once (won't change anyways)
	if (!recalc && memory_size > 0) {
		return memory_size;
	}

	uint32_t mem = sizeof(*this);
	mem += sizeof(Action*) * 3 * batch.size();

	for (const Action* action : batch) {
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

bool BatchAction::isNoSelection() const noexcept { return type != ACTION_SELECT && type != ACTION_UNSELECT; }

void BatchAction::addAction(Action* action)
{
	if (!action) {
		return;
	}

	if (action->empty() || !editor.CanEdit()) {
		delete action;
		return;
	}

	ASSERT(action->getType() == type);

	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::addAndCommitAction(Action* action)
{
	if (!action) {
		return;
	}

	if (!editor.CanEdit() || action->empty()) {
		delete action;
		return;
	}

	action->commit(nullptr);
	batch.push_back(action);
	timestamp = time(nullptr);
}

void BatchAction::commit()
{
	for (Action* action : batch) {
		if (action && !action->isCommited()) {
			action->commit(nullptr);
		}
	}
}

void BatchAction::undo()
{
	for (Action* action : std::views::reverse(batch)) {
		action->undo(nullptr);
	}
}

void BatchAction::redo()
{
	for (Action* action : batch) {
		action->redo(nullptr);
	}
}

void BatchAction::merge(BatchAction* other)
{
	batch.insert(batch.end(), other->batch.begin(), other->batch.end());
	other->batch.clear();
}

ActionQueue::ActionQueue(Editor& editor) : current(0), memory_size(0), editor(editor)
{
	////
}

ActionQueue::~ActionQueue()
{
	for (BatchAction* batch : actions) {
		delete batch;
	}
	actions.clear();
}

Action* ActionQueue::createAction(ActionIdentifier identifier) const { return new Action(editor, identifier); }

Action* ActionQueue::createAction(BatchAction* batch) const { return new Action(editor, batch->getType()); }

BatchAction* ActionQueue::createBatch(ActionIdentifier identifier) const { return new BatchAction(editor, identifier); }

void ActionQueue::resetTimer()
{
	if (!actions.empty()) actions.back()->resetTimer();
}

void ActionQueue::addBatch(BatchAction* batch, int stacking_delay)
{
	ASSERT(batch);
	ASSERT(current <= actions.size());

	if (batch->empty()) {
		delete batch;
		return;
	}

	// Commit any uncommited actions...
	batch->commit();

	// Update title
	if (batch->isNoSelection() && editor.getMap().doChange()) {
		g_gui.UpdateTitle();
	}

	if (batch->getType() == ACTION_REMOTE) {
		delete batch;
		return;
	}

	while (current != actions.size()) {
		memory_size -= actions.back()->memsize();
		BatchAction* todelete = actions.back();
		actions.pop_back();
		delete todelete;
	}

	while (memory_size > size_t(1024 * 1024 * g_settings.getInteger(Config::UNDO_MEM_SIZE)) && !actions.empty()) {
		memory_size -= actions.front()->memsize();
		delete actions.front();
		actions.pop_front();
		current--;
	}

	if (actions.size() > size_t(g_settings.getInteger(Config::UNDO_SIZE)) && !actions.empty()) {
		memory_size -= actions.front()->memsize();
		BatchAction* todelete = actions.front();
		actions.pop_front();
		delete todelete;
		current--;
	}

	do {
		if (!actions.empty()) {
			BatchAction* lastAction = actions.back();
			if (lastAction->type == batch->type && g_settings.getInteger(Config::GROUP_ACTIONS) &&
			    time(nullptr) - stacking_delay < lastAction->timestamp) {
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
	} while (false);
}

void ActionQueue::addAction(Action* action, int stacking_delay)
{
	BatchAction* batch = createBatch(action->getType());
	batch->addAndCommitAction(action);
	if (batch->empty()) {
		delete batch;
		return;
	}

	addBatch(batch, stacking_delay);
}

const BatchAction* ActionQueue::getAction(size_t index) const
{
	if (index >= 0 && index < actions.size()) {
		return actions.at(index);
	}
	return nullptr;
}

void ActionQueue::generateLabels()
{
	for (BatchAction* batch : actions) {
		if (batch && batch->label.IsEmpty()) {
			batch->label = createLabel(batch->getType());
		}
	}
}

bool ActionQueue::undo()
{
	if (current > 0) {
		current--;
		BatchAction* batch = actions.at(current);
		if (batch) {
			batch->undo();
		}

		// Update title
		if (batch->isNoSelection() && editor.getMap().doChange()) {
			g_gui.UpdateTitle();
		}
		return true;
	}
	return false;
}

bool ActionQueue::redo()
{
	if (current < actions.size()) {
		BatchAction* batch = actions.at(current);
		if (batch) {
			batch->redo();
		}
		current++;

		// Update title
		if (batch->isNoSelection() && editor.getMap().doChange()) {
			g_gui.UpdateTitle();
		}
		return true;
	}
	return false;
}

bool ActionQueue::hasChanges() const
{
	for (const BatchAction* batch : actions) {
		if (batch && !batch->empty() && batch->isNoSelection()) {
			return true;
		}
	}
	return false;
}

void ActionQueue::clear()
{
	for (BatchAction* batch : actions) {
		delete batch;
	}
	actions.clear();
	current = 0;
}

wxString ActionQueue::createLabel(ActionIdentifier type)
{
	switch (type) {
		case ACTION_MOVE:
			return "Move";
		case ACTION_SELECT:
			return "Select";
		case ACTION_UNSELECT:
			return "Unselect";
		case ACTION_DELETE_TILES:
			return "Delete";
		case ACTION_CUT_TILES:
			return "Cut";
		case ACTION_PASTE_TILES:
			return "Paste";
		case ACTION_RANDOMIZE:
			return "Randomize";
		case ACTION_BORDERIZE:
			return "Borderize";
		case ACTION_DRAW:
			return "Draw";
		case ACTION_ERASE:
			return "Erase";
		case ACTION_SWITCHDOOR:
			return "Switch Door";
		case ACTION_ROTATE_ITEM:
			return "Rotate Item";
		case ACTION_REPLACE_ITEMS:
			return "Replace";
		case ACTION_CHANGE_PROPERTIES:
			return "Change Properties";
		default:
			return wxEmptyString;
	}
}

void DirtyList::AddPosition(int x, int y, int z)
{
	uint32_t m = ((x >> 2) << 18) | ((y >> 2) << 4);
	ValueType fi = {m, 0};
	SetType::iterator s = iset.find(fi);
	if (s != iset.end()) {
		ValueType v = *s;
		iset.erase(s);
		v.floors = (1 << z) | v.floors;
		iset.insert(v);
	} else {
		ValueType v = {m, (uint32_t)(1 << z)};
		iset.insert(v);
	}
}

void DirtyList::AddChange(Change* c) { ichanges.push_back(c); }

DirtyList::SetType& DirtyList::GetPosList() { return iset; }

ChangeList& DirtyList::GetChanges() { return ichanges; }
