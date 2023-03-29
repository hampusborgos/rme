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

#include "sprites.h"
#include "graphics.h"
#include "filehandle.h"
#include "settings.h"
#include "gui.h"
#include "otml.h"
#include "sprite_appearances.h"

#include "pngfiles.h"

GraphicManager g_graphics;
GameSprite g_gameSprite;

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
	unloaded(true),
	otfi_found(false),
	is_extended(false),
	has_transparency(false),
	has_frame_durations(false),
	has_frame_groups(false),
	loaded_textures(0),
	lastclean(0)
{
	animation_timer = newd wxStopWatch();
	animation_timer->Start();
}

GraphicManager::~GraphicManager()
{
	for(SpriteMap::iterator iter = sprite_space.begin(); iter != sprite_space.end(); ++iter) {
		delete iter->second;
	}

	for(ImageMap::iterator iter = image_space.begin(); iter != image_space.end(); ++iter) {
		delete iter->second;
	}

	delete animation_timer;
}

bool GraphicManager::hasTransparency() const
{
	return has_transparency;
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
	for(SpriteMap::iterator iter = sprite_space.begin(); iter != sprite_space.end(); ++iter) {
		if(iter->first >= 0) { // Don't clean internal sprites
			delete iter->second;
		} else {
			new_sprite_space.insert(std::make_pair(iter->first, iter->second));
		}
	}

	for(ImageMap::iterator iter = image_space.begin(); iter != image_space.end(); ++iter) {
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
	for(SpriteMap::iterator iter = sprite_space.begin(); iter != sprite_space.end(); ++iter) {
		if(iter->first >= 0) { // Don't clean internal sprites
			iter->second->unloadDC();
		}
	}
}

Sprite* GraphicManager::getSprite(int id)
{
	SpriteMap::iterator it = sprite_space.find(id);
	if(it != sprite_space.end()) {
		return it->second;
	}
	return nullptr;
}

GameSprite* GraphicManager::getCreatureSprite(int id)
{
	if(id < 0) {
		return nullptr;
	}

	SpriteMap::iterator it = sprite_space.find(id + getItemSpriteMaxID());
	if(it != sprite_space.end()) {
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
	wxImage img(is, "image/png");
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

bool GraphicManager::loadItemSpriteMetadata(ItemType* t, wxString& error, wxArrayString& warnings)
{
	GameSprite* sType = newd GameSprite();
	sType->id = t->id;
	sprite_space[t->id] = sType;
	item_count = std::max<uint16_t>(item_count, t->id);

	// Number of blendframes (some sprites consist of several merged sprites
	sType->layers = t->layers;
	sType->pattern_x = t->pattern_width;
	sType->pattern_y = t->pattern_height;
	sType->pattern_z = t->pattern_depth;

	// Length of animation
	sType->sprite_phase_size = t->m_animationPhases.size();
	has_frame_durations = t->m_animationPhases.size() > 0;

	if(sType->sprite_phase_size > 0) {
		sType->animator = newd Animator(sType->sprite_phase_size, t->start_frame, t->loop_count, t->async_animation);
		if(has_frame_durations) {
			int frameIndex = 0;
			for (const auto phase : t->m_animationPhases) { 
				FrameDuration* frame_duration = sType->animator->getFrameDuration(frameIndex);
				frame_duration->setValues(phase.first, phase.second);
				frameIndex++;
			}
			sType->animator->reset();
		}
	}

	sType->numsprites =
		(int)sType->layers *
		(int)sType->pattern_x * (int)sType->pattern_y * sType->pattern_z *
		std::max<int>(1, sType->sprite_phase_size);

	// Read the sprite ids
	for(uint32_t i = 0; i < sType->numsprites; ++i) {
		uint32_t sprite_id = t->m_sprites[i];

		if(image_space[sprite_id] == nullptr) {
			GameSprite::NormalImage* img = newd GameSprite::NormalImage();
			img->id = sprite_id;
			image_space[sprite_id] = img;
		}
		sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(image_space[sprite_id]));
	}
	return true;
}

bool GraphicManager::loadOutfitSpriteMetadata(remeres::protobuf::appearances::Appearance outfit, wxString& error, wxArrayString& warnings)
{
	GameSprite* sType = newd GameSprite();
	sType->id = outfit.id();
	sprite_space[outfit.id() + getItemSpriteMaxID()] = sType;
	creature_count = std::max<uint16_t>(creature_count, outfit.id());

	// We dont need to worry about IDLE or MOVING frame group
	const auto &frameGroup = outfit.frame_group().Get(0);
	const auto &spriteInfo = frameGroup.sprite_info();
	const auto &animation = spriteInfo.animation();

	// Number of blendframes (some sprites consist of several merged sprites
	sType->layers = spriteInfo.layers();
	sType->pattern_x = spriteInfo.pattern_width();
	sType->pattern_y = spriteInfo.pattern_height();
	sType->pattern_z = spriteInfo.pattern_depth();

	// Length of animation
	sType->sprite_phase_size = animation.sprite_phase().size();
	has_frame_durations = animation.sprite_phase().size() > 0;

	if(sType->sprite_phase_size > 0) {
		sType->animator = newd Animator(sType->sprite_phase_size, animation.default_start_phase(), animation.loop_count(), !animation.synchronized());
		if(has_frame_durations) {
			int frameIndex = 0;
			for (const auto &phase : animation.sprite_phase()) { 
				FrameDuration* frame_duration = sType->animator->getFrameDuration(frameIndex);
				frame_duration->setValues(phase.duration_min(), phase.duration_max());
				frameIndex++;
			}
			sType->animator->reset();
		}
	}

	sType->numsprites =
		(int)sType->layers *
		(int)sType->pattern_x * (int)sType->pattern_y * sType->pattern_z *
		std::max<int>(1, sType->sprite_phase_size);

	// Read the sprite ids
	for(uint32_t i = 0; i < sType->numsprites; ++i) {
		uint32_t sprite_id = spriteInfo.sprite_id().Get(i);

		if(image_space[sprite_id] == nullptr) {
			GameSprite::NormalImage* img = newd GameSprite::NormalImage();
			img->id = sprite_id;
			image_space[sprite_id] = img;
		}
		sType->spriteList.push_back(static_cast<GameSprite::NormalImage*>(image_space[sprite_id]));
	}
	return true;
}

bool GraphicManager::loadSpriteMetadataFlags(const ItemType* t, GameSprite* sType, wxString& error, wxArrayString& warnings)
{
	return true;
}

bool GraphicManager::loadSpriteData(const FileName& datafile, wxString& error, wxArrayString& warnings)
{
	FileReadHandle fh(nstr(datafile.GetFullPath()));

	if(!fh.isOk()) {
		error = "Failed to open file for reading";
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
	if(is_extended) {
		safe_get(U32, total_pics);
	} else {
		uint16_t u16 = 0;
		safe_get(U16, u16);
		total_pics = u16;
	}

	if(!g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) {
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
	for(std::vector<uint32_t>::iterator sprite_iter = sprite_indexes.begin(); sprite_iter != sprite_indexes.end(); ++sprite_iter, ++id) {
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
					ss << "items.spr: Duplicate GameSprite id " << id;
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
	if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES))
		return false;

	if(sprite_id == 0) {
		// Empty GameSprite
		size = 0;
		target = nullptr;
		return true;
	}

	SpritePtr spr = g_spriteAppearances.getSprite(sprite_id);
	if (!spr)
		return false;

	size = spr->pixels.size();
	target = new uint8_t[size];
	std::memcpy(target, spr->pixels.data(), size);
	return true;
}

void GraphicManager::addSpriteToCleanup(GameSprite* spr)
{
	cleanup_list.push_back(spr);
	// Clean if needed
	if(cleanup_list.size() > std::max<uint32_t>(100, g_settings.getInteger(Config::SOFTWARE_CLEAN_THRESHOLD))) {
		for(int i = 0; i < g_settings.getInteger(Config::SOFTWARE_CLEAN_SIZE) && static_cast<uint32_t>(i) < cleanup_list.size(); ++i) {
			cleanup_list.front()->unloadDC();
			cleanup_list.pop_front();
		}
	}
}

void GraphicManager::garbageCollection()
{
	if(g_settings.getInteger(Config::TEXTURE_MANAGEMENT)) {
		int t = time(nullptr);
		if(loaded_textures > g_settings.getInteger(Config::TEXTURE_CLEAN_THRESHOLD) &&
			t - lastclean > g_settings.getInteger(Config::TEXTURE_CLEAN_PULSE)) {
			ImageMap::iterator iit = image_space.begin();
			while(iit != image_space.end()) {
				iit->second->clean(t);
				++iit;
			}
			SpriteMap::iterator sit = sprite_space.begin();
			while(sit != sprite_space.end()) {
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
	sprite_phase_size(0),
	numsprites(0),
	animator(nullptr),
	draw_height(0),
	drawoffset_x(0),
	drawoffset_y(0),
	minimap_color(0)
{
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

GameSprite::~GameSprite()
{
	unloadDC();
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
		delete *iter;
	}

	delete animator;
}

void GameSprite::clean(int time) {
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin();
			iter != instanced_templates.end();
			++iter)
	{
		(*iter)->clean(time);
	}
}

void GameSprite::unloadDC()
{
	delete dc[SPRITE_SIZE_16x16];
	delete dc[SPRITE_SIZE_32x32];
	dc[SPRITE_SIZE_16x16] = nullptr;
	dc[SPRITE_SIZE_32x32] = nullptr;
}

int GameSprite::getDrawHeight() const
{
	return draw_height;
}

std::pair<int, int> GameSprite::getDrawOffset()
{
	if (!isDrawOffsetLoaded && spriteList.size() > 0) {
		SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(spriteList[0]->id);
		if (!sheet)
			return std::make_pair(0, 0);

		drawoffset_x += sheet->getSpriteSize().width - 32;
		drawoffset_y += sheet->getSpriteSize().height - 32;
		isDrawOffsetLoaded = true;
	}
	return std::make_pair(drawoffset_x, drawoffset_y);
}

uint8_t GameSprite::getMiniMapColor() const
{
	return minimap_color;
}

int GameSprite::getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const
{
	return ((((frame % this->sprite_phase_size) *
		this->pattern_z + pattern_z) *
		this->pattern_y + pattern_y) *
		this->pattern_x + pattern_x) *
		this->layers + layer;
}

GLuint GameSprite::getHardwareID(int _layer, int _count, int _pattern_x, int _pattern_y, int _pattern_z, int _frame)
{
	uint32_t v;
	if(_count >= 0) {
		v = _count;
	} else {
		v = (((_frame)*pattern_y+_pattern_y)*pattern_x+_pattern_x)*layers+_layer;
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

GameSprite::TemplateImage* GameSprite::getTemplateImage(int sprite_index, const Outfit& outfit)
{
	if(instanced_templates.empty()) {
		TemplateImage* img = newd TemplateImage(this, sprite_index, outfit);
		instanced_templates.push_back(img);
		return img;
	}
	// While this is linear lookup, it is very rare for the list to contain more than 4-8 entries, so it's faster than a hashmap anyways.
	for(std::list<TemplateImage*>::iterator iter = instanced_templates.begin(); iter != instanced_templates.end(); ++iter) {
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

GLuint GameSprite::getHardwareID(int _dir, const Outfit& _outfit, int _frame)
{
	uint32_t v;
	v = _dir * layers;
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

wxMemoryDC* GameSprite::getDC(SpriteSize size)
{
	//ASSERT(size == SPRITE_SIZE_16x16 || size == SPRITE_SIZE_32x32);
	if(!dc[size]) {
		//ASSERT(width >= 1 && height >= 1);

		const int bgshade = g_settings.getInteger(Config::ICON_BACKGROUND);

		wxImage image(getWidth(), getHeight());
		image.Clear(bgshade);

		const int i = getIndex(0, 0, 0, 0, 0, 0, 0);
		uint8_t* data = spriteList[i]->getRGBData();
		if(data) {
			wxImage img(getWidth(), getHeight(), data);
			img.SetMaskColour(0xFF, 0x00, 0xFF);
			image.Paste(img, getWidth(), getHeight());
			img.Destroy();
		}

		wxBitmap bmp(image);
		dc[size] = newd wxMemoryDC(bmp);
		g_gui.gfx.addSpriteToCleanup(this);
		image.Destroy();
	}
	
	return dc[size];
}

void GameSprite::DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width, int height)
{
	if(width == -1 || height == -1) {
		if (spriteList.size() == 0) {
			return;
		}
		SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(spriteList[0]->id);
		if (!sheet)
			return;

		width = sheet->getSpriteSize().width;
		height = sheet->getSpriteSize().height;
	}

	wxDC* sdc = getDC(sz);
	if(sdc) {
		dc->Blit(start_x, start_y, width, height, sdc, 0, 0, wxCOPY, true);
	} else {
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
	////
}

GameSprite::Image::~Image()
{
	unloadGLTexture(0);
}

void GameSprite::Image::createGLTexture(GLuint whatid)
{
	ASSERT(!isGLLoaded);

	uint8_t* rgba = getRGBAData();
	if(!rgba) {
		return;
	}

	SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(whatid);
	if (!sheet)
		return;

	isGLLoaded = true;
	g_gui.gfx.loaded_textures += 1;

	glBindTexture(GL_TEXTURE_2D, whatid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); // GL_CLAMP_TO_EDGE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sheet->getSpriteSize().width, sheet->getSpriteSize().height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	delete[] rgba;
	#undef SPRITE_SIZE
}

void GameSprite::Image::unloadGLTexture(GLuint whatid)
{
	isGLLoaded = false;
	g_gui.gfx.loaded_textures -= 1;
	glDeleteTextures(1, &whatid);
}

void GameSprite::Image::visit()
{
	lastaccess = time(nullptr);
}

void GameSprite::Image::clean(int time)
{
	if(isGLLoaded && time - lastaccess > g_settings.getInteger(Config::TEXTURE_LONGEVITY)) {
		unloadGLTexture(0);
	}
}

GameSprite::NormalImage::NormalImage() :
	id(0),
	size(0),
	dump(nullptr)
{
	////
}

GameSprite::NormalImage::~NormalImage()
{
	delete[] dump;
}

void GameSprite::NormalImage::clean(int time)
{
	Image::clean(time);
	if(time - lastaccess > 5 && !g_settings.getInteger(Config::USE_MEMCACHED_SPRITES)) { // We keep dumps around for 5 seconds.
		delete[] dump;
		dump = nullptr;
	}
}

uint8_t* GameSprite::NormalImage::getRGBData()
{
	if(!dump) {
		if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES))
			return nullptr;

		if(!g_gui.gfx.loadSpriteDump(dump, size, id)) {
			spdlog::info("[GameSprite::NormalImage::getRGBData] - Failed when parsing sprite id {}", id);
			return nullptr;
		}
	}

	return dump;
}


uint8_t* GameSprite::NormalImage::getRGBAData()
{
	if(!dump) {
		if(g_settings.getInteger(Config::USE_MEMCACHED_SPRITES))
			return nullptr;

		if(!g_gui.gfx.loadSpriteDump(dump, size, id)) {
			spdlog::info("[GameSprite::NormalImage::getRGBAData] - Failed when parsing sprite id {}", id);
			return nullptr;
		}
	}

	return dump;
}

GLuint GameSprite::NormalImage::getHardwareID()
{
	if(!isGLLoaded) {
		createGLTexture(id);
	}
	visit();
	return id;
}

void GameSprite::NormalImage::createGLTexture(GLuint ignored)
{
	Image::createGLTexture(id);
}

void GameSprite::NormalImage::unloadGLTexture(GLuint ignored)
{
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
	////
}

GameSprite::TemplateImage::~TemplateImage()
{
	////
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

uint8_t* GameSprite::TemplateImage::getRGBData()
{
	uint8_t* rgbdata = parent->spriteList[sprite_index]->getRGBData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + 1]->getRGBData();

	SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(parent->spriteList[sprite_index]->id);
	if (!sheet || !sheet->loaded) {
		return nullptr;
	}

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

	int height = sheet->getSpriteSize().height;
	int width = sheet->getSpriteSize().width;
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			uint8_t& red   = rgbdata[y*width*3 + x*3 + 0];
			uint8_t& green = rgbdata[y*width*3 + x*3 + 1];
			uint8_t& blue  = rgbdata[y*width*3 + x*3 + 2];

			uint8_t& tred   = template_rgbdata[y*width*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*width*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*width*3 + x*3 + 2];

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

uint8_t GameSprite::getWidth()
{
	if (width <= 0) {
		SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(spriteList[0]->id, false);
		if (sheet) {
			width = sheet->getSpriteSize().width;
			height = sheet->getSpriteSize().height;
		}
	}

	return width;
}

uint8_t GameSprite::getHeight()
{
	if (height <= 0) {
		SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(spriteList[0]->id, false);
		if (sheet) {
			width = sheet->getSpriteSize().width;
			height = sheet->getSpriteSize().height;
		}
	}

	return height;
}

uint8_t* GameSprite::TemplateImage::getRGBAData()
{
	uint8_t* rgbadata = parent->spriteList[sprite_index]->getRGBAData();
	uint8_t* template_rgbdata = parent->spriteList[sprite_index + 1]->getRGBData();

	SpriteSheetPtr sheet = g_spriteAppearances.getSheetBySpriteId(parent->spriteList[sprite_index]->id);
	if (!sheet || !sheet->loaded) {
		return nullptr;
	}

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

	int height = sheet->getSpriteSize().height;
	int width = sheet->getSpriteSize().width;
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			uint8_t& red   = rgbadata[y*width*4 + x*4 + 0];
			uint8_t& green = rgbadata[y*width*4 + x*4 + 1];
			uint8_t& blue  = rgbadata[y*width*4 + x*4 + 2];

			uint8_t& tred   = template_rgbdata[y*width*3 + x*3 + 0];
			uint8_t& tgreen = template_rgbdata[y*width*3 + x*3 + 1];
			uint8_t& tblue  = template_rgbdata[y*width*3 + x*3 + 2];

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
	//delete[] template_rgbdata;
	return rgbadata;
}

GLuint GameSprite::TemplateImage::getHardwareID()
{
	if(!isGLLoaded) {
		if(gl_tid == 0) {
			gl_tid = g_gui.gfx.getFreeTextureID();
		}
		createGLTexture(gl_tid);
		if(!isGLLoaded) {
			return 0;
		}
	}
	visit();
	return gl_tid;
}

void GameSprite::TemplateImage::createGLTexture(GLuint unused)
{
	Image::createGLTexture(gl_tid);
}

void GameSprite::TemplateImage::unloadGLTexture(GLuint unused)
{
	Image::unloadGLTexture(gl_tid);
}

// ============================================================================
// Animator

Animator::Animator(int frame_count, int start_frame, int loop_count, bool async) :
	frame_count(frame_count),
	start_frame(start_frame),
	loop_count(loop_count),
	async(async),
	current_frame(0),
	current_loop(0),
	current_duration(0),
	total_duration(0),
	direction(ANIMATION_FORWARD),
	last_time(0),
	is_complete(false)
{
	ASSERT(start_frame >= -1 && start_frame < frame_count);

	for(int i = 0; i < frame_count; i++) {
		durations.push_back(newd FrameDuration(ITEM_FRAME_DURATION, ITEM_FRAME_DURATION));
	}

	reset();
}

Animator::~Animator()
{
	for(int i = 0; i < frame_count; i++) {
		delete durations[i];
	}
	durations.clear();
}

int Animator::getStartFrame() const
{
	if(start_frame > -1)
		return start_frame;
	return uniform_random(0, frame_count - 1);
}

FrameDuration* Animator::getFrameDuration(int frame)
{
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame];
}

int Animator::getFrame()
{
	long time = g_gui.gfx.getElapsedTime();
	if(time != last_time && !is_complete) {
		long elapsed = time - last_time;
		if(elapsed >= current_duration) {
			int frame = 0;
			if(loop_count < 0)
				frame = getPingPongFrame();
			else
				frame = getLoopFrame();

			if(current_frame != frame) {
				int duration = getDuration(frame) - (elapsed - current_duration);
				if(duration < 0 && !async) {
					calculateSynchronous();
				} else {
					current_frame = frame;
					current_duration = std::max<int>(0, duration);
				}
			} else {
				is_complete = true;
			}
		} else {
			current_duration -= elapsed;
		}

		last_time = time;
	}
	return current_frame;
}

void Animator::setFrame(int frame)
{
	ASSERT(frame == -1 || frame == 255 || frame == 254 || (frame >= 0 && frame < frame_count));

	if(current_frame == frame)
		return;

	if(async) {
		if(frame == 255) // Async mode
			current_frame = 0;
		else if(frame == 254) // Random mode
			current_frame = uniform_random(0, frame_count - 1);
		else if(frame >= 0 && frame < frame_count)
			current_frame = frame;
		else
			current_frame = getStartFrame();

		is_complete = false;
		last_time = g_gui.gfx.getElapsedTime();
		current_duration = getDuration(current_frame);
		current_loop = 0;
	} else {
		calculateSynchronous();
	}
}

void Animator::reset()
{
	total_duration = 0;
	for(int i = 0; i < frame_count; i++)
		total_duration += durations[i]->max;

	is_complete = false;
	direction = ANIMATION_FORWARD;
	current_loop = 0;
	async = false;
	setFrame(-1);
}

int Animator::getDuration(int frame) const
{
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame]->getDuration();
}

int Animator::getPingPongFrame()
{
	int count = direction == ANIMATION_FORWARD ? 1 : -1;
	int next_frame = current_frame + count;
	if(next_frame < 0 || next_frame >= frame_count) {
		direction = direction == ANIMATION_FORWARD ? ANIMATION_BACKWARD : ANIMATION_FORWARD;
		count *= -1;
	}
	return current_frame + count;
}

int Animator::getLoopFrame()
{
	int next_phase = current_frame + 1;
	if(next_phase < frame_count)
		return next_phase;

	if(loop_count == 0)
		return 0;

	if(current_loop < (loop_count - 1)) {
		current_loop++;
		return 0;
	}
	return current_frame;
}

void Animator::calculateSynchronous()
{
	long time = g_gui.gfx.getElapsedTime();
	if(time > 0 && total_duration > 0) {
		long elapsed = time % total_duration;
		int total_time = 0;
		for(int i = 0; i < frame_count; i++) {
			int duration = getDuration(i);
			if(elapsed >= total_time && elapsed < total_time + duration) {
				current_frame = i;
				current_duration = duration - (elapsed - total_time);
				break;
			}
			total_time += duration;
		}
		last_time = time;
	}
}
