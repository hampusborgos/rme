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

#ifndef RME_SELECTION_H
#define RME_SELECTION_H

#include "position.h"

class Action;
class Editor;
class BatchAction;

class SelectionThread;

class Selection
{
public:
	Selection(Editor& editor);
	~Selection();

	// Selects the items on the tile/tiles
	// Won't work outside a selection session
	void add(Tile* tile, Item* item);
	void add(Tile* tile, Spawn* spawn);
	void add(Tile* tile, Creature* creature);
	void add(Tile* tile);
	void remove(Tile* tile, Item* item);
	void remove(Tile* tile, Spawn* spawn);
	void remove(Tile* tile, Creature* creature);
	void remove(Tile* tile);

	// The tile will be added to the list of selected tiles, however, the items on the tile won't be selected
	void addInternal(Tile* tile);
	void removeInternal(Tile* tile);

	// Clears the selection completely
	void clear();

	// Returns true when inside a session
	bool isBusy() {return busy;}

	//
	Position minPosition() const;
	Position maxPosition() const;

	// This manages a "selection session"
	// Internal session doesn't store the result (eg. no undo)
	// Subthread means the session doesn't create a complete
	// action, just part of one to be merged with the main thread
	// later.
	enum SessionFlags {
		NONE,
		INTERNAL = 1,
		SUBTHREAD = 2,
	};

	void start(SessionFlags flags = NONE);
	void commit();
	void finish(SessionFlags flags = NONE);

	// Joins the selection instance in this thread with this instance
	// This deletes the thread
	void join(SelectionThread* thread);

	size_t size() {return tiles.size();}
	size_t size() const {return tiles.size();}
	void updateSelectionCount();
	TileSet::iterator begin() {return tiles.begin();}
	TileSet::iterator end() {return tiles.end();}
	TileSet& getTiles() {return tiles; }
	Tile* getSelectedTile() {ASSERT(size() == 1); return *tiles.begin();}

private:
	bool busy;
	Editor& editor;
	BatchAction* session;
	Action* subsession;

	TileSet tiles;

	friend class SelectionThread;
};

class SelectionThread : public wxThread {
public:
	SelectionThread(Editor& editor, Position start, Position end);
	virtual ~SelectionThread();

	void Execute(); // Calls "Create" and then "Run"
protected:
	virtual ExitCode Entry();
	Editor& editor;
	Position start, end;
	Selection selection;
	Action* result;

	friend class Selection;
};

#endif
