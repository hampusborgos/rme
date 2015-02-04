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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/graphics.h $
// $Id: graphics.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_GRAPHICS_H_
#define RME_GRAPHICS_H_


#include "outfit.h"
#include <deque>

#include "client_version.h"

enum SpriteSize {
	SPRITE_SIZE_16x16,
	//SPRITE_SIZE_24x24,
	SPRITE_SIZE_32x32,
	SPRITE_SIZE_COUNT
};

class MapCanvas;
class GraphicManager;
class FileReadHandle;

class Sprite {
public:
	Sprite() {}
	virtual ~Sprite() {}

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) = 0;
	virtual void unloadDC() = 0;
private:
	Sprite(const Sprite&);
	Sprite& operator=(const Sprite&);
};

class EditorSprite : public Sprite {
public:
	EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32);
	virtual ~EditorSprite();

	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1);
	virtual void unloadDC();
protected:
	wxBitmap* bm[SPRITE_SIZE_COUNT];
};


class GameSprite : public Sprite{
public:
	GameSprite();
	~GameSprite();

	GLuint getHardwareID(int _x, int _y, int _layer, int _subtype, int _pattern_x, int _pattern_y, int _pattern_z, int _frame);
	GLuint getHardwareID(int _x, int _y, int _dir, const Outfit& _outfit, int _frame); // CreatureDatabase
	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1);

	virtual void unloadDC();

	void clean(int time);

	int getDrawHeight() const;
	std::pair<int, int> getDrawOffset() const;
	uint8_t getMiniMapColor() const;

protected:
	class Image;
	class NormalImage;
	class TemplateImage;

	wxMemoryDC* getDC(SpriteSize sz);
	TemplateImage* getTemplateImage(int sprite_index, const Outfit& outfit);

	class Image {
	public:
		Image();
		virtual ~Image();

		bool isGLLoaded;
		int lastaccess;

		void visit();
		virtual void clean(int time);

		virtual GLuint getHardwareID() = 0;
		virtual uint8_t* getRGBData() = 0;
		virtual uint8_t* getRGBAData() = 0;
	protected:
		virtual void createGLTexture(GLuint whatid);
		virtual void unloadGLTexture(GLuint whatid);
	};

	class NormalImage : public Image {
	public:
		NormalImage();
		virtual ~NormalImage();

		// We use the sprite id as GL texture id
		uint32_t id;

		// This contains the pixel data
		uint16_t size;
		uint8_t* dump;

		virtual void clean(int time);

		virtual GLuint getHardwareID();
		virtual uint8_t* getRGBData();
		virtual uint8_t* getRGBAData();
	protected:
		virtual void createGLTexture(GLuint ignored = 0);
		virtual void unloadGLTexture(GLuint ignored = 0);
	};

	class TemplateImage : public Image {
	public:
		TemplateImage(GameSprite* parent, int v, const Outfit& outfit);
		virtual ~TemplateImage();

		virtual GLuint getHardwareID();
		virtual uint8_t* getRGBData();
		virtual uint8_t* getRGBAData();

		GLuint gl_tid;
		GameSprite* parent;
		int sprite_index;
		uint8_t lookHead;
		uint8_t lookBody;
		uint8_t lookLegs;
		uint8_t lookFeet;
	protected:
		void colorizePixel(uint8_t color, uint8_t &r, uint8_t &b, uint8_t &g);

		virtual void createGLTexture(GLuint ignored = 0);
		virtual void unloadGLTexture(GLuint ignored = 0);
	};

	uint32_t id;
	wxMemoryDC* dc[SPRITE_SIZE_COUNT];

public:
	// GameSprite info
	uint8_t height;
	uint8_t width;
	uint8_t layers;
	uint8_t pattern_x;
	uint8_t pattern_y;
	uint8_t pattern_z;
	uint8_t frames;
	uint32_t numsprites;

	uint16_t draw_height;
	uint16_t drawoffset_x;
	uint16_t drawoffset_y;

	uint16_t minimap_color;

	std::vector<NormalImage*> spriteList;
	std::list<TemplateImage*> instanced_templates; // Templates that use this sprite

	friend class GraphicManager;
};

class GraphicManager {
public:
	GraphicManager();
	~GraphicManager();

	void clear();
	void cleanSoftwareSprites();

	Sprite* getSprite(int id);
	GameSprite* getCreatureSprite(int id);
	
	uint16_t getItemSpriteMaxID() const;
	uint16_t getCreatureSpriteMaxID() const;

	// Get an unused texture id (this is acquired by simply increasing a value starting from 0x10000000)
	GLuint getFreeTextureID();

	// This is part of the binary
	bool loadEditorSprites();
	// Metadata should be loaded first
	// This fills the item / creature adress space
	bool loadSpriteMetadata(const FileName& datafile, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer74(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer76(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer78(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer86(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer1010(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteMetadataFlagsVer1021(FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	bool loadSpriteData(const FileName& datafile, wxString& error, wxArrayString& warnings);

	// Cleans old & unused textures according to config settings
	void garbageCollection();
	void addSpriteToCleanup(GameSprite* spr);

	bool isUnloaded() const;

	ClientVersion *client_version;

private:
	bool unloaded;
	// This is used if memcaching is NOT on
	std::string spritefile;
	bool loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id);

	typedef std::map<int, Sprite*> SpriteMap;
	SpriteMap sprite_space;
	typedef std::map<int, GameSprite::Image*> ImageMap;
	ImageMap image_space;
	std::deque<GameSprite*> cleanup_list;

	DatVersion datVersion;
	SprVersion sprVersion;
	uint16_t item_count;
	uint16_t creature_count;

	int loaded_textures;
	int lastclean;

	friend class GameSprite::Image;
	friend class GameSprite::NormalImage;
	friend class GameSprite::TemplateImage;
};


struct RGBQuad {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t reserved;

	RGBQuad(uint8_t r, uint8_t g, uint8_t b) : red(r), green (g), blue(b), reserved(0) {}

	operator uint32_t() {
		return (blue << 0) | (green << 8) | (red << 16);
	}

	operator bool() {
		//std::cout << "RGBQuad operator bool " << int(red) << " || " << int(blue) << " || " << int(green) << std::endl;
		return blue != 0 || red != 0 || green != 0;
	}
};

static RGBQuad minimap_color[256] = {
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //0
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //4
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //8
	RGBQuad(0,102,0),  RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //12
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //16
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //20
	RGBQuad(0,204,0),  RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //24
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,255,0),    RGBQuad(0,0,0),       //28
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //32
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //36
	RGBQuad(51,0,204), RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //40
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //44
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //48
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //52
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //56
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //60
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //64
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //68
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //72
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //76
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //80
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(102,102,102),RGBQuad(0,0,0),       //84
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //88
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //92
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //96
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //100
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //104
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //108
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(163,51,0),   RGBQuad(0,0,0),       //112
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //116
	RGBQuad(0,0,0),    RGBQuad(153,102,51), RGBQuad(0,0,0),      RGBQuad(0,0,0),       //120
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //124
	RGBQuad(0,0,0),    RGBQuad(153,153,153),RGBQuad(0,0,0),      RGBQuad(0,0,0),       //128
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //132
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //136
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //140
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //144
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //148
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //152
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //156
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //160
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //164
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //168
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //172
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(204,255,255), //176
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //180
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(255,51,0),   RGBQuad(0,0,0),       //184
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //188
	RGBQuad(255,102,0),RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //192
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //196
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //200
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(255,204,153), //204
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(255,255,0),  RGBQuad(0,0,0),       //208
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(255,255,255), //212
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //216
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //220
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //224
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //228
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //232
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //236
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //240
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //244
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0),       //248
	RGBQuad(0,0,0),    RGBQuad(0,0,0),      RGBQuad(0,0,0),      RGBQuad(0,0,0)        //252
};

#endif
