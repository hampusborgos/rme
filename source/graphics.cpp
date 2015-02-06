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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/graphics.hpp $
// $Id: graphics.hpp 310 2010-02-26 18:03:48Z admin $


#include "main.h"

#include "sprites.h"
#include "graphics.h"
#include "filehandle.h"
#include "settings.h"
#include "gui.h"

#include <wx/mstream.h>
#include "pngfiles.h"

// All 133 template colors
static uint32_t TemplateOutfitLookupTable[] = {
	0xFFFFFF, 0xFFD4BF, 0xFFE9BF, 0xFFFFBF, 0xE9FFBF, 0xD4FFBF,
	0xBFFFBF, 0xBFFFD4, 0xBFFFE9, 0xBFFFFF, 0xBFE9FF, 0xBFD4FF,
	0xBFBFFF, 0xD4BFFF, 0xE9BFFF, 0xFFBFFF, 0xFFBFE9, 0xFFBFD4,
	0xFFBFBF, 0xDADADA, 0xBF9F8F, 0xBFAF8F, 0xBFBF8F, 0xAFBF8F,
	0x9FBF8F, 0x8FBF8F, 0x8FBF9F, 0x8FBFAF, 0x8FBFBF, 0x8FAFBF,
	0x8F9FBF, 0x8F8FBF, 0x9F8FBF, 0xAF8FBF, 0xBF8FBF, 0xBF8FAF,
	0xBF8F9F, 0xBF8F8F, 0xB6B6B6, 0xBF7F5F, 0xBFAF8F, 0xBFBF5F,
	0x9FBF5F, 0x7FBF5F, 0x5FBF5F, 0x5FBF7F, 0x5FBF9F, 0x5FBFBF,
	0x5F9FBF, 0x5F7FBF, 0x5F5FBF, 0x7F5FBF, 0x9F5FBF, 0xBF5FBF,
	0xBF5F9F, 0xBF5F7F, 0xBF5F5F, 0x919191, 0xBF6A3F, 0xBF943F,
	0xBFBF3F, 0x94BF3F, 0x6ABF3F, 0x3FBF3F, 0x3FBF6A, 0x3FBF94,
	0x3FBFBF, 0x3F94BF, 0x3F6ABF, 0x3F3FBF, 0x6A3FBF, 0x943FBF,
	0xBF3FBF, 0xBF3F94, 0xBF3F6A, 0xBF3F3F, 0x6D6D6D, 0xFF5500,
	0xFFAA00, 0xFFFF00, 0xAAFF00, 0x54FF00, 0x00FF00, 0x00FF54,
	0x00FFAA, 0x00FFFF, 0x00A9FF, 0x0055FF, 0x0000FF, 0x5500FF,
	0xA900FF, 0xFE00FF, 0xFF00AA, 0xFF0055, 0xFF0000, 0x484848,
	0xBF3F00, 0xBF7F00, 0xBFBF00, 0x7FBF00, 0x3FBF00, 0x00BF00,
	0x00BF3F, 0x00BF7F, 0x00BFBF, 0x007FBF, 0x003FBF, 0x0000BF,
	0x3F00BF, 0x7F00BF, 0xBF00BF, 0xBF007F, 0xBF003F, 0xBF0000,
	0x242424, 0x7F2A00, 0x7F5500, 0x7F7F00, 0x557F00, 0x2A7F00,
	0x007F00, 0x007F2A, 0x007F55, 0x007F7F, 0x00547F, 0x002A7F,
	0x00007F, 0x2A007F, 0x54007F, 0x7F007F, 0x7F0055, 0x7F002A,
	0x7F0000,
};

GraphicManager::GraphicManager() :
	client_version(nullptr),
	unloaded(true),
	dat_format(DAT_FORMAT_UNKNOWN),
	is_extended(false),
	loaded_textures(0),
	lastclean(0)
{
	// ...
}

GraphicManager::~GraphicManager()
{
	for(SpriteMap::iterator iter = sprite_space.begin();
			iter != sprite_space.end();
			++iter)
	{
		delete iter->second;
	}

	for(ImageMap::iterator iter = image_space.begin();
			iter != image_space.end();
			++iter)
	{
		delete iter->second;
	}
}

bool GraphicManager::isUnloaded() const
{
	return unloaded;
}

GLuint GraphicManager::getFreeTextureID()
{
	static GLuint id_counter = 0x10000000;
	return id_counter++; // This should (hopefully) never run out
}

void GraphicManager::clear()
{
	SpriteMap new_sprite_space;
	for(SpriteMap::iterator iter = sprite_space.begin();
			iter != sprite_space.end();
			++iter)
	{
		if(iter->first >= 0) { // Don't clean internal sprites
			delete iter->second;
		} else {
			new_sprite_space.insert(std::make_pair(iter->first, iter->second));
		}
	}

	for(ImageMap::iterator iter = image_space.begin();
			iter != image_space.end();
			++iter)
	{
		delete iter->second;
	}

	sprite_space.swap(new_sprite_space);
	image_space.clear();
	cleanup_list.clear();

	item_count = 0;
	creature_count = 0;
	loaded_textures = 0;
	lastclean = time(nullptr);
	spritefile = "";

	unloaded = true;
}

void GraphicManager::cleanSoftwareSprites()
{
	for(SpriteMap::iterator iter = sprite_space.begin();
			iter != sprite_space.end();
			++iter)
	{
		if(iter->first >= 0) { // Don't clean internal sprites
			iter->second->unloadDC();
		}
	}
}

Sprite* GraphicManager::getSprite(int id)
{
	SpriteMap::iterator it = sprite_space.find(id);
	if(it != sprite_space.end())
	{
		return it->second;
	}
	return nullptr;
}

GameSprite* GraphicManager::getCreatureSprite(int id)
{
	if(id < 0)
	{
		return nullptr;
	}

	SpriteMap::iterator it = sprite_space.find(id+item_count);
	if(it != sprite_space.end())
	{
		return static_cast<GameSprite*>(it->second);
	}
	return nullptr;
}


uint16_t GraphicManager::getItemSpriteMaxID() const
{
	return item_count;
}

uint16_t GraphicManager::getCreatureSpriteMaxID() const
{
	return creature_count;
}

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length)
{
	wxMemoryInputStream is(data, length);
	wxImage img(is, wxT("image/png"));
	if(!img.IsOk()) return nullptr;
	return newd wxBitmap(img, -1);
}

bool GraphicManager::loadEditorSprites()
{
	// Unused graphics MIGHT be loaded here, but it's a neglectable loss
	sprite_space[EDITOR_SPRITE_SELECTION_MARKER] =
		newd EditorSprite(
			newd wxBitmap(selection_marker_xpm16x16),
			newd wxBitmap(selection_marker_xpm32x32)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_1x1] =
		newd EditorSprite(
			loadPNGFile(circular_1_small_png),
			loadPNGFile(circular_1_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_3x3] =
		newd EditorSprite(
			loadPNGFile(circular_2_small_png),
			loadPNGFile(circular_2_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_5x5] =
		newd EditorSprite(
			loadPNGFile(circular_3_small_png),
			loadPNGFile(circular_3_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_7x7] =
		newd EditorSprite(
			loadPNGFile(circular_4_small_png),
			loadPNGFile(circular_4_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_9x9] =
		newd EditorSprite(
			loadPNGFile(circular_5_small_png),
			loadPNGFile(circular_5_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_15x15] =
		newd EditorSprite(
			loadPNGFile(circular_6_small_png),
			loadPNGFile(circular_6_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_CD_19x19] =
		newd EditorSprite(
			loadPNGFile(circular_7_small_png),
			loadPNGFile(circular_7_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_1x1] =
		newd EditorSprite(
			loadPNGFile(rectangular_1_small_png),
			loadPNGFile(rectangular_1_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_3x3] =
		newd EditorSprite(
			loadPNGFile(rectangular_2_small_png),
			loadPNGFile(rectangular_2_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_5x5] =
		newd EditorSprite(
			loadPNGFile(rectangular_3_small_png),
			loadPNGFile(rectangular_3_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_7x7] =
		newd EditorSprite(
			loadPNGFile(rectangular_4_small_png),
			loadPNGFile(rectangular_4_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_9x9] =
		newd EditorSprite(
			loadPNGFile(rectangular_5_small_png),
			loadPNGFile(rectangular_5_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_15x15] =
		newd EditorSprite(
			loadPNGFile(rectangular_6_small_png),
			loadPNGFile(rectangular_6_png)
		);
	sprite_space[EDITOR_SPRITE_BRUSH_SD_19x19] =
		newd EditorSprite(
			loadPNGFile(rectangular_7_small_png),
			loadPNGFile(rectangular_7_png)
		);

	sprite_space[EDITOR_SPRITE_OPTIONAL_BORDER_TOOL] =
		newd EditorSprite(
			loadPNGFile(optional_border_small_png),
			loadPNGFile(optional_border_png)
		);
	sprite_space[EDITOR_SPRITE_ERASER] =
		newd EditorSprite(
			loadPNGFile(eraser_small_png),
			loadPNGFile(eraser_png)
		);
	sprite_space[EDITOR_SPRITE_PZ_TOOL] =
		newd EditorSprite(
			loadPNGFile(protection_zone_small_png),
			loadPNGFile(protection_zone_png)
		);
	sprite_space[EDITOR_SPRITE_PVPZ_TOOL] =
		newd EditorSprite(
			loadPNGFile(pvp_zone_small_png),
			loadPNGFile(pvp_zone_png)
		);
	sprite_space[EDITOR_SPRITE_NOLOG_TOOL] =
		newd EditorSprite(
			loadPNGFile(no_logout_small_png),
			loadPNGFile(no_logout_png)
		);
	sprite_space[EDITOR_SPRITE_NOPVP_TOOL] =
		newd EditorSprite(
			loadPNGFile(no_pvp_small_png),
			loadPNGFile(no_pvp_png)
		);

	sprite_space[EDITOR_SPRITE_DOOR_NORMAL] =
		newd EditorSprite(
			loadPNGFile(door_normal_small_png),
			loadPNGFile(door_normal_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_LOCKED] =
		newd EditorSprite(
			loadPNGFile(door_locked_small_png),
			loadPNGFile(door_locked_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_MAGIC] =
		newd EditorSprite(
			loadPNGFile(door_magic_small_png),
			loadPNGFile(door_magic_png)
		);
	sprite_space[EDITOR_SPRITE_DOOR_QUEST] =
		newd EditorSprite(
			loadPNGFile(door_quest_small_png),
			loadPNGFile(door_quest_png)
		);
	sprite_space[EDITOR_SPRITE_WINDOW_NORMAL] =
		newd EditorSprite(
			loadPNGFile(window_normal_small_png),
			loadPNGFile(window_normal_png)
		);
	sprite_space[EDITOR_SPRITE_WINDOW_HATCH] =
		newd EditorSprite(
			loadPNGFile(window_hatch_small_png),
			loadPNGFile(window_hatch_png)
		);

	sprite_space[EDITOR_SPRITE_SELECTION_GEM] =
		newd EditorSprite(
			loadPNGFile(gem_edit_png),
			nullptr
		);
	sprite_space[EDITOR_SPRITE_DRAWING_GEM] =
		newd EditorSprite(
			loadPNGFile(gem_move_png),
			nullptr
		);

	return true;
}

bool GraphicManager::loadSpriteMetadata(const FileName& datafile, wxString& error, wxArrayString& warnings)
{
	// items.otb has most of the info we need. This only loads the GameSprite metadata
	FileReadHandle file(nstr(datafile.GetFullPath()));

	if(file.isOk() == false) {
		error += wxT("Failed to open ") + datafile.GetFullPath() + wxT(" for reading\nThe error reported was:") + wxstr(file.getErrorMessage());
		return false;
	}

	uint16_t effect_count, distance_count;

	uint32_t datSignature;
	file.getU32(datSignature);
	//get max id
	file.getU16(item_count);
	file.getU16(creature_count);
	file.getU16(effect_count);
	file.getU16(distance_count);

	uint32_t minclientID = 100; // tibia.dat start with id 100
	// We don't load distance/effects, if we would, just add effect_count & distance_count here
	uint32_t maxclientID = item_count + creature_count;

	bool (GraphicManager::*loadFlags)(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	dat_format = client_version->getDatFormatForSignature(datSignature);
	switch (dat_format)
	{
		case DAT_FORMAT_74: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer74; break;
		case DAT_FORMAT_76: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer76; break;
		case DAT_FORMAT_78: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer78; break;
		case DAT_FORMAT_86: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer86; break;
			// Same .dat loader, the change is only sprite id -> u32
		case DAT_FORMAT_96: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer86; break;
		case DAT_FORMAT_1010: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer1010; break;
		case DAT_FORMAT_1021:
		case DAT_FORMAT_1050:
		case DAT_FORMAT_1056: loadFlags = &GraphicManager::loadSpriteMetadataFlagsVer1021; break;
		default:
			error = wxT("Unknown .dat format version.");
			return false;
	}

	is_extended = (dat_format >= DAT_FORMAT_96 || settings.getInteger(Config::SPR_EXTENDED));

	uint16_t id = minclientID;
	// loop through all ItemDatabase until we reach the end of file
	while(id <= maxclientID)
	{
		GameSprite* sType = newd GameSprite();
		sprite_space[id] = sType;

		sType->id = id;

		// Load the sprite flags
		if (!(this->*loadFlags)(file, sType, error, warnings))
		{
			wxString msg;
			msg << wxT("Failed to load flags for sprite ") << sType->id;
			warnings.push_back(msg);
		}

		bool has_frame_groups = (dat_format == DAT_FORMAT_1056 && id > item_count);
		uint8_t group_count = 1;

		// Reads the group count
		if (has_frame_groups) {
			file.getU8(group_count);
		}

		for (uint32_t k = 0; k < group_count; ++k)
		{
			// Skipping the group type
			if (has_frame_groups) {
				file.skip(1);
			}

			// Size and GameSprite data
			file.getByte(sType->width);
			file.getByte(sType->height);

			// Skipping the exact size
			if ((sType->width > 1) || (sType->height > 1)){
				file.skip(1);
			}

			file.getU8(sType->layers); // Number of blendframes (some sprites consist of several merged sprites)
			file.getU8(sType->pattern_x);
			file.getU8(sType->pattern_y);
			if (dat_format <= DAT_FORMAT_74)
				sType->pattern_z = 1;
			else
				file.getU8(sType->pattern_z);
			file.getU8(sType->frames); // Length of animation

			// Skipping the frame duration
			if (dat_format >= DAT_FORMAT_1050 && sType->frames > 1) {
				file.skip(6 + 8 * sType->frames);
			}

			sType->numsprites =
				(int)sType->width * (int)sType->height *
				(int)sType->layers *
				(int)sType->pattern_x * (int)sType->pattern_y * sType->pattern_z *
				(int)sType->frames;

			// Read the sprite ids
			for (uint32_t i = 0; i < sType->numsprites; ++i)
			{
				uint32_t sprite_id;
				if (is_extended) {
					file.getU32(sprite_id);
				} else {
					uint16_t u16 = 0;
					file.getU16(u16);
					sprite_id = u16;
				}

				if (image_space[sprite_id] == nullptr)
				{
					GameSprite::NormalImage* img = newd GameSprite::NormalImage();
					img->id = sprite_id;
					image_space[sprite_id] = img;
				}
				sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(image_space[sprite_id]));
			}
		}
		++id;
	}

	return true;
}

bool GraphicManager::loadSpriteMetadataFlagsVer74(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	// read the options until we find a 0xff
	uint8_t lastopt;
	uint8_t optbyte = 0xff;

	do
	{
		lastopt = optbyte;
		file.getByte(optbyte);
		switch(optbyte)
		{
			case 0x00:
				//is groundtile
				file.skip(2);
				break;
			case 0x01: // all OnTop
				break;
			case 0x02: // can walk trough (open doors, arces, bug pen fence ??)
				break;
			case 0x03:
				//is a container
				break;
			case 0x04:
				//is stackable
				break;
			case 0x05:
				//is useable
				break;
			case 0x06: // ladder up (id 1386)   why a group for just 1 item ???   
				break;
			case 0x07: // writtable objects
				file.skip(2);
				break;
			case 0x08: // writtable objects that can't be edited 
				file.skip(2);
				break;
			case 0x09: //can contain fluids
				break;
			case 0x0A:
				//is multitype !!! wrong definition (only water splash on floor)
				break;
			case 0x0B:
				//is blocking
				break;
			case 0x0C:
				//is on moveable
				break;
			case 0x0D: // blocks missiles (walls, magic wall etc)
				//iType->blockingProjectile = true;
				break;
			case 0x0E: // blocks monster movement (flowers, parcels etc)
				break;
			case 0x0F:
				//can be equipped
				break;
			case 0x10:
				//makes light (skip 4 bytes)
				file.skip(4);
				break;
			case 0x11: // can see what is under (ladder holes, stairs holes etc)
				break;
			case 0x12: // ground tiles that don't cause level change
				//iType->noFloorChange = true;
				break;
			case 0x13: // mostly blocking items, but also items that can pile up in level (boxes, chairs etc)
				file.skip(2);
				break;
			case 0x14: // player color templates
				break;
			case 0x18: // cropses that don't decay
				break;
			case 0x16: // ground, blocking items and mayby some more 
				file.skip(2);
				break;
			case 0x17:  // seems like decorables with 4 states of turning (exception first 4 are unique statues)
				break;
			case 0x19:  // wall items
				break;
			case 0x1A: 
				//7.4 (change no data ?? ) action that can be performed (doors-> open, hole->open, book->read) not all included ex. wall torches
				break;
			case 0x1B:  // walls 2 types of them same material (total 4 pairs)
				break;
			case 0x1C:  // ?? 
				break;
			case 0x1D:  // line spot ...
				file.skip(2);
				break;
			case 0xFF:
				break;
			default:
			{
				wxString err;
				err << wxT("Tibia.dat: Unknown optbyte '") 
					<< optbyte << wxT("'") << wxT(" after '") 
					<< lastopt << wxT("'");
				warnings.push_back(err);
				break;
			}
		}
	} while (optbyte != 0xff);
	return true;
}

bool GraphicManager::loadSpriteMetadataFlagsVer76(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	// read the options until we find a 0xff
	uint8_t lastopt;
	uint8_t optbyte = 0xff;

	do
	{
		lastopt = optbyte;
		file.getByte(optbyte);
		switch (optbyte)
		{
			case 0x00: //is groundtile
			{
				file.skip(2);
				break;
			}
			case 0x01: //all on top
			{
				//sType->alwaysOnTop = true;
				break;
			}
			case 0x02: //can walk trough (open doors, arces, bug pen fence ??)
			{
				//sType->alwaysOnTop = true;
				//sType->canWalkThrough = true;
				break;
			}
			case 0x03: //is a container
			{
				//sType->group = ITEM_GROUP_CONTAINER;
				break;
			}
			case 0x04: //is stackable
			{
				//sType->stackable = true;
				break;
			}
			case 0x05: //is useable
			{
				//sType->useable = true;
				break;
			}
			case 0x06: //ladder up (id 1386)   why a group for just 1 item ???   
			{
				//sType->ladderUp = true;
				break;
			}
			case 0x07: //writtable objects
			{
				//file.skip(2); // Maximum length
				break;
			}
			case 0x08: //writtable objects that can't be edited
			{
				file.skip(2);
				break;
			}
			case 0x09: //can contain fluids
			{
				file.skip(2);
				//sType->group = ITEM_GROUP_FLUID;
				break;
			}
			case 0x0A: //liquid with states
			{
				//sType->group = ITEM_GROUP_SPLASH;
				break;
			}
			case 0x0B: //is blocking
			{
				//sType->blockSolid = true;
				break;
			}
			case 0x0C: //is not moveable
			{
				//sType->moveable = false;
				break;
			}
			case 0x0D: //"visibility"- for drawing visible view
			{
				//sType->blockProjectile = true;
				break;
			}
			case 0x0E: //blocks creature movement (flowers, parcels etc)
			{
				//sType->blockPathFind = true;
				break;
			}
			case 0x0F: //can be equipped
			{
				//sType->pickupable = true;
				break;
			}
			case 0x10: //makes light (skip 4 bytes)
			{
				break;
			}
			case 0x11: //can see what is under (ladder holes, stairs holes etc)
			{
				//sType->canSeeThrough = true;
				break;
			}
			case 0x12: //ground tiles that don't cause level change
			{
				//sType->floorchange = false;
				break;
			}
			case 0x13: // isVertical
			{
				break;
			}
			case 0x14: //sprite-drawing related
			{
				//sType->hasParameter14 = true;
				break;
			}
			case 0x15:
			{
				file.skip(2); // lightlevel
				file.skip(2); // lightcolor
				break;
			}
			case 0x16: //minimap drawing
			{
				uint16_t u16;
				file.getU16(u16);
				sType->minimap_color = u16;
				break;
			}
			case 0x17: //seems like decorables with 4 states of turning (exception first 4 are unique statues
			{
				//sType->rotable = true;
				break;
			}
			case 0x18: //draw with height offset for all parts (2x2) of the sprite
			{
				file.skip(4);
				break;
			}
			case 0x19: //hangable objects
			{
				file.skip(2);
				break;
			}
			case 0x1A: //vertical objects (walls to hang objects on etc)
			{
				//sType->isHorizontal = true;
				break;  
			}
			case 0x1B: //walls 2 types of them same material (total 4 pairs)
			{
				//sType->isVertical = true;
				break;
			}
			case 0x1C: // minimap color
			{
				uint16_t u16;
				file.getU16(u16);
				sType->minimap_color = u16; // Height
				break;
			}
			case 0x1D: //line spot ...
			{
				file.skip(2); // ...?
				break;
			}
			case 0x1E: // ground items
			{
				break;
			}
			case 0xFF:
			{
				break;
			}
			default:
			{
				wxString err;
				err << wxT("Tibia.dat: Unknown optbyte '") 
					<< (int)optbyte << wxT("'") << wxT(" after '") 
					<< (int)lastopt << wxT("'");
				warnings.push_back(err);
				break;
			}
		}
	} while (optbyte != 0xff);
	return true;
}

bool GraphicManager::loadSpriteMetadataFlagsVer78(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	// read the options until we find a 0xff
	uint8_t lastopt;
	uint8_t optbyte = 0xff;

	do
	{
		lastopt = optbyte;
		file.getByte(optbyte);
		// > 7.6
		switch(optbyte)
		{
			case 0x00: //is groundtile
				file.skip(2); // speed modifier
				//speed = read_short;
				//sType->speed = speed;
				//sType->group = ITEM_GROUP_GROUND;
				break;
			case 0x01: //all OnTop
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 1;
				break;
			case 0x02: //can walk through
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 2;
				break;
			case 0x03: //can walk through
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 3;
				break;
			case 0x04: //is a container
				//sType->group = ITEM_GROUP_CONTAINER;
				break;
			case 0x05: //is stackable
				//sType->stackable = true;
				break;
			case 0x06: //ladders
				break;
			case 0x07: //is useable
				//sType->useable = true;
				break;
			case 0x08: //runes
				//sType->group = ITEM_GROUP_RUNE;
				break;
			case 0x09: //writeable objects
				//sType->group = ITEM_GROUP_WRITEABLE;
				//sType->readable = true;
				file.skip(2);
				//file.getU16(); //maximum text length?
				break;
			case 0x0A: //writeable objects that can't be edited
				//sType->readable = true;
				file.skip(2);
				//file.getU16(); //maximum text length?
				break;
			case 0x0B: //can contain fluids
				//sType->group = ITEM_GROUP_FLUID;
				break;
			case 0x0C: //liquid with states
				//sType->group = ITEM_GROUP_SPLASH;
				break;
			case 0x0D: //is blocking
				//sType->blockSolid = true;
				break;
			case 0x0E: //is not moveable
				//sType->moveable = false;
				break;
			case 0x0F: //blocks missiles (walls, magic wall etc)
				//sType->blockProjectile = true;
				break;
			case 0x10: //blocks monster movement (flowers, parcels etc)
				//sType->blockPathFind = true;
				break;
			case 0x11: //can be equipped
				//sType->pickupable = true;
				break;
			case 0x12: //wall items
				//sType->isHangable = true;
				break;
			case 0x13:
				//sType->isHorizontal = true;
				break;
			case 0x14:
				//sType->isVertical = true;
				break;
			case 0x15: //rotateable items
				//sType->rotateable = true;
				break;
			case 0x16: //light info ..
				file.skip(2);
				file.skip(2);
				//file.getU16(); // level
				//file.getU16(); // color
				//sType->lightColor = lightcolor;
				break;
			case 0x17: // No object has this...
				break;
			case 0x18: //floor change down
				break;
			case 0x19: { //Draw offset
				uint16_t x;
				uint16_t y;
				file.getU16(x);
				file.getU16(y);

				sType->drawoffset_x = x;
				sType->drawoffset_y = y;
			} break;
			case 0x1A: {
				uint16_t u16;
				file.getU16(u16);
				sType->draw_height = u16; // Height
			} break;
			case 0x1B://draw with height offset for all parts (2x2) of the sprite
				break;
			case 0x1C: // offset life-bar (for larger monsters)
				break;
			case 0x1D: { // Minimap color
				uint16_t u16;
				file.getU16(u16);
				sType->minimap_color = u16;
			} break;
			case 0x1E:  {// Floor change?
				file.skip(2);
				//file.getByte(); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,
				//file.getByte(); // always 4
			} break;
			case 0x1F:
				break;
			case 0x20: // LookThrough
				break;
			case 0xFF:
				break;
			default: {
				wxString err;
				err << wxT("Tibia.dat: Unknown optbyte '") 
					<< (int)optbyte << wxT("'") << wxT(" after '") 
					<< (int)lastopt << wxT("'");
				warnings.push_back(err);

				return false;
			} break;
		}
	} while (optbyte != 0xff);
	return true;
}

bool GraphicManager::loadSpriteMetadataFlagsVer86(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	// read the options until we find a 0xff
	uint8_t lastopt;
	uint8_t optbyte = 0xff;

	do
	{
		lastopt = optbyte;
		file.getByte(optbyte);
		switch(optbyte)
		{
			case 0x00: //is groundtile
				file.skip(2); // speed modifier
				//speed = read_short;
				//sType->speed = speed;
				//sType->group = ITEM_GROUP_GROUND;
				break;
			case 0x01: //all OnTop
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 1;
				break;
			case 0x02: //can walk through
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 2;
				break;
			case 0x03: //can walk through
				//sType->alwaysOnTop = true;
				//sType->alwaysOnTopOrder = 3;
				break;
			case 0x04: //is a container
				//sType->group = ITEM_GROUP_CONTAINER;
				break;
			case 0x05: //is stackable
				//sType->stackable = true;
				break;
			case 0x06: //ladders
				break;
			case 0x07: //is useable
				//sType->useable = true;
				break;
			case 0x08: //writeable objects
				//sType->group = ITEM_GROUP_WRITEABLE;
				//sType->readable = true;
				file.skip(2);
				//file.getU16(); //maximum text length?
				break;
			case 0x09: //writeable objects that can't be edited
				//sType->readable = true;
				file.skip(2);
				//file.getU16(); //maximum text length?
				break;
			case 0x0A: //can contain fluids
				//sType->group = ITEM_GROUP_FLUID;
				break;
			case 0x0B: //liquid with states
				//sType->group = ITEM_GROUP_SPLASH;
				break;
			case 0x0C: //is blocking
				//sType->blockSolid = true;
				break;
			case 0x0D: //is not moveable
				//sType->moveable = false;
				break;
			case 0x0E: //blocks missiles (walls, magic wall etc)
				//sType->blockProjectile = true;
				break;
			case 0x0F: //blocks monster movement (flowers, parcels etc)
				//sType->blockPathFind = true;
				break;
			case 0x10: //can be equipped
				//sType->pickupable = true;
				break;
			case 0x11: //wall items
				//sType->isHangable = true;
				break;
			case 0x12:
				//sType->isHorizontal = true;
				break;
			case 0x13:
				//sType->isVertical = true;
				break;
			case 0x14: //rotateable items
				//sType->rotateable = true;
				break;
			case 0x15: //light info ..
				file.skip(2);
				file.skip(2);
				//file.getU16(); // level
				//file.getU16(); // color
				//sType->lightColor = lightcolor;
				break;
			case 0x16: // No object has this...
				break;
			case 0x17: //floor change down
				break;
			case 0x18: { //Draw offset
				uint16_t x;
				uint16_t y;
				file.getU16(x);
				file.getU16(y);

				sType->drawoffset_x = x;
				sType->drawoffset_y = y;
			} break;
			case 0x19: {
				uint16_t u16;
				file.getU16(u16);
				sType->draw_height = u16; // Height
			} break;
			case 0x1A://draw with height offset for all parts (2x2) of the sprite
				break;
			case 0x1B: // offset life-bar (for larger monsters)
				break;
			case 0x1C: { // Minimap color
				uint16_t u16;
				file.getU16(u16);
				sType->minimap_color = u16;
			} break;
			case 0x1D:  {// Floor change?
				file.skip(2);
				//file.getByte(); // 86 -> openable holes, 77-> can be used to go down, 76 can be used to go up, 82 -> stairs up, 79 switch,
				//file.getByte(); // always 4
			} break;
			case 0x1E:
				break;
			case 0x1F: // LookThrough
				break;
			// All stuff below is hacky speculation
			case 0x20: // Cloth slot
				file.skip(2); // clothSlot
				break;
			case 0x21: // Market
				{
				/* marketCategory = */ file.skip(2);
                /* marketTradeAs  = */ file.skip(2);
                /* marketShowAs = */ file.skip(2);
				std::string marketName;
				file.getString(marketName);
                /* marketRestrictProfession = */ file.skip(2);
                /* marketRestrictLevel = */ file.skip(2);
                break;
			}
			case 0xFF:
				break;
			default: {
				wxString err;
				err << wxT("Tibia.dat: Unknown optbyte '") 
					<< (int)optbyte << wxT("'") << wxT(" after '") 
					<< (int)lastopt << wxT("'");
				warnings.push_back(err);

				return false;
			} break;
		}
	} while (optbyte != 0xff);
	return true;
}

bool GraphicManager::loadSpriteMetadataFlagsVer1010(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	uint8_t currentByte;
	uint8_t lastByte = VER_1010_FLAG_LAST;
	while (true) {
		file.getByte(currentByte);
		switch (currentByte) {
			case VER_1010_FLAG_BANK:
				file.skip(2);
				break;
			case VER_1010_FLAG_CLIP:
				break;
			case VER_1010_FLAG_BOTTOM:
				break;
			case VER_1010_FLAG_TOP:
				break;
			case VER_1010_FLAG_CONTAINER:
				break;
			case VER_1010_FLAG_CUMULATIVE:
				break;
			case VER_1010_FLAG_FORCEUSE:
				break;
			case VER_1010_FLAG_MULTIUSE:
				break;
			case VER_1010_FLAG_WRITE:
			case VER_1010_FLAG_WRITEONCE:
				file.skip(2);
				break;
			case VER_1010_FLAG_LIQUIDCONTAINER:
				break;
			case VER_1010_FLAG_LIQUIDPOOL:
				break;
			case VER_1010_FLAG_UNPASS:
				break;
			case VER_1010_FLAG_UNMOVE:
				break;
			case VER_1010_FLAG_UNSIGHT:
				break;
			case VER_1010_FLAG_AVOID:
				break;
			case VER_1010_FLAG_NOMOVEMENTANIMATION:
				break;
			case VER_1010_FLAG_TAKE:
				break;
			case VER_1010_FLAG_HANG:
				break;
			case VER_1010_FLAG_HOOKSOUTH:
				break;
			case VER_1010_FLAG_HOOKEAST:
				break;
			case VER_1010_FLAG_ROTATE:
				break;
			case VER_1010_FLAG_LIGHT: {
				file.skip(2);
				file.skip(2);
				break;
			}
			case VER_1010_FLAG_DONTHIDE:
				break;
			case VER_1010_FLAG_TRANSLUCENT:
				break;
			case VER_1010_FLAG_SHIFT: {
				file.getU16(sType->drawoffset_x);
				file.getU16(sType->drawoffset_y);
				break;
			}
			case VER_1010_FLAG_HEIGHT:
				file.getU16(sType->draw_height);
				break;
			case VER_1010_FLAG_LYINGOBJECT:
				break;
			case VER_1010_FLAG_ANIMATEALWAYS:
				break;
			case VER_1010_FLAG_AUTOMAP:
				file.getU16(sType->minimap_color);
				break;
			case VER_1010_FLAG_LENSHELP:
				file.skip(2);
				break;
			case VER_1010_FLAG_FULLBANK:
				break;
			case VER_1010_FLAG_IGNORELOOK:
				break;
			case VER_1010_FLAG_CLOTHES:
				file.skip(2);
				break;
			case VER_1010_FLAG_MARKET: {
				/* marketCategory = */ file.skip(2);
				/* marketTradeAs  = */ file.skip(2);
				/* marketShowAs = */ file.skip(2);
				std::string marketName;
				file.getString(marketName);
				/* marketRestrictProfession = */ file.skip(2);
				/* marketRestrictLevel = */ file.skip(2);
				break;
			}
			case VER_1010_FLAG_ANIMATION:
				break;
			case VER_1010_FLAG_LAST:
				return true;
			default: {
				warnings.push_back(
					wxT("Tibia.dat: Unknown optbyte '") + std::to_string(int32_t(currentByte)) + wxT("'") +
					wxT(" after '") + std::to_string(int32_t(lastByte)) + wxT("'")
				);
				return false;
			}
		}
		lastByte = currentByte;
	}
}

bool GraphicManager::loadSpriteMetadataFlagsVer1021(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	uint8_t currentByte;
	uint8_t lastByte = VER_1021_FLAG_LAST;
	while (true) {
		file.getByte(currentByte);
		switch (currentByte) {
			case VER_1021_FLAG_BANK:
				file.skip(2);
				break;
			case VER_1021_FLAG_CLIP:
				break;
			case VER_1021_FLAG_BOTTOM:
				break;
			case VER_1021_FLAG_TOP:
				break;
			case VER_1021_FLAG_CONTAINER:
				break;
			case VER_1021_FLAG_CUMULATIVE:
				break;
			case VER_1021_FLAG_FORCEUSE:
				break;
			case VER_1021_FLAG_MULTIUSE:
				break;
			case VER_1021_FLAG_WRITE:
			case VER_1021_FLAG_WRITEONCE:
				file.skip(2);
				break;
			case VER_1021_FLAG_LIQUIDCONTAINER:
				break;
			case VER_1021_FLAG_LIQUIDPOOL:
				break;
			case VER_1021_FLAG_UNPASS:
				break;
			case VER_1021_FLAG_UNMOVE:
				break;
			case VER_1021_FLAG_UNSIGHT:
				break;
			case VER_1021_FLAG_AVOID:
				break;
			case VER_1021_FLAG_NOMOVEMENTANIMATION:
				break;
			case VER_1021_FLAG_TAKE:
				break;
			case VER_1021_FLAG_HANG:
				break;
			case VER_1021_FLAG_HOOKSOUTH:
				break;
			case VER_1021_FLAG_HOOKEAST:
				break;
			case VER_1021_FLAG_ROTATE:
				break;
			case VER_1021_FLAG_LIGHT: {
				file.skip(2);
				file.skip(2);
				break;
			}
			case VER_1021_FLAG_DONTHIDE:
				break;
			case VER_1021_FLAG_TRANSLUCENT:
				break;
			case VER_1021_FLAG_SHIFT: {
				file.getU16(sType->drawoffset_x);
				file.getU16(sType->drawoffset_y);
				break;
			}
			case VER_1021_FLAG_HEIGHT:
				file.getU16(sType->draw_height);
				break;
			case VER_1021_FLAG_LYINGOBJECT:
				break;
			case VER_1021_FLAG_ANIMATEALWAYS:
				break;
			case VER_1021_FLAG_AUTOMAP:
				file.getU16(sType->minimap_color);
				break;
			case VER_1021_FLAG_LENSHELP:
				file.skip(2);
				break;
			case VER_1021_FLAG_FULLBANK:
				break;
			case VER_1021_FLAG_IGNORELOOK:
				break;
			case VER_1021_FLAG_CLOTHES:
				file.skip(2);
				break;
			case VER_1021_FLAG_MARKET: {
				/* marketCategory = */ file.skip(2);
				/* marketTradeAs  = */ file.skip(2);
				/* marketShowAs = */ file.skip(2);
				std::string marketName;
				file.getString(marketName);
				/* marketRestrictProfession = */ file.skip(2);
				/* marketRestrictLevel = */ file.skip(2);
				break;
			}
			case VER_1021_FLAG_DEFAULT_ACTION: {
				file.skip(2);
				break;
			}
			case VER_1021_FLAG_USABLE:
				break;
			case VER_1021_FLAG_LAST:
				return true;
			default: {
				warnings.push_back(
					wxT("Tibia.dat: Unknown optbyte '") + std::to_string(int32_t(currentByte)) + wxT("'") +
					wxT(" after '") + std::to_string(int32_t(lastByte)) + wxT("'")
				);
				return false;
			}
		}
		lastByte = currentByte;
	}
}

bool GraphicManager::loadSpriteData(const FileName& datafile, wxString& error, wxArrayString& warnings)
{
	FileReadHandle fh(nstr(datafile.GetFullPath()));

	if(fh.isOk() == false) {
		error = wxT("Failed to open file for reading");
		return false;
	}

#define safe_get(func, ...) do {\
		if(!fh.get##func(__VA_ARGS__)) {\
			error = wxstr(fh.getErrorMessage()); \
			return false; \
		} \
	} while(false)

	
	uint32_t sprSignature;
	safe_get(U32, sprSignature);

	uint32_t total_pics = 0;
	if (is_extended) {
		safe_get(U32, total_pics);
	} else {
		uint16_t u16 = 0;
		safe_get(U16, u16);
		total_pics = u16;
	}

	if(settings.getInteger(Config::USE_MEMCACHED_SPRITES) == false) {
		spritefile = nstr(datafile.GetFullPath());
		unloaded = false;
		return true;
	}

	std::vector<uint32_t> sprite_indexes;
	for(uint32_t i = 0; i < total_pics; ++i) {
		uint32_t index;
		safe_get(U32, index);
		sprite_indexes.push_back(index);
	}

	// Now read individual sprites
	int id = 1;
	for(std::vector<uint32_t>::iterator sprite_iter = sprite_indexes.begin();
			sprite_iter != sprite_indexes.end();
			++sprite_iter, ++id)
	{
		uint32_t index = *sprite_iter + 3;
		fh.seek(index);
		uint16_t size;
		safe_get(U16, size);

		ImageMap::iterator it = image_space.find(id);
		if(it != image_space.end()) {
			GameSprite::NormalImage* spr = dynamic_cast<GameSprite::NormalImage*>(it->second);
			if(spr && size > 0) {
				if(spr->size > 0) {
					wxString ss;
					ss << wxT("items.spr: Duplicate GameSprite id ") << id;
					warnings.push_back(ss);
					fh.seekRelative(size);
				} else {
					spr->id = id;
					spr->size = size;
					spr->dump = newd uint8_t[size];
					if(!fh.getRAW(spr->dump, size)) {
						error = wxstr(fh.getErrorMessage()); \
						return false;
					}
				}
			}
		} else {
			fh.seekRelative(size);
		}
	}
#undef safe_get
	unloaded = false;
	return true;
}

bool GraphicManager::loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id)
{
	if(settings.getInteger(Config::USE_MEMCACHED_SPRITES))
		return false;

	if(sprite_id == 0)
	{
		// Empty GameSprite
		size = 0;
		target = nullptr;
		return true;
	}

	FileReadHandle fh(spritefile);
	if(fh.isOk() == false)
		return false;
	unloaded = false;

	if (!fh.seek((is_extended ? 4 : 2) + sprite_id * sizeof(uint32_t)))
		return false;

	uint32_t to_seek = 0;
	if(fh.getU32(to_seek))
	{
		fh.seek(to_seek+3);
		uint16_t sprite_size;
		if(fh.getU16(sprite_size))
		{
			target = newd uint8_t[sprite_size];
			if(fh.getRAW(target, sprite_size))
			{
				size = sprite_size;
				return true;
			}
			delete[] target;
			target = nullptr;
		}
	}
	return false;
}

void GraphicManager::addSpriteToCleanup(GameSprite* spr)
{
	cleanup_list.push_back(spr);
	// Clean if needed
	if (cleanup_list.size() > std::max<uint32_t>(100, settings.getInteger(Config::SOFTWARE_CLEAN_THRESHOLD))) {
		for (int i = 0; i < settings.getInteger(Config::SOFTWARE_CLEAN_SIZE) && static_cast<uint32_t>(i) < cleanup_list.size(); ++i) {
			cleanup_list.front()->unloadDC();
			cleanup_list.pop_front();
		}
	}
}

void GraphicManager::garbageCollection()
{
	if(settings.getInteger(Config::TEXTURE_MANAGEMENT))
	{
		int t = time(nullptr);
		if(loaded_textures > settings.getInteger(Config::TEXTURE_CLEAN_THRESHOLD) &&
				t - lastclean > settings.getInteger(Config::TEXTURE_CLEAN_PULSE))
		{
			ImageMap::iterator iit = image_space.begin();
			while(iit != image_space.end())
			{
				iit->second->clean(t);
				++iit;
			}
			SpriteMap::iterator sit = sprite_space.begin();
			while(sit != sprite_space.end())
			{
				GameSprite* gs = dynamic_cast<GameSprite*>(sit->second);
				if(gs) gs->clean(t);
				++sit;
			}
			lastclean = t;
		}
	}
}

EditorSprite::EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32)
{
	bm[SPRITE_SIZE_16x16] = b16x16;
	bm[SPRITE_SIZE_32x32] = b32x32;
}

EditorSprite::~EditorSprite()
{
	unloadDC();
}

void EditorSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height)
{
	wxBitmap* sp = bm[sz];
	if(sp)
		dc->DrawBitmap(*sp, start_x, start_y, true);
}

void EditorSprite::unloadDC()
{
	delete bm[SPRITE_SIZE_16x16];
	delete bm[SPRITE_SIZE_32x32];
	bm[SPRITE_SIZE_16x16] = nullptr;
	bm[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::GameSprite() :
	id(0),
	height(0),
	width(0),
	layers(0),
	pattern_x(0),
	pattern_y(0),
	pattern_z(0),
	frames(0),
	numsprites(0),
	draw_height(0),
	drawoffset_x(0),
	drawoffset_y(0),
	minimap_color(0)
{
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::~GameSprite() {
	unloadDC();
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
			iter != instanced_templates.end();
			++iter)
	{
		delete *iter;
	}
}

void GameSprite::clean(int time) {
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
			iter != instanced_templates.end();
			++iter)
	{
		(*iter)->clean(time);
	}
}

void GameSprite::unloadDC() {
	delete dc[SPRITE_SIZE_16x16];
	delete dc[SPRITE_SIZE_32x32];
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

int GameSprite::getDrawHeight() const {
	return draw_height;
}

std::pair<int, int> GameSprite::getDrawOffset() const {
	return std::make_pair(drawoffset_x, drawoffset_y);
}

uint8_t GameSprite::getMiniMapColor() const {
	return minimap_color;
}

GLuint GameSprite::getHardwareID(int _x, int _y, int _layer, int _count, int _pattern_x, int _pattern_y, int _pattern_z, int _frame) {
	uint32_t v;
	if(_count >= 0 && height <= 1 && width <= 1) {
		v = _count;
	} else {
		v = ((((((_frame)*pattern_y+_pattern_y)*pattern_x+_pattern_x)*layers+_layer)*height+_y)*width+_x);
	}
	if(v >= numsprites) {
		if(numsprites == 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}
	return spriteList[v]->getHardwareID();
}

GameSprite::TemplateImage* GameSprite::getTemplateImage(int sprite_index, const Outfit& outfit) {
	if(instanced_templates.empty()) {
		TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
		instanced_templates.push_back(img);
		return img;
	}
	// While this is linear lookup, it is very rare for the list to contain more than 4-8 entries, so it's faster than a hashmap anyways.
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
			iter != instanced_templates.end();
			++iter)
	{
		TemplateImage* img = *iter;
		if(img->sprite_index == sprite_index) {
			uint32_t lookHash = img->lookHead << 24 | img->lookBody << 16 | img->lookLegs << 8 | img->lookFeet;
			if(outfit.getColorHash() == lookHash) {
				return img;
			}
		}
	}
	TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
	instanced_templates.push_back(img);
	return img;
}

GLuint GameSprite::getHardwareID(int _x, int _y, int _dir, const Outfit& _outfit, int _frame) {
	uint32_t v;
	v = ((((_dir) * layers) * height+_y) * width+_x);
	if(v >= numsprites) {
		if(numsprites == 1) {
			v = 0;
		} else {
			v %= numsprites;
		}
	}
	if(layers > 1) { // Template
		TemplateImage* img = getTemplateImage(v, _outfit);
		return img->getHardwareID();
	}
	return spriteList[v]->getHardwareID();
}

wxMemoryDC* GameSprite::getDC(SpriteSize sz) {
	if(!dc[sz]) {
		const int bgshade = settings.getInteger(Config::ICON_BACKGROUND);

		uint8_t* rgb = nullptr;
		uint32_t ht, wt;

		if(width == 2 && height == 2) {
			wt = 2, ht = 2;
			rgb = newd uint8_t[2*2*32*32*3];
			memset(rgb, bgshade, 2*2*32*32*3);

			for(int cf = 0; cf != layers; ++cf) {
				uint8_t* rgb32x32[2][2] = {
					{spriteList[(cf*2+1)*2+1]->getRGBData(),spriteList[(cf*2+1)*2]->getRGBData()},
					{spriteList[cf*4+1]->getRGBData(),spriteList[cf*4]->getRGBData()}
				};
				for(int cy = 0; cy < 64; ++cy) {
					for(int cx = 0; cx < 64; ++cx) {
						int cw = cx/32;
						int ch = cy/32;
						if(!rgb32x32[ch][cw]) continue;

						int rx = cx%32;
						int ry = cy%32;

						uint8_t r = rgb32x32[ch][cw][32*3*ry + 3*rx + 0];
						uint8_t g = rgb32x32[ch][cw][32*3*ry + 3*rx + 1];
						uint8_t b = rgb32x32[ch][cw][32*3*ry + 3*rx + 2];

						if(r == 0xFF && g == 0x00 && b == 0xFF) {
							// Transparent.. let it pass
						} else {
							rgb[64*3*cy + 3*cx + 0] = r;
							rgb[64*3*cy + 3*cx + 1] = g;
							rgb[64*3*cy + 3*cx + 2] = b;
						}
					}
				}
				delete[] rgb32x32[0][0];
				delete[] rgb32x32[0][1];
				delete[] rgb32x32[1][0];
				delete[] rgb32x32[1][1];
			}
			// (cf*2+ch)*2+cw
		} else if(width == 2 && height == 1) {
			wt = 2, ht = 2;
			rgb = newd uint8_t[2*2*32*32*3];
			memset(rgb, bgshade, 2*2*32*32*3);

			for(int cf = 0; cf != layers; ++cf) {
				uint8_t* rgb32x32[2] = {
					spriteList[cf*2+1]->getRGBData(),
					spriteList[cf*2+0]->getRGBData()
				};
				for(int cy = 16; cy < 48; ++cy) {
					for(int cx = 0; cx < 64; ++cx) {
						int cw = cx/32;
						int rx = cx%32;
						int ry = cy-16;
						if(!rgb32x32[cw]) continue;

						uint8_t r = rgb32x32[cw][32*3*ry + 3*rx + 0];
						uint8_t g = rgb32x32[cw][32*3*ry + 3*rx + 1];
						uint8_t b = rgb32x32[cw][32*3*ry + 3*rx + 2];

						if(r == 0xFF && g == 0x00 && b == 0xFF) {
							// Transparent.. let it pass
						} else {
							rgb[64*3*cy + 3*cx + 0] = r;
							rgb[64*3*cy + 3*cx + 1] = g;
							rgb[64*3*cy + 3*cx + 2] = b;
						}
					}
				}
				delete[] rgb32x32[0];
				delete[] rgb32x32[1];
			}
		} else if(width == 1 && height == 2) {
			wt = 2, ht = 2;
			rgb = newd uint8_t[2*2*32*32*3];
			memset(rgb, bgshade, 2*2*32*32*3);

			for(int cf = 0; cf != layers; ++cf) {
				uint8_t* rgb32x32[2] = {
					spriteList[cf*2+1]->getRGBData(),
					spriteList[cf*2+0]->getRGBData()
				};
				for(int cy = 0; cy < 64; ++cy) {
					for(int cx = 16; cx < 48; ++cx) {
						int ch = cy/32;
						int rx = cx-16;
						int ry = cy%32;
						if(!rgb32x32[ch]) continue;

						uint8_t r = rgb32x32[ch][32*3*ry + 3*rx + 0];
						uint8_t g = rgb32x32[ch][32*3*ry + 3*rx + 1];
						uint8_t b = rgb32x32[ch][32*3*ry + 3*rx + 2];

						if(r == 0xFF && g == 0x00 && b == 0xFF) {
							// Transparent.. let it pass
						} else {
							rgb[64*3*cy + 3*cx + 0] = r;
							rgb[64*3*cy + 3*cx + 1] = g;
							rgb[64*3*cy + 3*cx + 2] = b;
						}
					}
				}
				delete[] rgb32x32[0];
				delete[] rgb32x32[1];
			}
		} else if(width == 1 && height == 1) {
			wt = 1, ht = 1;
			rgb = newd uint8_t[32*32*3];
			memset(rgb, bgshade, 32*32*3);

			for(int cf = 0; cf != layers; ++cf) {
				uint8_t* rgb32x32 = spriteList[cf]->getRGBData();
				if(!rgb32x32) continue;

				for(int cy = 0; cy < 32; ++cy) {
					for(int cx = 0; cx < 32; ++cx) {
						uint8_t r = rgb32x32[32*3*cy + 3*cx + 0];
						uint8_t g = rgb32x32[32*3*cy + 3*cx + 1];
						uint8_t b = rgb32x32[32*3*cy + 3*cx + 2];

						if(r == 0xFF && g == 0x00 && b == 0xFF) {
							// Transparent.. let it pass
						} else {
							rgb[32*3*cy + 3*cx + 0] = r;
							rgb[32*3*cy + 3*cx + 1] = g;
							rgb[32*3*cy + 3*cx + 2] = b;
						}
					}
				}
				delete[] rgb32x32;
			}
		} else {
			return nullptr;
		}

		// Now comes the resizing / antialiasing
		if(sz == SPRITE_SIZE_16x16) {
			uint8_t* rgb16x16 = reinterpret_cast<uint8_t*>(malloc(16*16*3));

			uint32_t pixels_per_line = 32*wt;
			uint32_t pixels_per_pixel = 2*wt;
			uint32_t bytes_per_line = 3*pixels_per_line;
			uint32_t bytes_per_pixel = 3*pixels_per_pixel;


			for(uint32_t y = 0; y < 16; ++y) {
				for(uint32_t x = 0; x < 16; ++x) {
					uint32_t r = 0, g = 0, b = 0, c = 0;
					for(uint32_t u = 0; u < 2; ++u) {
						for(uint32_t v = 0; v < 2; ++v) {
							r += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 0];
							g += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 1];
							b += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 2];
							//r += rgb[96 * (2*y + u) + (6*x + v*3) + 0];
							//g += rgb[96 * (2*y + u) + (6*x + v*3) + 1];
							//b += rgb[96 * (2*y + u) + (6*x + v*3) + 2];
							++c;
						}
					}
					ASSERT(c);

					rgb16x16[48*y+x*3+0] = r/c;
					rgb16x16[48*y+x*3+1] = g/c;
					rgb16x16[48*y+x*3+2] = b/c;
				}
			}

			wxImage img(16,16, rgb16x16);
			wxBitmap bmp(img);
			dc[sz] = newd wxMemoryDC(bmp);
			delete[] rgb;
			gui.gfx.addSpriteToCleanup(this);
		} else if(sz == SPRITE_SIZE_32x32) {
			uint8_t* rgb32x32 = reinterpret_cast<uint8_t*>(malloc(32*32*3));

			uint32_t pixels_per_line = 32*wt;
			uint32_t pixels_per_pixel = wt;
			uint32_t bytes_per_line = 3*pixels_per_line;
			uint32_t bytes_per_pixel = 3*pixels_per_pixel;


			for(uint32_t y = 0; y < 32; ++y) {
				for(uint32_t x = 0; x < 32; ++x) {
					uint32_t r = 0, g = 0, b = 0, c = 0;
					for(uint32_t u = 0; u < ht; ++u) {
						for(uint32_t v = 0; v < wt; ++v) {
							r += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 0];
							g += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 1];
							b += rgb[bytes_per_line * (pixels_per_pixel*y + u) + (bytes_per_pixel*x + 3*v) + 2];
							++c;
						}
					}
					ASSERT(c);

					rgb32x32[96*y+x*3+0] = r/c;
					rgb32x32[96*y+x*3+1] = g/c;
					rgb32x32[96*y+x*3+2] = b/c;
				}
			}

			wxImage img(32,32, rgb32x32);
			wxBitmap bmp(img);
			dc[sz] = newd wxMemoryDC(bmp);
			delete[] rgb;
			gui.gfx.addSpriteToCleanup(this);
		} else {
			ASSERT(sz == 0);
		}
	}
	return dc[sz];
}

void GameSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height) {
	if(width == -1)  width = sz == SPRITE_SIZE_32x32? 32 : 16;
	if(height == -1) height= sz == SPRITE_SIZE_32x32? 32 : 16;
	wxDC* sdc = getDC(sz);
	if(sdc)
	{
		dc->Blit(start_x, start_y, width, height, sdc, 0, 0, wxCOPY, true);
	}
	else
	{
		const wxBrush& b = dc->GetBrush();
		dc->SetBrush(*wxRED_BRUSH);
		dc->DrawRectangle(start_x, start_y, width, height);
		dc->SetBrush(b);
	}
}


GameSprite::Image::Image() :
	isGLLoaded(false),
	lastaccess(0)
{
}

GameSprite::Image::~Image()
{
	unloadGLTexture(0);
}

void GameSprite::Image::createGLTexture(GLuint whatid)
{
	ASSERT(!isGLLoaded);

	uint8_t* rgba = getRGBAData();
	if (!rgba) {
		return;
	}

	isGLLoaded = true;
	gui.gfx.loaded_textures += 1;

	glBindTexture(GL_TEXTURE_2D, whatid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); // GL_CLAMP_TO_EDGE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	delete[] rgba;
	#undef SPRITE_SIZE
}

void GameSprite::Image::unloadGLTexture(GLuint whatid) {
	isGLLoaded = false;
	gui.gfx.loaded_textures -= 1;
	glDeleteTextures(1, &whatid);
}

void GameSprite::Image::visit() {
	lastaccess = time(nullptr);
}

void GameSprite::Image::clean(int time) {
	if(isGLLoaded && time - lastaccess > settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		unloadGLTexture(0);
	}
}

GameSprite::NormalImage::NormalImage() :
	id(0),
	size(0),
	dump(nullptr)
{
}

GameSprite::NormalImage::~NormalImage() {
	delete[] dump;
}

void GameSprite::NormalImage::clean(int time) {
	Image::clean(time);
	if(time - lastaccess > 5 && !settings.getInteger(Config::USE_MEMCACHED_SPRITES)) { // We keep dumps around for 5 seconds.
		delete[] dump;
		dump = nullptr;
	}
}

uint8_t* GameSprite::NormalImage::getRGBData() {
	if(dump == nullptr) {
		if(settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		} else {
			if(!gui.gfx.loadSpriteDump(dump, size, id)) {
				return nullptr;
			}
		}
	}
	uint8_t* rgb32x32 = newd uint8_t[32*32*3];
	/* SPR dump format
	 *  The spr format contains chunks, a chunk can either be transparent or contain pixel data.
	 *  First 2 bytes (unsigned short) are read, which tells us how long the chunk is. One
	 * chunk can stretch several rows in the outputted image, for example, if the chunk is 400
	 * pixels long. We will have to wrap it over 14 rows in the image.
	 *  If the chunk is transparent. Set that many pixels to be transparent.
	 *  If the chunk is pixel data, read from the cursor that many pixels. One pixel is 3 bytes in
	 * in RGB aligned data (eg. char R, char B, char G) so if the unsigned short says 20, we
	 * read 20*3 = 60 bytes.
	 *  Once we read one chunk, we switch to the other type of chunk (if we've just read a transparent
	 * chunk, we read a pixel chunk and vice versa). And then start over again.
	 *  All sprites start with a transparent chunk.
	 */

	int bytes = 0;
	int x = 0;
	int y = 0;
	uint16_t chunk_size;
	uint8_t channels = (settings.getInteger(Config::SPR_TRANSPARENCY) == 1) ? 4 : 3;

	while(bytes < size && y < 32) {
		chunk_size = dump[bytes] | dump[bytes+1] << 8;
		bytes += 2;

		for(int i = 0; i < chunk_size; ++i) {
			// Transparent pixel
			rgb32x32[96*y+x*3+0] = 0xFF;
			rgb32x32[96*y+x*3+1] = 0x00;
			rgb32x32[96*y+x*3+2] = 0xFF;
			x++;
			if(x >= 32) {
				x = 0;
				++y;
				if(y >= 32)
					break;
			}
		}

		if(bytes >= size || y >= 32)
			break; // We're done
		// Now comes a pixel chunk, read it!
		chunk_size = dump[bytes] | dump[bytes+1] << 8;
		bytes += 2;
		//printf("Reading pixel chunk size %d\n", int(chunk_size));
		for(int i = 0; i < chunk_size; ++i) {
			uint8_t red  = dump[bytes+0];
			uint8_t green= dump[bytes+1];
			uint8_t blue = dump[bytes+2];
			rgb32x32[96*y+x*3+0] = red;
			rgb32x32[96*y+x*3+1] = green;
			rgb32x32[96*y+x*3+2] = blue;

			bytes += channels;

			x++;
			if(x >= 32) {
				x = 0;
				++y;
				if(y >= 32)
					break;
			}
		}
	}

	// Fill up any trailing pixels
	while(y<32 && x<32) {
		rgb32x32[96*y+x*3+0] = 0xFF; // Transparent
		rgb32x32[96*y+x*3+1] = 0x00; // Transparent
		rgb32x32[96*y+x*3+2] = 0xFF; // Transparent
		x++;
		if(x >= 32) {
			x = 0;
			++y;
		}
	}

	return rgb32x32;
}

uint8_t* GameSprite::NormalImage::getRGBAData() {
	if(dump == nullptr) {
		if(settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
			return nullptr;
		} else {
			if(!gui.gfx.loadSpriteDump(dump, size, id)) {
				return nullptr;
			}
		}
	}
	uint8_t* rgba32x32 = newd uint8_t[32*32*4];
	memset(rgba32x32, 0, 32*32*4);
	/* SPR dump format
	 *  The spr format contains chunks, a chunk can either be transparent or contain pixel data.
	 *  First 2 bytes (unsigned short) are read, which tells us how long the chunk is. One
	 * chunk can stretch several rows in the outputted image, for example, if the chunk is 400
	 * pixels long. We will have to wrap it over 14 rows in the image.
	 *  If the chunk is transparent. Set that many pixels to be transparent.
	 *  If the chunk is pixel data, read from the cursor that many pixels. One pixel is 3 bytes in
	 * in RGB aligned data (eg. char R, char B, char G) so if the unsigned short says 20, we
	 * read 20*3 = 60 bytes.
	 *  Once we read one chunk, we switch to the other type of chunk (if we've just read a transparent
	 * chunk, we read a pixel chunk and vice versa). And then start over again.
	 *  All sprites start with a transparent chunk.
	 */

	int bytes = 0;
	int x = 0;
	int y = 0;
	uint16_t chunk_size;
	bool transparency = (settings.getInteger(Config::SPR_TRANSPARENCY) == 1);
	uint8_t channels = transparency ? 4 : 3;

	while(bytes < size && y < 32) {
		chunk_size = dump[bytes] | dump[bytes+1] << 8;
		//printf("pos:%d\n", bytes);
		//printf("Reading transparent chunk size %d\n", int(chunk_size));
		bytes += 2;

		for(int i = 0; i < chunk_size; ++i) {
			//printf("128*%d+%d*4+3\t= %d\n", y, x, 128*y+x*4+3);
			rgba32x32[128*y+x*4+3] = 0x00; // Transparent pixel
			x++;
			if(x >= 32) {
				x = 0;
				y++;
				if(y >= 32)
					break;
			}
		}

		if(bytes >= size || y >= 32)
			break; // We're done
		// Now comes a pixel chunk, read it!
		chunk_size = dump[bytes] | dump[bytes+1] << 8;
		bytes += 2;
		//printf("Reading pixel chunk size %d\n", int(chunk_size));
		for(int i = 0; i < chunk_size; ++i) {
			uint8_t red   = dump[bytes+0];
			uint8_t green = dump[bytes+1];
			uint8_t blue  = dump[bytes+2];
			uint8_t alpha = transparency ? dump[bytes + 3] : 0xFF;
			rgba32x32[128*y+x*4+0] = red;
			rgba32x32[128*y+x*4+1] = green;
			rgba32x32[128*y+x*4+2] = blue;
			rgba32x32[128*y+x*4+3] = alpha; //Opaque pixel

			bytes += channels;

			x++;
			if(x >= 32) {
				x = 0;
				y++;
				if(y >= 32)
					break;
			}
		}
	}

	// Fill up any trailing pixels
	while(y<32 && x<32) {
		rgba32x32[128*y+x*4+3] = 0x00; // Transparent pixel
		x++;
		if(x >= 32) {
			x = 0;
			y++;
		}
	}

	return rgba32x32;
}

GLuint GameSprite::NormalImage::getHardwareID() {
	if(isGLLoaded == false) {
		createGLTexture(id);
	}
	visit();
	return id;
}

void GameSprite::NormalImage::createGLTexture(GLuint ignored) {
	Image::createGLTexture(id);
}

void GameSprite::NormalImage::unloadGLTexture(GLuint ignored) {
	Image::unloadGLTexture(id);
}

GameSprite::TemplateImage::TemplateImage(GameSprite* parent, int v, const Outfit& outfit) :
	gl_tid(0),
	parent(parent),
	sprite_index(v),
	lookHead(outfit.lookHead),
	lookBody(outfit.lookBody),
	lookLegs(outfit.lookLegs),
	lookFeet(outfit.lookFeet)
{
}

GameSprite::TemplateImage::~TemplateImage() {
}

void GameSprite::TemplateImage::colorizePixel(uint8_t color, uint8_t& red, uint8_t& green, uint8_t& blue) {
	// Thanks! Khaos, or was it mips? Hmmm... =)
	uint8_t ro = (TemplateOutfitLookupTable[color] & 0xFF0000) >> 16; // rgb outfit
	uint8_t go = (TemplateOutfitLookupTable[color] & 0xFF00) >> 8;
	uint8_t bo = (TemplateOutfitLookupTable[color] & 0xFF);
	red = (uint8_t)(red * (ro / 255.f));
	green = (uint8_t)(green * (go / 255.f));
	blue = (uint8_t)(blue * (bo / 255.f));
}

uint8_t* GameSprite::TemplateImage::getRGBData() {
	uint8_t* rgbdata = parent->spriteList[sprite_index]->getRGBData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if(!rgbdata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if(!template_rgbdata) {
		delete[] rgbdata;
		return nullptr;
	}

	if(lookHead > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookHead = 0;
	}
	if(lookBody > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookBody = 0;
	}
	if(lookLegs > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookLegs = 0;
	}
	if(lookFeet > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookFeet = 0;
	}

	for(int y = 0; y < 32; ++y) {
		for(int x = 0; x < 32; ++x) {
			uint8_t& red    = rgbdata[y*32*3 + x*3 + 0];
			uint8_t& green  = rgbdata[y*32*3 + x*3 + 1];
			uint8_t& blue   = rgbdata[y*32*3 + x*3 + 2];

			uint8_t& tred   = template_rgbdata[y*32*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*32*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*32*3 + x*3 + 2];

			if(tred && tgreen && !tblue) { // yellow => head
				colorizePixel(lookHead, red, green, blue);
			} else if(tred && !tgreen && !tblue) { // red => body
				colorizePixel(lookBody, red, green, blue);
			} else if(!tred && tgreen && !tblue) { // green => legs
				colorizePixel(lookLegs, red, green, blue);
			} else if(!tred && !tgreen && tblue) { // blue => feet
				colorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	delete[] template_rgbdata;
	return rgbdata;
}

uint8_t* GameSprite::TemplateImage::getRGBAData() {
	uint8_t* rgbadata = parent->spriteList[sprite_index]->getRGBAData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + parent->height * parent->width]->getRGBData();

	if(!rgbadata) {
		delete[] template_rgbdata;
		return nullptr;
	}
	if(!template_rgbdata) {
		delete[] rgbadata;
		return nullptr;
	}

	if(lookHead > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookHead = 0;
	}
	if(lookBody > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookBody = 0;
	}
	if(lookLegs > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookLegs = 0;
	}
	if(lookFeet > (sizeof(TemplateOutfitLookupTable) / sizeof(TemplateOutfitLookupTable[0]))) {
		lookFeet = 0;
	}

	for(int y = 0; y < 32; ++y) {
		for(int x = 0; x < 32; ++x) {
			uint8_t& red    = rgbadata[y*32*4 + x*4 + 0];
			uint8_t& green  = rgbadata[y*32*4 + x*4 + 1];
			uint8_t& blue   = rgbadata[y*32*4 + x*4 + 2];

			uint8_t& tred   = template_rgbdata[y*32*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*32*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*32*3 + x*3 + 2];

			if(tred && tgreen && !tblue) { // yellow => head
				colorizePixel(lookHead, red, green, blue);
			} else if(tred && !tgreen && !tblue) { // red => body
				colorizePixel(lookBody, red, green, blue);
			} else if(!tred && tgreen && !tblue) { // green => legs
				colorizePixel(lookLegs, red, green, blue);
			} else if(!tred && !tgreen && tblue) { // blue => feet
				colorizePixel(lookFeet, red, green, blue);
			}
		}
	}
	delete[] template_rgbdata;
	return rgbadata;
}

GLuint GameSprite::TemplateImage::getHardwareID() {
	if(isGLLoaded == false) {
		if(gl_tid == 0) {
			gl_tid = gui.gfx.getFreeTextureID();
		}
		createGLTexture(gl_tid);
		if(!isGLLoaded) {
			return 0;
		}
	}
	visit();
	return gl_tid;
}

void GameSprite::TemplateImage::createGLTexture(GLuint unused) {
	Image::createGLTexture(gl_tid);
}

void GameSprite::TemplateImage::unloadGLTexture(GLuint unused) {
	Image::unloadGLTexture(gl_tid);
}
