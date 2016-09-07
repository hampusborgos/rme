//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EDITOR_H
#define RME_EDITOR_H

#include "item.h"
#include "tile.h"
#include "iomap.h"
#include "map.h"

#include "action.h"
#include "selection.h"

class BaseMap;
class CopyBuffer;
class LiveClient;
class LiveServer;
class LiveSocket;

class Editor {
public:
	Editor(CopyBuffer& copybuffer, LiveClient* client);
	Editor(CopyBuffer& copybuffer, const FileName& fn);
	Editor(CopyBuffer& copybuffer);
	~Editor();
protected:
	
	// Live Server
	LiveServer* live_server;
	LiveClient* live_client;

public:
	// Public members
	ActionQueue* actionQueue;
	SelectionArea selection;
	CopyBuffer& copybuffer;
	GroundBrush* replace_brush;
	Map map; // The map that is being edited

public: // Functions
	// Live Server handling
	LiveClient* GetLiveClient() const;
	LiveServer* GetLiveServer() const;
	LiveSocket& GetLive() const;
	bool CanEdit() const {return true;}
	bool IsLocal() const;
	bool IsLive() const;
	bool IsLiveServer() const;
	bool IsLiveClient() const;

	// Server side
	LiveServer* StartLiveServer();
	void CloseLiveServer();
	void BroadcastNodes(DirtyList& dirty_list);

	// Client side
	void QueryNode(int ndx, int ndy, bool underground);
	void SendNodeRequests();


	// Map handling
	void saveMap(FileName filename, bool showdialog); // "" means default filename

	wxString getLoaderError() const {return map.getError();}
	bool importMap(FileName filename, int import_x_offset, int import_y_offset, ImportType house_import_type, ImportType spawn_import_type);
	bool importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset);
	bool exportMiniMap(FileName filename, int floor /*= GROUND_LAYER*/, bool displaydialog);

	// Adds an action to the action queue (this allows the user to undo the action)
	// Invalidates the action pointer
	void addBatch(BatchAction* action, int stacking_delay = 0);
	void addAction(Action* action, int stacking_delay = 0);

	// Some simple actions that work on the map (these will work through the undo queue)
	// Creates a new selected area.
	bool createSelection(Position start, Position end);
	// Moves the selected area by the offset
	bool moveSelection(Position offset);
	// Deletes all selected items
	bool destroySelection();
	// Borderizes the selected region
	bool borderizeSelection();
	// Randomizes the ground in the selected region
	bool randomizeSelection();

	// Same as above although it applies to the entire map
	// action queue is flushed when these functions are called
	// showdialog is whether a progress bar should be shown
	void borderizeMap(bool showdialog);
	void randomizeMap(bool showdialog);
	void clearInvalidHouseTiles(bool showdialog);
	void clearModifiedTileState(bool showdialog);

	// Draw using the current brush to the target position
	// alt is whether the ALT key is pressed
	void draw(const Position& offset, bool alt);
	void undraw(const Position& offset, bool alt);
	void draw(const PositionVector& posvec, bool alt);
	void draw(const PositionVector& todraw, PositionVector& toborder, bool alt);
	void undraw(const PositionVector& posvec, bool alt);
	void undraw(const PositionVector& todraw, PositionVector& toborder, bool alt);

protected:
	void drawInternal(const Position offset, bool alt, bool dodraw);
	void drawInternal(const PositionVector& posvec, bool alt, bool dodraw);
	void drawInternal(const PositionVector& todraw, PositionVector& toborder, bool alt, bool dodraw);

	Editor(const Editor&);
	Editor& operator=(const Editor&);
};

inline void Editor::draw(const Position& offset, bool alt) { drawInternal(offset, alt, true); }
inline void Editor::undraw(const Position& offset, bool alt) { drawInternal(offset, alt, false); }
inline void Editor::draw(const PositionVector& posvec, bool alt) {drawInternal(posvec, alt, true);}
inline void Editor::draw(const PositionVector& todraw, PositionVector& toborder, bool alt) {drawInternal(todraw, toborder, alt, true);}
inline void Editor::undraw(const PositionVector& posvec, bool alt) {drawInternal(posvec, alt, false);}
inline void Editor::undraw(const PositionVector& todraw, PositionVector& toborder, bool alt) {drawInternal(todraw, toborder, alt, false);}

#endif
