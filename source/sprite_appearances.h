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

#ifndef SPRITEAPPEARANCES_H
#define SPRITEAPPEARANCES_H

#include "definitions.h"
#include "main.h"
#include "graphics.h"

class GameSprite;

enum class SpriteLayout
{
	ONE_BY_ONE = 0,
	ONE_BY_TWO = 1,
	TWO_BY_ONE = 2,
	TWO_BY_TWO = 3
};

struct SpritesSize {
	public:
		SpritesSize() {}
		SpritesSize(int height, int width) : height(height), width(width) {}

		void resize(int height, int width) {
			this->height = height;
			this->width = width;
		}
		void setHeight(int height) {
			this->height = height;
		}
		void setWidth(int width) {
			this->width = width;
		}
		int area() const {
			return width * height;
		}

		int height = 32;
		int width = 32;
};

struct Sprites {
	public:
		Sprites() {
			pixels.resize(32 * 32 * 4, 0);
		}

		bool save(const std::string& file, bool fixMagenta = false) {
			wxImage image(size.width, size.height);
	
			for (int y = 0; y < size.height; ++y) {
				for (int x = 0; x < size.width; ++x) {
					const int index = (y * size.width + x) * 4;
					const uint8_t r = pixels[index + 2];
					const uint8_t g = pixels[index + 1];
					const uint8_t b = pixels[index];
					image.SetRGB(x, y, r, g, b);
				}
			}

			return image.SaveFile(wxString(file), wxBITMAP_TYPE_PNG);
		}

		std::vector<uint8_t> pixels;
		SpritesSize size;
};

class SpriteSheet {
	public:
		SpriteSheet(int firstId, int lastId, SpriteLayout spriteLayout, const std::string& path) : firstId(firstId), lastId(lastId), spriteLayout(spriteLayout), path(path) {}

		SpritesSize getSpriteSize() {
			SpritesSize size(SPRITE_SIZE, SPRITE_SIZE);

			switch (spriteLayout) {
				case SpriteLayout::ONE_BY_ONE: break;
				case SpriteLayout::ONE_BY_TWO: size.setHeight(64); break;
				case SpriteLayout::TWO_BY_ONE: size.setWidth(64); break;
				case SpriteLayout::TWO_BY_TWO: size.resize(64, 64); break;
				default: break;
			}
			return size;
		}

		bool exportSheetImage(const std::string& file, bool fixMagenta = false) {
			wxImage image(384, 384, data.get(), true);
			return image.SaveFile(wxString(file), wxBITMAP_TYPE_PNG);
		};

		int firstId = 0;
		int lastId = 0;
		SpriteLayout spriteLayout = SpriteLayout::ONE_BY_ONE;
		std::unique_ptr<uint8_t[]> data;
		std::string path;
		bool loaded = false;
};

using SpritePtr = std::shared_ptr<Sprites>;
using SpriteSheetPtr = std::shared_ptr<SpriteSheet>;

//@bindsingleton g_spriteAppearances
class SpriteAppearances
{
	public:
		void init();
		void terminate();

		void unload();

		// sprites
        void exportSpriteImage(int id, const std::string& path);
        SpritePtr getSprite(int spriteId);

        int getSpritesCount() {
            return spritesCount;
        }

		void setSpritesCount(int count) { spritesCount = count; }
		int getSpritesCount() const { return spritesCount; }
		/**
			Returns a wxImage object containing the sprite image data for a given sprite ID.
			@param id The ID of the sprite to retrieve.
			@param toSavePng Determines if the image should be saved as a PNG file.
			@return A wxImage object containing the sprite image data.
			@remarks The image data is retrieved from a sprite sheet using the provided ID.
			If toSavePng is set to true, the black color (0, 0, 0) is set to transparent by default and the image will be saved as a PNG file (with transparent background).
			Use to save image: image.SaveFile(g_gui.GetDataDirectory().ToStdString() + "image.png", wxBITMAP_TYPE_PNG);
		*/
		wxImage getWxImageBySpriteId(int id, bool toSavePng = false);

		const std::string getAppearanceFileName() const {
			return appearanceFile;
		}

		bool loadCatalogContent(const std::string& dir, bool loadData = true);
		bool loadSpriteSheet(const SpriteSheetPtr& sheet);
		void saveSheetToFileBySprite(int id, const std::string& file);
		void saveSheetToFile(const SpriteSheetPtr& sheet, const std::string& file);
		SpriteSheetPtr getSheetBySpriteId(int id, bool load = true);

		void addSpriteSheet(SpriteSheetPtr sheet) {
			sheets.push_back(sheet);
		}

		void saveSpriteToFile(int id, const std::string& file);

	private:

		int spritesCount = 0;
		std::vector<SpriteSheetPtr> sheets;
		std::map<int, SpritePtr> sprites;
		std::string appearanceFile;
};

extern SpriteAppearances g_spriteAppearances;

#endif
