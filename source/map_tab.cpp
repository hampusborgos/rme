//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/map_tab.hpp $
// $Id: map_tab.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "gui.h"
#include "editor.h"
#include "map.h"
#include "sprites.h"
#include "map_tab.h"
#include "editor_tabs.h"
#include "map_display.h"

MapTab::MapTab(MapTabbook* aui, Editor* editor) :
	EditorTab(),
	MapWindow(aui, *editor),
	aui(aui)
{
	iref = newd InternalReference;
	iref->editor = editor;
	iref->owner_count = 1;

	aui->AddTab(this, true);
	FitToMap();
}

MapTab::MapTab(const MapTab* other) :
	EditorTab(),
	MapWindow(other->aui, *other->iref->editor),
	aui(other->aui),
	iref(other->iref)
{
	iref->owner_count++;
	aui->AddTab(this, true);
	FitToMap();
	int x, y;
	other->GetCanvas()->GetScreenCenter(&x, &y);
	CenterOnPosition(Position(x, y, other->GetCanvas()->GetFloor()));
}

MapTab::~MapTab()
{
	iref->owner_count--;
	if(iref->owner_count <= 0) {
		delete iref->editor;
		delete iref;
	}
}

bool MapTab::IsUniqueReference() const
{
	return iref->owner_count == 1;
}

wxWindow* MapTab::GetWindow() const
{
	return const_cast<MapTab*>(this);
}

MapCanvas* MapTab::GetCanvas() const
{
	return canvas;
}

MapWindow* MapTab::GetView() const
{
	return const_cast<MapWindow*>((const MapWindow*)this);
}

wxString MapTab::GetTitle() const
{
	wxString ss;
	ss << wxstr(iref->editor->map.getName()) << (iref->editor->map.hasChanged()? wxT("*"): wxT(""));
	return ss;
}

Editor* MapTab::GetEditor() const
{
	return &editor;
}

Map* MapTab::GetMap() const
{
	return &editor.map;
}

void MapTab::OnSwitchEditorMode(EditorMode mode)
{
	gem->SetSprite(mode == DRAWING_MODE? EDITOR_SPRITE_DRAWING_GEM : EDITOR_SPRITE_SELECTION_GEM);
	if(mode == SELECTION_MODE)
		canvas->EnterSelectionMode();
	else
		canvas->EnterDrawingMode();
}
