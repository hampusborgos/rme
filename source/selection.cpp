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

#include "selection.h"
#include "tile.h"
#include "creature.h"
#include "item.h"
#include "editor.h"
#include "gui.h"

Selection::Selection(Editor& editor) :
	busy(false),
	editor(editor),
	session(nullptr),
	subsession(nullptr)
{
	////
}

Selection::~Selection()
{
	delete subsession;
	delete session;
}

Position Selection::minPosition() const
{
	Position minPos(0x10000, 0x10000, 0x10);
	for(TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if(minPos.x > pos.x)
			minPos.x = pos.x;
		if(minPos.y > pos.y)
			minPos.y = pos.y;
		if(minPos.z > pos.z)
			minPos.z = pos.z;
	}
	return minPos;
}

Position Selection::maxPosition() const
{
	Position maxPos(0, 0, 0);
	for(TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if(maxPos.x < pos.x)
			maxPos.x = pos.x;
		if(maxPos.y < pos.y)
			maxPos.y = pos.y;
		if(maxPos.z < pos.z)
			maxPos.z = pos.z;
	}
	return maxPos;
}

void Selection::add(Tile* tile, Item* item)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	if(item->isSelected()) return;

	// Make a copy of the tile with the item selected
	item->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	item->deselect();

	if(g_settings.getInteger(Config::BORDER_IS_GROUND))
		if(item->isBorder())
			new_tile->selectGround();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Spawn* spawn)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	if(spawn->isSelected()) return;

	// Make a copy of the tile with the item selected
	spawn->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	spawn->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Creature* creature)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	if(creature->isSelected()) return;

	// Make a copy of the tile with the item selected
	creature->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	creature->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile)
{
	ASSERT(subsession);
	ASSERT(tile);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Item* item)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	bool tmp = item->isSelected();
	item->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if(tmp) item->select();
	if(item->isBorder() && g_settings.getInteger(Config::BORDER_IS_GROUND)) new_tile->deselectGround();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Spawn* spawn)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	bool tmp = spawn->isSelected();
	spawn->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if(tmp) spawn->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Creature* creature)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	bool tmp = creature->isSelected();
	creature->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if(tmp) creature->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile)
{
	ASSERT(subsession);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::addInternal(Tile* tile)
{
	ASSERT(tile);

	tiles.insert(tile);
}

void Selection::removeInternal(Tile* tile)
{
	ASSERT(tile);
	tiles.erase(tile);
}

void Selection::clear()
{
	if(session) {
		for(TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			Tile* new_tile = (*it)->deepCopy(editor.map);
			new_tile->deselect();
			subsession->addChange(newd Change(new_tile));
		}
	} else {
		for(TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			(*it)->deselect();
		}
		tiles.clear();
	}
}

void Selection::start(SessionFlags flags)
{
	if(!(flags & INTERNAL)) {
		if(flags & SUBTHREAD) {
			;
		} else {
			session = editor.actionQueue->createBatch(ACTION_SELECT);
		}
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
	}
	busy = true;
}

void Selection::commit()
{
	if(session) {
		ASSERT(subsession);
		// We need to step out of the session before we do the action, else peril awaits us!
		BatchAction* tmp = session;
		session = nullptr;

		// Do the action
		tmp->addAndCommitAction(subsession);

		// Create a newd action for subsequent selects
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
		session = tmp;
	}
}

void Selection::finish(SessionFlags flags)
{
	if(!(flags & INTERNAL)) {
		if(flags & SUBTHREAD) {
			ASSERT(subsession);
			subsession = nullptr;
		} else {
			ASSERT(session);
			ASSERT(subsession);
			// We need to exit the session before we do the action, else peril awaits us!
			BatchAction* tmp = session;
			session = nullptr;

			tmp->addAndCommitAction(subsession);
			editor.addBatch(tmp, 2);

			session = nullptr;
			subsession = nullptr;
		}
	}
	busy = false;
}

void Selection::updateSelectionCount()
{
	if(size() > 0) {
		wxString ss;
		if(size() == 1) {
			ss << "One tile selected.";
		} else {
			ss << size() << " tiles selected.";
		}
		g_gui.SetStatusText(ss);
	}
}

void Selection::join(SelectionThread* thread)
{
	thread->Wait();

	ASSERT(session);
	session->addAction(thread->result);
	thread->selection.subsession = nullptr;

	delete thread;
}

SelectionThread::SelectionThread(Editor& editor, Position start, Position end) :
	wxThread(wxTHREAD_JOINABLE),
	editor(editor),
	start(start),
	end(end),
	selection(editor),
	result(nullptr)
{
	////
}

SelectionThread::~SelectionThread()
{
	////
}

void SelectionThread::Execute()
{
	Create();
	Run();
}

wxThread::ExitCode SelectionThread::Entry()
{
	selection.start(Selection::SUBTHREAD);
	for(int z = start.z; z >= end.z; --z) {
		for(int x = start.x; x <= end.x; ++x) {
			for(int y = start.y; y <= end.y; ++y) {
				Tile* tile = editor.map.getTile(x, y, z);
				if(!tile)
					continue;

				selection.add(tile);
			}
		}
		if(z <= GROUND_LAYER && g_settings.getInteger(Config::COMPENSATED_SELECT)) {
			++start.x; ++start.y;
			++end.x; ++end.y;
		}
	}
	result = selection.subsession;
	selection.finish(Selection::SUBTHREAD);

	return nullptr;
}
