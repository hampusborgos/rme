//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/settings.h $
// $Id: settings.h 323 2010-04-12 20:59:41Z admin $

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
		USE_MEMCACHED_SPRITES,
		USE_MEMCACHED_SPRITES_TO_SAVE,
		SOFTWARE_CLEAN_THRESHOLD,
		SOFTWARE_CLEAN_SIZE,
		TRANSPARENT_FLOORS,
		TRANSPARENT_ITEMS,
		SHOW_INGAME_BOX,
		SHOW_GRID,
		SHOW_EXTRA,
		SHOW_ALL_FLOORS,
		SHOW_CREATURES,
		SHOW_SPAWNS,
		SHOW_HOUSES,
		SHOW_SHADE,
		SHOW_SPECIAL_TILES,
		HIGHLIGHT_ITEMS,
		SHOW_ITEMS,
		SHOW_BLOCKING,
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
		USE_OTGZ,
		SAVE_WITH_OTB_MAGIC_NUMBER,
		
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
		
		TIBIA_DATA_DIRS,
		DEFAULT_CLIENT_VERSION,
		CHECK_SIGNATURES,
		
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
		MAX_SPAWN_RADIUS,
		CURRENT_SPAWN_RADIUS,
		AUTO_CREATE_SPAWN,
		DEFAULT_SPAWNTIME,
		CREATE_MAP_ON_STARTUP,
		SWITCH_MOUSEBUTTONS,
		DOUBLECLICK_PROPERTIES,
		LISTBOX_EATS_ALL_EVENTS,
		RAW_LIKE_SIMONE,
		WORKER_THREADS,

		GOTO_WEBSITE_ON_BOOT,
		INDIRECTORY_INSTALLATION,
		AUTOCHECK_FOR_UPDATES,
		ONLY_ONE_INSTANCE,

		PALETTE_LAYOUT,
		MINIMAP_VISIBLE,
		MINIMAP_LAYOUT,
		MINIMAP_UPDATE_DELAY,
		MINIMAP_VIEW_BOX,
		WINDOW_HEIGHT,
		WINDOW_WIDTH,
		WINDOW_MAXIMIZED,

		NUMERICAL_HOTKEYS,
		RECENT_FILES,

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

extern Settings settings;

#endif
