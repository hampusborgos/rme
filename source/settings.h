//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_SETTINGS_H_
#define RME_SETTINGS_H_

#include "main.h"

namespace Config {
	enum Key {
		NONE,
		VERSION_ID,

		USE_CUSTOM_DATA_DIRECTORY,
		DATA_DIRECTORY,
		EXTENSIONS_DIRECTORY,

		MERGE_MOVE,
		TEXTURE_MANAGEMENT,
		TEXTURE_CLEAN_PULSE,
		TEXTURE_CLEAN_THRESHOLD,
		TEXTURE_LONGEVITY,
		HARD_REFRESH_RATE,
		SOFTWARE_CLEAN_THRESHOLD,
		SOFTWARE_CLEAN_SIZE,
		TRANSPARENT_FLOORS,
		TRANSPARENT_ITEMS,
		SHOW_INGAME_BOX,
		SHOW_GRID,
		SHOW_EXTRA,
		SHOW_ALL_FLOORS,
		SHOW_MONSTERS,
		SHOW_SPAWNS_MONSTER,
		SHOW_NPCS,
		SHOW_SPAWNS_NPC,
		SHOW_HOUSES,
		SHOW_SHADE,
		SHOW_SPECIAL_TILES,
		HIGHLIGHT_ITEMS,
		SHOW_ITEMS,
		SHOW_BLOCKING,
		SHOW_TOOLTIPS,
		SHOW_PREVIEW,
		SHOW_WALL_HOOKS,
		SHOW_AS_MINIMAP,
		SHOW_ONLY_TILEFLAGS,
		SHOW_ONLY_MODIFIED_TILES,
		HIDE_ITEMS_WHEN_ZOOMED,
		GROUP_ACTIONS,
		SCROLL_SPEED,
		ZOOM_SPEED,
		UNDO_SIZE,
		UNDO_MEM_SIZE,
		MERGE_PASTE,
		SELECTION_TYPE,
		COMPENSATED_SELECT,
		BORDER_IS_GROUND,
		BORDERIZE_PASTE,
		BORDERIZE_DRAG,
		BORDERIZE_DRAG_THRESHOLD,
		BORDERIZE_PASTE_THRESHOLD,
		ICON_BACKGROUND,
		ALWAYS_MAKE_BACKUP,
		USE_AUTOMAGIC,
		HOUSE_BRUSH_REMOVE_ITEMS,
		AUTO_ASSIGN_DOORID,
		ERASER_LEAVE_UNIQUE,
		DOODAD_BRUSH_ERASE_LIKE,
		WARN_FOR_DUPLICATE_ID,
		USE_UPDATER,
		USE_OTBM_4_FOR_ALL_MAPS,
		SAVE_WITH_OTB_MAGIC_NUMBER,
		REPLACE_SIZE,

		USE_LARGE_CONTAINER_ICONS,
		USE_LARGE_CHOOSE_ITEM_ICONS,
		USE_LARGE_TERRAIN_TOOLBAR,
		USE_LARGE_DOODAD_SIZEBAR,
		USE_LARGE_ITEM_SIZEBAR,
		USE_LARGE_HOUSE_SIZEBAR,
		USE_LARGE_RAW_SIZEBAR,
		USE_GUI_SELECTION_SHADOW,
		PALETTE_COL_COUNT,
		PALETTE_TERRAIN_STYLE,
		PALETTE_DOODAD_STYLE,
		PALETTE_ITEM_STYLE,
		PALETTE_RAW_STYLE,

		ASSETS_DATA_DIRS,
		DEFAULT_CLIENT_VERSION,

		CURSOR_RED,
		CURSOR_GREEN,
		CURSOR_BLUE,
		CURSOR_ALPHA,

		CURSOR_ALT_RED,
		CURSOR_ALT_GREEN,
		CURSOR_ALT_BLUE,
		CURSOR_ALT_ALPHA,

		SCREENSHOT_DIRECTORY,
		SCREENSHOT_FORMAT,

		MAX_SPAWN_MONSTER_RADIUS,
		CURRENT_SPAWN_MONSTER_RADIUS,
		AUTO_CREATE_SPAWN_MONSTER,
		DEFAULT_SPAWN_MONSTER_TIME,

		MAX_SPAWN_NPC_RADIUS,
		CURRENT_SPAWN_NPC_RADIUS,
		AUTO_CREATE_SPAWN_NPC,
		DEFAULT_SPAWN_NPC_TIME,

		SWITCH_MOUSEBUTTONS,
		DOUBLECLICK_PROPERTIES,
		LISTBOX_EATS_ALL_EVENTS,
		RAW_LIKE_SIMONE,
		WORKER_THREADS,
		COPY_POSITION_FORMAT,

		GOTO_WEBSITE_ON_BOOT,
		INDIRECTORY_INSTALLATION,
		AUTOCHECK_FOR_UPDATES,
		ONLY_ONE_INSTANCE,

		PALETTE_LAYOUT,
		MINIMAP_VISIBLE,
		MINIMAP_LAYOUT,
		MINIMAP_UPDATE_DELAY,
		MINIMAP_VIEW_BOX,
		MINIMAP_EXPORT_DIR,
		WINDOW_HEIGHT,
		WINDOW_WIDTH,
		WINDOW_MAXIMIZED,
		WELCOME_DIALOG,

		NUMERICAL_HOTKEYS,
		RECENT_FILES,

		RECENT_EDITED_MAP_PATH,
		RECENT_EDITED_MAP_POSITION,

		FIND_ITEM_MODE,
		JUMP_TO_ITEM_MODE,

		SHOW_TOOLBAR_STANDARD,
		SHOW_TOOLBAR_BRUSHES,
		SHOW_TOOLBAR_POSITION,
		SHOW_TOOLBAR_SIZES,
		TOOLBAR_STANDARD_LAYOUT,
		TOOLBAR_BRUSHES_LAYOUT,
		TOOLBAR_POSITION_LAYOUT,
		TOOLBAR_SIZES_LAYOUT,

		LAST,
	};
}

class wxConfigBase;

class Settings {
public:
	Settings();
	~Settings();

	bool getBoolean(uint32_t key) const;
	int getInteger(uint32_t key) const;
	float getFloat(uint32_t key) const;
	std::string getString(uint32_t key) const;

	void setInteger(uint32_t key, int newval);
	void setFloat(uint32_t key, float newval);
	void setString(uint32_t key, std::string newval);

	wxConfigBase& getConfigObject();
	void setDefaults() {IO(DEFAULT);}
	void load();
	void save(bool endoftheworld = false);
public:
	enum DynamicType {
		TYPE_NONE,
		TYPE_STR,
		TYPE_INT,
		TYPE_FLOAT,
	};
	class DynamicValue {
	public:
		DynamicValue() : type(TYPE_NONE) {
			intval = 0;
		};
		DynamicValue(DynamicType t) : type(t) {
			if(t == TYPE_STR) strval = nullptr;
			else if(t == TYPE_INT) intval = 0;
			else if(t == TYPE_FLOAT) floatval = 0.0;
			else intval = 0;
		};
		~DynamicValue() {
			if(type == TYPE_STR)
				delete strval;
		}
		DynamicValue(const DynamicValue& dv) : type(dv.type) {
			if(dv.type == TYPE_STR) strval = newd std::string(*dv.strval);
			else if(dv.type == TYPE_INT) intval = dv.intval;
			else if(dv.type == TYPE_FLOAT) floatval = dv.floatval;
			else intval = 0;
		};

		std::string str();
	private:
		DynamicType type;
		union {
			int intval;
			std::string* strval;
			float floatval;
		};

		friend class Settings;
	};
private:
	enum IOMode {
		DEFAULT,
		LOAD,
		SAVE,
	};
	void IO(IOMode mode);
	std::vector<DynamicValue> store;
#ifdef __WINDOWS__
	bool use_file_cfg;
#endif
};

extern Settings g_settings;

#endif
