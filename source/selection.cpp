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
	editor(editor),
	session(nullptr),
	subsession(nullptr),
	busy(false)
{
	////
}

Selection::~Selection()
{
	tiles.clear();

	delete subsession;
	delete session;
}

Position Selection::minPosition() const
{
	Position min_pos(0x10000, 0x10000, 0x10);
	for(const Tile* tile : tiles) {
		if(!tile) continue;
		const Position& tile_pos = tile->getPosition();
		if(min_pos.x > tile_pos.x)
			min_pos.x = tile_pos.x;
		if(min_pos.y > tile_pos.y)
			min_pos.y = tile_pos.y;
		if(min_pos.z > tile_pos.z)
			min_pos.z = tile_pos.z;
	}
	return min_pos;
}

Position Selection::maxPosition() const
{
	Position max_pos;
	for(const Tile* tile : tiles) {
		if(!tile) continue;
		const Position& tile_pos = tile->getPosition();
		if(max_pos.x < tile_pos.x)
			max_pos.x = tile_pos.x;
		if(max_pos.y < tile_pos.y)
			max_pos.y = tile_pos.y;
		if(max_pos.z < tile_pos.z)
			max_pos.z = tile_pos.z;
	}
	return max_pos;
}

void Selection::add(Tile* tile, Item* item)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	if(item->isSelected()) return;

	// Make a copy of the tile with the item selected
	item->select();
	Tile* new_tile = tile->deepCopy(editor.getMap());
	item->deselect();

	if(g_settings.getInteger(Config::BORDER_IS_GROUND)) {
		if(item->isBorder())
			new_tile->selectGround();
	}

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
	Tile* new_tile = tile->deepCopy(editor.getMap());
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
	Tile* new_tile = tile->deepCopy(editor.getMap());
	creature->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile)
{
	ASSERT(subsession);
	ASSERT(tile);

	Tile* new_tile = tile->deepCopy(editor.getMap());
	new_tile->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Item* item)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	bool selected = item->isSelected();
	item->deselect();
	Tile* new_tile = tile->deepCopy(editor.getMap());
	if(selected) item->select();
	if(item->isBorder() && g_settings.getInteger(Config::BORDER_IS_GROUND)) new_tile->deselectGround();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Spawn* spawn)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	bool selected = spawn->isSelected();
	spawn->deselect();
	Tile* new_tile = tile->deepCopy(editor.getMap());
	if(selected) spawn->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Creature* creature)
{
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	bool selected = creature->isSelected();
	creature->deselect();
	Tile* new_tile = tile->deepCopy(editor.getMap());
	if(selected) creature->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile)
{
	ASSERT(subsession);

	Tile* new_tile = tile->deepCopy(editor.getMap());
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
		for(Tile* tile : tiles) {
			Tile* new_tile = tile->deepCopy(editor.getMap());
			new_tile->deselect();
			subsession->addChange(newd Change(new_tile));
		}
	} else {
		for(Tile* tile : tiles) {
			tile->deselect();
		}
		tiles.clear();
	}
}

void Selection::start(SessionFlags flags)
{
	if(!(flags & INTERNAL)) {
		if(!(flags & SUBTHREAD)) {
			session = editor.createBatch(ACTION_SELECT);
		}
		subsession = editor.createAction(ACTION_SELECT);
	}
	busy = true;
}

void Selection::commit()
{
	if(session) {
		ASSERT(subsession);
		// We need to step out of the session before we do the action, else peril awaits us!
		BatchAction* batch = session;
		session = nullptr;

		// Do the action
		batch->addAndCommitAction(subsession);

		// Create a newd action for subsequent selects
		subsession = editor.createAction(ACTION_SELECT);
		session = batch;
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
			BatchAction* batch = session;
			session = nullptr;

			batch->addAndCommitAction(subsession);
			editor.addBatch(batch, 2);

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

void SelectionThread::Execute()
{
	Create();
	Run();
}

wxThread::ExitCode SelectionThread::Entry()
{
	selection.start(Selection::SUBTHREAD);
	bool compesated = g_settings.getInteger(Config::COMPENSATED_SELECT);
	for(int z = start.z; z >= end.z; --z) {
		for(int x = start.x; x <= end.x; ++x) {
			for(int y = start.y; y <= end.y; ++y) {
				Tile* tile = editor.getMap().getTile(x, y, z);
				if(!tile)
					continue;

				selection.add(tile);
			}
		}
		if(compesated && z <= GROUND_LAYER) {
			++start.x; ++start.y;
			++end.x; ++end.y;
		}
	}
	result = selection.subsession;
	selection.finish(Selection::SUBTHREAD);
	return nullptr;
}
