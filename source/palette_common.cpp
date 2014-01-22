//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/palette_common.hpp $
// $Id: palette_common.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "palette_common.h"
#include "brush.h"
#include "sprites.h"
#include "gui.h"
#include "common_windows.h"
#include "application.h"
#include "palette_waypoints.h"

// ============================================================================
// Palette Panel

BEGIN_EVENT_TABLE(PalettePanel, wxPanel)
	EVT_TIMER(PALETTE_DELAYED_REFRESH_TIMER, WaypointPalettePanel::OnRefreshTimer)
END_EVENT_TABLE()

PalettePanel::PalettePanel(wxWindow* parent, wxWindowID id, long style) :
	wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, style),
	refresh_timer(this, PALETTE_DELAYED_REFRESH_TIMER),
	last_brush_size(0)
{
}

PalettePanel::~PalettePanel() {
}

PaletteWindow* PalettePanel::GetParentPalette() const {	
	const wxWindow* w = this;
	while((w = w->GetParent()) && dynamic_cast<const PaletteWindow*>(w) == nullptr);
	return const_cast<PaletteWindow*>(static_cast<const PaletteWindow*>(w));
}

void PalettePanel::InvalidateContents() {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->InvalidateContents();
	}
}

void PalettePanel::LoadCurrentContents() {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->OnSwitchIn();
	}
	Fit();
}

void PalettePanel::LoadAllContents() {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->LoadAllContents();
	}
}

void PalettePanel::AddToolPanel(PalettePanel* panel) {
	wxSizer* sp_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, panel->GetName());
	sp_sizer->Add(panel, 0, wxEXPAND);
	GetSizer()->Add(sp_sizer, 0, wxEXPAND);
	
	//GetSizer()->SetDimension(wxDefaultCoord, wxDefaultCoord, wxDefaultCoord, wxDefaultCoord);
	//GetSizer()->Layout();
	Fit();
	//

	tool_bars.push_back(panel);
}

void PalettePanel::SetToolbarIconSize(bool large_icons) {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->SetToolbarIconSize(large_icons);
	}
}

wxString PalettePanel::GetName() const {
	switch(GetType()) {
		case TILESET_TERRAIN: return wxT("Terrain Palette");
		case TILESET_DOODAD: return wxT("Doodad Palette");
		case TILESET_ITEM: return wxT("Item Palette");
		case TILESET_CREATURE: return wxT("Creature Palette");
		case TILESET_HOUSE: return wxT("House Palette");
		case TILESET_RAW: return wxT("RAW Palette");
		case TILESET_WAYPOINT: return wxT("Waypoint Palette");
		case TILESET_UNKNOWN: return wxT("Unknown");
	}
	return wxEmptyString;
}

PaletteType PalettePanel::GetType() const {
	return TILESET_UNKNOWN;
}

Brush* PalettePanel::GetSelectedBrush() const {
	return nullptr;
}

int PalettePanel::GetSelectedBrushSize() const {
	return 0;
}

void PalettePanel::SelectFirstBrush() {
	// Do nothing
}

bool PalettePanel::SelectBrush(const Brush* whatbrush) {
	return false;
}

void PalettePanel::OnUpdateBrushSize(BrushShape shape, int size) {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->OnUpdateBrushSize(shape, size);
	}
}

void PalettePanel::OnSwitchIn() {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->OnSwitchIn();
	}
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushSize(last_brush_size);
}

void PalettePanel::OnSwitchOut() {
	last_brush_size = gui.GetBrushSize();
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->OnSwitchOut();
	}
}

void PalettePanel::OnUpdate() {
	for(ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
		(*iter)->OnUpdate();
	}
}

void PalettePanel::RefreshOtherPalettes()
{
	refresh_timer.Start(100, true);
}

void PalettePanel::OnRefreshTimer(wxTimerEvent&)
{
	gui.RefreshOtherPalettes(GetParentPalette());
}

// ============================================================================
// Size Page

BEGIN_EVENT_TABLE(BrushSizePanel, wxPanel)
	EVT_TOGGLEBUTTON(PALETTE_BRUSHSHAPE_SQUARE, BrushSizePanel::OnClickSquareBrush)
	EVT_TOGGLEBUTTON(PALETTE_BRUSHSHAPE_CIRCLE, BrushSizePanel::OnClickCircleBrush)

	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_0, BrushSizePanel::OnClickBrushSize0)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_1, BrushSizePanel::OnClickBrushSize1)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_2, BrushSizePanel::OnClickBrushSize2)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_4, BrushSizePanel::OnClickBrushSize4)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_6, BrushSizePanel::OnClickBrushSize6)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_8, BrushSizePanel::OnClickBrushSize8)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_11,BrushSizePanel::OnClickBrushSize11)
END_EVENT_TABLE()

BrushSizePanel::BrushSizePanel(wxWindow* parent) :
	PalettePanel(parent, wxID_ANY),
	loaded(false),
	large_icons(true),
	brushshapeSquareButton(nullptr),
	brushshapeCircleButton(nullptr),
	brushsize0Button(nullptr),
	brushsize1Button(nullptr),
	brushsize2Button(nullptr),
	brushsize4Button(nullptr),
	brushsize6Button(nullptr),
	brushsize8Button(nullptr),
	brushsize11Button(nullptr)
{
}

void BrushSizePanel::InvalidateContents() {
	if(loaded) {
		DestroyChildren();
		SetSizer(nullptr);

		brushshapeSquareButton =
		brushshapeCircleButton =
		brushsize0Button =
		brushsize1Button =
		brushsize2Button =
		brushsize4Button =
		brushsize6Button =
		brushsize8Button =
		brushsize11Button = nullptr;

		loaded = false;
	}
}

void BrushSizePanel::LoadCurrentContents() {
	LoadAllContents();
}

void BrushSizePanel::LoadAllContents() {
	if(loaded) return;

	wxSizer* size_sizer = newd wxBoxSizer(wxVERTICAL);;
	wxSizer* sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	RenderSize render_size;

	if(large_icons) {
		// 32x32
		render_size = RENDER_SIZE_32x32;
	} else {
		// 16x16
		render_size = RENDER_SIZE_16x16;
	}

	sub_sizer->Add(brushshapeSquareButton = newd DCButton(this, PALETTE_BRUSHSHAPE_SQUARE, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_9x9));
		brushshapeSquareButton->SetToolTip(wxT("Square brush"));
	
	sub_sizer->Add(brushshapeCircleButton = newd DCButton(this, PALETTE_BRUSHSHAPE_CIRCLE, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_CD_9x9));
		brushshapeCircleButton->SetToolTip(wxT("Circle brush"));
	brushshapeSquareButton->SetValue(true);

	if(large_icons) {
		sub_sizer->AddSpacer(36);
	} else {
		sub_sizer->AddSpacer(18);
	}

	sub_sizer->Add(brushsize0Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_0, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_CD_1x1));
		brushsize0Button->SetToolTip(wxT("Brush size 1"));
		brushsize0Button->SetValue(true);

	sub_sizer->Add(brushsize1Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_1, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_3x3));
		brushsize1Button->SetToolTip(wxT("Brush size 2"));

	if(large_icons) {
		size_sizer->Add(sub_sizer);
		sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	}

	sub_sizer->Add(brushsize2Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_2, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_5x5));
		brushsize2Button->SetToolTip(wxT("Brush size 3"));

	sub_sizer->Add(brushsize4Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_4, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_7x7));
		brushsize4Button->SetToolTip(wxT("Brush size 5"));

	sub_sizer->Add(brushsize6Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_6, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_9x9));
		brushsize6Button->SetToolTip(wxT("Brush size 7"));

	sub_sizer->Add(brushsize8Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_8, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_15x15));
	brushsize8Button->SetToolTip(wxT("Brush size 9"));
	
	sub_sizer->Add(brushsize11Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_11, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_19x19));
		brushsize11Button->SetToolTip(wxT("Brush size 12"));

	size_sizer->Add(sub_sizer);
	SetSizerAndFit(size_sizer);

	loaded = true;
}

wxString BrushSizePanel::GetName() const {
	return wxT("Brush Size");
}

void BrushSizePanel::SetToolbarIconSize(bool d) {
	InvalidateContents();
	large_icons = d;
}

void BrushSizePanel::OnSwitchIn() {
	LoadCurrentContents();
}

void BrushSizePanel::OnUpdateBrushSize(BrushShape shape, int size) {
	if(shape == BRUSHSHAPE_SQUARE) {
		brushshapeCircleButton->SetValue(false);
		brushshapeSquareButton->SetValue(true);

		brushsize0Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_1x1);
		brushsize1Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_3x3);
		brushsize2Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_5x5);
		brushsize4Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_7x7);
		brushsize6Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_9x9);
		brushsize8Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_15x15);
		brushsize11Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_19x19);
	} else {
		brushshapeSquareButton->SetValue(false);
		brushshapeCircleButton->SetValue(true);

		brushsize0Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_1x1);
		brushsize1Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_3x3);
		brushsize2Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_5x5);
		brushsize4Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_7x7);
		brushsize6Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_9x9);
		brushsize8Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_15x15);
		brushsize11Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_19x19);
	}

	if(brushsize0Button)  brushsize0Button->SetValue(false);
	if(brushsize1Button)  brushsize1Button->SetValue(false);
	if(brushsize2Button)  brushsize2Button->SetValue(false);
	if(brushsize4Button)  brushsize4Button->SetValue(false);
	if(brushsize6Button)  brushsize6Button->SetValue(false);
	if(brushsize8Button)  brushsize8Button->SetValue(false);
	if(brushsize11Button) brushsize11Button->SetValue(false);

	switch(size) {
		case 0:  brushsize0Button->SetValue(true);  break;
		case 1:  brushsize1Button->SetValue(true);  break;
		case 2:  brushsize2Button->SetValue(true);  break;
		case 4:  brushsize4Button->SetValue(true);  break;
		case 6:  brushsize6Button->SetValue(true);  break;
		case 8:  brushsize8Button->SetValue(true);  break;
		case 11: brushsize11Button->SetValue(true); break;
		default: brushsize0Button->SetValue(true);  break;
	}
}

void BrushSizePanel::OnClickCircleBrush(wxCommandEvent &event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushShape(BRUSHSHAPE_CIRCLE);
}

void BrushSizePanel::OnClickSquareBrush(wxCommandEvent &event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushShape(BRUSHSHAPE_SQUARE);
}

void BrushSizePanel::OnClickBrushSize(int which) {
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushSize(which);
}

// ============================================================================
// Tool Brush Panel

BEGIN_EVENT_TABLE(BrushToolPanel, PalettePanel)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, BrushToolPanel::OnClickGravelButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_ERASER, BrushToolPanel::OnClickEraserButton)
	
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_NORMAL_DOOR,BrushToolPanel::OnClickNormalDoorButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_LOCKED_DOOR,BrushToolPanel::OnClickLockedDoorButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_MAGIC_DOOR, BrushToolPanel::OnClickMagicDoorButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_QUEST_DOOR, BrushToolPanel::OnClickQuestDoorButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_HATCH_DOOR, BrushToolPanel::OnClickHatchDoorButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_WINDOW_DOOR,BrushToolPanel::OnClickWindowDoorButton)
	
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_PZ_TOOL,BrushToolPanel::OnClickPZBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_NOPVP_TOOL,BrushToolPanel::OnClickNOPVPBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_NOLOGOUT_TOOL,BrushToolPanel::OnClickNoLogoutBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_TERRAIN_PVPZONE_TOOL,BrushToolPanel::OnClickPVPZoneBrushButton)
END_EVENT_TABLE()

BrushToolPanel::BrushToolPanel(wxWindow* parent) :
	PalettePanel(parent, wxID_ANY),
	loaded(false),
	large_icons(true),
	optionalBorderButton(nullptr),
	eraserButton(nullptr),
	normalDoorButton(nullptr),
	lockedDoorButton(nullptr),
	magicDoorButton(nullptr),
	questDoorButton(nullptr),
	hatchDoorButton(nullptr),
	windowDoorButton(nullptr),
	pzBrushButton(nullptr),
	nopvpBrushButton(nullptr),
	nologBrushButton(nullptr),
	pvpzoneBrushButton(nullptr)
{
}

BrushToolPanel::~BrushToolPanel() {
}

void BrushToolPanel::InvalidateContents() {
	if(loaded) {
		DestroyChildren();
		SetSizer(nullptr);

		optionalBorderButton =
		eraserButton =
		normalDoorButton =
		lockedDoorButton =
		magicDoorButton = 
		questDoorButton = 
		hatchDoorButton = 
		windowDoorButton = 
		pzBrushButton = 
		nopvpBrushButton = 
		nologBrushButton = 
		pvpzoneBrushButton = nullptr;

		loaded = false;
	}
}

void BrushToolPanel::LoadCurrentContents() {
	LoadAllContents();
}

void BrushToolPanel::LoadAllContents() {
	if(loaded) return;

	wxSizer* size_sizer = newd wxBoxSizer(wxVERTICAL);;
	wxSizer* sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	RenderSize render_size;

	if(large_icons) {
		// 32x32
		render_size = RENDER_SIZE_32x32;
	} else {
		// 16x16
		render_size = RENDER_SIZE_16x16;
	}
	
	if(large_icons) {
		// Create the tool page with 32x32 icons

		ASSERT(gui.optional_brush);
		sub_sizer->Add(optionalBorderButton = newd BrushButton(this, gui.optional_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL));
			optionalBorderButton->SetToolTip(wxT("Optional Border Tool"));

		ASSERT(gui.eraser);
		sub_sizer->Add(eraserButton = newd BrushButton(this, gui.eraser, RENDER_SIZE_32x32, PALETTE_TERRAIN_ERASER));
			eraserButton->SetToolTip(wxT("Eraser"));

		ASSERT(gui.pz_brush);
		sub_sizer->Add(pzBrushButton = newd BrushButton(this, gui.pz_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_PZ_TOOL));
			pzBrushButton->SetToolTip(wxT("PZ Tool"));

		ASSERT(gui.rook_brush);
		sub_sizer->Add(nopvpBrushButton = newd BrushButton(this, gui.rook_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_NOPVP_TOOL));
			nopvpBrushButton->SetToolTip(wxT("NO PVP Tool"));

		ASSERT(gui.nolog_brush);
		sub_sizer->Add(nologBrushButton = newd BrushButton(this, gui.nolog_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_NOLOGOUT_TOOL));
			nologBrushButton->SetToolTip(wxT("No Logout Tool"));

		ASSERT(gui.pvp_brush);
		sub_sizer->Add(pvpzoneBrushButton = newd BrushButton(this, gui.pvp_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_PVPZONE_TOOL));
			pvpzoneBrushButton->SetToolTip(wxT("PVP Zone Tool"));
		
		// New row
		size_sizer->Add(sub_sizer);
		sub_sizer = newd wxBoxSizer(wxHORIZONTAL);

		ASSERT(gui.normal_door_brush);
		sub_sizer->Add(normalDoorButton = newd BrushButton(this, gui.normal_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_NORMAL_DOOR));
			normalDoorButton->SetToolTip(wxT("Normal Door Tool"));

		ASSERT(gui.locked_door_brush);
		sub_sizer->Add(lockedDoorButton = newd BrushButton(this, gui.locked_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_LOCKED_DOOR));
			lockedDoorButton->SetToolTip(wxT("Locked Door Tool"));

		ASSERT(gui.magic_door_brush);
		sub_sizer->Add(magicDoorButton = newd BrushButton(this, gui.magic_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_MAGIC_DOOR));
			magicDoorButton->SetToolTip(wxT("Magic Door Tool"));
			
		ASSERT(gui.quest_door_brush);
		sub_sizer->Add(questDoorButton = newd BrushButton(this, gui.quest_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_QUEST_DOOR));
			questDoorButton->SetToolTip(wxT("Quest Door Tool"));
			
		ASSERT(gui.hatch_door_brush);
		sub_sizer->Add(hatchDoorButton = newd BrushButton(this, gui.hatch_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_HATCH_DOOR));
			hatchDoorButton->SetToolTip(wxT("Hatch Window Tool"));

		ASSERT(gui.window_door_brush);
		sub_sizer->Add(windowDoorButton = newd BrushButton(this, gui.window_door_brush, RENDER_SIZE_32x32, PALETTE_TERRAIN_WINDOW_DOOR));
			windowDoorButton->SetToolTip(wxT("Window Tool"));
	} else {
		// Create the tool page with 16x16 icons
		// Create tool window #1

		ASSERT(gui.optional_brush);
		sub_sizer->Add(optionalBorderButton = newd BrushButton(this, gui.optional_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL));
			optionalBorderButton->SetToolTip(wxT("Optional Border Tool"));

		ASSERT(gui.eraser);
		sub_sizer->Add(eraserButton = newd BrushButton(this, gui.eraser, RENDER_SIZE_16x16, PALETTE_TERRAIN_ERASER));
			eraserButton->SetToolTip(wxT("Eraser"));

		sub_sizer->AddSpacer(20);
		
		ASSERT(gui.normal_door_brush);
		sub_sizer->Add(normalDoorButton = newd BrushButton(this, gui.normal_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_NORMAL_DOOR));
			normalDoorButton->SetToolTip(wxT("Normal Door Tool"));

		ASSERT(gui.locked_door_brush);
		sub_sizer->Add(lockedDoorButton = newd BrushButton(this, gui.locked_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_LOCKED_DOOR));
			lockedDoorButton->SetToolTip(wxT("Locked Door Tool"));
			
		ASSERT(gui.magic_door_brush);
		sub_sizer->Add(magicDoorButton = newd BrushButton(this, gui.magic_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_MAGIC_DOOR));
			magicDoorButton->SetToolTip(wxT("Magic Door Tool"));
			
		ASSERT(gui.quest_door_brush);
		sub_sizer->Add(questDoorButton = newd BrushButton(this, gui.quest_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_QUEST_DOOR));
			questDoorButton->SetToolTip(wxT("Quest Door Tool"));
			
		ASSERT(gui.hatch_door_brush);
		sub_sizer->Add(hatchDoorButton = newd BrushButton(this, gui.hatch_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_HATCH_DOOR));
			hatchDoorButton->SetToolTip(wxT("Hatch Window Tool"));

		ASSERT(gui.window_door_brush);
		sub_sizer->Add(windowDoorButton = newd BrushButton(this, gui.window_door_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_WINDOW_DOOR));
			windowDoorButton->SetToolTip(wxT("Window Tool"));

		// Next row
		size_sizer->Add(sub_sizer);
		sub_sizer = newd wxBoxSizer(wxHORIZONTAL);

		
		ASSERT(gui.pz_brush);
		sub_sizer->Add(pzBrushButton = newd BrushButton(this, gui.pz_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_PZ_TOOL));
			pzBrushButton->SetToolTip(wxT("PZ Tool"));

		ASSERT(gui.rook_brush);
		sub_sizer->Add(nopvpBrushButton = newd BrushButton(this, gui.rook_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_NOPVP_TOOL));
			nopvpBrushButton->SetToolTip(wxT("NO PVP Tool"));

		ASSERT(gui.nolog_brush);
		sub_sizer->Add(nologBrushButton = newd BrushButton(this, gui.nolog_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_NOLOGOUT_TOOL));
			nologBrushButton->SetToolTip(wxT("No Logout Tool"));

		ASSERT(gui.pvp_brush);
		sub_sizer->Add(pvpzoneBrushButton = newd BrushButton(this, gui.pvp_brush, RENDER_SIZE_16x16, PALETTE_TERRAIN_PVPZONE_TOOL));
			pvpzoneBrushButton->SetToolTip(wxT("PVP Zone Tool"));
	}

	size_sizer->Add(sub_sizer);
	SetSizerAndFit(size_sizer);

	loaded = true;
}

wxString BrushToolPanel::GetName() const {
	return wxT("Tools");
}

void BrushToolPanel::SetToolbarIconSize(bool d) {
	InvalidateContents();
	large_icons = d;
}

void BrushToolPanel::DeselectAll() {
	if(loaded) {
		optionalBorderButton->SetValue(false);
		eraserButton->SetValue(false);
		normalDoorButton->SetValue(false);
		lockedDoorButton->SetValue(false);
		magicDoorButton->SetValue(false);
		questDoorButton->SetValue(false);
		hatchDoorButton->SetValue(false);
		windowDoorButton->SetValue(false);
		pzBrushButton->SetValue(false);
		nopvpBrushButton->SetValue(false);
		nologBrushButton->SetValue(false);
		pvpzoneBrushButton->SetValue(false);
	}
}

Brush* BrushToolPanel::GetSelectedBrush() const {
	if(optionalBorderButton->GetValue())
		return gui.optional_brush;
	if(eraserButton->GetValue())
		return gui.eraser;
	if(normalDoorButton->GetValue())
		return gui.normal_door_brush;
	if(lockedDoorButton->GetValue())
		return gui.locked_door_brush;
	if(magicDoorButton->GetValue())
		return gui.magic_door_brush;
	if(questDoorButton->GetValue())
		return gui.quest_door_brush;
	if(hatchDoorButton->GetValue())
		return gui.hatch_door_brush;
	if(windowDoorButton->GetValue())
		return gui.window_door_brush;
	if(pzBrushButton->GetValue())
		return gui.pz_brush;
	if(nopvpBrushButton->GetValue())
		return gui.rook_brush;
	if(nologBrushButton->GetValue())
		return gui.nolog_brush;
	if(pvpzoneBrushButton->GetValue())
		return gui.pvp_brush;
	return nullptr;
}

bool BrushToolPanel::SelectBrush(const Brush* whatbrush) {
	BrushButton* button = nullptr;
	if(whatbrush == gui.optional_brush) {
		button = optionalBorderButton;
	} else if(whatbrush == gui.eraser) {
		button = eraserButton;
	} else if(whatbrush == gui.normal_door_brush) {
		button = normalDoorButton;
	} else if(whatbrush == gui.locked_door_brush) {
		button = lockedDoorButton;
	} else if(whatbrush == gui.magic_door_brush) {
		button = magicDoorButton;
	} else if(whatbrush == gui.quest_door_brush) {
		button = questDoorButton;
	} else if(whatbrush == gui.hatch_door_brush) {
		button = hatchDoorButton;
	} else if(whatbrush == gui.window_door_brush) {
		button = windowDoorButton;
	} else if(whatbrush == gui.pz_brush) {
		button = pzBrushButton;
	} else if(whatbrush == gui.rook_brush) {
		button = nopvpBrushButton;
	} else if(whatbrush == gui.nolog_brush) {
		button = nologBrushButton;
	} else if(whatbrush == gui.pvp_brush) {
		button = pvpzoneBrushButton;
	} 
	
	DeselectAll();
	if(button) {
		button->SetValue(true);
		return true;
	}
	
	return false;
}

void BrushToolPanel::OnSwitchIn() {
	LoadCurrentContents();
}

void BrushToolPanel::OnClickGravelButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.optional_brush);
}

void BrushToolPanel::OnClickEraserButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.eraser);
}

void BrushToolPanel::OnClickNormalDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.normal_door_brush);
}

void BrushToolPanel::OnClickLockedDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.locked_door_brush);
}

void BrushToolPanel::OnClickMagicDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.magic_door_brush);
}

void BrushToolPanel::OnClickQuestDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.quest_door_brush);
}

void BrushToolPanel::OnClickHatchDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.hatch_door_brush);
}

void BrushToolPanel::OnClickWindowDoorButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.window_door_brush);
}

void BrushToolPanel::OnClickPZBrushButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.pz_brush);
}

void BrushToolPanel::OnClickNOPVPBrushButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.rook_brush);
}

void BrushToolPanel::OnClickNoLogoutBrushButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.nolog_brush);
}

void BrushToolPanel::OnClickPVPZoneBrushButton(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush(gui.pvp_brush);
}

// ============================================================================
// Brush Button

BEGIN_EVENT_TABLE(BrushButton, ItemToggleButton)
	EVT_KEY_DOWN(BrushButton::OnKey)
END_EVENT_TABLE()

BrushButton::BrushButton(wxWindow* parent, Brush* _brush, RenderSize sz, uint32_t id) :
	ItemToggleButton(parent, sz, uint16_t(0), id),
	brush(_brush)
{
	ASSERT(sz != RENDER_SIZE_64x64);
	ASSERT(brush);
	SetSprite(brush->getLookID());
	SetToolTip(wxstr(brush->getName()));
}

BrushButton::~BrushButton() {
	// ...
}

void BrushButton::OnKey(wxKeyEvent& event) {
	gui.AddPendingCanvasEvent(event);
}

// ============================================================================
// Brush Thickness Panel

BEGIN_EVENT_TABLE(BrushThicknessPanel, PalettePanel)
#ifdef __WINDOWS__
	// This only works in wxmsw
	EVT_COMMAND_SCROLL_CHANGED(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
#else
	EVT_COMMAND_SCROLL_TOP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_BOTTOM(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_LINEUP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_LINEDOWN(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_PAGEUP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_PAGEDOWN(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
	EVT_COMMAND_SCROLL_THUMBRELEASE(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
#endif

	EVT_CHECKBOX(PALETTE_DOODAD_USE_THICKNESS, BrushThicknessPanel::OnClickCustomThickness)
END_EVENT_TABLE()

BrushThicknessPanel::BrushThicknessPanel(wxWindow* parent) :
	PalettePanel(parent, wxID_ANY)
{
	wxSizer* thickness_sizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* thickness_sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	thickness_sub_sizer->Add(20,10);
	use_button = newd wxCheckBox(this, PALETTE_DOODAD_USE_THICKNESS, wxT("Use custom thickness"));
	thickness_sub_sizer->Add(use_button);
	thickness_sizer->Add(thickness_sub_sizer, 1, wxEXPAND);

	slider = newd wxSlider(this, PALETTE_DOODAD_SLIDER, 5, 1, 10, wxDefaultPosition);
	thickness_sizer->Add(slider, 1, wxEXPAND);

	SetSizerAndFit(thickness_sizer);
}

BrushThicknessPanel::~BrushThicknessPanel() {
}

wxString BrushThicknessPanel::GetName() const {
	return wxT("Brush Thickness");
}

void BrushThicknessPanel::OnScroll(wxScrollEvent& event) {
	static const int lookup_table[10] = {1,2,3,5,8,13,23,35,50,80};
	use_button->SetValue(true);

	ASSERT(event.GetPosition() >= 1);
	ASSERT(event.GetPosition() <= 10);

	//printf("SELECT[%d] = %d\n", event.GetPosition()-1, lookup_table[event.GetPosition()-1]);
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushThickness(true, lookup_table[event.GetPosition()-1], 100);
}

void BrushThicknessPanel::OnClickCustomThickness(wxCommandEvent& event) {
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushThickness(event.IsChecked());
}

void BrushThicknessPanel::OnSwitchIn() {
	static const int lookup_table[10] = {1,2,3,5,8,13,23,35,50,80};
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushThickness(lookup_table[slider->GetValue()-1], 100);
}

