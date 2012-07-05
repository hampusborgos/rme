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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/settings.hpp $
// $Id: settings.hpp 323 2010-04-12 20:59:41Z admin $

#include "main.h"

#include "settings.h"
#include "gui_ids.h"
#include "client_version.h"

#include <wx/confbase.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>

#include <iostream>
#include <string>

Settings settings;

Settings::Settings() : store(Config::LAST)
#ifdef __WINDOWS__
		       , use_file_cfg(false) 
#endif
{
	setDefaults();
}

Settings::~Settings() {
}

wxConfigBase& Settings::getConfigObject()
{
	return *dynamic_cast<wxConfigBase*>(wxConfig::Get());
}

int Settings::getInteger(uint key) const {
	if(key > Config::LAST) return 0;
	const DynamicValue& dv = store[key];
	if(dv.type == TYPE_INT)
		return dv.intval;
	return 0;
}

float Settings::getFloat(uint key) const {
	if(key > Config::LAST) return 0.0;
	const DynamicValue& dv = store[key];
	if(dv.type == TYPE_FLOAT) {
		return dv.floatval;
	}
	return 0.0;
}

std::string Settings::getString(uint key) const {
	if(key > Config::LAST) return "";
	const DynamicValue& dv = store[key];
	if(dv.type == TYPE_STR && dv.strval != NULL)
		return *dv.strval;
	return "";
}


void Settings::setInteger(uint key, int newval) {
	if(key > Config::LAST) return;
	DynamicValue& dv = store[key];
	if(dv.type == TYPE_INT) {
		dv.intval = newval;
	} else if(dv.type == TYPE_NONE) {
		dv.type = TYPE_INT;
		dv.intval = newval;
	}
}

void Settings::setFloat(uint key, float newval) {
	if(key > Config::LAST) return;
	DynamicValue& dv = store[key];
	if(dv.type == TYPE_FLOAT) {
		dv.floatval = newval;
	} else if(dv.type == TYPE_NONE) {
		dv.type = TYPE_FLOAT;
		dv.floatval = newval;
	}
}

void Settings::setString(uint key, std::string newval) {
	if(key > Config::LAST) return;
	DynamicValue& dv = store[key];
	if(dv.type == TYPE_STR) {
		delete dv.strval;
		dv.strval = newd std::string(newval);
	} else if(dv.type == TYPE_NONE) {
		dv.type = TYPE_STR;
		dv.strval = newd std::string(newval);
	}
}

std::string Settings::DynamicValue::str() {
	switch(type) {
		case TYPE_FLOAT:return f2s(floatval);
		case TYPE_STR:  return std::string(*strval);
		case TYPE_INT:  return i2s(intval);
		default:
		case TYPE_NONE: return "";
	}
}

void Settings::IO(IOMode mode) {
	wxConfigBase* conf = (mode == DEFAULT? NULL : dynamic_cast<wxConfigBase*>(wxConfig::Get()));

	using namespace Config;
#define section(s) if(conf) conf->SetPath(wxT("/") wxT(s))
#define Int(key, dflt) \
	do { \
		if(mode == DEFAULT) { \
			setInteger(key, dflt); \
		} else if(mode == SAVE) { \
			conf->Write(wxT(#key), getInteger(key)); \
		} else if(mode == LOAD) { \
			setInteger(key, conf->Read(wxT(#key), long(dflt))); \
		} \
	} while(false)
#define IntToSave(key, dflt) \
	do { \
		if(mode == DEFAULT) { \
			setInteger(key, dflt); \
		} else if(mode == SAVE) { \
			conf->Write(wxT(#key), getInteger(key##_TO_SAVE)); \
		} else if(mode == LOAD) { \
			setInteger(key, conf->Read(wxT(#key), (long)dflt)); \
			setInteger(key##_TO_SAVE , getInteger(key)); \
		} \
	} while(false)
#define Float(key, dflt) \
	do {\
		if(mode == DEFAULT) { \
			setFloat(key, dflt); \
		} else if(mode == SAVE) { \
			conf->Write(wxT(#key), getFloat(key)); \
		} else if(mode == LOAD) { \
			double tmp_float;\
			conf->Read(wxT(#key), &tmp_float, dflt); \
			setFloat(key, tmp_float); \
		} \
	} while(false)
#define String(key, dflt) \
	do { \
		if(mode == DEFAULT) { \
			setString(key, dflt); \
		} else if(mode == SAVE) { \
			conf->Write(wxT(#key), wxstr(getString(key))); \
		} else if(mode == LOAD) { \
			wxString str; \
			conf->Read(wxT(#key), &str, wxT(dflt)); \
			setString(key, nstr(str)); \
		} \
	} while(false)

	section("View");
	Int(TRANSPARENT_FLOORS, 0);
	Int(TRANSPARENT_ITEMS, 0);
	Int(SHOW_ALL_FLOORS, 1);
	Int(SHOW_INGAME_BOX, 0);
	Int(SHOW_GRID, 0);
	Int(SHOW_EXTRA, 1);
	Int(SHOW_SHADE, 1);
	Int(SHOW_SPECIAL_TILES, 1);
	Int(SHOW_SPAWNS, 1);
	Int(SHOW_ITEMS, 1);
	Int(HIGHLIGHT_ITEMS, 0);
	Int(SHOW_CREATURES, 1);
	Int(SHOW_HOUSES, 1);
	Int(SHOW_BLOCKING, 0);
	Int(SHOW_ONLY_TILEFLAGS, 0);
	Int(SHOW_ONLY_MODIFIED_TILES, 0);

	section("Version");
	Int(VERSION_ID, 0);
	Int(CHECK_SIGNATURES, 1);
	Int(USE_CUSTOM_DATA_DIRECTORY, 0);
	String(DATA_DIRECTORY, "");
	String(EXTENSIONS_DIRECTORY, "");
	String(TIBIA_DATA_DIRS, "");

	section("Editor");
	String(RECENT_FILES, "");
	Int(WORKER_THREADS, 1);
	Int(MERGE_MOVE, 0);
	Int(MERGE_PASTE, 0);
	Int(UNDO_SIZE, 400);
	Int(UNDO_MEM_SIZE, 40);
	Int(GROUP_ACTIONS, 1);
	Int(SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	Int(COMPENSATED_SELECT, 1);
	Float(SCROLL_SPEED, 3.5f);
	Float(ZOOM_SPEED, 1.4f);
	Int(SWITCH_MOUSEBUTTONS, 0);
	Int(DOUBLECLICK_PROPERTIES, 1);
	Int(LISTBOX_EATS_ALL_EVENTS, 1);
	Int(BORDER_IS_GROUND, 1);
	Int(BORDERIZE_PASTE, 1);
	Int(BORDERIZE_DRAG, 1);
	Int(BORDERIZE_DRAG_THRESHOLD, 6000);
	Int(BORDERIZE_PASTE_THRESHOLD, 10000);
	Int(ALWAYS_MAKE_BACKUP, 0);
	Int(USE_AUTOMAGIC, 1);
	Int(HOUSE_BRUSH_REMOVE_ITEMS, 0);
	Int(AUTO_ASSIGN_DOORID, 1);
	Int(ERASER_LEAVE_UNIQUE, 1);
	Int(DOODAD_BRUSH_ERASE_LIKE, 0);
	Int(WARN_FOR_DUPLICATE_ID, 1);
	Int(ALLOW_CREATURES_WITHOUT_SPAWN, 0);
	Int(DEFAULT_SPAWNTIME, 60);
	Int(MAX_SPAWN_RADIUS, 30);
	Int(CURRENT_SPAWN_RADIUS, 5);
	Int(CREATE_MAP_ON_STARTUP, 1);
	Int(DEFAULT_CLIENT_VERSION, CLIENT_VERSION_NONE);
	Int(RAW_LIKE_SIMONE, 1);
	Int(USE_UPDATER, -1);
	Int(ONLY_ONE_INSTANCE, 1);
	Int(USE_OTBM_4_FOR_ALL_MAPS, 0);
	Int(USE_OTGZ, 1);

	section("Graphics");
	Int(TEXTURE_MANAGEMENT, 1);
	Int(TEXTURE_CLEAN_PULSE, 15);
	Int(TEXTURE_LONGEVITY, 20);
	Int(TEXTURE_CLEAN_THRESHOLD, 2500);
	Int(SOFTWARE_CLEAN_THRESHOLD, 1800);
	Int(SOFTWARE_CLEAN_SIZE, 500);
	Int(ICON_BACKGROUND, 0);
	Int(HARD_REFRESH_RATE, 200);
	Int(HIDE_ITEMS_WHEN_ZOOMED, 1);
	String(SCREENSHOT_DIRECTORY, "");
	String(SCREENSHOT_FORMAT, "png");
	IntToSave(USE_MEMCACHED_SPRITES, 0);
	Int(MINIMAP_UPDATE_DELAY, 333);
	Int(MINIMAP_VIEW_BOX, 1);
	
	Int(CURSOR_RED, 0);
	Int(CURSOR_GREEN, 166);
	Int(CURSOR_BLUE, 0);
	Int(CURSOR_ALPHA, 128);
	Int(CURSOR_ALT_RED, 0);
	Int(CURSOR_ALT_GREEN, 166);
	Int(CURSOR_ALT_BLUE, 0);
	Int(CURSOR_ALT_ALPHA, 128);

	section("UI");
	Int(USE_LARGE_CONTAINER_ICONS, 1);
	Int(USE_LARGE_CHOOSE_ITEM_ICONS, 1);
	Int(USE_LARGE_TERRAIN_TOOLBAR, 1);
	Int(USE_LARGE_DOODAD_SIZEBAR, 1);
	Int(USE_LARGE_ITEM_SIZEBAR, 1);
	Int(USE_LARGE_HOUSE_SIZEBAR, 1);
	Int(USE_LARGE_RAW_SIZEBAR, 1);
	Int(USE_GUI_SELECTION_SHADOW, 0);
	Int(PALETTE_COL_COUNT, 8);
	String(PALETTE_TERRAIN_STYLE, "large icons");
	String(PALETTE_DOODAD_STYLE, "large icons");
	String(PALETTE_ITEM_STYLE, "listbox");
	String(PALETTE_RAW_STYLE, "listbox");

	section("Window");
	String(PALETTE_LAYOUT, "name=02c30f6048629894000011bc00000002;caption=Palette;state=2099148;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=245;besth=100;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1");
	Int(MINIMAP_VISIBLE, 0);
	String(MINIMAP_LAYOUT, "name=066e2bc8486298990000259a00000003;caption=Minimap;state=2099151;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=170;besth=130;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=221;floath=164");
	Int(WINDOW_HEIGHT, 500);
	Int(WINDOW_WIDTH, 700);
	Int(WINDOW_MAXIMIZED, 0);

	section("Hotkeys");
	String(NUMERICAL_HOTKEYS, "none:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\nnone:{}\n");

	section("");
	Int(GOTO_WEBSITE_ON_BOOT, 0);
#undef section
#undef Int
#undef IntToSave
#undef Float
#undef String
}


void Settings::load() {
	wxConfigBase* conf;
#ifdef __WINDOWS__
	FileName filename(wxT("rme.cfg"));
	if(filename.FileExists()) { // Use local file if it exists
		wxFileInputStream file(filename.GetFullPath());
		conf = newd wxFileConfig(file);
		use_file_cfg = true;
		settings.setInteger(Config::INDIRECTORY_INSTALLATION, 1);
	} else { // Use registry
		conf = newd wxConfig(wxT("Remere's Map Editor"), wxT("Remere"), wxT(""), wxT(""), wxCONFIG_USE_GLOBAL_FILE);
		settings.setInteger(Config::INDIRECTORY_INSTALLATION, 0);
	}
#else
	FileName filename(wxT("./rme.cfg"));
	if(filename.FileExists())
	{ // Use local file if it exists
		wxFileInputStream file(filename.GetFullPath());
		conf = newd wxFileConfig(file);
		settings.setInteger(Config::INDIRECTORY_INSTALLATION, 1);
	}
	else
	{ // Else use global (user-specific) conf
		filename.Assign(wxStandardPaths::Get().GetUserConfigDir() + wxT("/.rme/rme.cfg"));
		if(filename.FileExists())
		{
			wxFileInputStream file(filename.GetFullPath());
			conf = newd wxFileConfig(file);
		}
		else
		{
			wxStringInputStream dummy(wxT(""));
			conf = newd wxFileConfig(dummy, wxConvAuto());
		}		
		settings.setInteger(Config::INDIRECTORY_INSTALLATION, 0);
	}
#endif
	wxConfig::Set(conf);
	IO(LOAD);
}

void Settings::save(bool endoftheworld)
{
	IO(SAVE);
#ifdef __WINDOWS__
	if(use_file_cfg)
	{
		wxFileConfig* conf = dynamic_cast<wxFileConfig*>(wxConfig::Get());
		if(!conf)
			return;
		FileName filename(wxT("rme.cfg"));
		wxFileOutputStream file(filename.GetFullPath());
		conf->Save(file);
	}
#else
	wxFileConfig* conf = dynamic_cast<wxFileConfig*>(wxConfig::Get());
	if(!conf)
		return;
	FileName filename(wxT("./rme.cfg"));
	if(filename.FileExists())
	{ // Use local file if it exists
		wxFileOutputStream file(filename.GetFullPath());
		conf->Save(file);
	}
	else
	{ // Else use global (user-specific) conf
		wxString path = wxStandardPaths::Get().GetUserConfigDir() + wxT("/.rme/rme.cfg");
		filename.Assign(path);
		filename.Mkdir(0755, wxPATH_MKDIR_FULL);
		wxFileOutputStream file(filename.GetFullPath());
		conf->Save(file);
	}
#endif
	if(endoftheworld)
	{
		wxConfigBase* conf = dynamic_cast<wxConfigBase*>(wxConfig::Get());
		wxConfig::Set(NULL);
		delete conf;
	}
}



